#include "Communicate.h"

//function to set inital conditions to avoid hardcoding
void Communicate::init(String _beaconCharge, bool _beaconOn, String _serverName){
  beaconCharge = _beaconCharge;
  beaconOn = _beaconOn;
  serverName = _serverName;
}

//set the beacon charge
void Communicate::setBeaconCharge(String _beaconCharge) { beaconCharge = _beaconCharge; }

//get if the beacon is on
bool Communicate::getBeaconOn() { return beaconOn; }

//ping the server
void Communicate::pingBeaconOn(){
  //setup HTTP paths
  String pathName = serverName + "/Beacon/BeaconPing?ChargeStatus=" + beaconCharge;
  httpBeacon.begin(pathName.c_str());

  String params = "ChargeStatus="  + beaconCharge;;
  
  //make HTTP PATCH
  int httpBeaconResponseCode = httpBeacon.PATCH(params);

  //handle responses
  if (httpBeaconResponseCode>0) {
    //extract responses
    Serial.print("Beacon ping Response Code: ");
    Serial.println(httpBeaconResponseCode);
    String payload = httpBeacon.getString();
    Serial.println(payload);

    //check if expected response code
    if(httpBeaconResponseCode == 200){
      Serial.println("Ok, Response successfully processed");

      //handle items in the response and set Beacons On or Off accordingly
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
      //incorrect response
      Serial.print("Incorrect response code: ");
      Serial.println(httpBeaconResponseCode);
    }
  }
  else {
    //error in making the http request
    Serial.print("Error code: ");
    Serial.println(httpBeaconResponseCode);
  }

  //free resources
  httpBeacon.end();
}

