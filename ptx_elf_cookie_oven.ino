/*
We need your help to stop forest fires and bake tasty cookies!

See `requirements.md` for how to help.

Then check `notes.md`.
*/
#include "api.h"


void setup() {
  Serial.begin(115200);
  setup_api();

  Serial.println("Elf oven 2000 starting up.");
  serial_printf("Days without fire incident: %i\n", 0);
}


void door_sensor_interrupt_handler(bool voltage_high)
{
  // TODO: implement
}


void loop() {
  // Some example code below to help show how to use API.
  // Please delete it and replace with your own code.
  uint16_t sensor_voltage = read_voltage(TEMPERATURE_SENSOR);
  serial_printf("sensor_voltage %i\n", sensor_voltage);
  set_output(GAS_VALVE, sensor_voltage > 2000);

  delay(1000); // feel free to change. What would you use for an actual iteration period?

  while(1)
  {
    set_output(GAS_VALVE, true);
    set_output(SYS_STATUS, true);
    set_output(IGNITER, true);
    delay(1000);

    set_output(GAS_VALVE, false);
    set_output(SYS_STATUS, false);
    set_output(IGNITER, false);
    delay(1000);
  }

}
