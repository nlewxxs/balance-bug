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
    D8M.update();
    Matrix frame = D8M.getBoxMatrix();
    
    for (int i = 4; i < 16; i++){
      char tmp[64];
      sprintf(tmp, " Box %d: {%d, %d, %d, %d}", i+1, frame.boxes[i][0], frame.boxes[i][1], frame.boxes[i][2], frame.boxes[i][3]);
      Serial.print(tmp);
    }
    Serial.println("");
}