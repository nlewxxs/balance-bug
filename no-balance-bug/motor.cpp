#include "motor.h"
// timer and ISR for stepper control
// hw_timer_t *step_timer = NULL;
hw_timer_t *step_timer = NULL;

void IRAM_ATTR Motor::onStepTimer(){
  // ISR, check if need to pulse
  // lower limit for counter is given by (speed - 1000) * 1000
  // timer goes off every 1 microsecond
  // counter is incremented every time
  // step is change if counter limit exceeded
  leftCounter++;
  rightCounter++;
  


  if (leftCounter > leftTarget){
    leftCounter = 0;
    digitalWrite(stepLeft, !digitalRead(stepLeft));
  }
  if (rightCounter > rightTarget){
    rightCounter = 0;
    digitalWrite(stepRight, !digitalRead(stepRight));
  }
}

void Motor::setForward(bool side){
  // if side true - left
  if (side){
    digitalWrite(dirLeft, 1);
  } else {
    digitalWrite(dirRight, 1);
  }
}

void Motor::setBackwards(bool side){
  // if side true - left
  if (side){
    digitalWrite(dirLeft, 0);
  } else {
    digitalWrite(dirRight, 0);
  }
}

void Motor::setSpeed(bool side, int desiredSpeed){
  Serial.println("Speed set");
  // if side true - left
  if (side){
    leftSpeed = desiredSpeed;
    leftTarget = (5000 - leftSpeed)*1;
  } else {
    rightSpeed = desiredSpeed;
    rightTarget = (5000 - rightSpeed)*1;
  }
  Serial.print("LeftTarget: ");
  Serial.println(leftTarget);
}


void Motor::init() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(dir, OUTPUT);
  pinMode(step, OUTPUT);
  step_timer = timerBegin(0, 8, true);
  timerAttachInterrupt(step_timer, &onStepTimer, true);
  timerAlarmWrite(step_timer, 100, true);
  timerAlarmEnable(step_timer);
}
