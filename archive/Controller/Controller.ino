// #include <ArduinoEigenDense.h>
// using namespace Eigen;
#include <Wire.h>
#include "mpu6050.h"

// heading PD controller
double Kp_heading = 0.8;
double Ki_heading = 0; 
double Kd_heading = 0.2;

// position PD controller
double Kp_position = 0.0005;
double Ki_position = 0; 
double Kd_position = 0.75;

// balance PID controller
double Kp_tilt = 5; //175
double Ki_tilt = 0.75; //5
double Kd_tilt = 1; //8.5
double balanceCenter = 0; // whatever tilt value is balanced
double P_bias, T_bias, H_bias = 0;

double H_derivative, H_out, P_derivative, P_out, T_derivative, T_out;
double H_error[2], P_error[2], T_error[2] = {0, 0}; //stores current and previous value for derivative calculation
double H_integral[2], P_integral[2], T_integral[2] = {0, 0}; //stores current and previous value for integral calculation
double iteration_time;
unsigned long oldMillis = 0;

// sensor definitions
const int mpu_addy = 0x68;
// double AccX, AccY, AccZ;
// double GyroX, GyroY, GyroZ;  
// double accAngleX, accAngleY, gyroAngleX, gyroAngleY, gyroAngleZ;
// double roll, pitch, yaw;
// double AccErrorX, AccErrorY, GyroErrorX, GyroErrorY, GyroErrorZ;
// double elapsedTime, currentTime, previousTime;
// int calibrate = 0;  // calibration counter

mpu6050 mpu = mpu6050();

void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  // how to define matrices using eigen for when we do LQR
  // MatrixXf m1(2,4);
  // m1 << 1, 2, 3, 4, 5, 6, 7, 8;
  // MatrixXf m2(4,3);
  // m2 << 9, 3, 2, 6, 4, 2, 6, 7, 3, 1, 4, 2;
  // MatrixXf outm = m1 * m2;

  // Wire.begin();                      
  // Wire.beginTransmission(mpu_addy);  
  // Wire.write(0x6B);                  // Talk to the register 6B
  // Wire.write(0x00);                  // placing 0 in the PWR_MGMT_1 register (wakes mpu)
  // Wire.endTransmission(true);        //end the transmission
  mpu.init();

  mpu.calibrate();
  delay(20);
}

void loop() {
  mpu.update();

  // unsigned long iteration_time_temp = (millis() - oldMillis);
  // iteration_time = iteration_time_temp / 1000;
  double iteration_time = (millis() - oldMillis) / 1000.0;

  
  // Serial.print("Old millis: ");
  // Serial.print(oldMillis);


  oldMillis = millis();
  
  // Serial.print(" Current millis: ");
  // Serial.print(oldMillis);

  // position control first
  double positionReading = getPosition(); // example I used for the controller has position read from rotary encoders - this value is total distance travelled
  P_error[1] = getPosSetpoint() - positionReading;
  P_integral[1] = P_integral[0] + P_error[1] * iteration_time; // 1 is current, 0 is old
  P_derivative = (P_error[1] - P_error[0]) / iteration_time;
  P_out = Kp_position*P_error[1] + Ki_position*P_integral[1] + Kd_position*P_derivative + P_bias;
  // make this output positive to move forwards, negative to move backwards

  P_integral[0] = P_integral[1]; // time shift integral readings after out calculated
  P_error[0] = P_error[1];

  // balance control
  double tiltReading = mpu.getPitch();
  // double tiltReading = 1;
  T_error[1] = balanceCenter + P_out - tiltReading; // offset the tilt reading with the distance output to allow us to manipulate the position of the robot via tilt
  T_integral[1] = T_integral[0] + T_error[1] * iteration_time; // 1 is current, 0 is old
  T_derivative = (T_error[1] - T_error[0]) / iteration_time;
  T_out = Kp_tilt*T_error[1] + Ki_tilt*T_integral[1] + Kd_tilt*T_derivative + T_bias;

  // Serial.print(" Tilt: ");
  // Serial.print(tiltReading);
  // Serial.print(" Tilt output: ");
  // Serial.print(T_out);
  // Serial.print(" Time: ");
  // Serial.println(iteration_time);


  T_integral[0] = T_integral[1]; // time shift integral readings after out calculated
  T_error[0] = T_error[1];

  // heading control to offset the wheels for rotation
  double headingReading = mpu.getYaw();
  H_error[1] = getHeadingSetpoint() - headingReading;
  H_integral[1] = H_integral[0] + H_error[1] * iteration_time; // 1 is current, 0 is old
  H_derivative = (H_error[1] - H_error[0]) / iteration_time;
  H_out = Kp_heading*H_error[1] + Ki_heading*H_integral[1] + Kd_heading*H_derivative + H_bias;

  H_integral[0] = H_integral[1]; // time shift integral readings after out calculated
  H_error[0] = H_error[1];


  double leftWheelDrive = T_out - H_out; // voltage / pwm that will actually drive the wheels
  double rightWheelDrive = T_out + H_out;

  // Serial.print("Currently at:  Position: ");
  // Serial.print(positionReading);
  // Serial.print(" Tilt: ");
  // Serial.print(tiltReading);
  // Serial.print(" Heading: ");
  // Serial.print(headingReading);
  
  // Serial.print("\tControl outputs  Position: ");
  // Serial.print(P_out);
  // Serial.print("| Tilt: ");
  // Serial.print(T_out);
  // Serial.print("| Heading: ");
  // Serial.print(H_out);

  Serial.print("\tWheel inputs: ");
  Serial.print(leftWheelDrive);
  Serial.print(" / ");
  Serial.println(rightWheelDrive);

}

double getPosition() { //unsure of how this reading will work - needs to be 1D (as in just x)
  // could potentially have position just be a relative thing i.e. move forwards 1 / backwards 1 rather than move to position 23?
  return 0;
}

double getPosSetpoint() {
  // output from the imaging program to tell us where to be
  return 0;
}

double getHeadingSetpoint() {
  // output from the imaging program to tell us where to face
  return 0;
}


