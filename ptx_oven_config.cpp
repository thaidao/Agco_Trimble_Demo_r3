/**
 * @file ptx_oven_config.cpp
 * @brief Implementation of runtime-configurable oven parameters
 */
#include "ptx_oven_config.h"
#include <stddef.h>

/* Internal configuration state */
static ptx_oven_config_t pti_oven_config = {
    .ignition_duration_ms   = 5000U,  	/* 5 seconds igniter ON */
    .periodic_log_ms        = 1000U,  	/* log every second */
    .sensor_fault_window_ms = 1000U,  	/* fault after 1s out-of-range */
    .auto_resume_delay_ms   = 3000U,  	/* resume after 3s valid */
    .vref_min_v             = 4.5f,   	/* min vref */
    .vref_max_v             = 5.5f,   	/* max vref */
    .temp_target_c          = 180.0f, 	/* target temperature */
    .temp_delta_c           = 5.0f,   	/* hysteresis half-band */
    .max_ignition_attempts  = 3U,     	/* 3 ignition retry attempts */
	  .iteration_period        = 50U,		/* 50 or 20Hz */
};

const ptx_oven_config_t* ptx_oven_get_config(void) {
    return &pti_oven_config;
}

void ptx_oven_set_config(const ptx_oven_config_t* config) {
    if (config != NULL) {
        pti_oven_config = *config;
    }
}

void ptx_oven_reset_config_to_defaults(void) {
    pti_oven_config.ignition_duration_ms   	= 5000U;
    pti_oven_config.periodic_log_ms        	= 1000U;
    pti_oven_config.sensor_fault_window_ms 	= 1000U;
    pti_oven_config.auto_resume_delay_ms   	= 3000U;
    pti_oven_config.vref_min_v             	= 4.5f;
    pti_oven_config.vref_max_v             	= 5.5f;
    pti_oven_config.temp_target_c          	= 180.0f;
    pti_oven_config.temp_delta_c           	= 2.0f;
    pti_oven_config.max_ignition_attempts  	= 3U;
    pti_oven_config.iteration_period        = 50U;
}

uint16_t ptx_oven_get_iteration_period(void) {
    return pti_oven_config.iteration_period;
}