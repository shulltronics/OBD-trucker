#include "PCMParam.h"
#include "ThermoCouple.h"
#include "UI.h"
#include "TextWidget.h"
#include <SPI.h>

float calcTFT(int v)
{
  float a = (v & 0xFF00) >> 8;
  float b = (v & 0x00FF);
  return (((a*256.0 + b) * -0.0036) + 212.98);
}

float calcEOT(int v)
{
  float a = (v & 0xFF00) >> 8;
  float b = (v & 0x00FF);
  return ((((a*256.0) + b) / 100.0) - 40.0);
}

float calcBAR(int v)
{
  float a = (v & 0xFF00) >> 8;
  float b = (v & 0x00FF);
  return (((a * 256.0) + b) * 0.03625);
}

//float (*calcMAP)(int) = *calcBAR;

// Constructors for PCM sensors
PCMParam TFT = PCMParam("TFT", SENSOR_TYPE_OBJECT_TEMPERATURE, "2211BD",
                        0.0, 250.0, 0.1, &calcTFT, 0);
PCMParam EOT = PCMParam("EOT", SENSOR_TYPE_OBJECT_TEMPERATURE, "221310",
                        0.0, 250.0, 0.1, &calcEOT, 1);
//Parameter MFD = Parameter("MFD", "221412");
//Parameter IPR = Parameter("IPR", "221434");
PCMParam MAP = PCMParam("MAP", SENSOR_TYPE_PRESSURE, "221440",
                         0.0, 45.0, 0.1, &calcBAR, 4);
PCMParam BAR = PCMParam("BAR", SENSOR_TYPE_PRESSURE, "221442",
                        10.0, 15.0, 0.1, &calcBAR, 5);
//Parameter EBP = Parameter("EBP", "221445\r");
//Parameter ICP = Parameter("ICP", "221446\r");

// Constructor for external sensors:
ThermoCouple EGT = ThermoCouple();
 
// UI Constructor
UI ui = UI();

// TODO: move widgets into UI logic (maybe?)
region_t r1 = {{0, 0}, {80, 50}};
region_t r2 = {{80, 0}, {160, 50}};
region_t r3 = {{0, 50}, {80, 104}};
region_t r4 = {{80, 50}, {160, 104}};
//Graph g = Graph(&TFT, &ui.screen);
TextWidget tw1(&ui.screen, r1);
TextWidget tw2(&ui.screen, r2);
TextWidget tw3(&ui.screen, r3);
TextWidget tw4(&ui.screen, r4);

void setup(void) {
  ui.init();
  
  // initialize the static serial comms object for parameters
  PCMParam::initComms();
  EGT.init();
}

void loop(void) {

  EOT.update();
  TFT.update();
  EGT.update();
  BAR.update();
  MAP.update();

  UI::clearBuffer();

  sensor_t s;
  sensors_event_t e;
  
  EOT.getSensor(&s);
  EOT.getEvent(&e);
  tw1.renderContent(s, e);

  TFT.getSensor(&s);
  TFT.getEvent(&e);
  tw2.renderContent(s, e);

  EGT.getSensor(&s);
  EGT.getEvent(&e);
  tw4.renderContent(s, e);

  sensor_t bar_sensor;
  sensors_event_t bar_event;
  BAR.getSensor(&bar_sensor);
  BAR.getEvent(&bar_event);

  sensor_t map_sensor;
  sensors_event_t map_event;
  MAP.getSensor(&map_sensor);
  MAP.getEvent(&map_event);

  sensor_t bst_sensor;
  strncpy (bst_sensor.name, "BST", sizeof(bst_sensor.name) - 1);
  bst_sensor.name[sizeof(bst_sensor.name)- 1] = 0;
  bst_sensor.version     = 1;
  bst_sensor.sensor_id   = 0;
  bst_sensor.type        = SENSOR_TYPE_PRESSURE;
  bst_sensor.min_delay   = 0;
  bst_sensor.min_value   = 0.0;
  bst_sensor.max_value   = 45.0;
  bst_sensor.resolution  = 0.1;
  
  sensors_event_t event;
  event.version = sizeof(sensors_event_t);
  event.sensor_id = 0;
  event.type = SENSOR_TYPE_PRESSURE;
  event.timestamp = millis();
  event.pressure = map_event.pressure - bar_event.pressure;

  tw3.renderContent(bst_sensor, event);
  
  /* TODO: move this to an OBDWidget
  char resp[16];
  EOT.getOBD(resp);
  UI::screen.setCursor(0, 10);
  UI::screen.clearBuffer();
  UI::screen.print(resp);
  UI::screen.sendBuffer();
  */
  
  delay(500);
}
