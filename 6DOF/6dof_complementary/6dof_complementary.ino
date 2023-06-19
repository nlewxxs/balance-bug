#include "mpu6050.h"

mpu6050 mpu;

void setup(){
  Serial.begin(115200);
  mpu.init();
  mpu.calibrate();
  delay(1000);
}

void loop(){
  mpu.update();
  Serial.print("Upper:50,Lower:-50,");
  Serial.print("Pitch:");
  Serial.print(mpu.getPitch());
  Serial.println(",");
  delay(100);
}