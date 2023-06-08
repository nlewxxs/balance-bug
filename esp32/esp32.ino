#include <HardwareSerial.h>

//MACROS
const int8_t tx = 17;
const int8_t rx = 16;
const uint8_t baudrate = 115200;
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


void loop() {

  if (SerialPortNios.available()) {

    Serial.print("Reading: ");
    char tmp[32];
    uint32_t uart_rx = SerialPortNios.read() | (SerialPortNios.read() << 8) | (SerialPortNios.read() << 16) | (SerialPortNios.read() << 24);
  
    sprintf(tmp, "0x%.8X", uart_rx);
    Serial.println(tmp);
  }

  delay(1);
}

