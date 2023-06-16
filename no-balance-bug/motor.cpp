#include "motor.h"
// timer and ISR for stepper control
// hw_timer_t *step_timer = NULL;

static void IRAM_ATTR Motor::onStepTimer(){
  // ISR, check if need to pulse
  // lower limit for counter is given by (speed - 1000) * 1000
  // timer goes off every 1 microsecond
  // counter is incremented every time
  // step is change if counter limit exceeded
  leftCounter++;
  rightCounter++;

  if (leftCounter > leftTarget){
    leftCounter = 0;
    digitalWrite(leftStep, !digitalRead(leftStep));
  }
  if (rightCounter > rightTarget){
    rightCounter = 0;
    digitalWrite(rightStep, !digitalRead(rightStep));
  }
}

static void Motor::setForward(bool side){
  // if side true - right
  if (side){
    digitalWrite(rightDir, 1);
  } else {
    digitalWrite(leftDir, 1);
  }
}

static void Motor::setBackwards(bool side){
  // if side true - right
  if (side){
    digitalWrite(rightDir, 0);
  } else {
      digitalWrite(leftDir, 0);
  }
}

static void Motor::setSpeed(bool side, int desiredSpeed){
  Serial.println("Speed set");
  if (side){
    rightSpeed = desiredSpeed;
    rightTarget = (5000 - rightSpeed)*1;
  } else {
    leftSpeed = desiredSpeed;
    leftTarget = (5000 - leftSpeed)*1;
  }
}


static void Motor::init() {
  // put your setup code here, to run once:
  pinMode(leftDir, OUTPUT);
  pinMode(leftStep, OUTPUT);

  pinMode(rightDir, OUTPUT);
  pinMode(rightStep, OUTPUT);

  step_timer = timerBegin(0, 8, true);
  timerAttachInterrupt(step_timer, &onStepTimer, true);
  timerAlarmWrite(step_timer, 100, true);
  timerAlarmEnable(step_timer);
}
