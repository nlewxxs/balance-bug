// #include <ArduinoEigenDense.h>
// using namespace Eigen;
#include <Wire.h>
#include <MPU6050.h>

// heading PD controller
double Kp_heading = 2.0;
double Ki_heading = 0; 
double Kd_heading = 0.75;

// position PD controller
double Kp_position = 0.0005;
double Ki_position = 0; 
double Kd_position = 0.75;

// balance PID controller
double Kp_tilt = 175;
double Ki_tilt = 5.0;
double Kd_tilt = 8.5;
double balanceCenter = 90; // whatever tilt value is balanced
double P_bias, T_bias, H_bias = 0;

double H_derivative, H_out, P_derivative, P_out, T_derivative, T_out;
double H_error[2], P_error[2], T_error[2] = {0, 0}; //stores current and previous value for derivative calculation
double H_integral[2], P_integral[2], T_integral[2] = {0, 0}; //stores current and previous value for integral calculation
int iteration_time;
int oldMillis = 0;

// sensor definitions
const int mpu_addy = 0x68;
double AccX, AccY, AccZ;
double GyroX, GyroY, GyroZ;  
double accAngleX, accAngleY, gyroAngleX, gyroAngleY, gyroAngleZ;
double roll, pitch, yaw;
double AccErrorX, AccErrorY, GyroErrorX, GyroErrorY, GyroErrorZ;
double elapsedTime, currentTime, previousTime;
int calibrate = 0;  // calibration counter


void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  // how to define matrices using eigen for when we do LQR
  // MatrixXf m1(2,4);
  // m1 << 1, 2, 3, 4, 5, 6, 7, 8;
  // MatrixXf m2(4,3);
  // m2 << 9, 3, 2, 6, 4, 2, 6, 7, 3, 1, 4, 2;
  // MatrixXf outm = m1 * m2;

  Wire.begin();                      
  Wire.beginTransmission(mpu_addy);  
  Wire.write(0x6B);                  // Talk to the register 6B
  Wire.write(0x00);                  // placing 0 in the PWR_MGMT_1 register (wakes mpu)
  Wire.endTransmission(true);        //end the transmission

  calculate_IMU_error();
  delay(20);
}

void loop() {

  iteration_time = millis() - oldMillis;

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
  double tiltReading = getTiltReading();
  T_error[1] = balanceCenter + P_out - tiltReading; // offset the tilt reading with the distance output to allow us to manipulate the position of the robot via tilt
  T_integral[1] = T_integral[0] + T_error[1] * iteration_time; // 1 is current, 0 is old
  T_derivative = (T_error[1] - T_error[0]) / iteration_time;
  T_out = Kp_position*T_error[1] + Ki_position*T_integral[1] + Kd_position*T_derivative + T_bias;

  T_integral[0] = T_integral[1]; // time shift integral readings after out calculated
  T_error[0] = T_error[1];

  // heading control to offset the wheels for rotation
  double headingReading = getHeading();
  H_error[1] = getHeadingSetpoint() - headingReading;
  H_integral[1] = H_integral[0] + H_error[1] * iteration_time; // 1 is current, 0 is old
  H_derivative = (H_error[1] - H_error[0]) / iteration_time;
  H_out = Kp_heading*H_error[1] + Ki_heading*H_integral[1] + Kd_heading*H_derivative + H_bias;

  H_integral[0] = H_integral[1]; // time shift integral readings after out calculated
  H_error[0] = H_error[1];


  double leftWheelDrive = T_out - H_out; // voltage / pwm that will actually drive the wheels
  double rightWheelDrive = T_out + H_out;

  Serial.print("Currently at\nPosition: ");
  Serial.print(positionReading);
  Serial.print("| Tilt: ");
  Serial.print(tiltReading);
  Serial.print("| Heading: ");
  Serial.println(headingReading);
  Serial.print("Control outputs\nPosition: ");
  Serial.print(P_out);
  Serial.print("| Tilt: ");
  Serial.print(T_out);
  Serial.print("| Heading: ");
  Serial.println(H_out);

  Serial.print("Wheel inputs: ");
  Serial.print(leftWheelDrive);
  Serial.print(" / ");
  Serial.println(rightWheelDrive);

}

// double getTilt() {
//   // tilt of inverted pendulum
//   return 95.0;
// }

double getPosition() { //unsure of how this reading will work - needs to be 1D (as in just x)
  // could potentially have position just be a relative thing i.e. move forwards 1 / backwards 1 rather than move to position 23?
  return 0;
}

double getPosSetpoint() {
  // output from the imaging program to tell us where to be
  return 0;
}

double getHeading() { // probably range from 0->2pi
  return yaw;
}

double getHeadingSetpoint() {
  // output from the imaging program to tell us where to face
  return 0;
}

