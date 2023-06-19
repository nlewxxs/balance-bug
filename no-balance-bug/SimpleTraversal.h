#ifndef SimpleTraversal_h
#define SimpleTraversal_h

#include <Arduino.h>
#include "Esp.h"
#include "Communicate.h"
#include <HardwareSerial.h>


//define decision enumeration
enum Decision{
  Stationary,
  Forward,
  Left,
  Right,
  Backwards,
  MoveThenLeft,
  MoveThenRight
};

struct Coords{
  int x;
  int y;
};

class SimpleTraversal {
  public: 
    void init(char *_ssid, char *_password, char *_serverName, String _bugId, float timeout);
    void setBugId(String _bugId);
    void setDistance(float _distance);
    void setAngle(float _angle);
    void setTurnAngle(float _angle);
    void makeDecision(bool _isEnd, bool _isNode, bool _isPath, bool _isClear, bool _leftWall, bool _rightWall, bool _leftTurn, bool _rightTurn);
    Decision getDecision();
    Communicate communicate;
  private:
    // bool initialised;
    String bugId;
    enum Decision movementDecision;
    void calculateCoords();
    float distance;
    float angle;
    Coords coords;
    int nodeNameCtr;
    String prevNode;
};


#endif

