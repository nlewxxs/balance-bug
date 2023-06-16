#include "Communicate.h"

void Communicate::init(){
  WiFi.begin(ssid, password);
    debugOutput("Connecting");

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      debugOutput("Attempting WiFi connection...");
    } 

    HTTPClient http;

    while(!initialised){
      String serverPath = serverName + "/BugInformation/Add?BugId=MazEERunnEEr&BugName=MazEERunnEEr";
      
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
        if(httpResponseCode == 201){
          Serial.println!("Initialised");
          initialised = true;
        }
        
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
    }
    // Free resources
    http.end();
      

    debugOutput("");
    debugOutput("Connected to WiFi network with IP Address: ", false);
    debugOutput(WiFi.localIP());
    debugOutput("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
}
    
// bool checkNewSession();