double getTiltReading() {
  // reading accelerometer data
  Wire.beginTransmission(mpu_addy);
  Wire.write(0x3B);                     
  Wire.endTransmission(false);          
  Wire.requestFrom(mpu_addy, 6, true);

  // normalising
  // for a range of +-2g, we need to divide the raw values by 16384, according to the datasheet
  AccX = (Wire.read() << 8 | Wire.read()) / 16384.0; // X-axis value
  AccY = (Wire.read() << 8 | Wire.read()) / 16384.0; // Y-axis value
  AccZ = (Wire.read() << 8 | Wire.read()) / 16384.0; // Z-axis value

  // Calculating Roll and Pitch from the accelerometer data
  accAngleX = (atan(AccY / sqrt(pow(AccX, 2) + pow(AccZ, 2))) * 180 / PI) - AccErrorX;
  accAngleY = (atan(-1 * AccX / sqrt(pow(AccY, 2) + pow(AccZ, 2))) * 180 / PI) - AccErrorY;

  // reading gyro data 
  previousTime = currentTime;                         
  currentTime = millis();                             
  elapsedTime = (currentTime - previousTime) / 1000;  

  Wire.beginTransmission(mpu_addy);
  Wire.write(0x43); 
  Wire.endTransmission(false);
  Wire.requestFrom(mpu_addy, 6, true); 

  // For a 250deg/s range we have to divide first the raw value by 131.0, according to the datasheet
  GyroX = (Wire.read() << 8 | Wire.read()) / 131.0; 
  GyroY = (Wire.read() << 8 | Wire.read()) / 131.0;
  GyroZ = (Wire.read() << 8 | Wire.read()) / 131.0;

  // Correct the outputs with the calculated error values
  GyroX = GyroX - GyroErrorX; 
  GyroY = GyroY - GyroErrorY; 
  GyroZ = GyroZ - GyroErrorZ; 

  gyroAngleX = gyroAngleX + GyroX * elapsedTime; 
  gyroAngleY = gyroAngleY + GyroY * elapsedTime;
  yaw =  yaw + GyroZ * elapsedTime;

  // Complementary filter
  roll = 0.98 * gyroAngleX + 0.02 * accAngleX;
  pitch = 0.98 * gyroAngleY + 0.02 * accAngleY;
  
  // Print the values on the serial monitor
  // Serial.print(roll);
  // Serial.print("/");
  // Serial.print(pitch);
  // Serial.print("/");
  // Serial.println(yaw);

  return pitch;
}

void calculate_IMU_error() {

  while (calibrate < 200) {
    Wire.beginTransmission(mpu_addy);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(mpu_addy, 6, true);

    AccX = (Wire.read() << 8 | Wire.read()) / 16384.0 ;
    AccY = (Wire.read() << 8 | Wire.read()) / 16384.0 ;
    AccZ = (Wire.read() << 8 | Wire.read()) / 16384.0 ;

    // Sum all readings
    AccErrorX = AccErrorX + ((atan((AccY) / sqrt(pow((AccX), 2) + pow((AccZ), 2))) * 180 / PI));
    AccErrorY = AccErrorY + ((atan(-1 * (AccX) / sqrt(pow((AccY), 2) + pow((AccZ), 2))) * 180 / PI));
    calibrate++;
  }
  
  //average out over 200 cycles above
  AccErrorX = AccErrorX / calibrate;
  AccErrorY = AccErrorY / calibrate;
  calibrate = 0;
  
  while (calibrate < 200) {

    Wire.beginTransmission(mpu_addy);
    Wire.write(0x43);
    Wire.endTransmission(false);
    Wire.requestFrom(mpu_addy, 6, true);

    GyroX = Wire.read() << 8 | Wire.read();
    GyroY = Wire.read() << 8 | Wire.read();
    GyroZ = Wire.read() << 8 | Wire.read();

    // Sum all readings
    GyroErrorX = GyroErrorX + (GyroX / 131.0);
    GyroErrorY = GyroErrorY + (GyroY / 131.0);
    GyroErrorZ = GyroErrorZ + (GyroZ / 131.0);
    calibrate++;
  }

  GyroErrorX = GyroErrorX / calibrate;
  GyroErrorY = GyroErrorY / calibrate;
  GyroErrorZ = GyroErrorZ / calibrate;

  // Serial.print("AccErrorX: ");
  // Serial.println(AccErrorX);
  // Serial.print("AccErrorY: ");
  // Serial.println(AccErrorY);
  // Serial.print("GyroErrorX: ");
  // Serial.println(GyroErrorX);
  // Serial.print("GyroErrorY: ");
  // Serial.println(GyroErrorY);
  // Serial.print("GyroErrorZ: ");
  // Serial.println(GyroErrorZ);
}


