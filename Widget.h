#ifndef _WIDGET_H
#define _WIDGET_H

#include "Arduino.h"
#include <Adafruit_Sensor.h>

class Widget
{
  public:
    Widget() {}
    void renderContent(sensor_t sensor, sensors_event_t event);
};

#endif
