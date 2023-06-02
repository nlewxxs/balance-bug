#include "mpu6050.h"

mpu6050 mpu = mpu6050();

void setup(){

  Serial.begin(115200);

  mpu.init();

  delay(100);
  
  mpu.calibrate();

  delay(100);

}

void loop(){

  Serial.print("GyroErrorX: ");
  Serial.println(mpu.getGyroErrorX());
  Serial.print("GyroErrorY: ");
  Serial.println(mpu.getGyroErrorY());
  Serial.print("GyroErrorZ: ");
  Serial.println(mpu.getGyroErrorZ());

  delay(5000);

}