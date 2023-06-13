#include "Camera.h"

Camera D8M;

void setup(){
  Serial.begin(115200);
  Serial.println("Starting camera");
  D8M.init();
  Serial.println("Started camera");
}

void loop(){
    D8M.update();
    Matrix frame = D8M.getBoxMatrix();
    char tmp[64];
    
    for (int i = 4; i < 15; i++){
      sprintf(tmp, "%d,%d,%d,%d,", frame.boxes[i][0], frame.boxes[i][1], frame.boxes[i][2], frame.boxes[i][3]);
      Serial.print(tmp);
    }

    sprintf(tmp, "%d,%d,%d,%d", frame.boxes[15][0], frame.boxes[15][1], frame.boxes[15][2], frame.boxes[15][3]);
    Serial.println(tmp);
}