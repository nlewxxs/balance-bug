#include <Wire.h>
#include "mpu6050.h"


mpu6050 mpu = mpu6050();

void setup() {

  Serial.begin(9600);
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
  // Serial.println("test");

  Serial.print(mpu.getPitch());
  Serial.print("\t");
  Serial.print(mpu.getYaw());
  Serial.print("\t");
  Serial.print(mpu.getRoll());
  Serial.print("\t");
  Serial.println(mpu.getElapsedTime());

  //-------------------- PUT PID LOOP HERE ----------------------//



  //--------------------------------------------------------------//
}