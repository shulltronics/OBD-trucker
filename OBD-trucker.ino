/*
 *  OBD-trucker
 *  A data logger and display for powertrain information.
 *  Written by Carsten Thue-Bludworth
 *  March, 2021
 */

#include "System.h"
#include "PCMParam.h"
#include "ThermoCouple.h"
#include "UI.h"
#include "TextWidget.h"
#include <SPI.h>

System sys = System();
UI     ui = UI();

void setup(void) {
  sys.begin();
  ui.begin();
}

void loop(void) {
  sys.updateSensors();
  sys.log();

  ui.update();
}
