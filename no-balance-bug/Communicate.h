#ifndef Communicate_h
#define Communicate_h

#include <WiFi.h>
#include <HTTPClient.h>
#include <String>
#include "Esp.h"

class Communicate {
  public: 
    char* ssid;
    char* password;
    String serverName;
    void init(char *_ssid, char *_password, char *_serverName);
    // bool checkNewSession();
  private:
    bool initialised;
};

#endif

