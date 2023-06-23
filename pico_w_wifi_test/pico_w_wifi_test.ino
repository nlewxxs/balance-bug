/*
  Pi Pico W WiFi Station Demo
  picow-wifi-station.ino
  Use WiFi library to connect Pico W to WiFi in Station mode
 
  DroneBot Workshop 2022
  https://dronebotworkshop.com
*/
 
// Include the WiFi Library
#include <WiFi.h>
 
// Replace with your network credentials
const char* ssid = "Mi 9T Pro";
const char* password = "randompass";
 
void setup() {
 
  // Start the Serial Monitor
  Serial.begin(115200);

  // Start WiFi with supplied parameters
  //WiFi.begin(ssid, password);
 
  // Operate in WiFi Station mode
  WiFi.mode(WIFI_STA);
 
  // WiFi.disconnect();
  IPAddress ip(192,168,69,1); 
  IPAddress gateway(192,168,132,245);   
  IPAddress subnet(255,255,255,0); 
  WiFi.config(ip, gateway, subnet);
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
 
}
 
void loop() {
 
  delay(2000);
 
  // Print IP Address
  Serial.print("Assigned IP Address: ");
  Serial.println(WiFi.localIP());
 
}