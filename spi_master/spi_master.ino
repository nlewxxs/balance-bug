#include <SPI.h>

// #define MOSI 23  // these are the default for our ESP-32s, just putting them here so yk
// #define MISO 19
// #define SCK 18
// #define SS 5

const uint8_t fpga_cs = 4;  // fpga "chip select" - selects the FPGA as the slave
uint8_t buf = 0;      // recv buffer

void setup() {

  pinMode(fpga_cs, OUTPUT);
  SPI.begin();

  Serial.begin(115200);
  delay(2000);
}

void loop() {

  digitalWrite(fpga_cs, LOW); // SPI is active-low
  delay(10);              

  for (int i = 0; i < 100; i++){
    buf = SPI.transfer(i);
    Serial.print("MOSI: ");
    Serial.print(i);
    Serial.print(" \tMISO: ");
    Serial.println(buf);
    delay(1);
  }

  digitalWrite(fpga_cs, HIGH); // stop FPGA sending
  delay(10);
}
