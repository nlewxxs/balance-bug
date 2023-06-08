#include <HardwareSerial.h>

//MACROS
const int8_t tx = 17;
const int8_t rx = 16;
const String bt_name = "TestRig";
const uint8_t nios_serialport = 2;

HardwareSerial SerialPortNios(nios_serialport);  //if using UART2

void setup() {

 
  //SerialBT.begin();
  Serial.begin(115200);
  //Serial.setTimeout(100);
  Serial.println("Opened locally");
  SerialPortNios.begin(115200, SERIAL_8N1, rx, tx);

  SerialPortNios.setTimeout(10);
  delay(1000);
}

uint32_t client_output;
String output;

void loop() {

  client_output = 0;
  // SerialPortNios.readBytes(buffer, 4);
  // Serial.println(String(buffer, HEX));
  //SerialPortNios.readString();
  client_output = (SerialPortNios.read() << 24) | (SerialPortNios.read() << 16) | (SerialPortNios.read() << 8) | SerialPortNios.read();
  output = String(client_output, HEX);

  if (output != "ffffffff") { Serial.print("Reading: "); Serial.println(output);}

  delay(10);
}
