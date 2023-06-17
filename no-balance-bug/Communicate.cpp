#include "Communicate.h"

void Communicate::init(char *_ssid, char *_password, char *_serverName, String _bugId){
  inSession = false;
  ssid = _ssid;
  password = _password;
  serverName = _serverName;
  bugId = _bugId;
  http.setReuse(true);

  const char * headerkeys[] = {"Access-Control-Allow-Origin","*","Access-Control-Allow-Headers","access-control-allow-origin, access-control-allow-headers"} ;
  size_t headerkeyssize = sizeof(headerkeys)/sizeof(char*);
  http.collectHeaders(headerkeys,headerkeyssize);

  WiFi.begin(ssid, password);
  // debugOutput("Connecting");
  Serial.println("Connecting");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    // debugOutput("Attempting WiFi connection...");
    Serial.println("Attempting WiFi connection...");
  } 

  String pathName = serverName + "/BugInformation/Ping?BugId=" + bugId;
  http.begin(pathName.c_str());

  while(!initialised){
    // HTTPClient http;    
    // Your Domain name with URL path or IP address with path

    // Send HTTP GET request
    Serial.println("Subscribing to Server");
    String Data = "BugId=" + bugId;
    int httpResponseCode = http.PATCH(Data);
    
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
  http.collectHeaders(headerkeys,headerkeyssize);

  // Your Domain name with URL path or IP address with path
  // http.begin(serverPath.c_str());


  // Send HTTP GET request
  Serial.println("Pinging Server");
  String Data = "BugId=" + bugId;
  int httpResponseCode = http.PATCH(Data);
  
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

  // http.close();
}

bool Communicate::getInitialised(){ return initialised; }
bool Communicate::getInSession(){ return inSession; }
    
// bool checkNewSession();