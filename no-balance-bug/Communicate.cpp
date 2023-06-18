#include "Communicate.h"

void Communicate::init(char *_ssid, char *_password, char *_serverName, String _bugId, float checkNewSessionTimeout){
  inSession = false;
  ssid = _ssid;
  password = _password;
  serverName = _serverName;
  bugId = _bugId;
  httpPing.setReuse(true);
  httpGetNewSession.setReuse(true);
  httpNodeSetup = false;
  httpEdgeSetup = false;


  //set up http rooutes
  const char * headerkeys[] = {"Access-Control-Allow-Origin","*","Access-Control-Allow-Headers","access-control-allow-origin, access-control-allow-headers"} ;
  size_t headerkeyssize = sizeof(headerkeys)/sizeof(char*);
  httpPing.collectHeaders(headerkeys,headerkeyssize);
  httpGetNewSession.collectHeaders(headerkeys,headerkeyssize);
  httpNode.collectHeaders(headerkeys,headerkeyssize);
  httpEdge.collectHeaders(headerkeys,headerkeyssize);

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

  pathName = serverName + "/Nodes/CreateTable?BugId=" + bugId;
  httpNode.begin(pathName.c_str());

  pathName = serverName + "/Nodes/CreateTable?BugId=" + bugId;
  httpEdge.begin(pathName.c_str());

  
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
bool Communicate::getStatusMapSetup() { return mapSetup; }

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

void Communicate::setUpMap(){
  if(inSession) {
    String params = "BugId=" + bugId;

    //node table request handling
    if(!httpNodeSetup) {
      int httpNodeResponseCode = httpNode.PUT(params);
      if (httpNodeResponseCode>0) {
        Serial.print("Check New Node Table Response Code: ");
        Serial.println(httpNodeResponseCode);
        String payload = httpNode.getString();
        Serial.println(payload);

        if(httpNodeResponseCode == 201){
          Serial.println("Ok Response, Node Table successfully created");
          httpNodeSetup = true;
        }
        else{
          Serial.print("Incorrect response code: ");
          Serial.println(httpNodeResponseCode);
        }
        
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpNodeResponseCode);
        checkConnection();
      }
      httpNode.end();
    }

    if(!httpEdgeSetup) {
      //edge table request handling
      int httpEdgeResponseCode = httpEdge.PUT(params);

      if (httpEdgeResponseCode>0) {
        Serial.print("Check New Node Table Response Code: ");
        Serial.println(httpEdgeResponseCode);
        String payload = httpEdge.getString();
        Serial.println(payload);

        if(httpEdgeResponseCode == 201){
          Serial.println("Ok Response, Node Table successfully created");
          httpEdgeSetup = true;
        }
        else{
          Serial.print("Incorrect response code: ");
          Serial.println(httpEdgeResponseCode);
        }
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpEdgeResponseCode);
        checkConnection();
      }
      httpEdge.end();
    }

    if((httpEdgeSetup == true) && (httpNodeSetup == true)){
      mapSetup = true;
    }
  } else {
    Serial.println("Usage Error, please connect to a new session first.");
  }
}

void Communicate::addNode(String _nodeId, String _xCoord, String _yCoord){
  if(mapSetup) {
    String pathName = serverName + "/Nodes/Add?BugId=" + bugId + "&NodeId=" + _nodeId + "&xCoord=" + _xCoord + "&yCoord=" + _yCoord;
    httpNode.begin(pathName.c_str());

    String params = "BugId=" + bugId + "&NodeId=" + _nodeId + "&xCoord=" + _xCoord + "&yCoord=" + _yCoord;

    int httpNodeResponseCode = httpNode.PUT(params);

    if (httpNodeResponseCode>0) {
      Serial.print("Check New Node Table Response Code: ");
      Serial.println(httpNodeResponseCode);
      String payload = httpNode.getString();
      Serial.println(payload);

      if(httpNodeResponseCode == 200){
        Serial.println("Successfully added Node");
      }
      else{
        Serial.print("Incorrect response code: ");
        Serial.println(httpNodeResponseCode);
      }
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpNodeResponseCode);
      checkConnection();
    }
    httpNode.end();
  } else {
    Serial.println("First Set up the tables");
  }
}

void Communicate::addEdge(String _nodeId, String _edgeNodeId, String _distance, String _angle){
  if(mapSetup) {
    String pathName = serverName + "/Edges/Add?BugId=" + bugId + "&NodeId=" + _nodeId + "&EdgeNodeId=" + _edgeNodeId + "&Distance=" + _distance + "&Angle=" + _angle;
    httpEdge.begin(pathName.c_str());

    String params = "BugId=" + bugId + "&NodeId=" + _nodeId + "&EdgeNodeId=" + _edgeNodeId + "&Distance=" + _distance + "&Angle=" + _angle;
    int httpEdgeResponseCode = httpEdge.PUT(params);

    if (httpEdgeResponseCode>0) {
      Serial.print("Check New Node Table Response Code: ");
      Serial.println(httpEdgeResponseCode);
      String payload = httpEdge.getString();
      Serial.println(payload);

      if(httpEdgeResponseCode == 201){
        Serial.println("Ok Response, Node Table successfully created");
        httpEdgeSetup = true;
      }
      else{
        Serial.print("Incorrect response code: ");
        Serial.println(httpEdgeResponseCode);
      }
      
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpEdgeResponseCode);
      checkConnection();
    }
    httpEdge.end();
  } else {
    Serial.println("First Set up the tables");
  }
}
