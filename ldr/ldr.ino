#include "LDR.h"

LDR left;
LDR right;

void setup(){
  Serial.begin(9600);
  left.init(A1);
  right.init(A0);
}

void loop(){
  // char tmp[64];
  // sprintf(tmp, "LEFT: \t%d\tRIGHT:\t%d", left.getReading(), right.getReading());
  // Serial.println(tmp);

  if (left.isWall()){
    Serial.println("LEFT WALL");
  }

  if (right.isWall()){
    Serial.println("RIGHT WALL");
  }
}