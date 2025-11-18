/**
 * @file ptx_oven_control.cpp
 * @brief Control logic per requirements:
 *        - Maintain near 180C using hysteresis (ON at 175C, OFF at 185C).
 *        - Door open overrides everything -> gas OFF, igniter OFF immediately.
 *        - Igniter ON only first 5s after gas turns ON.
 *        - vref must be 4.5–5.5V; signal must be within 10–90% vref; else fault -> shutdown.
 *        - Periodically log vref, signal, computed temperature, and state.
 */
#include "ptx_oven_control.h"
#include "ptx_oven_config.h"
#include "ptx_sensor_filter.h"
#include "ptx_actuator.h"
#include "api.h"
#include "ptx_logging.h"

/* Feature flags */
#ifndef PTX_FLAME_DETECT_ENABLED
#define PTX_FLAME_DETECT_ENABLED 0  /* Disable flame detection by default (assume ignition success) */
#endif

#define FAST_BLINK_MS 500    // quick blink when system fault
#define SLOW_BLINK_MS 3000   // slow blink when system normal

/* Internal state */
static ptx_oven_status_t pti_status;
static uint32_t pti_ignition_start_ms = 0;
static uint32_t pti_last_log_ms = 0;
static uint32_t pti_last_sys_led_status_ms = 0;

/* Timed sensor fault management */
static uint32_t pti_out_of_range_since_ms = 0;   /* 0 means not currently out of range */
static uint32_t pti_valid_since_ms = 0;          /* 0 means not in continuous valid window */

/* Ignition retry management */
static uint8_t pti_ignition_attempt = 0;         /* Current attempt number (0 = not started) */
static uint32_t pti_purge_start_ms = 0;          /* Start time of purge phase */
static float pti_temp_at_ignition_start = 0.0f;  /* Temperature when ignition started (for flame detection) */

/* Local function */
static void dummytest_statemachine();                   /* Dummy test for real hardware */

// Set door status
static bool ptx_read_door_open(void) {
    return pti_status.door_open;
}

// Check sensor out of range
static void ptx_eval_sensor_faults_with_timing(uint32_t now_ms, float vref_mv, float signal_mv) {
	const ptx_oven_config_t* cfg = ptx_oven_get_config();
		
	/* Update instantaneous readings */
    pti_status.vref_volts   = vref_mv / 1000.0f;
    pti_status.signal_volts = signal_mv / 1000.0f;

    /* Instantaneous violations (not latched) */
    bool vref_bad = (pti_status.vref_volts < cfg->vref_min_v) || (pti_status.vref_volts > cfg->vref_max_v);

    float lo = 0.10f * vref_mv;
    float hi = 0.90f * vref_mv;
    bool signal_bad = (signal_mv < lo) || (signal_mv > hi);

    pti_status.vref_fault = vref_bad;        /* expose instantaneous state */
    pti_status.signal_fault = signal_bad;

    bool out_of_range = vref_bad || signal_bad;

    /* Handle an exception */
    if (out_of_range) {
		pti_status.sensor_fault = true;
        PTX_LOGF("[ERROR] Sensor fault error");

    } else {
        /* Readings are valid; clear out-of-range window */
		pti_status.sensor_fault = false; /* clear latched fault */
		pti_valid_since_ms = 0;
		//PTX_LOGF("sensor fault cleared");
    }
	
}

// Calculate a temperature from vref and signal
static float ptx_compute_temperature(float vref_mv, float signal_mv) {

    // @Debug purpose
    PTX_DBG_LOGF("ptx_compute_temperature[begin]: vref=%dmV signal=%dmV", (int)vref_mv, (int)signal_mv);

	/* Linear map -10C at 10% vref to 300C at 90% vref (span 310C over 0.8*vref). */
    float low = 0.10f * vref_mv;
    float high = 0.90f * vref_mv;
    float temperature;

    /* Handle exception */
    if (signal_mv <= low) return -10.0f;
    if (signal_mv >= high) 
    {
        PTX_DBG_LOGF("[ERROR]Over temperature !!!");
        return 300.0f;
    }
#if 0
    temperature = -10.0f + ((signal_mv - low) / (0.80f * vref_mv)) * 310.0f;
#else
    // Normalize voltage
    float x = signal_mv / vref_mv; // fraction of Vref

    // Calculate temperature
    temperature = 387.5 * x - 48.75;

#endif

    // @Debug purpose
    PTX_DBG_LOGF("ptx_compute_temperature[end]: temperature=%i ", (int)temperature);
    return temperature;

}

