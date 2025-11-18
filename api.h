// NOTE!!!!!!
// Don't assume the below API follows good coding or PTx conventions.
// Follow your own best judgement for your own code.
// Feel free to rework this API if you like (not required).

#pragma once

#include <stdint.h>
#include <stdbool.h> 

#ifdef __cplusplus
extern "C" {
#endif

#define GAS_VALVE_PIN           2       //Gas valve:    out
#define DOOR_SWITCH_PIN         3       //Door switch:  in
#define SYS_LED_STATUS_PIN      6       //System LED:   out
#define IGNITER_PIN             7       //Igniter:      out

typedef enum
{
    // pin A0
    TEMPERATURE_SENSOR,

    // pin A1
    // referred to as vref
    TEMPERATURE_SENSOR_REFERENCE,
} input_t;

typedef enum
{
    // pin D2
    // When on, the valve is open (gas flowing).
    // When off, the valve is closed (no gas flowing).
    GAS_VALVE,

    // pin D6
    // system status
    SYS_LED_STATUS,

    // pin D7
    // when on, it sparks to ignite gas
    IGNITER,
} output_t;

void setup_api();

// returns voltage in millivolts
uint16_t read_voltage(input_t input);

/**
 * @brief This is an IRQ callback prototype. You must implement this function in your code.
 * Please don't remove this interrupt handler if you make changes to the api.
 * Note that this callback is called from an interrupt context.
 * 
 * @param voltage_high true if the door sensor voltage is high, false otherwise
 */
void door_sensor_interrupt_handler(bool voltage_high);

// true for on, false for off
void set_output(output_t output, bool output_state);
// read the internal state of a pin configured as an output
bool read_output(output_t output);

// returns the current number of milliseconds since the Arduino board began running
uint32_t get_millis();

// note that float %f format is not supported
void serial_printf(const char * format, ...);


#ifdef __cplusplus
}
#endif
