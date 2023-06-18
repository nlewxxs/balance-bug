#include "Communicate.h"

void Communicate::init(char *_ssid, char *_password, char *_serverName, String _bugId, float checkNewSessionTimeout){
  inSession = false;
  ssid = _ssid;
  password = _password;
  serverName = _serverName;
  bugId = _bugId;
  httpPing.setReuse(true);
  httpGetNewSession.setReuse(true);

  const char * headerkeys[] = {"Access-Control-Allow-Origin","*","Access-Control-Allow-Headers","access-control-allow-origin, access-control-allow-headers"} ;
  size_t headerkeyssize = sizeof(headerkeys)/sizeof(char*);
  httpPing.collectHeaders(headerkeys,headerkeyssize);
  httpGetNewSession.collectHeaders(headerkeys,headerkeyssize);

  WiFi.begin(ssid, password);
  // debugOutput("Connecting");
  Serial.println("Connecting");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    // debugOutput("Attempting WiFi connection...");
    Serial.println("Attempting WiFi connection...");
  } 

  String pathName = serverName + "/Session/CheckNewSession?BugId=" + bugId + "&TimeDiff=" + String(checkNewSessionTimeout);
  httpGetNewSession.begin(pathName.c_str());

  pathName = serverName + "/BugInformation/Ping?BugId=" + bugId;
  String Data = "BugId=" + bugId;
  httpPing.begin(pathName.c_str());

  
  while(!initialised){
    // HTTPClient http;    
    // Your Domain name with URL path or IP address with path

    // Send HTTP GET request
    Serial.println("Subscribing to Server");
    String Data = "BugId=" + bugId;
    int httpResponseCode = httpPing.PATCH(Data);
    
    if (httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = httpPing.getString();
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
    httpPing.end();
    // http.close();
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
  const char * headerkeys[] = {"Access-Control-Allow-Origin","*","Access-Control-Allow-Headers","access-control-allow-origin, access-control-allow-headers"} ;
  size_t headerkeyssize = sizeof(headerkeys)/sizeof(char*);
  httpPing.collectHeaders(headerkeys,headerkeyssize);

  // Your Domain name with URL path or IP address with path
  // http.begin(serverPath.c_str());


  // Send HTTP GET request
  String pathName = serverName + "/BugInformation/Ping?BugId=" + bugId;
  httpPing.begin(pathName.c_str());

  Serial.println("Pinging Server");
  String Data = "BugId=" + bugId;
  int httpResponseCode = httpPing.PATCH(Data);
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = httpPing.getString();
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
  httpPing.end();

  // http.close();
}

bool Communicate::getInitialised(){ return initialised; }
bool Communicate::getInSession() { return inSession; }

void Communicate::checkNewSession(){
  int httpResponseCode = httpGetNewSession.GET();

  if (httpResponseCode>0) {
    Serial.print("Check New Session Response Code: ");
    Serial.println(httpResponseCode);
    String payload = httpGetNewSession.getString();
    Serial.println(payload);

    if(httpResponseCode == 200){
      Serial.println("Ok Response");
      if(payload == "\"1\""){
        Serial.println("Now Entering Session");
        inSession = true;
      }
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
  httpGetNewSession.end();
}