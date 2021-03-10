#include "System.h"

/*************** PCMParam calculators ****************************************/
float calcEBP(int v)
{
  float a = (v & 0xFF00) >> 8;
  float b = (v & 0x00FF);
  return (((a * 256.0) + b) * 0.03625);
}

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


/*************** PCMParam objects ********************************************/
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
PCMParam EBP = PCMParam("EBP", SENSOR_TYPE_PRESSURE, "221445",
                         0.0, 55.0, 0.1, &calcEBP, 6);
//Parameter EBP = Parameter("EBP", "221445\r");
//Parameter ICP = Parameter("ICP", "221446\r");


/*************** Other sensor objects **************************************/
ThermoCouple EGT = ThermoCouple();

/*************** RTC object ************************************************/
RTC_PCF8523 rtc;

/*************** Other helper functions ************************************/
void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

// Write a new file to the SD card. This will overwrite if the file already exists!
bool writeFile(fs::FS &fs, const char * path, const char * message){
  File file = fs.open(path, FILE_WRITE);
  if (!file)
  {
    Serial.println("Error opening file.");
    return false;
  }
  if (!file.println(message))
  {
    Serial.println("Error writing to file.");
    return false;
  }
  file.close();
  return true;
}

// Append to a file on the SD card
bool appendFile(fs::FS &fs, const char * path, const char * message){
  File file = fs.open(path, FILE_APPEND);
  if (!file)
  {
    Serial.println("Error opening file for append.");
    return false;
  }
  if (!file.println(message))
  {
    Serial.println("Error appending to file.");
    return false;
  }
  file.close();
  return true;
}


/*************** Class implimentation **************************************/
System::System()
{
  strncpy(_timeStamp, "YYYY-MM-DD_hh:mm:ss", sizeof(_timeStamp)-1);
  _timeStamp[sizeof(_timeStamp)-1] = NULL;
  _fileNumber = 0;  // TODO
  _t = millis();
}

void System::begin()
{
  // Serial comms for USB interface
  Serial.begin(115200);

  // initialize static serial comms for PCMParam objects
  PCMParam::initComms();
  EGT.init();

  // Start the RTC and set the time if needed
  if (!rtc.begin())
  {
    Serial.println("RTC init error.");
  }

  if (!rtc.initialized() || rtc.lostPower())
  {
    Serial.println("Adjusting RTC clock...");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  rtc.start();
  delay(500);
  DateTime now = getDateTime();
  now.toString(_timeStamp);
  Serial.print("OBD-trucker ");
  Serial.println(_timeStamp);

  if (!SD.begin(SD_CARD_CS))
  {
    Serial.println("SD card init error.");
  }

  if (!appendFile(SD, _fileName, _timeStamp))
  {
    Serial.println("Error writing timeStamp to SD card.");
  }

}

void System::updateSensors()
{
  TFT.update();
  EOT.update();
  MAP.update();
  BAR.update();
  EBP.update();
  EGT.update();
}

bool System::log()
{
  if (millis() - _t > LOGGING_PERIOD)
  {
    _t = millis();

    char str[21];

    char time[10] = "hh:mm:ss ";
    DateTime now = getDateTime();
    now.toString(time);

    // e.g. "EOT: 193.6"
    char data[11];
    sensor_t s;
    sensors_event_t ev;
    EOT.getSensor(&s);
    EOT.getEvent(&ev);
    float val;
    switch(s.type)
    {
      case (SENSOR_TYPE_OBJECT_TEMPERATURE):
        val = ev.temperature;
        break;
      case (SENSOR_TYPE_PRESSURE):
        val = ev.pressure;
        break;
      default:
        val = -1.0;
        break;
    }
    sprintf(data, "EOT: %05.1f", centigradeToF(val));

    strncpy(str, time, sizeof(time)-1);
    strncpy(str + (sizeof(time)-1), data, sizeof(data));
    if (appendFile(SD, _fileName, str))
    {
      Serial.println("appended to log");
    } else {
      Serial.println("failure appending");
    }
    return true;
  } else {
    return true;
  }
}

DateTime System::getDateTime()
{
  return rtc.now();
}

// Convert degrees centigrade to degrees Farenheight
float System::centigradeToF(float dC)
{
  return ((dC * 1.8) + 32);
}

