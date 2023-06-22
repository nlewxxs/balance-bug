// esp32 FPGA to wifi relay
// takes in SPI for values
// other FPGA pins used:
// state_number across 3 pins (parallel)
#include <SPI.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include "Communicate.h"

//"http://90.196.3.86:8081" -> current address of the server, initialise with this

TaskHandle_t spiFPGA;
TaskHandle_t backend;

// check and change
int onFlag = 14;
int powerFlag = 12;

bool prev = false;

#define SCK 14
#define MISO 12
#define MOSI 13
int spiTriggerPin = 34; // change as needed
int spiTrigger = 0;
int fpga_cs = 26;
int currentState = 0;
int toSendOut = 0;
int toSendFPGA = 0;

uint16_t buf = 0;      // fpga SPI recv buffer

int status = WL_IDLE_STATUS;
const char* ssid = "Mi 9T Pro"; //Mi 9T Pro
const char* password = "randompass";
int keyIndex = 0;            // your network key Index number (needed only for WEP)

unsigned int localPort = 2390;
int listen = 0;

// target devices
IPAddress broadcastIP(192,168,107,255);

char packetBuffer[255]; //buffer to hold incoming packet
byte sendBuffer[2] = {0x0, 0x0};   // a string to send 

WiFiUDP Udp;

//Your Domain name with URL path or IP address with path
String serverName = "http://192.168.243.152:8081"; // local ip of the backend host (NOT localhost)

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
unsigned long timerDelay = 10;



// setup up communication to server
Communicate backEndComms;

void IRAM_ATTR spiStartCallback(){
  spiTrigger = 1;
  // Serial.println("SPI Trigger");
}

void getCurrentState(int spiReturn){
  // SPI returns an integer number which is val and the state number
  // need to extract the state number - i.e. bits 10 and 11 - and 12
  // state mapping:
  // 0: IDLE
  // 1: charge_buck
  // 2: charge_boost
  // 3: beacons_panel
  // 4: beacons_cap
  currentState = (spiReturn & 3072) >> 10;   // shift right by 10
  currentState = 0;   // change to be correct
}


int sendLEDDrivers(){
  Serial.println("Sending to LED drivers");
  // each led driver will return power(mW) measurement as a response
  // not necessarily needed
  // return the average 
  //temporary:
  // sends the value hex 130
  sendBuffer[0] = (byte) (toSendOut & 255);
  sendBuffer[1] = (byte) (toSendOut >> 8);
  Udp.beginPacket(broadcastIP, 2390);
  Udp.write(sendBuffer, 2);
  Udp.endPacket();
  Serial.println("Packets Sent");
  return 0;
}