// Control output: igniter and gas
static void ptx_apply_outputs(uint32_t now_ms) {

    static bool sys_led_status = false;
    static uint32_t sys_led_last_toggle_ms = 0;

    // Control gas and igniter
    ptx_actuator_set_gas(pti_status.gas_on);
    ptx_actuator_set_igniter(pti_status.igniter_on);
	
    // Select an interval
    uint32_t blink_interval = SLOW_BLINK_MS;
    if(pti_status.door_open || pti_status.vref_fault||
            pti_status.signal_fault|| pti_status.sensor_fault)
    {
        blink_interval = FAST_BLINK_MS;
    }

    // Toggle LED if time is reached
    if ((uint32_t)(now_ms - sys_led_last_toggle_ms) >= blink_interval) {
            sys_led_status = !sys_led_status;
        set_output(SYS_LED_STATUS, sys_led_status);
        sys_led_last_toggle_ms = now_ms;
    }
}

// Main state machine
static void ptx_update_heating(uint32_t now_ms) {
    const ptx_oven_config_t* cfg = ptx_oven_get_config();
    
    /* Door and sensor faults override everything - force shutdown regardless of state */
	if (pti_status.door_open || pti_status.sensor_fault) 
	{
		if (pti_status.gas_on || pti_status.igniter_on) 
		{
			PTX_LOGF("[ERROR]shutdown: door open or sensor fault");
		}
		
		pti_status.gas_on = false;
		pti_status.igniter_on = false;
		pti_status.state = PTX_HEATING_STATE_IDLE;
		pti_ignition_attempt = 0; /* Reset attempt counter on fault */
		return;
    }
	
	/* Hysteresis thresholds */
    float temp_on = cfg->temp_target_c - cfg->temp_delta_c;
    float temp_off = cfg->temp_target_c + cfg->temp_delta_c;
	
    /* State machine logic */
    switch (pti_status.state) {
        case PTX_HEATING_STATE_IDLE:

		    /* Check if heating is needed */
            if (pti_status.temperature_c <= temp_on) {
                /* Start ignition sequence */
                pti_ignition_attempt++;
                pti_status.gas_on = true;
                pti_status.igniter_on = true;
                pti_status.state = PTX_HEATING_STATE_IGNITING;
                pti_ignition_start_ms = now_ms;
                pti_temp_at_ignition_start = pti_status.temperature_c;
                
				//int temp_c_i = (int)(pti_status.temperature_c + 0.5f);
                PTX_LOGF("ignite start attempt=%d temp=%d°C", pti_ignition_attempt, (int)pti_status.temperature_c);
            }
            break;

        case PTX_HEATING_STATE_IGNITING:
            /* Wait for ignition period to complete */
            if ((now_ms - pti_ignition_start_ms) >= cfg->ignition_duration_ms) {
                /* Ignition period ended, check for flame */
                float temp_rise = pti_status.temperature_c - pti_temp_at_ignition_start;

#if (PTX_FLAME_DETECT_ENABLED)
                /* Flame detected - successful ignition */
				// @todo
				PTX_LOGF("Flame detected successful ignition");
#else
                /* Flame detection disabled - assume success */
                pti_status.igniter_on = false;
                pti_status.state = PTX_HEATING_STATE_HEATING;
                pti_ignition_attempt = 0;
                PTX_LOGF("ignition assumed success (flame detect disabled)");
#endif
			}
            /* Else keep igniter on and wait */
            break;

        case PTX_HEATING_STATE_HEATING:
            /* Check if reached upper temperature threshold */
            if (pti_status.temperature_c >= temp_off) {
                pti_status.gas_on = false;
                pti_status.igniter_on = false;
                pti_status.state = PTX_HEATING_STATE_IDLE;
                pti_ignition_attempt = 0; /* Successful heating cycle */
                //int temp_c_i = (int)(pti_status.temperature_c + 0.5f);
                PTX_LOGF("heat off temp=%dC", pti_status.temperature_c);
            }
            /* Else keep heating */
            break;

        case PTX_HEATING_STATE_LOCKOUT:
            /* Require manual reset - no automatic recovery */
            /* Stay in lockout until ptx_oven_reset_ignition_lockout() called */
            break;

        default:
            /* Invalid state - reset to IDLE */
            PTX_LOGF("invalid state %d, reset to IDLE", (int)pti_status.state);
            break;
    }
}

// Capture and show system log
static void ptx_oven_run_log(uint32_t now_ms) {
	const ptx_oven_config_t* cfg = ptx_oven_get_config();
    
    if ((now_ms - pti_last_log_ms) < cfg->periodic_log_ms) return;
    pti_last_log_ms = now_ms;

    int vref_mV = (int)(pti_status.vref_volts * 1000.0f + 0.5f);
    int signal_mV = (int)(pti_status.signal_volts * 1000.0f + 0.5f);
    //int temp_c_i = (int)(pti_status.temperature_c + 0.5f);
    
    /* Main status log */
    PTX_LOGF("temp=%d°C door=%s state=%d gas=%d ign=%d attempt=%d lockout=%d",
             (int)pti_status.temperature_c,
             pti_status.door_open ? "OPEN" : "CLOSED",
             (int)pti_status.state,
             pti_status.gas_on ? 1 : 0,
             pti_status.igniter_on ? 1 : 0,
             pti_status.ignition_attempt,
             pti_status.ignition_lockout ? 1 : 0);
    
    /* Sensor and fault log */
    PTX_LOGF("vref=%dmV signal=%dmV vref_fault=%d signal_fault=%d sensor_fault=%d",
             vref_mV,
             signal_mV,
             pti_status.vref_fault ? 1 : 0,
             pti_status.signal_fault ? 1 : 0,
             pti_status.sensor_fault ? 1 : 0);
}

