#ifndef Communicate_h
#define Communicate_h

//imports
#include <HTTPClient.h>
#include <String>
#include "Esp.h"


//definition of communicate class
class Communicate {
  public: 
    //functions
    void init(String _beaconCharge, bool _beaconOn, String _serverName);
    void setBeaconCharge(String _beaconCharge);
    bool getBeaconOn();
    void pingBeaconOn();

  private:
    //internal variables used to interact to/from server and store responses
    String serverName;
    String beaconCharge;
    bool beaconOn;
    
    //HTTPClient object
    HTTPClient httpBeacon;
};

#endif