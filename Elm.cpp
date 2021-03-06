#include "Elm.h"
#include "Arduino.h"

// Constructor
Elm::Elm()
{
  
}

/*
 * Connect to the transciever on Serial1
 */
bool Elm::init()
{
  Serial1.begin(9600);
  char buf[ELM_MAX_RESPONSE_LENGTH] = "";
  Serial1.write("ate0\r");
  delay(50);
  getResponse(buf);
  Serial1.write("atl0\r");
  delay(50);
  getResponse(buf);
  Serial1.write("atsp1\r");
  delay(50);
  getResponse(buf);
  
  return true;
}

bool Elm::reset()
{
  Serial1.write("atz\r");
  delay(100);
  char buf[32] = "";
  getResponse(buf);
  return true;
}

bool Elm::setHeader(const char* header)
{
  Serial1.write("atsh");
  Serial1.write(header);
  Serial1.write("\r");
  char buf[32] = "";
  getResponse(buf);
  return true;
}

/*
 * Request a PID value from the PCM.
 * Put the response value into resp.
 */
elm_response_t Elm::requestPID(const char *PID, char *req)
{
  Serial1.write(PID);
  Serial1.write("\r");
  delay(50);
  char resp[ELM_MAX_RESPONSE_LENGTH] = "";
  getResponse(resp);
  // get the substrings for response error checking:
  char RESP_check[5], PID_check[5];
  strncpy(RESP_check, resp + 2, 4);
  RESP_check[4] = '\0';
  strncpy(PID_check, PID + 2, 4);
  PID_check[4] = '\0';
  if (strcmp(PID_check, RESP_check) != 0)
  {
    strncpy(req, resp, ELM_MAX_RESPONSE_LENGTH);
    return ELM_RESPONSE_ERROR;
  } else {
    // only return the value
    strncpy(req, resp + 6, ELM_PID_RESPONSE_LENGTH + 1);
    return ELM_RESPONSE_OK;
  }
}

/*
 * Read the serial buffer and collect the characters
 *  into b
 *
 */
void Elm::getResponse(char* b)
{
  int i = 0;
  while (Serial1.available())
  {
    char c = Serial1.read();
    if (c == 32 || c == 13 || c == 62)
    {
      // Skip over ' ' and CR
      continue;
    //} 
    //else if (c == 62) {
      // if '>' was received we can move on
    //  _busy = false;
    //  continue;
    } else {
      b[i] = c;
      i++;
    }
  }
  b[i] = '\0';  // terminate
}
