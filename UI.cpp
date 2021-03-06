#include "UI.h"
#include "Arduino.h"
#include <EEPROM.h>

/*************** Static Objects ***************/

// Define the screen and touchscreen
#define LCD_CS_PIN 14
#define LCD_CD_PIN 21
#define LCD_RESET_PIN U8X8_PIN_NONE
#define LCD_BACKLIGHT_PIN 13
#define LCD_BACKLIGHT_PWM 100
U8G2_UC1610_EA_DOGXL160_F_4W_HW_SPI UI::screen(U8G2_R2, /* cs=*/ LCD_CS_PIN, /* dc=*/ LCD_CD_PIN, /* reset=*/  LCD_RESET_PIN);

// Touchscreen Constructor
#define TS_CAL_COMP_EEPROM 0
#define TS_CAL_DATA_EEPROM 1
#define XM A7  // must be an analog pin, use "An" notation!
#define YP A8  // must be an analog pin, use "An" notation!
#define XP 25  // can be a digital pin
#define YM 4  // can be a digital pin
TouchScreen UI::ts = TouchScreen(XP, YP, XM, YM, 388);

#define UI_UPDATE_PERIOD 50

/*************** HELPER FUNCTIONS ***************/

// Returns a slightly modified TSPoint
TSPoint getTSPoint()
{
  TSPoint p = UI::ts.getPoint();
  p.y = 1023 - p.y;
  p.z = -p.z;
  return p;
}

/*************** VARIABLES **********************/

// These are the pressures to expect per region
// (used because x y values vary drastically with pressure)
int ts_region_pressures[NUMBER_OF_REGIONS] = 
{
  42,
  72,
  82,
  52,
  82,
  92
};

// These are the boundaries of the touch zones
region_t UI::ts_region_boundaries[NUMBER_OF_REGIONS];

// These are the regions on the screen
region_t UI::screen_region_boundaries[NUMBER_OF_REGIONS] =
{
  {{0  ,  0}, { 52, 52}},
  {{53 ,  0}, {107, 52}},
  {{107,  0}, {160, 52}},
  {{0  , 53}, {52 ,104}},
  {{53 , 53}, {107,104}},
  {{107, 53}, {160,104}},
};

/*************** Class Methods ******************/

UI::UI()
{
  
}

void UI::init()
{
  analogReadResolution(10); // needed for the touchscreen library
  screen.begin();
  //screen.setFont(u8g2_font_missingplanet_t_all);
  screen.setFont(u8g2_font_tom_thumb_4x6_tr);
  screen.setContrast(66);
  screen.setCursor(0, 10);

  ledcSetup(0, 100, 8);     // Backlight setup
  ledcAttachPin(LCD_BACKLIGHT_PIN, 0);
  setBacklight(true);
  
  EEPROM.begin(2*sizeof(ts_region_boundaries)); // for saving touchscreen calibration
  // TODO: get EEPROM size right.. it works for now
  //bool is_ts_cald = EEPROM.read(TS_CAL_COMP_EEPROM);
  EEPROM.get(TS_CAL_DATA_EEPROM, ts_region_boundaries);

  // Used for debouncing
  lastTsRegion = NONE;
  tsTimer = millis();
  
}

void UI::update()
{
  TouchLocation tl = getTouch();
  displayTSRegion(tl);
  screen.setCursor(0, 10);
  screen.print(tl);
  screen.sendBuffer();
  //touchTest();
  delay(UI_UPDATE_PERIOD);
}

/*
 * Calibrate the touchscreen:
 *  Display a region, and have the user outline it for
 *  TS_CAL_DURATION milliseconds. Store the results in
 *  ts_region_boundaries array and in EEPROM.
 *  Repeat for all regions.
 *  TODO: fine tune calibration parameters and procedure.
 */
#define TS_CAL_DURATION 1000
#define TS_CAL_PRESSURE_THRESH 5
void UI::calibrateTouchScreen()
{
  displayTSCal();
  delay(2500);
  UI::clear();
  for (int tl = UL; tl <= LR; tl++)
  {
    displayTSRegion((TouchLocation)tl);
    int cx = screen_region_boundaries[tl]._start.x + 10;
    int cy = screen_region_boundaries[tl]._start.y + 10;
    screen.setCursor(cx, cy);
    int xmin = 1023;
    int xmax = 0;
    int ymin = 1023;
    int ymax = 0;
    int ms = 0;
    while (ms < TS_CAL_DURATION)
    {
      screen.setCursor(cx, cy);
      screen.print("    ");
      screen.setCursor(cx, cy);
      TSPoint p = getTSPoint();
      // filter based on pressure (dependent upon location)
      if (p.z >= ts_region_pressures[tl] &&
          p.z <= (ts_region_pressures[tl] + TS_CAL_PRESSURE_THRESH))
      {
        xmin = min(xmin, (int)p.x);
        xmax = max(xmax, (int)p.x);
        ymin = min(ymin, (int)(p.y));
        ymax = max(ymax, (int)(p.y));
        ms += 10;
      }
      screen.print(ms);
      screen.sendBuffer();
      delay(10);
    }
    ts_region_boundaries[tl]._start.x = xmin;
    ts_region_boundaries[tl]._start.y = ymin;
    ts_region_boundaries[tl]._end.x = xmax;
    ts_region_boundaries[tl]._end.y = ymax;
    UI::clear();
    screen.setCursor(5, 20);
    screen.println(xmin);
    screen.print(" ");
    screen.print(xmax);
    screen.setCursor(5, 40);
    screen.println(ymin);
    screen.print(" ");
    screen.print(ymax);
    screen.sendBuffer();
    delay(1000);
    UI::clear();
  }
  EEPROM.put(TS_CAL_DATA_EEPROM, ts_region_boundaries);
  EEPROM.commit();
  screen.print("ts cal complete");
  screen.sendBuffer();
  delay(1000);
  UI::clear();
  displayTSCal();
}

