#include "SimpleTraversal.h"

void SimpleTraversal::setBugId(String _bugId) { bugId = _bugId; }

void SimpleTraversal::init(char *_ssid, char *_password, char *_serverName, String _bugId, float timeout){
  if (!communicate.getInitialised()){
    bugId = _bugId;
    communicate.init(_ssid, _password, _serverName, bugId, timeout);
  }

  angle = 0;
  distance = 0;
  movementDecision = Stationary;
}

void SimpleTraversal::setDistance(float _distance){ distance = _distance; }

void SimpleTraversal::setAngle(float _angle) { angle = _angle; }

void SimpleTraversal::setTurnAngle(float _angle) {

}

Decision SimpleTraversal::getDecision(){ return movementDecision; }

void SimpleTraversal::calculateCoords(){
  coords.x = distance * sin(angle);
  coords.y = distance * cos(angle);
  Serial.print("Calculated Coords: x=");
  Serial.println(coords.x);
  Serial.print(", y=");
  Serial.print(coords.y);

}

void SimpleTraversal::makeDecision(bool _isEnd, bool _isNode, bool _isPath, bool _isClear, bool _leftWall, bool _rightWall, bool _leftTurn, bool _rightTurn) {
  if(communicate.getInSession()){
    if (_isEnd) {
      Serial.println("Is End");
      movementDecision = Left;
    }
    else if(_isPath && _isClear){
      Serial.println("Move");
      movementDecision = Forward;
    }
    else if(_leftTurn){
      Serial.println("Move Then Left");
      movementDecision = MoveThenLeft;
    }
    else if(_leftTurn){
      Serial.println("Move Then Right");
      movementDecision = MoveThenRight;
    }

    if (_isNode) {
      Serial.println("Adding Node");
      calculateCoords();
      communicate.addNode(String(nodeNameCtr), String(coords.x), String(coords.y));
      communicate.addEdge(String(nodeNameCtr), String(prevNode), String(distance), String(angle));
    }
  }
  else {
    Serial.println("First join a session");
  }
}

