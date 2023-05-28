#include <SPI.h>

// #define MOSI 23
// #define MISO 19
// #define SCK 18
// #define SS 5

int sckdelay = 0.001;
int fpga_cs = 4;
int buf = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.print("MOSI: ");
  Serial.println(MOSI);
  Serial.print("MISO: ");
  Serial.println(MISO);
  Serial.print("SCK: ");
  Serial.println(SCK);
  Serial.print("SS: ");
  Serial.println(SS);  

  pinMode(fpga_cs, OUTPUT);
  SPI.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("SS set to low");
  digitalWrite(fpga_cs, LOW);
  delay(1000);

  for (int i = 0; i < 100; i++){
    digitalWrite(SCK, HIGH);
    Serial.println("1");
    delay(sckdelay);
    Serial.print("MISO: ");
    buf = SPI.transfer(0xF0);
    Serial.println(buf);
    digitalWrite(SCK, LOW);
    Serial.println("0");
    delay(sckdelay);
  }

  digitalWrite(fpga_cs, HIGH);
  Serial.println("SS set to high");
  delay(1000);
}