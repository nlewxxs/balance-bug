#ifndef Communicate_h
#define Communicate_h

#include <WiFi.h>
#include <HTTPClient.h>
#include <String>
#include "Esp.h"

class Communicate {
  public: 
    bool getInitialised();
    void init(char *_ssid, char *_password, char *_serverName, String _bugId, float checkNewSessionTimeout);
    void ping();
    void checkNewSession();
    bool getInSession();

  private:
    bool initialised;
    bool inSession;
    String bugId;
    char* ssid;
    char* password;
    void checkConnection();
    HTTPClient httpPing;
    HTTPClient httpGetNewSession;
    String serverName;
};

#endif

