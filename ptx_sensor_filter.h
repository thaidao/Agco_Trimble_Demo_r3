/**
 * @file ptx_sensor_filter.h
 * @brief Sensor reading wrapper with some filtering methods (e.g. median noise filtering)
 * @details Provides filtered temperature sensor, vref
 */
#ifndef PTX_SENSOR_FILTER_H
#define PTX_SENSOR_FILTER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Filtered sensor readings
 */
typedef struct {
    uint16_t vref_mv;           /**< Filtered reference voltage (mV) */
    uint16_t signal_mv;         /**< Filtered signal voltage (mV) */
    bool     valid;             /**< True if filter has enough samples */
} ptx_sensor_reading_t;

void ptx_sensor_filter_init(uint8_t window_size);
void ptx_sensor_filter_reset(void);

/**
 * @brief Read sensors from hardware and apply median filtering
 * @return Filtered sensor reading
 * @note Call this once per control update cycle
 */
ptx_sensor_reading_t ptx_sensor_filter_read_and_update(void);


#ifdef __cplusplus
}
#endif

#endif /* PTX_SENSOR_FILTER_H */
