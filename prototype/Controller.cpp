#include <Arduino.h>
#include <AccelStepper.h>
#include <Preferences.h>
#include "Controller.h"

Preferences preferences;

#define btTuning

// Define motor interface type
#define motorInterfaceType 1
#define stepsPerRevolution 200
#define HEADING_SETPOINT 0
#define POSITION_SETPOINT 0

// Define pin connections
const int leftDirPin = 26; //A1
const int leftStepPin = 25; //A2
// Define pin connections
const int rightDirPin = 33; //A3
const int rightStepPin = 32; //A4
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

// balance PID controller
float Kp_tilt, Ki_tilt, Kd_tilt;

float Pp1, Pp2, P1, P2, I1, I2, D1, D2, hahha, B2, B3, maxTiltOffset;
int S1, S2, S3;

bool correcting1 = false;
bool correcting2 = false;
bool microStepping = false;

// acceptable pid values -- 200,8,1,20 -- 160,25,0.02,30. Correcting: 4,4,-0.5

int maxSpeed = 1000;

float balanceCenter = 0;  // whatever tilt value is balanced
float P_bias, T_bias, Trate_bias, H_bias = 0;

double H_derivative, H_out, P_derivative, P_out, T_derivative, T_out, Trate_derivative, Trate_out;
float H_error[2], P_error[2], T_error[2], Trate_error[2] = { 0, 0 };           //stores current and previous value for derivative calculation
float H_integral[2], P_integral[2], T_integral[2], Trate_integral[2] = { 0, 0 };  //stores current and previous value for integral calculation
float iteration_time;
unsigned long oldMillis = 0;

double leftWheelDrive = 0;
double rightWheelDrive = 0;

unsigned long lastStabilityTime = 0;
float stabBuffer[3] = {720.0, 720.0, 720.0}; //impossible pitch readings

void Controller::setup() {
  preferences.begin("pidvalues", false);

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
  
  Pp1 = preferences.getFloat("Pp1", 0);
  Pp2 = preferences.getFloat("Pp2", 0);
  P1 = preferences.getFloat("P1", 0);
  P2 = preferences.getFloat("P2", 0);
  I1 = preferences.getFloat("I1", 0);
  I2 = preferences.getFloat("I2", 0);
  D1 = preferences.getFloat("D1", 0);
  D2 = preferences.getFloat("D2", 0);
  hahha = preferences.getFloat("B1", 0);
  B2 = preferences.getFloat("B2", 0);
  B3 = preferences.getFloat("B3", 0);
  S1 = preferences.getInt("S1", 0);
  S2 = preferences.getInt("S2", 0);
  S3 = preferences.getInt("S3", 0);
  maxTiltOffset = preferences.getFloat("MTO", 0);
  
	// leftStepper.setMaxSpeed(S2);
	// leftStepper.setAcceleration(acceleration);
	leftStepper.setMinPulseWidth(20);
	// rightStepper.setMaxSpeed(S2);
	// rightStepper.setAcceleration(acceleration);
	rightStepper.setMinPulseWidth(20);
}

