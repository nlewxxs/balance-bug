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
    bool getStatusMapSetup();
    void setUpMap();
    void addNode(String _nodeId, String _xCoord, String _yCoord);
    void addEdge(String _nodeId, String _edgeNodeId, String _distance, String _angle);
    bool getSufficientCharge();
    bool getBeaconStatusChange();
    void setBeacon(String _beaconValue);

  private:
    bool initialised;
    bool inSession;
    String bugId;
    char* ssid;
    char* password;
    void checkConnection();
    HTTPClient httpPing;
    HTTPClient httpGetNewSession;
    HTTPClient httpNode;
    HTTPClient httpEdge;
    HTTPClient httpBeacon;
    String serverName;
    bool mapSetup;
    bool httpNodeSetup;
    bool httpEdgeSetup;
    bool sufficientCharge;
    bool beaconStatusChange;
};

#endif

