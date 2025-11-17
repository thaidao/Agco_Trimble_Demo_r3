/**
 * @file ptx_sensor_filter.cpp
 * @brief Implementation of median sensor filtering
 */
#include "ptx_sensor_filter.h"
#include "api.h"
#include <string.h>

void ptx_sensor_filter_init(uint8_t window_size) {
	//@todo
}

ptx_sensor_reading_t ptx_sensor_filter_update(uint16_t raw_vref_mv, 
                                                uint16_t raw_signal_mv) {
    ptx_sensor_reading_t result = {0};

	//@todo filter algorithm depend of configuration
	result.vref_mv = raw_vref_mv;
	result.signal_mv = raw_signal_mv;
	result.valid = true;
    
    return result;
}

ptx_sensor_reading_t ptx_sensor_filter_read_and_update(void) {
    /* Read raw sensor values from hardware */
    uint16_t raw_vref_mv   = read_voltage(TEMPERATURE_SENSOR_REFERENCE);
    uint16_t raw_signal_mv = read_voltage(TEMPERATURE_SENSOR);
    
    /* Apply median filter */
    return ptx_sensor_filter_update(raw_vref_mv, raw_signal_mv);
}
