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

  mpu.update();

  Serial.print("GyroX: ");
  Serial.println(mpu.getRawXGyro());
  Serial.print("GyroY: ");
  Serial.println(mpu.getRawYGyro());
  Serial.print("GyroZ: ");
  Serial.println(mpu.getRawZGyro());
  delay(10);

}