/* Public API */
const ptx_oven_status_t* ptx_oven_get_status(void) {
    return &pti_status;
}

// Initialize oven controller
void ptx_oven_control_init(void) {
	
    /* Initialize actuators and sensor filter */
    pti_status.vref_volts = 0.0f;
    pti_status.signal_volts = 0.0f;
    pti_status.temperature_c = -10.0f;
    pti_status.door_open = false;
    pti_status.gas_on = false;
    pti_status.igniter_on = false;
    pti_status.state = PTX_HEATING_STATE_IDLE;
    pti_status.vref_fault = false;
    pti_status.signal_fault = false;
    pti_status.sensor_fault = false;
    pti_status.ignition_attempt = 0;
    pti_status.ignition_lockout = false;

    pti_ignition_start_ms = 0;
    pti_last_log_ms = 0;
    pti_ignition_attempt = 0;
    pti_temp_at_ignition_start = 0.0f;
    
    /* Initialize actuators and sensor filter */
    ptx_actuator_init();
    ptx_sensor_filter_init(5);

    PTX_LOGF("oven control init");
}

// The heart of an oven controller program
void ptx_oven_control_update(void) {
    uint32_t now = millis();

    /* Read and filter sensor data */
    ptx_sensor_reading_t filtered = ptx_sensor_filter_read_and_update();
    
    float vref_mv   = (float)filtered.vref_mv;
    float signal_mv = (float)filtered.signal_mv;

    PTX_DBG_LOGF("ptx_oven_control_update[begin]: vref=%dmV signal=%dmV", (int)vref_mv, (int)signal_mv);

#if 1
    /* Evaluate faults with timing first. */
    ptx_eval_sensor_faults_with_timing(now, vref_mv, signal_mv);
    pti_status.door_open = ptx_read_door_open();

    /* Compute temperature (for display/log); control will still be overridden on faults. */
    pti_status.temperature_c = ptx_compute_temperature(vref_mv, signal_mv);
#else
    /* @ for debug only */
    dummytest_statemachine();
#endif

    /* Control decision. */
    ptx_update_heating(now);

    /* Apply outputs and log. */
    ptx_apply_outputs(now);
    ptx_oven_run_log(now);
    
    /* Update public status */
    pti_status.ignition_attempt = pti_ignition_attempt;
}

// Set door state
void ptx_oven_set_door_state(bool open) {
    pti_status.door_open = open;
}

// Simple stratgy to test hw without peripheral
void dummytest_statemachine()
{
    static int cnt = 0;
    //NOTE: no real interrupt, then some status could be wrong
    
    switch(cnt)
    {
        case 0:
            // pti_status.vref_volts = 0.0f;
            // pti_status.signal_volts = 0.0f;
            // pti_status.temperature_c = -10.0f;
            pti_status.door_open = true;

            break;
        case 1:
            //door close, expect ignitier on, gas on
            pti_status.door_open = false;
            pti_status.temperature_c = 150.0f;
            break;
        case 7: //after 5 sec for igniter 

            //door close, expect ignitier off, gas on
            pti_status.door_open = false;
            pti_status.temperature_c = 150.0f;

            break;
        case 8:
            pti_status.door_open = false;
            pti_status.temperature_c = 175.0f;
            break;
        case 9:
            pti_status.door_open = false;
            pti_status.temperature_c = 180.0f;
            break;
        case 10:
            //door close, expect ignitier off, gas off
            pti_status.door_open = false;
            pti_status.temperature_c = 186.0f;
            break;
        
        case 11:
            //door close, expect ignitier on, gas on
            pti_status.door_open = false;
            pti_status.temperature_c = 174.0f;
            break;
        case 17:
            //door close, expect ignitier off, gas on
            pti_status.door_open = false;
            pti_status.temperature_c = 181.0f;
            break;
        case 18:
            //door open, expect ignitier off, gas off
            pti_status.door_open = true;
            break;
        case 19:
            //door close, expect ignitier on, gas off
            pti_status.door_open = false;
            pti_status.temperature_c = 170.0f;  //182
            break;
        case 20:
            //door close, overheat, expect ignitier on, gas off
            pti_status.door_open = false;
            pti_status.temperature_c = 301.0f;
            break;

    }
    if (cnt++ > 21)
        cnt = 0;
}