// Pretty-print the cal data to the screen
void UI::displayTSCal()
{
  UI::clear();
  int cx = 0;
  int cy = 5;
  for (int tl = UL; tl <= LR; tl++)
  {
    screen.setCursor(cx, cy);
    screen.print("xmin: ");
    screen.print(ts_region_boundaries[tl]._start.x);
    screen.print(" xmax: ");
    screen.print(ts_region_boundaries[tl]._end.x);
    screen.print(" ymin: ");
    screen.print(ts_region_boundaries[tl]._start.y);
    screen.print(" ymax: ");
    screen.print(ts_region_boundaries[tl]._end.y);
    cy += 8;
  }
  int mx = abs(ts_region_boundaries[UL]._end.x - ts_region_boundaries[UL]._start.x) / 2;
  int my = abs(ts_region_boundaries[UL]._end.y - ts_region_boundaries[UL]._start.y) / 2;
  screen.setCursor(5, 90);
  screen.print(mx);
  screen.print(" ");
  screen.print(my);
  screen.sendBuffer();
}

// set the backlight on of off
void UI::setBacklight(bool state)
{
  if (state)
  {
    ledcWrite(0, LCD_BACKLIGHT_PWM);
  } else {
    ledcWrite(0, 0);
  }
}

// Draw a frame around the touchscreen region
void UI::displayTSRegion(TouchLocation tl)
{
  //screen.clear();
  if (tl == NONE)
  {
    return;
  } else {
    screen.clear();
    int sx = screen_region_boundaries[tl]._start.x;
    int sy = screen_region_boundaries[tl]._start.y;
    int w  = screen_region_boundaries[tl]._end.x - sx;
    int h  = screen_region_boundaries[tl]._end.y - sy;
    screen.setCursor(sx + 3, sy + 12);
    screen.print(tl);
    screen.drawFrame(sx, sy, w, h);
    screen.sendBuffer();
  }
}

void UI::touchTest()
{
  screen.clearBuffer();
  int mx = screen.getDisplayWidth() / 2;
  int my = screen.getDisplayHeight() / 2;
  screen.setCursor(mx-30, my);
  TSPoint p = getTSPoint();
  if (p.z > 0)
  {
    screen.print("p.x: ");
    screen.print(p.x);
    screen.print("p.y ");
    screen.print(p.y);
    screen.print("p.z ");
    screen.print(p.z);
  } else {
    displayTSCal();
  }
  screen.sendBuffer();
}

//#define TS_REGION_THRESH 120
//#define BUTTON_HYSTERESIS 0
TouchLocation UI::getTouch()
{
  // TODO: debounce by looking at elapsed time since last state change
  
  TouchLocation _tl;
  TSPoint p = getTSPoint();
  if (p.z == 0)
  {
    _tl = NONE;
  } else {
    // iterate through the regions and see if the 
    // touch location is within
    bool have_x, have_y, have_z;
    for (int tl = UL; tl <= LR; tl++)
    {
      have_x = false;
      have_y = false;
      have_z = false;
      if (p.z >= ts_region_pressures[tl] &&
          p.z <= (ts_region_pressures[tl] + TS_CAL_PRESSURE_THRESH))
      {
        have_z = true;
      }
      #ifdef TS_REGION_THRESH
        int mx = abs(ts_region_boundaries[tl]._end.x - ts_region_boundaries[tl]._start.x) / 2;
        int my = abs(ts_region_boundaries[tl]._end.y - ts_region_boundaries[tl]._start.y) / 2;
        if (abs(p.x - mx) <= TS_REGION_THRESH)
        {
          have_x = true;
        }
        if (abs(p.y - my) <= TS_REGION_THRESH)
        {
          have_y = true;
        }
      #else
        if (p.x >= ts_region_boundaries[tl]._start.x &&
            p.x <= ts_region_boundaries[tl]._end.x)
        {
          have_x = true;
        }
        if (p.y >= ts_region_boundaries[tl]._start.y &&
            p.y <= ts_region_boundaries[tl]._end.y)
        {
          have_y = true;
        }
      #endif
      if (have_x && have_y && have_z)
      {
        _tl = (TouchLocation)tl;
      }
    }
  }
  #ifdef BUTTON_HYSTERESIS
  long dt = millis() - tsTimer;
  if (_tl != lastTsRegion && dt >= BUTTON_HYSTERESIS)
  {
    lastTsRegion = _tl;
    tsTimer = millis();
  } else {
    _tl = NONE;
  }
  #endif
  return _tl;
}