void Controller::update(float pitch, float heading) {
  float iteration_time = (millis() - oldMillis) / 1000.0;
  oldMillis = millis();

  if (abs(pitch) <= hahha && !(correcting1 || correcting2)) {
    microStepping = true;
    // Kp_tilt = 200;
    // Ki_tilt = 8;
    // Kd_tilt = 1;
    // Kp_position = 20;
    // Ki_position = 0;
    // Kd_position = 0;
    // maxSpeed = 1000;
    Kp_tilt = P1;
    Ki_tilt = I1;
    Kd_tilt = D1;
    Kp_position = Pp1;
    Ki_position = 0;
    Kd_position = 0;
    maxSpeed = S1;
	  leftStepper.setMaxSpeed(maxSpeed);
	  rightStepper.setMaxSpeed(maxSpeed);
    digitalWrite(leftMicro1, LOW);
    digitalWrite(leftMicro2, HIGH);
    digitalWrite(leftMicro3, LOW);
    digitalWrite(rightMicro1, LOW);
    digitalWrite(rightMicro2, HIGH);
    digitalWrite(rightMicro3, LOW);
  } else {
    microStepping = false;
    // Kp_tilt = 160;
    // Ki_tilt = 25;
    // Kd_tilt = 0.02;
    // Kp_position = 30;
    // Ki_position = 0;
    // Kd_position = 0;
    // maxSpeed = 500;
    Kp_tilt = P2;
    Ki_tilt = I2;
    Kd_tilt = D2;
    Kp_position = Pp2;
    Ki_position = 0;
    Kd_position = 0;
    maxSpeed = S2;
	  leftStepper.setMaxSpeed(maxSpeed);
	  rightStepper.setMaxSpeed(maxSpeed);
    digitalWrite(leftMicro1, LOW);
    digitalWrite(leftMicro2, LOW);
    digitalWrite(leftMicro3, LOW);
    digitalWrite(rightMicro1, LOW);
    digitalWrite(rightMicro2, LOW);
    digitalWrite(rightMicro3, LOW);
  }

  // position control first
  float positionReading = getDistance();
  P_error[1] = POSITION_SETPOINT - positionReading;
  P_integral[1] = P_integral[0] + P_error[1] * iteration_time;  // 1 is current, 0 is old
  P_derivative = (P_error[1] - P_error[0]) / iteration_time;
  P_out = Kp_position * P_error[1] + Ki_position * P_integral[1] + Kd_position * P_derivative + P_bias;

  P_integral[0] = P_integral[1];  // time shift integral readings after out calculated
  P_error[0] = P_error[1];

  P_out = constrain(P_out, -maxTiltOffset, maxTiltOffset); // so as to not overcompensate and make us fall over

  // balance control
  T_error[1] = balanceCenter + P_out - pitch;             // offset the tilt reading with the distance output to allow us to manipulate the position of the robot via tilt
  T_integral[1] = T_integral[0] + T_error[1] * iteration_time;  // 1 is current, 0 is old
  T_derivative = (T_error[1] - T_error[0]) / iteration_time;
  T_out = Kp_tilt * T_error[1] + Ki_tilt * T_integral[1] + Kd_tilt * T_derivative + T_bias;

  T_integral[0] = T_integral[1];  // time shift integral readings after out calculated
  T_error[0] = T_error[1];

  // heading control to offset the wheels for rotation
  H_error[1] = HEADING_SETPOINT - heading;
  H_integral[1] = H_integral[0] + H_error[1] * iteration_time;  // 1 is current, 0 is old
  H_derivative = (H_error[1] - H_error[0]) / iteration_time;
  H_out = Kp_heading * H_error[1] + Ki_heading * H_integral[1] + Kd_heading * H_derivative + H_bias;

  H_integral[0] = H_integral[1];  // time shift integral readings after out calculated
  H_error[0] = H_error[1];

  leftWheelDrive = T_out + H_out;  // voltage / pwm that will actually drive the wheels
  rightWheelDrive = -(T_out - H_out);

  leftWheelDrive = constrain(leftWheelDrive, -maxSpeed, maxSpeed);
  rightWheelDrive = constrain(rightWheelDrive, -maxSpeed, maxSpeed);

  if (pitch > B2 || correcting1) {
    leftWheelDrive = -S3;
    rightWheelDrive = S3;
    correcting1 = true;
  } else if (pitch < -B2 || correcting2) {
    leftWheelDrive = S3;
    rightWheelDrive = -S3;
    correcting2 = true;
  }

  // if ((correcting1 || correcting2) && (lastStabilityTime - millis() > 333)) {
  //   stabBuffer[0] = stabBuffer[1];
  //   stabBuffer[1] = stabBuffer[2];
  //   stabBuffer[2] = T_error[1];
  //   if (abs(stabBuffer[0]) < 1.0 && abs(stabBuffer[1]) < 1.0 && abs(stabBuffer[2]) < 1.0) {
  //     correcting1 = correcting2 = false;
  //     stabBuffer[0] = stabBuffer[1] = stabBuffer[2] = 720; // impossible pitch reading
  //   }
  //   lastStabilityTime = millis();
  // }

  if (correcting1 && pitch < hahha) {
    correcting1 = false;
  } else if (correcting2 && pitch > hahha) {
    correcting2 = false;
  }

  leftStepper.setSpeed(leftWheelDrive);
	leftStepper.runSpeed();
  rightStepper.setSpeed(rightWheelDrive);
	rightStepper.runSpeed();
}

