#ifndef _ELM_H_
#define _ELM_H_

/*
 * An instance of this class is a serial connection
 * to the ELM327 OBD transciever.
 */

 #include "Arduino.h"

#define ELM_MAX_RESPONSE_LENGTH 16
#define ELM_PID_RESPONSE_LENGTH 4

typedef enum
{
  ELM_RESPONSE_BUSY,
  ELM_RESPONSE_NO_DATA,
  ELM_RESPONSE_ERROR,
  ELM_RESPONSE_OK
} elm_response_t;

 
class Elm
{
  public:
    Elm(void);
    bool init();
    bool reset();
    bool setHeader(const char *header);
    elm_response_t requestPID(const char *PID, char *resp);
    void getResponse(char* b);
  private:
    bool _busy = false;
};

#endif
