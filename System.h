#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include "Arduino.h"
#include "PCMParam.h"
#include "ThermoCouple.h"
#include "Adafruit_Sensor.h"
#include "RTClib.h"
#include <SD.h>

#define NUMBER_OF_SENSORS 6

#define SD_CARD_CS 33

// Number of milliseconds between SD card writes
#define LOGGING_PERIOD 5000

/*
 * System is responsible for managing system parameters,
 * timekeeping, and data logging
 */
class System
{
  public:
    System();
    void begin();
    void updateSensors();
    bool log();
    DateTime getDateTime();
    static float centigradeToF(float);
  private:
    // a string with the date and time that sys.begin() is called
    char _timeStamp[20];
    // the number to use for the file name (TODO)
    int _fileNumber;
    const char _fileName[13] = "/trucker.log";
    long _t;
};

#endif