// void Controller::updateController() {
//   // offset speed out - acceleration control
// }

float Controller::getDistance() {
  float microStep = microStepping ? 8.0 : 1.0;
  float leftDistance = leftStepper.currentPosition() * M_PI * 0.175 / 200.0 / microStep;
  float rightDistance = rightStepper.currentPosition() * M_PI * 0.175 / 200.0 / microStep;

  float totalDistance = (rightDistance - leftDistance) / 2.0;

  // POSITION_SETPOINT = totalDistance + 0.1;

  return totalDistance;
}

float Controller::getLeftOutput() {
  return leftWheelDrive;
}
float Controller::getRightOutput() {
  return rightWheelDrive;
}

void Controller::updateValues(String param, float val) {
  if (param == "Pp1") {
    Pp1 = val;
    preferences.putFloat("Pp1", Pp1);
  } else if (param == "Pp2") {
    Pp2 = val;
    preferences.putFloat("Pp2", Pp2);
  } else if (param == "P1") {
    P1 = val;
    preferences.putFloat("P1", P1);
  } else if (param == "P2") {
    P2 = val;
    preferences.putFloat("P2", P2);
  } else if (param == "I1") {
    I1 = val;
    preferences.putFloat("I1", I1);
  } else if (param == "I2") {
    I2 = val;
    preferences.putFloat("I2", I2);
  } else if (param == "D1") {
    D1 = val;
    preferences.putFloat("D1", D1);
  } else if (param == "D2") {
    D2 = val;
    preferences.putFloat("D2", D2);
  } else if (param == "B1") {
    hahha = val;
    preferences.putFloat("B1", hahha);
  } else if (param == "B2") {
    B2 = val;
    preferences.putFloat("B2", B2);
  } else if (param == "B3") {
    B3 = val;
    preferences.putFloat("B3", B3);
  } else if (param == "MTO") {
    maxTiltOffset = val;
    preferences.putFloat("MTO", maxTiltOffset);
  }
}

void Controller::updateValues(String param, int val) {
  if (param == "S1") {
    S1 = val;
    preferences.putInt("S1", S1);
  } else if (param == "S2") {
    S2 = val;
    preferences.putInt("S2", S2);
  } else if (param == "S3") {
    S3 = val;
    preferences.putInt("S3", S3);
  }
}

float Controller::getValue(String param) {
  if (param == "Pp1") {
    return preferences.getFloat("Pp1", Pp1);
  } else if (param == "Pp2") {
    return preferences.getFloat("Pp2", Pp2);
  } else if (param == "P1") {
    return preferences.getFloat("P1", P1);
  } else if (param == "P2") {
    return preferences.getFloat("P2", P2);
  } else if (param == "I1") {
    return preferences.getFloat("I1", I1);
  } else if (param == "I2") {
    return preferences.getFloat("I2", I2);
  } else if (param == "D1") {
    return preferences.getFloat("D1", D1);
  } else if (param == "D2") {
    return preferences.getFloat("D2", D2);
  } else if (param == "B1") {
    return preferences.getFloat("B1", hahha);
  } else if (param == "B2") {
    return preferences.getFloat("B2", B2);
  } else if (param == "B3") {
    return preferences.getFloat("B3", B3);
  } else if (param == "S1") {
    return preferences.getInt("S1", S1);
  } else if (param == "S2") {
    return preferences.getInt("S2", S2);
  } else if (param == "S3") {
    return preferences.getInt("S3", S3);
  } else if (param == "MTO") {
    return preferences.getInt("MTO", maxTiltOffset);
  }
}























