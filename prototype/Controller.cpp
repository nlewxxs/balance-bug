#include <Arduino.h>
#include <AccelStepper.h>
// #include "Preferences.h"
#include "Controller.h"

// Preferences preferences;

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
float Kp_heading = 0;
float Ki_heading = 0;
float Kd_heading = 0;

// position PID controller
float Kp_position = 40;
float Ki_position = 0;
float Kd_position = 0;

// balance PID controller
float Kp_tilt = 75;
float Ki_tilt = 0;
float Kd_tilt = 0;

bool correcting = false;
bool microStepping = false;

// acceptable pid values -- 200,8,1,20 -- 160,25,0.02,30. Correcting: 4,4,-0.5

int maxSpeed = 1000;

float balanceCenter = 0;  // whatever tilt value is balanced
float P_bias, T_bias, Trate_bias, H_bias = 0;

float H_derivative, H_out, P_derivative, P_out, T_derivative, T_out, Trate_derivative, Trate_out;
float H_error[2], P_error[2], T_error[2], Trate_error[2] = { 0, 0 };           //stores current and previous value for derivative calculation
float H_integral[2], P_integral[2], T_integral[2], Trate_integral[2] = { 0, 0 };  //stores current and previous value for integral calculation
float iteration_time;
unsigned long oldMillis = 0;

float leftWheelDrive;
float rightWheelDrive;

unsigned long lastStabilityTime = 0;
float stabBuffer[3] = {720.0, 720.0, 720.0}; //impossible pitch readings

void Controller::setup() {
  // preferences.begin("pidvalues", false);

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
  
	leftStepper.setMaxSpeed(maxSpeed);
	// leftStepper.setAcceleration(acceleration);
	leftStepper.setMinPulseWidth(20);
	rightStepper.setMaxSpeed(maxSpeed);
	// rightStepper.setAcceleration(acceleration);
	rightStepper.setMinPulseWidth(20);
}

