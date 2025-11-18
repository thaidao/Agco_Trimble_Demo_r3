/*
We need your help to stop forest fires and bake tasty cookies!

See `requirements.md` for how to help.

Then check `notes.md`.
*/
#include "api.h"
#include "ptx_logging.h"
#include "ptx_actuator.h"
#include "ptx_oven_config.h"
#include "ptx_oven_control.h"

void setup() {

  //Initialize serial
  Serial.begin(115200);
  
  //Wrapper API
  setup_api();

  // Intialize controller
  ptx_oven_control_init();

  PTX_LOGF("Elf oven 2000 starting up.");
  PTX_LOGF("Days without fire incident: %i\n", 0);
}


void door_sensor_interrupt_handler(bool voltage_high)
{
  // @TODO: add small filtering for stability if needed
  if (voltage_high) {
    PTX_LOGF("[WARNING] Door is opened");
    ptx_actuator_emergency_stop();
  }

  // Propagate state to controller; controller loop will handle any logging.
  ptx_oven_set_door_state(voltage_high);
}

void loop() {
  // Some example code below to help show how to use API.
  // Please delete it and replace with your own code.
  // uint16_t sensor_voltage = read_voltage(TEMPERATURE_SENSOR);
  // serial_printf("sensor_voltage %i\n", sensor_voltage);
  // set_output(GAS_VALVE, sensor_voltage > 2000);

  //delay(1000); // feel free to change. What would you use for an actual iteration period?

  //quick test hardware
#if 0
  test_hardware();
#endif
  // Run oven control loop
  ptx_oven_control_update();

  // Delay 100 ms. Because temperature changes very slowly, Safety is guaranteed by the door interrupt, not the loop speed
  //delay(ptx_oven_get_iteration_period());
  delay(100);

}

// Simple hardware test
void test_hardware()
{
  uint16_t sensor_voltage = 0;

  while(1)
  {
    //Read sensor
    sensor_voltage = read_voltage(TEMPERATURE_SENSOR);
    serial_printf("sensor_voltage %i\n", sensor_voltage);

    sensor_voltage = read_voltage(TEMPERATURE_SENSOR_REFERENCE);
    serial_printf("sensor_voltage_ref %i\n", sensor_voltage);

    //Toggle all ouput
    // set_output(GAS_VALVE, true);
    // set_output(SYS_LED_STATUS, true);
    // set_output(IGNITER, true);

    ptx_actuator_set_gas(true);
    ptx_actuator_set_igniter(true);
    ptx_actuator_set_system_led_status(true);
    delay(1000);

    // set_output(GAS_VALVE, false);
    // set_output(SYS_LED_STATUS, false);
    // set_output(IGNITER, false);

    ptx_actuator_set_gas(false);
    ptx_actuator_set_igniter(false);
    ptx_actuator_set_system_led_status(false);
    delay(1000);
  }
}