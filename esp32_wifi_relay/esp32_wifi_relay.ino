// esp32 FPGA to wifi relay
// takes in SPI for values
// other FPGA pins used:
// state_number across 3 pins (parallel)
#include <SPI.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>

TaskHandle_t spiFPGA;
TaskHandle_t backend;

// check and change
int onFlag = 30;
int powerFlag = 31;


int curr0, curr1, curr2;
int fpga_cs = 4;
int currentState = 0;
int toSendOut = 0;
int toSendFPGA = 0;

uint16_t buf = 0;      // fpga SPI recv buffer

int status = WL_IDLE_STATUS;
const char* ssid = "Mi 9T Pro";
const char* password = "randompass";
int keyIndex = 0;            // your network key Index number (needed only for WEP)

unsigned int localPort = 2390;
int listen = 0;

// target devices
IPAddress broadcastIP(192,168,72,255);

char packetBuffer[255]; //buffer to hold incoming packet
byte sendBuffer[] = {0x0, 0x0};   // a string to send 

WiFiUDP Udp;

//Your Domain name with URL path or IP address with path
String serverName = "http://192.168.1.16:8081"; // local ip of the backend host (NOT localhost)

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)
unsigned long timerDelay = 5000;


void getCurrentState(int spiReturn){
  // SPI returns an integer number which is val and the state number
  // need to extract the state number
  // state mapping:
  // 0: IDLE
  // 1: charge_buck
  // 2: charge_boost
  // 3: beacons_panel
  // 4: bacons_cap
  currentState = 0;   // change to be correct
}

int sendChargeBuck(){
  Serial.println("Sending to buck controller");
  // will send a voltage(mV) measurment as a response
  return 4672;
}

int sendChargeBoost(){
  Serial.println("Sending to boost controller");
  // will send a voltage(mV) measurement as a response
  return 3456;
}

int sendLEDDrivers(){
  Serial.println("Sending to LED drivers");
  // each led driver will return power(mW) measurement as a response
  // not necessarily needed
  // return the average 
  //temporary:
  // sends the value hex 130
  sendBuffer[0] = 0xFF;
  sendBuffer[1] = 0x2;
  Udp.beginPacket(broadcastIP, 2390);
  Udp.write(sendBuffer, 2);
  Udp.endPacket();
  return 0;
}

void fpgaTransfer(){
  digitalWrite(4, LOW); // SPI is active-low
  delay(10);              

  for (int i = 256; i < 356; i++){
    buf = SPI.transfer16(i);
    Serial.print("MOSI: ");
    Serial.print(i);
    Serial.print(" \tMISO: ");
    Serial.println(buf);
    delayMicroseconds(1);
  }

  digitalWrite(4, HIGH); // stop FPGA sending
  delay(10);
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
  // update current state from system
  // getCurrentState();
  // // read if there is any SPI incoming
  // toSendOut = SPI.transfer(toSendFPGA);
  // // direct on based on current state
  // if (currentState == 1){
  //   // charge buck
  //   toSendFPGA = sendChargeBuck();
  // } else if (currentState == 2){
  //   toSendFPGA = sendChargeBoost();
  // } else if ((currentState == 3) || (currentState == 4)){
  //   toSendFPGA = sendLEDDrivers();
  // }
  fpgaTransfer();
  toSendOut = buf;
  Serial.print("toSendOut:  ");
  Serial.println(toSendOut);
  Serial.println("LooP");
  toSendFPGA = sendLEDDrivers();
  delay(5000);
}

void serverBackendLoop(void * pvParameters){
  // this loop runs the backend polling code every 10ms
  // its queries the backend via http to get whether the LEDs should be on,
  // and sends it whether there is power in the system to turn on the LEDs
    // Send an HTTP POST request every 10 minutes
  if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      // WiFiClient client;
      HTTPClient http;      
      // String serverPath = serverName + "/Nodes/Add?SessionId=1&NodeId=3&XCoord=72&YCoord=56";
      String serverPath = serverName + "/Beacon/BeaconPingSide?chargeStatus=" + digitalRead(powerFlag);
      Serial.println(serverPath);
    
      // Your Domain name with URL path or IP address with path
      http.begin(serverPath.c_str());
      
      // HTTP GET request
      int httpResponseCode = http.GET();

      if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode); // HTTP response code e.g. 200
        String payload = http.getString();
        Serial.println(payload); // HTTP response package e..g JSON object
        // payload should be whether to turn on LEDs
        if (payload == "1"){
          digitalWrite(onFlag, 1);
        } else {
          digitalWrite(onFlag, 0);
        }
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
        
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("test");
  // setup SPI pins
  SPI.begin();
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

  //setup mulitasking
  xTaskCreatePinnedToCore(
                    fpgaWifiLoop,   /* Task function. */
                    "fpga to wifi",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &spiFPGA,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
  delay(500); 

  xTaskCreatePinnedToCore(
                    serverBackendLoop,   /* Task function. */
                    "server backend comms",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &backend,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 0 */                  
  delay(500); 
}

void loop() {  
}
