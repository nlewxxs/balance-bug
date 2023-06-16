#ifndef Communicate_h
#define Communicate_h

#include <WiFi.h>
#include <HTTPClient.h>
#include <String>

class Communicate {
  public: 
    char* ssid;
    char* password;
    String serverName
    bool initialised;
    void init();
    // bool checkNewSession();
  // private:
  //   bool initialised;
};

#endif

