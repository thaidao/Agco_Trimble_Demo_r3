#include "api.h"
#include "Arduino.h"
#include <stdarg.h>

// NOTE!!!
// Don't assume the below API follows good coding or PTx conventions.
// Follow your own best judgement for your own code.
// Feel free to rework this API if you like (not required).

static void door_sensor_IRQ_handler();


void setup_api()
{
  pinMode(GAS_VALVE_PIN, OUTPUT);       // digital pin 2
  pinMode(DOOR_SWITCH_PIN, INPUT);      // digital pin 3
  pinMode(SYS_LED_STATUS_PIN, OUTPUT);  // digital pin 6
  pinMode(IGNITER_PIN, OUTPUT);         // digital pin 7

  door_sensor_interrupt_handler(digitalRead(DOOR_SWITCH_PIN) == HIGH); // we may not get an interrupt at startup, so we call the handler manually
  attachInterrupt(digitalPinToInterrupt(DOOR_SWITCH_PIN), door_sensor_IRQ_handler, CHANGE); // digital pin 3
}

static void door_sensor_IRQ_handler()
{
  door_sensor_interrupt_handler(digitalRead(DOOR_SWITCH_PIN) == HIGH);
}

// returns voltage in millivolts
uint16_t read_voltage(input_t input)
{ 
  if (input == TEMPERATURE_SENSOR)
  {
    return (uint32_t)analogRead(A0) * 5000 / 1023;
    //return //450 ~-10V;//2598 ~175;//2715 ~185;//2656;//~180
  }
  else if (input == TEMPERATURE_SENSOR_REFERENCE)
  {
    //return (uint32_t)analogRead(A1) * 5000 / 1023;
    return ((uint32_t)analogRead(A1) * 1000 / 1023) + 4500; //Range from 4.5V to 5.5V for easier testing
    //return 4500;
  }

  return 0;
}

// true for on, false for off
void set_output(output_t output, bool output_state)
{
  if (output == GAS_VALVE)
  {
    digitalWrite(GAS_VALVE_PIN, output_state);
  }
  else if (output == SYS_LED_STATUS)
  {
    digitalWrite(SYS_LED_STATUS_PIN, output_state);
  }
  else if (output == IGNITER)
  {
    digitalWrite(IGNITER_PIN, output_state);
  }

}

// read current output state
bool read_output(output_t output)
{
  if (output == GAS_VALVE)
  {
    return digitalRead(GAS_VALVE_PIN) == HIGH;
  }
  else if (output == SYS_LED_STATUS)
  {
    return digitalRead(SYS_LED_STATUS_PIN) == HIGH;
  }
  else if (output == IGNITER)
  {
    return digitalRead(IGNITER_PIN) == HIGH;
  }
  return false;
}

uint32_t get_millis()
{
  return millis();
}

void serial_printf(const char * format, ...)
{
  char buffer[256];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, 256, format, args);
  va_end (args);

  Serial.print(buffer);
}
