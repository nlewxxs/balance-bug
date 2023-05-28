#include <SPI.h>
#include <BluetoothSerial.h>

// #define MOSI 23
// #define MISO 19
// #define SCK 18
// #define SS 5

int sckdelay = 0.001;
int fpga_cs = 4;
int buf = 0;

BluetoothSerial SerialBT;

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! please run 'make menuconfig' to enable it
#endif

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  SerialBT.begin();
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
    delay(sckdelay);
    SerialBT.print("MISO: ");
    buf = SPI.transfer(0xF0);
    SerialBT.println(buf);
    digitalWrite(SCK, LOW);
    delay(sckdelay);
  }

  digitalWrite(fpga_cs, HIGH);
  Serial.println("SS set to high");
  delay(1000);
}