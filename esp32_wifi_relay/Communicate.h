#ifndef Communicate_h
#define Communicate_h

#include <HTTPClient.h>
#include <String>
#include "Esp.h"

class Communicate {
  public: 
    void init(String _beaconCharge, bool _beaconOn, String _serverName);
    void setBeaconCharge(String _beaconCharge);
    bool getBeaconOn();
    void pingBeaconOn();

  private:
    String beaconCharge;
    bool beaconOn;
    HTTPClient httpBeacon;
    String serverName;
};

#endif