/**
 * @file ptx_oven_config.h
 * @brief Configuration parameters for oven controller
 * @details Centralized timing, sensor thresholds, and safety parameters.
 *          All parameters are runtime-configurable via setter functions.
 */
#ifndef PTX_OVEN_CONFIG_H
#define PTX_OVEN_CONFIG_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Oven configuration structure with runtime-adjustable parameters
 */
typedef struct {
    uint32_t 	ignition_duration_ms;    // Duration igniter stays ON after gas opens (default: 5000ms)
    uint32_t 	periodic_log_ms;         // Interval between periodic status logs (default: 1000ms)
    uint32_t 	sensor_fault_window_ms;  // Out-of-range duration before latching fault (default: 1000ms)
    uint32_t 	auto_resume_delay_ms;    // Valid readings duration before clearing fault (default: 3000ms)
    float    	vref_min_v;              // Minimum acceptable reference voltage (default: 4.5V)
    float    	vref_max_v;              // Maximum acceptable reference voltage (default: 5.5V)
    float    	temp_target_c;           // Target temperature for control (default: 180.0°C)
    float    	temp_delta_c;            // Hysteresis half-band around target (default: 2.0°C)
    
    /* Ignition safety parameters */
    uint8_t  	max_ignition_attempts;   // Maximum number of ignition retry attempts (default: 3) 
	
	/* Others */
	uint16_t	iteration_period;		// Default 50 or ~20 Hz control loop; module logs once per second	

    
} ptx_oven_config_t;

uint16_t ptx_oven_get_iteration_period(void);


#ifdef __cplusplus
}
#endif

#endif /* PTX_OVEN_CONFIG_H */
