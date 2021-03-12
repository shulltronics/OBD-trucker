#include "TextWidget.h"

/*
 * Constructor
 */
TextWidget::TextWidget(U8G2_UC1610_EA_DOGXL160_F_4W_HW_SPI *screen, region_t region)
{
  _screen = screen;
  _region = region;
  _region_width = _region._end.x - _region._start.x;
  _region_height = _region._end.y - _region._start.y;
  _font_height = _screen->getMaxCharHeight();
  _cursor = {_region._start.x, _region._start.y};
}

/*
 * draws the sensor in its region.
 * Lists a sensor's:
 *   name
 *   value
 *   units (TODO)
 *   value range (TODO)
 */
void TextWidget::renderContent(sensor_t sensor, sensors_event_t event)
{
  // update the cursor and font height variables
  _cursor = {_region._start.x, _region._start.y + REGION_PX_MARGIN};
  _font_height = _screen->getMaxCharHeight();
  _cursor.y += _font_height;
  
  int display_width = _screen->getDisplayWidth();
  int display_height = _screen->getDisplayHeight();
  
  _screen->drawFrame(_region._start.x, _region._start.y, _region_width, _region_height);

  /* get the appropriate string name for the region size,
  // with 2px padding on each side
  char *names[3] =
  {"Hi from TextWidget!",
   "TextWidget",
   "TW"
  };
  char name[24] = "";
  for (int i = 0; i < 3; i++)
  {
    if (_screen->getStrWidth(names[i]) < _region_width-5)
    {
      strncpy(name, names[i], 24);
      break;
    }
  }
  print_centered(name);
  */

  // get the sensor value and units
  float value;
  char value_str[DISPLAY_VALUE_CHAR_WIDTH];
  char units_str[DISPLAY_VALUE_CHAR_WIDTH];
  switch(sensor.type)
  {
    case (SENSOR_TYPE_OBJECT_TEMPERATURE):
      value = System::centigradeToF(event.temperature);
      strncpy(units_str, "F", 2);
      break;
    case (SENSOR_TYPE_PRESSURE):
      value = event.pressure;
      strncpy(units_str, "PSI", 4);
      break;
    default:
      value = -1.0;
      break;
  }

  dtostrf(value, 4, 1, value_str);  // put the value into a string

  /* 
  char range_str[24];  // construct the range string
  sprintf(range_str, "range: %.1f -> %.1f", sensor.min_value, sensor.max_value);
  _screen->setCursor(0, 20);
  _screen->print(range_str);
    _screen->print(units_str);
  */
  char title[4];
  strncpy(title, sensor.name, 4);
  print_centered(title);

  setFont(u8g2_font_bauhaus2015_tn);
  print_centered(value_str);

  setFont(u8g2_font_tom_thumb_4x6_tr);
  print_centered(units_str);
  
  _screen->sendBuffer();

}

// print the string centered in the region
void TextWidget::print_centered(char *name)
{
  int strw = _screen->getStrWidth(name);
  int str_margin = (_region_width - strw) / 2;
  _cursor.x = _region._start.x + REGION_PX_MARGIN + str_margin;
  _screen->setCursor(_cursor.x, _cursor.y);
  _screen->print(name);
  _cursor.y += _font_height;
}

void TextWidget::setFont(const uint8_t *font)
{
  int oldcy = _cursor.y;
  int oldth = _font_height;
  _screen->setFont(font);
  _font_height = _screen->getMaxCharHeight();
  // put the cursor at the right spot for the new font
  _cursor.y = oldcy - oldth + _font_height;
  _screen->setCursor(_cursor.x, _cursor.y);
}
