#include "SimpleTraversal.h"

//initialise
void SimpleTraversal::init(char *_ssid, char *_password, char *_serverName, String _bugId, float timeout){
  if (!communicate.getInitialised()){
    bugId = _bugId;
    communicate.init(_ssid, _password, _serverName, bugId, timeout);
  }

  angle = 0;
  distance = 0;
  movementDecision = Stationary;
  prevNode = "0";
  nodeNameCtr = 1;
  prevDistance = 0;
}

//Setters
void SimpleTraversal::setDistance(float _distance){ distance = _distance; }

void SimpleTraversal::setAngle(float _angle) { angle = _angle; }

void SimpleTraversal::setTurnAngle(float _angle) {}

void SimpleTraversal::setBugId(String _bugId) { bugId = _bugId; }

void SimpleTraversal::setMovementDecision(Decision _movementDecision) { movementDecision = _movementDecision; }

//getters
Decision SimpleTraversal::getDecision(){ return movementDecision; }


//calculate new coordinates relative to previous coordinates to add in the query
void SimpleTraversal::calculateCoords(){
  prevCoords = coords;


  // coords.x = distance * sin(angle);
  // coords.y = distance * cos(angle);

  coords.x = prevCoords.x + ((distance - prevDistance) * sin(angle));
  coords.y = prevCoords.y + ((distance - prevDistance) * cos(angle));

  prevDistance = distance;
  // prevAngle = angle;


  Serial.print("Calculated Coords: x=");
  Serial.print(coords.x);
  Serial.print(", y=");
  Serial.println(coords.y);

}

//make a decision on the given inputs from Classify
void SimpleTraversal::makeDecision(bool _isEnd, bool _isNode, bool _isPath, bool _isClear, bool _leftWall, bool _rightWall, bool _leftTurn, bool _rightTurn) {
  //check if in a session
  if(communicate.getInSession()){
    //process movementDecision based on result from Classify
    if (_isEnd && _isClear) {
      Serial.println("Stationary");
      movementDecision = Stationary;
    }
    // else if(_isEnd && !_leftTurn && !_rightTurn){
    //   Serial.println("Dead End");
    //   movementDecision = turningLeft ? Left : Right;

    // }
    else if(_leftTurn){

      Serial.println("Move Then Left");
      turningLeft = true;
      movementDecision = MoveThenLeft;
    }
    else if(_isClear/*&& isPath*/){
      Serial.println("Move");
      movementDecision = Forward;
    }
    else if(_rightTurn){
      Serial.println("Move Then Right");
      turningLeft = false;
      movementDecision = MoveThenRight;
    }
    else {
      Serial.println("Is End");
      movementDecision = turningLeft ? Right : Left; //THIS IS 100% THE CORRECT WAY DONT CHANGE
    }

    //send API request if Classify detects as a node
    if (_isNode) {
      Serial.println("Adding Node");
      // float tmpDist = distance-prevDistance

      calculateCoords();
      communicate.addNode(String(nodeNameCtr), String(coords.x*15), String(coords.y*15));
      communicate.addEdge(String(nodeNameCtr), String(prevNode), String(distance), String(angle));
      prevNode = nodeNameCtr;
      nodeNameCtr++;
    }
  }
  else {
    //otherwise we need to join a session first
    Serial.println("First join a session");
  }
}

