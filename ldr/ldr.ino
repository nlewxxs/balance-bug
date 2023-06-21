#include "LDR.h"

LDR left;

void setup(){
  left.init(10); // init on pin 10
}

void loop(){
  Serial.println(left.isWall());
}