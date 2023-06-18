#include "Communicate.h"

//function to set inital conditions to avoid hardcoding
void Communicate::init(String _beaconCharge, bool _beaconOn, String _serverName){
  beaconCharge = _beaconCharge;
  beaconOn = _beaconOn;
  serverName = _serverName;
}
//"http://90.196.3.86:8081"

void Communicate::setBeaconCharge(String _beaconCharge) { beaconCharge = _beaconCharge; }
bool Communicate::getBeaconOn() { return beaconOn; }

void Communicate::pingBeaconOn(){
  String pathName = serverName + "/Beacon/BeaconPing?ChargeStatus=" + beaconCharge;
  httpBeacon.begin(pathName.c_str());

  String params = "ChargeStatus="  + beaconCharge;;
  int httpBeaconResponseCode = httpBeacon.PATCH(params);

  if (httpBeaconResponseCode>0) {
    Serial.print("Beacon ping Response Code: ");
    Serial.println(httpBeaconResponseCode);
    String payload = httpBeacon.getString();
    Serial.println(payload);

    if(httpBeaconResponseCode == 200){
      Serial.println("Ok, Response successfully processed");

      if(payload == "\"0\"") {
        Serial.println("Beacon Off");
        beaconOn = false;
      }
      if(payload == "\"1\"") {
        Serial.println("Beacon On");
        beaconOn = true;
      }
    }
    else{
      Serial.print("Incorrect response code: ");
      Serial.println(httpBeaconResponseCode);
    }
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpBeaconResponseCode);
  }
  httpBeacon.end();
}

