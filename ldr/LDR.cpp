#include "LDR.h"

void LDR::init(uint8_t _pin){
  pin = _pin;
  pinMode(pin, INPUT);
}

void LDR::update(){
  reading = analogRead(pin);
  Serial.print(reading);
}

bool LDR::isWall(){
  update();
  return (reading > LDR_THRESHOLD) ? 1 : 0;
}

