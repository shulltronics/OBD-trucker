#include "PCMParam.h"
#include "Arduino.h"

// Elm Constructor
#define OBD_HEADER "C410F1"
Elm PCMParam::truck = Elm();

PCMParam::PCMParam(const char *name, sensors_type_t sensor_type, const char *PID,
                   float min_value, float max_value, float resolution, 
                   float (*f)(int), int sensorID)
{
  _name = name;
  _sensor_type = sensor_type;
  _PID = PID;
  _min_value = min_value;
  _max_value = max_value;
  _resolution = resolution;
  _value = 0.0;
  _f = f;
  _sensorID = sensorID;
}

// Start the OBD communication
void PCMParam::initComms()
{
  truck.init();
  truck.setHeader(OBD_HEADER);
}

/*
 * writes the raw OBD response to buf;
 */
void PCMParam::getOBD(char *buf)
{
  elm_response_t r = truck.requestPID(_PID, buf);
  switch (r)
  {
    case (ELM_RESPONSE_OK):
      break;
    case (ELM_RESPONSE_ERROR):
      strncpy(buf, "ERROR", 6);
      break;
    case (ELM_RESPONSE_BUSY):
      strncpy(buf, "BUSY", 5);
      break;
  }
}

void PCMParam::update()
{
  char resp[16] = "";
  elm_response_t r = truck.requestPID(_PID, resp);
  if (r != ELM_RESPONSE_OK)
  {
    // set _value to 0 if we don't get a valid response
    _value = 0.0;
    return;
  }
  // otherwise convert the 4 byte hex string into an int
  int v = strtol(resp, NULL, 16);
  // and use the conversion to get our float value
  _value = _f(v);
}

bool PCMParam::getEvent(sensors_event_t *event)
{
  memset(event, 0, sizeof(sensors_event_t));
  event->version = sizeof(sensors_event_t);
  event->sensor_id = _sensorID;
  event->type = _sensor_type;
  event->timestamp = millis();
  switch (_sensor_type)
  {
    case (SENSOR_TYPE_OBJECT_TEMPERATURE):
      event->temperature = _value;
      break;
    case (SENSOR_TYPE_PRESSURE):
      event->pressure = _value;
      break;
    default:
      return false;
      break;
  }

  return true;
}

void PCMParam::getSensor(sensor_t *sensor)
{
  memset(sensor, 0, sizeof(sensor_t));
  strncpy (sensor->name, _name, sizeof(sensor->name) - 1);
  sensor->name[sizeof(sensor->name)- 1] = 0;
  sensor->version     = 1;
  sensor->sensor_id   = _sensorID;
  sensor->type        = _sensor_type;
  sensor->min_delay   = 0;
  sensor->min_value   = _min_value;
  sensor->max_value   = _max_value;
  sensor->resolution  = _resolution;
}
