#include "motor.h"
// timer and ISR for stepper control
// hw_timer_t *step_timer = NULL;

void IRAM_ATTR onStepTimer(int *counter, int *step, int *target){
  // ISR, check if need to pulse
  // lower limit for counter is given by (speed - 1000) * 1000
  // timer goes off every 1 microsecond
  // counter is incremented every time
  // step is change if counter limit exceeded
  *counter++;

  if (*counter > *target){
    counter = 0;
    digitalWrite(*step, !digitalRead(*step));
  }
}

void Motor::setForward(bool side){
  // if side true - left
  digitalWrite(dir, 1);
}

void Motor::setBackwards(bool side){
  digitalWrite(dir, 0);
}

void Motor::setSpeed(bool side, int desiredSpeed){
  Serial.println("Speed set");
  speed = desiredSpeed;
  target = (5000 - speed)*1;
}


void Motor::init() {
  // put your setup code here, to run once:
  pinMode(dir, OUTPUT);
  pinMode(step, OUTPUT);
  step_timer = timerBegin(0, 8, true);
  timerAttachInterrupt(step_timer, *onStepTimer(&counter, &step, &target), true);
  timerAlarmWrite(step_timer, 100, true);
  timerAlarmEnable(step_timer);
}
