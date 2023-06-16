#ifndef Communicate_h
#define Communicate_h

#include <WiFi.h>
#include <HTTPClient.h>
#include <String>
#include "Esp.h"

class Communicate {
  public: 
    bool getInitialised();
    bool getInSession();
    void init(char *_ssid, char *_password, char *_serverName, String _bugId);
    void ping();
    // bool checkNewSession();
  private:
    bool initialised;
    bool inSession;
    String bugId;
    char* ssid;
    char* password;
    void checkConnection();
    // HTTPClient http;
    String serverName;
};

#endif

