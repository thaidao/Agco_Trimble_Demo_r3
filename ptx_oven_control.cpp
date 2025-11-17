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

/* Internal state */
static ptx_oven_status_t pti_status;
static uint32_t pti_ignition_start_ms = 0;
static uint32_t pti_last_log_ms = 0;

/* Timed sensor fault management */
static uint32_t pti_out_of_range_since_ms = 0;   /* 0 means not currently out of range */
static uint32_t pti_valid_since_ms = 0;          /* 0 means not in continuous valid window */

/* Ignition retry management */
static uint8_t pti_ignition_attempt = 0;         /* Current attempt number (0 = not started) */
static uint32_t pti_purge_start_ms = 0;          /* Start time of purge phase */
static float pti_temp_at_ignition_start = 0.0f;  /* Temperature when ignition started (for flame detection) */

static bool ptx_read_door_open(void) {
    return pti_status.door_open;
}

static void ptx_eval_sensor_faults_with_timing(uint32_t now_ms, float vref_mv, float signal_mv) {
}

static float ptx_compute_temperature(float vref_mv, float signal_mv) {
}

static void ptx_apply_outputs(void) {
    /* Optional LED debug (guard with your own defines to avoid build errors)
    // set_output(LED_STATUS, pti_status.sensor_fault ? 1 : 0);
    */
}

// Main state machine
static void ptx_update_heating(uint32_t now_ms) {
    const ptx_oven_config_t* cfg = ptx_oven_get_config();
    
    /* Door and sensor faults override everything - force shutdown regardless of state */
    /* State machine logic */
    switch (pti_status.state) {
        case PTX_HEATING_STATE_IDLE:
            /* Check if heating is needed */
            break;

        case PTX_HEATING_STATE_IGNITING:
            /* Wait for ignition period to complete */
            /* Ignition period ended, check for flame */

#if (PTX_FLAME_DETECT_ENABLED)
                /* Flame detected - successful ignition */
				// @todo
				PTX_LOGF("Flame detected successful ignition");
#else
                /* Flame detection disabled - assume success */
#endif
            /* Else keep igniter on and wait */
            break;

        case PTX_HEATING_STATE_HEATING:
            /* Check if reached upper temperature threshold */
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

static void ptx_oven_run_log(uint32_t now_ms) {
}

/* Public API */
const ptx_oven_status_t* ptx_oven_get_status(void) {
    return &pti_status;
}

void ptx_oven_control_init(void) {
	
	/* General initialization */    
    /* Initialize actuators and sensor filter */
    ptx_actuator_init();
    ptx_sensor_filter_init(5);	//Configuratble
    PTX_LOGF("oven control init");
}

void ptx_oven_control_update(void) {
    uint32_t now = millis();
	
	/* Evaluate faults with timing first. */
	float vref_mv   = 0;
    float signal_mv = 0;
	
    ptx_eval_sensor_faults_with_timing(now, vref_mv, signal_mv);
    pti_status.door_open = ptx_read_door_open();


    /* Read and filter sensor data */
    ptx_sensor_reading_t filtered = ptx_sensor_filter_read_and_update();

    /* Compute temperature (for display/log); control will still be overridden on faults. */
    pti_status.temperature_c = ptx_compute_temperature(vref_mv, signal_mv);

    /* Control decision. */
    ptx_update_heating(now);

    /* Apply outputs and log. */
    ptx_apply_outputs();
    ptx_oven_run_log(now);
    
    /* Update public status */
    pti_status.ignition_attempt = pti_ignition_attempt;
}

void ptx_oven_set_door_state(bool open) {
    pti_status.door_open = open;
}

void ptx_oven_reset_ignition_lockout(void) {
    if (pti_status.state == PTX_HEATING_STATE_LOCKOUT) {
		//@todo
    }
}