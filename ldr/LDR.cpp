#include "LDR.h"

void LDR::init(uint8_t _pin){
  pin = _pin;
  pinMode(pin, INPUT);
}

void LDR::update(){
  reading = analogRead(pin);
}

bool LDR::isWall(){
  update();
  return (reading > LDR_THRESHOLD) ? 1 : 0;
}

int LDR::getReading(){
  update();
  return reading;
}

