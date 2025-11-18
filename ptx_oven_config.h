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
	uint16_t	iteration_period;		// 100ms	

    
} ptx_oven_config_t;

/**
 * @brief Get pointer to current configuration (read-only access)
 * @return Pointer to const configuration structure
 */
const ptx_oven_config_t* ptx_oven_get_config(void);

/**
 * @brief Update oven configuration with new parameters
 * @param config Pointer to new configuration structure
 * @note Changes take effect immediately on next control update
 */
void ptx_oven_set_config(const ptx_oven_config_t* config);

/**
 * @brief Reset configuration to default values
 */
void ptx_oven_reset_config_to_defaults(void);


void ptx_oven_set_ignition_duration_ms(uint32_t duration_ms);
uint32_t ptx_oven_get_ignition_duration_ms(void);
void ptx_oven_set_periodic_log_ms(uint32_t interval_ms);
uint32_t ptx_oven_get_periodic_log_ms(void);
void ptx_oven_set_sensor_fault_window_ms(uint32_t window_ms);
uint32_t ptx_oven_get_sensor_fault_window_ms(void);
void ptx_oven_set_auto_resume_delay_ms(uint32_t delay_ms);
uint32_t ptx_oven_get_auto_resume_delay_ms(void);
void ptx_oven_set_vref_range_v(float min_v, float max_v);
float ptx_oven_get_vref_min_v(void);
float ptx_oven_get_vref_max_v(void);
void ptx_oven_set_temp_target_c(float target_c);
float ptx_oven_get_temp_target_c(void);
void ptx_oven_set_temp_delta_c(float delta_c);
float ptx_oven_get_temp_delta_c(void);
void ptx_oven_set_max_ignition_attempts(uint8_t attempts);
uint8_t ptx_oven_get_max_ignition_attempts(void);
uint16_t ptx_oven_get_iteration_period(void);


#ifdef __cplusplus
}
#endif

#endif /* PTX_OVEN_CONFIG_H */
