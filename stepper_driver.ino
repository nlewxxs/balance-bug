// stepper motor code
// based on hardcoded PWM using timers and interrupts for stepping

#define left 1
#define right 0

int dirLeft = 33;
int stepLeft = 32;
int dirRight = 26;
int stepRight = 25;

// speeds vary between 1 and 5000
// speed 10000 corresponds to 200 microsecond between steps
// so the time between pulses is (5000 - speed) * 200 micro seconds
int leftSpeed = 1;
int rightSpeed = 1;

int leftCounter = 0;
int rightCounter = 0;

long leftTarget;
long rightTarget;
// timer and ISR for stepper control
hw_timer_t *step_timer = NULL;

void IRAM_ATTR onStepTimer(){
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

void setForward(bool side){
  // if side true - left
  if (side){
    digitalWrite(dirLeft, 1);
  } else {
    digitalWrite(dirRight, 1);
  }
}

void setBackwards(bool side){
  // if side true - left
  if (side){
    digitalWrite(dirLeft, 0);
  } else {
    digitalWrite(dirRight, 0);
  }
}

void setSpeed(bool side, int desiredSpeed){
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


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(dirLeft, OUTPUT);
  pinMode(dirRight, OUTPUT);
  pinMode(stepLeft, OUTPUT);
  pinMode(stepRight, OUTPUT);
  step_timer = timerBegin(0, 8, true);
  timerAttachInterrupt(step_timer, &onStepTimer, true);
  timerAlarmWrite(step_timer, 100, true);
  timerAlarmEnable(step_timer);
}

void loop() {
  // put your main code here, to run repeatedly:
  // Serial.println("5000");
  // setForward(left);
  // setForward(right);
  // setSpeed(left, 5000);
  // setSpeed(right, 5000);
  // vTaskDelay(5000);
  // Serial.println("10000");
  // setSpeed(left, 10000);
  // setSpeed(right, 10000);
  // vTaskDelay(5000);
  // Serial.println("10");
  // setSpeed(left, 10);
  // setSpeed(right, 10);
  // vTaskDelay(5000);

  setForward(left);
  setForward(right);
  for (int speed = 4000; speed < 5000; speed = speed + 10){
    Serial.print("Speed: ");
    Serial.println(speed);
    setSpeed(left, speed);
    setSpeed(right, speed);
    vTaskDelay(2000);
  }

  for (int speed = 5000; speed > 4000; speed = speed - 10){
    Serial.print("Speed: ");
    Serial.println(speed);
    setSpeed(left, speed);
    setSpeed(right, speed);
    vTaskDelay(2000);
  }
}
