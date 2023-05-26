#include <Wire.h>
#include "mpu6050.h"

const int mpu_addy = 0x68;  // designated mpu_addy I2C address


int calibrate = 0;  // calibration counter 
mpu6050 mpu = mpu6050();

void setup() {

  Serial.begin(19200);
  mpu.init();

  Serial.println("la mpu esta hablando con el monitor serial!!!");

  //-------------------- PUT PID SETUP HERE ----------------------//



  //--------------------------------------------------------------//


  /*
  // Configure Accelerometer Sensitivity - Full Scale Range (default +/- 2g)
  Wire.beginTransmission(mpu_addy);
  Wire.write(0x1C);                  //Talk to the ACCEL_CONFIG register (1C hex)
  Wire.write(0x10);                  //Set the register bits as 00010000 (+/- 8g full scale range)
  Wire.endTransmission(true);
  // Configure Gyro Sensitivity - Full Scale Range (default +/- 250deg/s)
  Wire.beginTransmission(mpu_addy);
  Wire.write(0x1B);                   // Talk to the GYRO_CONFIG register (1B hex)
  Wire.write(0x10);                   // Set the register bits as 00010000 (1000deg/s full scale)
  Wire.endTransmission(true);
  delay(20);
  */
  // Call this function if you need to get the IMU error values for your module

  mpu.calibrate();
  delay(20);

}

void loop() {
  
  // reading accelerometer data
  mpu.update();

  Serial.println(mpu.getPitch());

  //-------------------- PUT PID LOOP HERE ----------------------//



  //--------------------------------------------------------------//

}