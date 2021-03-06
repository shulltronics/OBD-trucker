#ifndef _TEXTWIDGET_H_
#define _TEXTWIDGET_H_

#include "Arduino.h"
#include "Widget.h"
#include <U8g2lib.h>
#include "UI.h"
#include <Adafruit_Sensor.h>

// allows for values up to 999.9
#define DISPLAY_VALUE_CHAR_WIDTH 6

class TextWidget : public Widget
{
  public:
    TextWidget(U8G2_UC1610_EA_DOGXL160_F_4W_HW_SPI *screen, region_t region);
    void renderContent(sensor_t sensor, sensors_event_t event);
  private:
    U8G2_UC1610_EA_DOGXL160_F_4W_HW_SPI *_screen;
    region_t _region;
    int _region_width;
    int _region_height;
    // cursor location
    coordinate_t _cursor;
    int _font_height;
    void print_centered(char *str);
};

#endif
