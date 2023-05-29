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
// mpu6050 mpu = mpu6050();

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! please run 'make menuconfig' to enable it
#endif

void setup() {

  pinMode(fpga_cs, OUTPUT);
  SPI.begin();

  SerialBT.begin();

  // mpu.init();
  // mpu.calibrate();
  // delay(200);

}

void loop() {

  digitalWrite(fpga_cs, LOW); // SPI is active-low
  delay(10);

  for (int i = 0; i < 8; i++){
    buf = SPI.transfer(0xFF);
    SerialBT.print("MISO: ");
    SerialBT.println(buf);
    delay(sckdelay);
  }

  digitalWrite(fpga_cs, HIGH); // stop FPGA sending
  delay(10);

  // SerialBT.print("MOSI: ");
  // SerialBT.println(MOSI);
  // SerialBT.print("MISO: ");
  // SerialBT.println(MISO);
  // SerialBT.print("SCK: ");
  // SerialBT.println(SCK);
  // SerialBT.print("SS: ");
  // SerialBT.println(SS);

  // mpu.update();
  // SerialBT.print("IMU Pitch: ");
  // SerialBT.println(mpu.getRawXGyro());
  // SerialBT.print("IMU Roll: ");
  // SerialBT.println(mpu.getRawYGyro());
  // SerialBT.print("IMU Yaw: ");
  // SerialBT.println(mpu.getRawZGyro());

}