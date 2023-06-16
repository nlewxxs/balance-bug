#include "Communicate.h"

void Communicate::init(char *_ssid, char *_password, char *_serverName, String _bugId){
  inSession = false;
  ssid = _ssid;
  password = _password;
  serverName = _serverName;
  bugId = _bugId;

  WiFi.begin(ssid, password);
  // debugOutput("Connecting");
  Serial.println("Connecting");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    // debugOutput("Attempting WiFi connection...");
    Serial.println("Attempting WiFi connection...");
  } 

  while(!initialised){
    HTTPClient http;
    String serverPath = serverName + "/BugInformation/Ping?BugId=" + bugId;
    
    // Your Domain name with URL path or IP address with path
    http.begin(serverPath.c_str());

    // Send HTTP GET request
    Serial.println("PingingServer");
    int httpResponseCode = http.GET();
    
    if (httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
      if(httpResponseCode == 200){
        Serial.println("Initialised");
        initialised = true;
      }
      else{
        vTaskDelay(1000);
      }
      
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
      vTaskDelay(1000);
    }
    http.end();
  }
  // Free resources
    

  // debugOutput("");
  // debugOutput("Connected to WiFi network with IP Address: ", false);
  // debugOutput(WiFi.localIP());
  // debugOutput("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
}

void Communicate::checkConnection(){
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      // debugOutput("Attempting WiFi connection...");
      Serial.println("WiFi Connection lost, attempting to reconnect...");
      WiFi.begin(ssid, password);
  } 
}


void Communicate::ping(){

  checkConnection();

  String serverPath = serverName + "/BugInformation/Ping?BugId=" + bugId;
  HTTPClient http;
    // Your Domain name with URL path or IP address with path
    http.begin(serverPath.c_str());

    // Send HTTP GET request
    Serial.println("Pinging Server");
    int httpResponseCode = http.GET();
    
    if (httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
      if(httpResponseCode == 200){
        return;
      }
      else{
        Serial.print("Incorrect response code: ");
        Serial.println(httpResponseCode);
      }
      
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
      checkConnection();
    }
    http.end();
}

bool Communicate::getInitialised(){ return initialised; }
bool Communicate::getInSession(){ return inSession; }
    
// bool checkNewSession();