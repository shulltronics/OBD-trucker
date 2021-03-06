#ifndef _PCMPARAM_H_
#define _PCMPARAM_H_

#include <Adafruit_Sensor.h>
#include "Arduino.h"
#include "Elm.h"

/*
 * An instance of this class is a value read 
 * from the OBD port and all it's associated
 * information.
 */

class PCMParam : public Adafruit_Sensor
{
  public:
    const char* _name;
    const char* _PID;
    PCMParam(const char *name, sensors_type_t sensor_type, const char *PID,
             float min_value, float max_value, float _resolution, 
             float (*f)(int), int sensorID);
    static void initComms();
    void getOBD(char *buf);
    void update();
    bool getEvent(sensors_event_t*);
    void getSensor(sensor_t*);
  private:
    // the shared interface to the OBD port
    static Elm truck;
    // variables defining the specific sensor
    sensors_type_t _sensor_type;
    float _min_value;
    float _max_value;
    float _resolution;
    float (*_f)(int);
    int _sensorID;
    float _value;
};

#endif
