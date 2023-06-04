#include <SPI.h>
volatile byte Slavereceived, Slavesend;
volatile boolean received;

void setup() {
  Serial.begin(9600);
  pinMode(MISO, OUTPUT);

  SPCR |= _BV(SPE);  //Turn on SPI in Slave Mode
  SPCR |= _BV(SPIE);
  received = false;
  // SPI.attachInterrupt();  //Interuupt ON is set for SPI commnucation
}

ISR(SPI_STC_vect) {
  Slavereceived = SPDR;
  received = true;
}

void loop() {
  if (received) {
    Serial.println(Slavereceived);
    SPDR = 0x9E;
    received = false;
    delay(1000);
  }
}