void Controller::update(float pitch, float heading) {
  float iteration_time = (millis() - oldMillis) / 1000.0;
  oldMillis = millis();

  if (abs(pitch) < 2 && !correcting) {
    microStepping = true;
    Kp_tilt = 200;
    Ki_tilt = 8;
    Kd_tilt = 1;
    Kp_position = 20;
    Ki_position = 0;
    Kd_position = 0;
    maxSpeed = 1000;
    digitalWrite(leftMicro1, HIGH);
    digitalWrite(leftMicro2, HIGH);
    digitalWrite(leftMicro3, LOW);
    digitalWrite(rightMicro1, HIGH);
    digitalWrite(rightMicro2, HIGH);
    digitalWrite(rightMicro3, LOW);
  } else {
    microStepping = false;
    Kp_tilt = 160;
    Ki_tilt = 25;
    Kd_tilt = 0.02;
    Kp_position = 30;
    Ki_position = 0;
    Kd_position = 0;
    maxSpeed = 500;
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

  P_out = constrain(P_out, -3, 3); // so as to not overcompensate and make us fall over

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
  rightWheelDrive = T_out - H_out;

  leftWheelDrive = constrain(leftWheelDrive, -maxSpeed, maxSpeed);
  rightWheelDrive = -constrain(rightWheelDrive, -maxSpeed, maxSpeed);

  if (pitch > 4) {
    correcting = true;
  } else if (pitch < -4) {
    correcting = true;
  }

  if (correcting && (lastStabilityTime - millis() > 666)) {
    stabBuffer[0] = stabBuffer[1];
    stabBuffer[1] = stabBuffer[2];
    stabBuffer[2] = T_error[1];
    if (abs(stabBuffer[0]) < 1.0 && abs(stabBuffer[1]) < 1.0 && abs(stabBuffer[2]) < 1.0) {
      correcting = false;
      stabBuffer[0] = stabBuffer[1] = stabBuffer[2] = 720; // impossible pitch reading
    }
    lastStabilityTime = millis();
  }

  leftStepper.setSpeed(leftWheelDrive);
	leftStepper.runSpeed();
  rightStepper.setSpeed(rightWheelDrive);
	rightStepper.runSpeed();
}

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

// void Controller::updateValues() {
//   String test = SerialBT.readString();
//   if (test.substring(0,3) == "Pp1") {
//     preferences.putFloat("Pp1", test.substring(3,test.length()-1).toFloat());
//     // Kp_position = preferences.getInt("Pp", 0);
//     SerialBT.print("Set Pp1 to ");
//     SerialBT.println(preferences.getFloat("Pp1", 0));
//   } else if (test.substring(0,3) == "Ii1") {
//     preferences.putFloat("Ii1", test.substring(3,test.length()-1).toFloat());
//     // Ki_position = preferences.getInt("Ii", 0);
//     SerialBT.print("Set Ii1 to ");
//     SerialBT.println(preferences.getFloat("Ii1", 0));
//   } else if (test.substring(0,3) == "Dd1") {
//     preferences.putFloat("Dd1", test.substring(3,test.length()-1).toFloat());
//     // Kd_position = preferences.getInt("Dd", 0);
//     SerialBT.print("Set Dd1 to ");
//     SerialBT.println(preferences.getFloat("Dd1", 0));
//   } else if (test.substring(0,3) == "Pp2") {
//     preferences.putFloat("Pp2", test.substring(3,test.length()-1).toFloat());
//     // Kp_position = preferences.getInt("Pp", 0);
//     SerialBT.print("Set Pp2 to ");
//     SerialBT.println(preferences.getFloat("Pp2", 0));
//   } else if (test.substring(0,3) == "Ii2") {
//     preferences.putFloat("Ii2", test.substring(3,test.length()-1).toFloat());
//     // Ki_position = preferences.getInt("Ii", 0);
//     SerialBT.print("Set Ii2 to ");
//     SerialBT.println(preferences.getFloat("Ii2", 0));
//   } else if (test.substring(0,3) == "Dd2") {
//     preferences.putFloat("Dd2", test.substring(3,test.length()-1).toFloat());
//     // Kd_position = preferences.getInt("Dd", 0);
//     SerialBT.print("Set Dd2 to ");
//     SerialBT.println(preferences.getFloat("Dd2", 0));
//   } else if (test.substring(0,2) == "P1") {
//     preferences.putFloat("P1", test.substring(2,test.length()-1).toFloat());
//     // Kp_tilt = preferences.getFloat("P", 0);
//     SerialBT.print("Set P1 to ");
//     SerialBT.println(preferences.getFloat("P1", 0), 4);
//   } else if (test.substring(0,2) == "I1") {
//     preferences.putFloat("I1", test.substring(2,test.length()-1).toFloat());
//     // Kp_tilt = preferences.getFloat("P", 0);
//     SerialBT.print("Set I1 to ");
//     SerialBT.println(preferences.getFloat("I1", 0), 4);
//   } else if (test.substring(0,2) == "D1") {
//     preferences.putFloat("D1", test.substring(2,test.length()-1).toFloat());
//     // Kp_tilt = preferences.getFloat("P", 0);
//     SerialBT.print("Set D1 to ");
//     SerialBT.println(preferences.getFloat("D1", 0), 4);
//   } else if (test.substring(0,2) == "P2") {
//     preferences.putFloat("P2", test.substring(2,test.length()-1).toFloat());
//     // Kp_tilt = preferences.getFloat("P", 0);
//     SerialBT.print("Set P2 to ");
//     SerialBT.println(preferences.getFloat("P2", 0), 4);
//   } else if (test.substring(0,2) == "I2") {
//     preferences.putFloat("I2", test.substring(2,test.length()-1).toFloat());
//     // Kp_tilt = preferences.getFloat("P", 0);
//     SerialBT.print("Set I2 to ");
//     SerialBT.println(preferences.getFloat("I2", 0), 4);
//   } else if (test.substring(0,2) == "D2") {
//     preferences.putFloat("D2", test.substring(2,test.length()-1).toFloat());
//     // Kp_tilt = preferences.getFloat("P", 0);
//     SerialBT.print("Set D2 to ");
//     SerialBT.println(preferences.getFloat("D2", 0), 4);
//   } else if (test.substring(0,2) == "S1") {
//     preferences.putInt("S1", test.substring(2,test.length()-1).toInt());
//     // maxSpeed1 = preferences.getInt("S1", 0);
//     SerialBT.print("Set S1 to ");
//     SerialBT.println(preferences.getInt("S1", 0));
//   } else if (test.substring(0,2) == "S2") {
//     preferences.putInt("S2", test.substring(2,test.length()-1).toInt());
//     // maxSpeed2 = preferences.getInt("S2", 0);
//     SerialBT.print("Set S2 to ");
//     SerialBT.println(preferences.getInt("S2", 0));
//   } else if (test.substring(0,2) == "S3") {
//     preferences.putInt("S3", test.substring(2,test.length()-1).toInt());
//     // maxSpeed2 = preferences.getInt("S2", 0);
//     SerialBT.print("Set S3 to ");
//     SerialBT.println(preferences.getInt("S3", 0));
//   } else if (test.substring(0,3) == "PR1") {
//     preferences.putFloat("PR1", test.substring(3,test.length()-1).toFloat());
//     // maxSpeed2 = preferences.getInt("S2", 0);
//     SerialBT.print("Set PR1 to ");
//     SerialBT.println(preferences.getFloat("PR1", 0));
//   } else if (test.substring(0,3) == "IR1") {
//     preferences.putFloat("IR1", test.substring(3,test.length()-1).toFloat());
//     // maxSpeed2 = preferences.getInt("S2", 0);
//     SerialBT.print("Set IR1 to ");
//     SerialBT.println(preferences.getFloat("IR1", 0));
//   } else if (test.substring(0,3) == "DR1") {
//     preferences.putFloat("DR1", test.substring(3,test.length()-1).toFloat());
//     // maxSpeed2 = preferences.getInt("S2", 0);
//     SerialBT.print("Set DR1 to ");
//     SerialBT.println(preferences.getFloat("DR1", 0));
//   } else if (test.substring(0,3) == "PR2") {
//     preferences.putFloat("PR2", test.substring(3,test.length()-1).toFloat());
//     // maxSpeed2 = preferences.getInt("S2", 0);
//     SerialBT.print("Set PR2 to ");
//     SerialBT.println(preferences.getFloat("PR2", 0));
//   } else if (test.substring(0,3) == "IR2") {
//     preferences.putFloat("IR2", test.substring(3,test.length()-1).toFloat());
//     // maxSpeed2 = preferences.getInt("S2", 0);
//     SerialBT.print("Set IR2 to ");
//     SerialBT.println(preferences.getFloat("IR2", 0));
//   } else if (test.substring(0,3) == "DR2") {
//     preferences.putFloat("DR2", test.substring(3,test.length()-1).toFloat());
//     // maxSpeed2 = preferences.getInt("DR2", 0);
//     SerialBT.print("Set DR2 to ");
//     SerialBT.println(preferences.getFloat("DR2", 0));
//   } else if (test.substring(0,2) == "B1") {
//     preferences.putFloat("B1", test.substring(2,test.length()-1).toFloat());
//     // maxSpeed2 = preferences.getInt("S2", 0);
//     SerialBT.print("Set B1 to ");
//     SerialBT.println(preferences.getFloat("B1", 0));
//   } else if (test.substring(0,2) == "B2") {
//     preferences.putFloat("B2", test.substring(2,test.length()-1).toFloat());
//     // maxSpeed2 = preferences.getInt("S2", 0);
//     SerialBT.print("Set B2 to ");
//     SerialBT.println(preferences.getFloat("B2", 0));
//   } else if (test.substring(0,2) == "B3") {
//     preferences.putFloat("B3", test.substring(2,test.length()-1).toFloat());
//     // maxSpeed2 = preferences.getInt("S2", 0);
//     SerialBT.print("Set B3 to ");
//     SerialBT.println(preferences.getFloat("B3", 0));
//   } else if (test.substring(0,2) == "SM") {
//     preferences.putInt("SM", test.substring(2,test.length()-1).toInt());
//     // maxSpeed2 = preferences.getInt("S2", 0);
//     SerialBT.print("Set SM to ");
//     SerialBT.println(preferences.getInt("SM", 0));
//   } else if (test[0] == 'C') {
//     SerialBT.print("P1: ");
//     SerialBT.print(preferences.getFloat("P1", 0), 4);
//     SerialBT.print(" I1: ");
//     SerialBT.print(preferences.getFloat("I1", 0), 4);
//     SerialBT.print(" D1: ");
//     SerialBT.print(preferences.getFloat("D1", 0), 4);
//     SerialBT.print(" P2: ");
//     SerialBT.print(preferences.getFloat("P2", 0), 4);
//     SerialBT.print(" I2: ");
//     SerialBT.print(preferences.getFloat("I2", 0), 4);
//     SerialBT.print(" D2: ");
//     SerialBT.print(preferences.getFloat("D2", 0), 4);
//     SerialBT.print(" Pp1: ");
//     SerialBT.print(preferences.getFloat("Pp1", 0), 4);
//     SerialBT.print(" Ii1: ");
//     SerialBT.print(preferences.getFloat("Ii1", 0), 4);
//     SerialBT.print(" Dd1: ");
//     SerialBT.print(preferences.getFloat("Dd1", 0), 4);
//     SerialBT.print(" Pp2: ");
//     SerialBT.print(preferences.getFloat("Pp2", 0), 4);
//     SerialBT.print(" Ii2: ");
//     SerialBT.print(preferences.getFloat("Ii2", 0), 4);
//     SerialBT.print(" Dd2: ");
//     SerialBT.print(preferences.getFloat("Dd2", 0), 4);
//     SerialBT.print(" maxSpeed1: ");
//     SerialBT.print(preferences.getInt("S1", 0));
//     SerialBT.print(" maxSpeed2: ");
//     SerialBT.print(preferences.getInt("S2", 0));
//     SerialBT.print(" correct speed: ");
//     SerialBT.print(preferences.getInt("S3", 0));
//     SerialBT.print(" B1: ");
//     SerialBT.print(preferences.getFloat("B1", 0), 4);
//     SerialBT.print(" B2: ");
//     SerialBT.print(preferences.getFloat("B2", 0), 4);
//     SerialBT.print(" B3: ");
//     SerialBT.print(preferences.getFloat("B3", 0), 4);
//   }
// }

























