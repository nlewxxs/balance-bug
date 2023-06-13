#include "Camera.h"

Camera D8M;

#define INTERVAL 150

long prevTime;


void setup(){
  Serial.begin(115200);
  Serial.println("Starting camera");
  D8M.init();
  Serial.println("Started camera");
  prevTime = 0;
}

void loop(){
  
  // for (int c = 0; c < INTERNALCLKDIV; c++){
    // Serial.println("Getting new segments");
    // for (int k = 0; k < 1000; k++){
    // if(c==(INTERNALCLKDIV-1)){
    // Serial.println(esp_timer_get_time());
    if((esp_timer_get_time() - prevTime) >= (INTERVAL * 1000)){
      Serial.print("In loop after: ");
      Serial.println(esp_timer_get_time() - prevTime);
      D8M.update();
      Matrix frame = D8M.getBoxMatrix();

      for (int i = 15; i < 16; i++){
        char tmp[64];
        sprintf(tmp, " Box %d: {%d, %d, %d, %d}", i+1, frame.boxes[i][0], frame.boxes[i][1], frame.boxes[i][2], frame.boxes[i][3]);
        Serial.print(tmp);
      }
      Serial.println("");
      prevTime = esp_timer_get_time();
    }

    

    // for (int i = 15; i < 16; i++){
    //   char tmp[64];
    //   sprintf(tmp, " Box %d: {%d, %d, %d, %d}", i+1, frame.boxes[i][0], frame.boxes[i][1], frame.boxes[i][2], frame.boxes[i][3]);
    //   Serial.print(tmp);
    // }
    // Serial.println("");
  // }
}