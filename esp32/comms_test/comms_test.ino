#include <SPI.h>
#include <BluetoothSerial.h>
#include "mpu6050.h"

// #define MOSI 23  // these are the default for our ESP-32s, just putting them here so yk
// #define MISO 19
// #define SCK 18
// #define SS 5

int sckdelay = 0.001;  // arbitrary number tbh can tweak this
int fpga_cs = 4;  // fpga "chip select" - selects the FPGA as the slave
int buf = 0;  // recv buffer

BluetoothSerial SerialBT;
mpu6050 mpu = mpu6050();

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! please run 'make menuconfig' to enable it
#endif

void setup() {

  // pinMode(fpga_cs, OUTPUT);
  // SPI.begin();

  //SerialBT.begin();
  Serial.begin(115200);

  mpu.init();
  mpu.calibrate();
  delay(2000);

}

void loop() {

  mpu.update();
  Serial.print(mpu.getPitch());
  Serial.print(" | ");
  Serial.print(mpu.getRoll());
  Serial.print(" | ");
  Serial.println(mpu.getYaw());
  delay(10);

  // SerialBT.println("Sending 125,000 bytes ...");

  // digitalWrite(fpga_cs, LOW); // SPI is active-low
  // delay(10);

  // double oldtime = millis();

  // for (int j = 0; j < 100; j++){
  //   for (int i = 0; i < 125; i++){
  //     buf = SPI.transfer(0xFF);
  //     // SerialBT.print("MISO: ");
  //     // SerialBT.println(buf);
  //     // delay(sckdelay);
  //   }
  //   mpu.update();
  //   // SerialBT.print(mpu.getPitch());
  // }

  // double timetaken = millis() - oldtime;

  // digitalWrite(fpga_cs, HIGH); // stop FPGA sending
  // delay(10);

  // SerialBT.print("time taken for 1Mb: ");
  // SerialBT.print(timetaken);
  // SerialBT.println("ms");
  // SerialBT.println("(with mpu readings every 125 bytes)");
  // delay(3000);

  // SerialBT.print("MOSI: ");
  // SerialBT.println(MOSI);
  // SerialBT.print("MISO: ");
  // SerialBT.println(MISO);
  // SerialBT.print("SCK: ");
  // SerialBT.println(SCK);
  // SerialBT.print("SS: ");
  // SerialBT.println(SS);
  // SerialBT.print("IMU Pitch: ");
  // SerialBT.println(mpu.getRawXGyro());
  // SerialBT.print("IMU Roll: ");
  // SerialBT.println(mpu.getRawYGyro());
  // SerialBT.print("IMU Yaw: ");
  // SerialBT.println(mpu.getRawZGyro());

}