bool fpgaTransfer(){
  digitalWrite(fpga_cs, LOW); // SPI is active-low
  delay(50);              

  // for (int i = 256; i < 356; i++){
  //   buf = SPI.transfer16(i);
  //   Serial.print("MOSI: ");
  //   Serial.print(i);
  //   Serial.print(" \tMISO: ");
  //   Serial.println(buf);
  //   delayMicroseconds(1);
  // }
  buf = SPI.transfer16(toSendFPGA);
  // check if last three bits are expected 101 pattern
  // return true if transaction correct

  digitalWrite(fpga_cs, HIGH); // stop FPGA sending
  delay(10);
  Serial.println(buf, BIN);
  Serial.print("Stop bits:  ");
  Serial.print((buf&32771));
  Serial.print("\t\tVal:  ");
  Serial.println(((buf&4092)>>2));
  toSendOut = ((buf&4092)>>2);
  if ((buf&32771) ==32769){
    // last three bits are 101
    return true;
  } else {
    return false;
  }
  vTaskDelay(100);
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void fpgaWifiLoop(void * pvParameters){
  // this loop for core 1 takes an spi transfer, sends it out where it needs to go
  // gets the return value, and spi transfers it back to the fpga on the next iteration
  if (spiTrigger==1){
    spiTrigger = 0;
    if(fpgaTransfer()){
      // true so correct transaction
      toSendOut = (uint16_t) (buf >> 3); // shift right by three bits to clear out stop sequence
      Serial.print("toSendOut:  ");
      Serial.println(toSendOut);
      Serial.println("LooP");
      toSendFPGA = sendLEDDrivers();
    } else {
      // bad transaction, ignore
      Serial.println("SPI bad");
    }
    
  }
  vTaskDelay(100);  
}

//void serverBackendLoop(void * pvParameters){
//  // this loop runs the backend polling code every 10ms
//  // its queries the backend via http to get whether the LEDs should be on,
//  // and sends it whether there is power in the system to turn on the LEDs
//
//  Serial.println("Backend Polling");
//  //Check WiFi connection status
//  if(WiFi.status()== WL_CONNECTED){
//    // read becon charge flag and update server with that
//    //backEndComms.setBeaconCharge(String(digitalRead(powerFlag)));
//    // speak to the server
//    backEndComms.pingBeaconOn();
//    // read the new beacon status and write the flag
//    digitalWrite(onFlag, backEndComms.getBeaconOn());
//    Serial.println("onFlag Set");
//  }
//
//  //vTaskDelay(1000);
//}

void setup() {
  Serial.begin(9600);
  delay(2000);
  pinMode(fpga_cs, OUTPUT);
  pinMode(SCK, OUTPUT);
  backEndComms.init(String(0), 0, serverName);
  pinMode(powerFlag, INPUT);
  pinMode(spiTriggerPin, INPUT);
  pinMode(onFlag, OUTPUT);
  
  Serial.println("test");
  // setup SPI pins
  // SPI.begin(SCK, MISO, MOSI, );
  // SPI.begin(SCK, MISO, MOSI, 27);
  //vspi.begin(VSPI_CLK, VSPI_MISO, VSPI_MOSI, VSPI_SS);

  SPI.begin(SCK, MISO, MOSI, 27);
  SPI.setFrequency(125000);
  //SPI.beginTransaction(SPISettings(14000000, MSBFIRST, SPI_MODE0));
  Serial.println("SPI started");
  // setup state number pins
  // pinMode(state0, INPUT);
  // pinMode(state1, INPUT);
  // pinMode(state2, INPUT);

  // attempt to connect to Wifi network:
  // Operate in WiFi Station mode
  WiFi.mode(WIFI_STA);
  // Start WiFi with supplied parameters
  WiFi.begin(ssid, password);
  // Print periods on monitor while establishing connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    delay(500);
  }
  Serial.println("Connected to wifi");
  printWifiStatus();
  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
  Udp.begin(localPort);

  attachInterrupt(spiTriggerPin, spiStartCallback, FALLING);

  //setup mulitasking
  // xTaskCreatePinnedToCore(
  //                   fpgaWifiLoop,   /* Task function. */
  //                   "fpga to wifi",     /* name of task. */
  //                   10000,       /* Stack size of task */
  //                   NULL,        /* parameter of the task */
  //                   1,           /* priority of the task */
  //                   &spiFPGA,      /* Task handle to keep track of created task */
  //                   0);          /* pin task to core 0 */                  
  // delay(500); 
//
//  xTaskCreatePinnedToCore(
//                    serverBackendLoop,   /* Task function. */
//                    "server backend comms",     /* name of task. */
//                    10000,       /* Stack size of task */
//                    NULL,        /* parameter of the task */
//                    1,           /* priority of the task */
//                    &backend,      /* Task handle to keep track of created task */
//                    1);          /* pin task to core 0 */                  
//  delay(500); 
}

void loop() {  
   // this loop for core 1 takes an spi transfer, sends it out where it needs to go
  // gets the return value, and spi transfers it back to the fpga on the next iteration
  //Serial.print("spi trigger: ");
  //Serial.println(spiTrigger);
  // if(digitalRead(spiTriggerPin) == LOW){
  //   Serial.print("spi high trigger found: ");
  //   spiTrigger == 1;
  //   Serial.print("spi trigger found: ");
  //   Serial.println(spiTrigger);
    if (spiTrigger==1){
      spiTrigger = 0;
      if(fpgaTransfer()){
        Serial.println("OK");
        // true so correct transaction
        Serial.print("toSendOut:  ");
        Serial.println(toSendOut);
        Serial.println("LooP");
        toSendFPGA = sendLEDDrivers();
      } else {
        // bad transaction, ignore
        // Serial.println("SPI bad");
      }
      
    }
    vTaskDelay(100);  

  // }
  
}
