#include <WiFi.h>
#include <HTTPClient.h>

// fill these in for local network
const char* ssid = "";
const char* password = "";

//Your Domain name with URL path or IP address with path
String serverName = "http://192.168.1.16:8081"; // local ip of the backend host (NOT localhost)

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)
unsigned long timerDelay = 5000;

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
 
  Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
}

void loop() {
  // Send an HTTP POST request every 10 minutes
  if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      // WiFiClient client;
      HTTPClient http;

      
      // String serverPath = serverName + "/Nodes/Add?SessionId=1&NodeId=3&XCoord=72&YCoord=56";
      String serverPath = serverName + "/Edges/Add?SessionId=1&NodeId=2&EdgeNodeId=3&Distance=34.2&Angle=72.0";
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



















