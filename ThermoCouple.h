#ifndef _THERMOCOUPLE_H_
#define _THERMOCOUPLE_H_

#include "Arduino.h"
#include <Adafruit_MAX31856.h>
#include <Adafruit_Sensor.h>

#define MAX31856_CS_PIN 27

/*
 * An instance is a MAX31865 Thermocouple amplifier
 *   over HW SPI and implimenting Adafruit's Unified Sensor Driver
 */
class ThermoCouple : public Adafruit_Sensor
{
  public:
    static Adafruit_MAX31856 _tc;   
    ThermoCouple();
    void init();
    void update();
    bool getEvent(sensors_event_t*);
    void getSensor(sensor_t*);
  private:
    float _value;
};

#endif
