#include "ThermoCouple.h"
//#include <Adafruit_MAX31856.h>

Adafruit_MAX31856 ThermoCouple::_tc(MAX31856_CS_PIN);

ThermoCouple::ThermoCouple()
{
 
}

void ThermoCouple::init()
{
  _tc.begin();
  _tc.setThermocoupleType(MAX31856_TCTYPE_K);
}

void ThermoCouple::update()
{
  _value = _tc.readThermocoupleTemperature();
}

bool ThermoCouple::getEvent(sensors_event_t *event)
{
  memset(event, 0, sizeof(sensors_event_t));
  event->version = sizeof(sensors_event_t);
  event->sensor_id = 0;
  event->type = SENSOR_TYPE_OBJECT_TEMPERATURE;
  event->timestamp = millis();
  event->temperature = _value;
  return true;
}

void ThermoCouple::getSensor(sensor_t *sensor)
{
  memset(sensor, 0, sizeof(sensor_t));
  strncpy (sensor->name, "EGT", sizeof(sensor->name) - 1);
  sensor->name[sizeof(sensor->name)- 1] = 0;
  sensor->version     = 1;
  sensor->sensor_id   = 0;
  sensor->type        = SENSOR_TYPE_OBJECT_TEMPERATURE;
  sensor->min_delay   = 0;
  sensor->min_value   = 0.0;
  sensor->max_value   = 1400.0;
  sensor->resolution  = 1.0;
}
