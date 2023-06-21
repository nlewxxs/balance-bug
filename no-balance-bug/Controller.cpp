#include <Arduino.h>
#include <AccelStepper.h>
#include "Controller.h"

// Define motor interface type
#define motorInterfaceType 1
#define stepsPerRevolution 200

float positionSetpoint = 0;
float headingSetpoint = 0;

// Define pin connections
const int leftDirPin = 32; //A4
const int leftStepPin = 33; //A3
// Define pin connections
const int rightDirPin = 25; //A2
const int rightStepPin = 26; //A1
//4 is sda is orange, 3 is scl is yellow

const int leftMicro1 = 5; // 7 is blue - GPIO5
const int leftMicro2 = 18; // 6 is white - GPIO18
const int leftMicro3 = 19; // 5 is red - GPIO19
const int rightMicro1 = 15; // 12 is blue - GPIO15
const int rightMicro2 = 4; // 11 is white - GPIO4
const int rightMicro3 = 14; // 10 is red - GPIO14

// Creates an instance
AccelStepper leftStepper(motorInterfaceType, leftStepPin, leftDirPin);
AccelStepper rightStepper(motorInterfaceType, rightStepPin, rightDirPin);

// heading PID controller
float Kp_heading, Ki_heading, Kd_heading;

// position PID controller
float Kp_position, Ki_position, Kd_position;

bool microStepping = false;

int maxSpeed = 500;

double H_derivative, H_out, P_derivative, P_out;
float H_error[2], P_error[2] = { 0, 0 };           //stores current and previous value for derivative calculation
float H_integral[2], P_integral[2] = { 0, 0 };  //stores current and previous value for integral calculation
float iteration_time;
unsigned long oldMillis = 0;

double leftWheelDrive = 0;
double rightWheelDrive = 0;

void Controller::setup() {

  pinMode(leftMicro1, OUTPUT);
  pinMode(leftMicro2, OUTPUT);
  pinMode(leftMicro3, OUTPUT);
  pinMode(rightMicro1, OUTPUT);
  pinMode(rightMicro2, OUTPUT);
  pinMode(rightMicro3, OUTPUT);
  
  digitalWrite(leftMicro1, LOW);
  digitalWrite(leftMicro2, LOW);
  digitalWrite(leftMicro3, LOW);
  digitalWrite(rightMicro1, LOW);
  digitalWrite(rightMicro2, LOW);
  digitalWrite(rightMicro3, LOW);
  
	// leftStepper.setMaxSpeed(S2);
	// leftStepper.setAcceleration(acceleration);
	leftStepper.setMinPulseWidth(20);
	// rightStepper.setMaxSpeed(S2);
	// rightStepper.setAcceleration(acceleration);
	rightStepper.setMinPulseWidth(20);

  leftStepper.setMaxSpeed(maxSpeed);
  rightStepper.setMaxSpeed(maxSpeed);
}

void Controller::update(float heading) {
  float iteration_time = (millis() - oldMillis) / 1000.0;
  oldMillis = millis();

  microStepping = false;
  Kp_position = 100;
  Ki_position = 0;
  Kd_position = 0;
  Kp_heading = 10;
  Ki_heading = 0;
  Kd_heading = 0;

  // position control first
  float positionReading = getDistance();
  P_error[1] = positionSetpoint - positionReading;
  // P_integral[1] = P_integral[0] + P_error[1] * iteration_time;  // 1 is current, 0 is old
  // P_derivative = (P_error[1] - P_error[0]) / iteration_time;
  // P_out = Kp_position * P_error[1] + Ki_position * P_integral[1] + Kd_position * P_derivative;

  // P_integral[0] = P_integral[1];  // time shift integral readings after out calculated
  // P_error[0] = P_error[1];
  if (P_error[1] > 0.01) {
    P_out = 200;
  } else if (P_error[1] < -0.01) {
    P_out = -200;
  } else {
    P_out = 0;
  }

  // heading control to offset the wheels for rotation
  H_error[1] = headingSetpoint - heading;
  H_integral[1] = H_integral[0] + H_error[1] * iteration_time;  // 1 is current, 0 is old
  H_derivative = (H_error[1] - H_error[0]) / iteration_time;
  H_out = Kp_heading * H_error[1] + Ki_heading * H_integral[1] + Kd_heading * H_derivative;

  H_integral[0] = H_integral[1];  // time shift integral readings after out calculated
  H_error[0] = H_error[1];

  leftWheelDrive = -(P_out + H_out);  // voltage / pwm that will actually drive the wheels
  rightWheelDrive = (P_out - H_out);

  leftWheelDrive = constrain(leftWheelDrive, -maxSpeed, maxSpeed);
  rightWheelDrive = constrain(rightWheelDrive, -maxSpeed, maxSpeed);

  leftStepper.setSpeed(leftWheelDrive);
	leftStepper.runSpeed();
  rightStepper.setSpeed(rightWheelDrive);
	rightStepper.runSpeed();
}

bool Controller::getMoving() {
  if (P_out != 0) {
    return true;
  }

  if (abs(H_error[1]) < 0.01) {
    return true;
  }

  return false;
}

float Controller::getDistance() {
  float microStep = microStepping ? 4.0 : 1.0;
  float leftDistance = leftStepper.currentPosition() * M_PI * 0.175 / 200.0 / microStep;
  float rightDistance = rightStepper.currentPosition() * M_PI * 0.175 / 200.0 / microStep;

  float totalDistance = (rightDistance - leftDistance) / 2.0;

  return totalDistance;
}

void Controller::updatePositionSetpoint(float setpoint) {
  positionSetpoint = setpoint;
}

void Controller::updateHeadingSetpoint(float setpoint) {
  headingSetpoint = setpoint;
}

float Controller::getLeftOutput() {
  return leftWheelDrive;
}
float Controller::getRightOutput() {
  return rightWheelDrive;
}