/*
  Pi Pico W WiFi Station Demo
  picow-wifi-station.ino
  Use WiFi library to connect Pico W to WiFi in Station mode
 
  DroneBot Workshop 2022
  https://dronebotworkshop.com
*/
 
// Include the WiFi Library
#include <WiFi.h>
#include <WiFiUDP.h>

int localPort = 2390;
char packetBuffer[255]; // buffer to hold incoming packets
char replyBuffer[4] = ""; // buffer for outgoing packets

WiFiUDP udp;

// Replace with your network credentials
const char* ssid = "Mi 9T Pro";
const char* password = "randompass";
 
void setup() {
 
  // Start the Serial Monitor
  Serial.begin(115200);
    while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Serial connected and now waiting");
  delay(10000);

  // Start WiFi with supplied parameters
  //WiFi.begin(ssid, password);
 
  // Operate in WiFi Station mode
  WiFi.mode(WIFI_STA);
 
  // WiFi.disconnect();
  IPAddress ip(192,168,69,1); 
  IPAddress gateway(192,168,28,21);   
  IPAddress subnet(255,255,255,0); 
  IPAddress dns(8,8,8,8);

  //WiFi.config(ip, gateway, subnet);
  WiFi.begin(ssid, password);
 
  // Print periods on monitor while establishing connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    delay(500);
  }

  // Connection established
  Serial.println("");
  Serial.print("Pico W is connected to WiFi network ");
  Serial.println(WiFi.SSID());
 
  // Print IP Address
  Serial.print("Assigned IP Address: ");
  Serial.println(WiFi.localIP());

  Serial.print("Subnet mask:  ");
  Serial.println(WiFi.subnetMask());


  udp.begin(localPort);
 
}
 
void loop() {
 
  delay(10);
 
  // Print IP Address
  //Serial.print("Assigned IP Address: ");
  //Serial.println(WiFi.localIP());
  int packetSize = udp.parsePacket();
  if (packetSize) {
    Serial.print("Received packet of size ");
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remoteIp = udp.remoteIP();
    Serial.print(remoteIp);
    Serial.print(", port ");
    Serial.println(udp.remotePort());


  }
}