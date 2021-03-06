#ifndef _UI_H_
#define _UI_H_

#include "Arduino.h"
#include <U8g2lib.h>
#include <TouchScreen.h>
#include "Widget.h"

/*************** TYPES ***************/
// A coordinate is a point
typedef struct
{
  int x;
  int y;
} coordinate_t;

// A region is a rectangle defined by a start and end coordinate
typedef struct
{
  coordinate_t _start;
  coordinate_t _end;
} region_t;

typedef enum
{
  UL,
  UM,
  UR,
  LL,
  LM,
  LR,
  NONE
} TouchLocation;

#define NUMBER_OF_REGIONS 6

class UI
{
  public:
    static U8G2_UC1610_EA_DOGXL160_F_4W_HW_SPI screen;
    static TouchScreen ts;
    UI();
    void init();
    void update();
    static void clear() {screen.clear();}
    static void clearBuffer() {screen.clearBuffer();}
    void calibrateTouchScreen();
    void displayTSCal();
    void setBacklight(bool state);
  private:
    static region_t screen_region_boundaries[NUMBER_OF_REGIONS];
    static region_t ts_region_boundaries[NUMBER_OF_REGIONS];
    bool lastTsRegion;
    long tsTimer;
    void displayTSRegion(TouchLocation tl);
    void touchTest();
    TouchLocation getTouch();
};

#endif
