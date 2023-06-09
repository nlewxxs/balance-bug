// esp32 FPGA to wifi relay
// takes in SPI for values
// other FPGA pins used:
// state_number across 3 pins (parallel)
#include <SPI.h>
#include <WiFi.h>
#include <WiFiUdp.h>

int curr0, curr1, curr2;
int state0 = 1;
int state1 = 2;
int state2 = 3;
int currentState = 0;
int toSendOut = 0;
int toSendFPGA = 0;

int status = WL_IDLE_STATUS;
const char* ssid = "Mi 9T Pro";
const char* password = "randompass";
int keyIndex = 0;            // your network key Index number (needed only for WEP)

unsigned int localPort = 2390;
int listen = 0;

// target devices
IPAddress drive1Ip(192,168,59,180);

char packetBuffer[255]; //buffer to hold incoming packet
byte sendBuffer[] = {0x0, 0x0};   // a string to send 

WiFiUDP Udp;

void getCurrentState(){
  curr0 = digitalRead(state0);
  curr1 = digitalRead(state1);
  curr2 = digitalRead(state2);
  currentState = 4*curr2 + 2*curr1 + curr0; // calculate current state number
  // state mapping:
  // 0: IDLE
  // 1: charge_buck
  // 2: charge_boost
  // 3: beacons_panel
  // 4: bacons_cap
  
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
  sendBuffer[0] = 0x1;
  sendBuffer[1] = 0x30;
  Udp.beginPacket(drive1Ip, 2390);
  Udp.write(sendBuffer, 2);
  Udp.endPacket();

  // no wait for the response
  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  listen = 1;
  while (listen){
    if (packetSize) {
      Serial.print("Received packet of size ");
      Serial.println(packetSize);
      Serial.print("From ");
      IPAddress remoteIp = Udp.remoteIP();
      Serial.print(remoteIp);
      Serial.print(", port ");
      Serial.println(Udp.remotePort());
      int len = Udp.read(packetBuffer, 255);
      int val[2]; 
      Serial.println(len);
      for(byte i=0; i<len; i++){
        val[i] = packetBuffer[i];
        Serial.println((int)(val[i]));
      }
    }
  }
  return 224;
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

void setup() {
  Serial.begin(115200);
  Serial.println("test");
  // setup SPI pins
  SPI.begin(SCK, MISO, MOSI, SS);
  //SPI.beginTransaction(SPISettings(14000000, MSBFIRST, SPI_MODE0));
  Serial.println("SPI started");
  // setup state number pins
  pinMode(state0, INPUT);
  pinMode(state1, INPUT);
  pinMode(state2, INPUT);

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
}

void loop() {
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
  Serial.println("LooP");
  toSendFPGA = sendLEDDrivers();
  delay(1000);
}
