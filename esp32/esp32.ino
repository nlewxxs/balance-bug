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
    
    for (int i = 4; i < 16; i++){
      char tmp[64];
      String colour_out;
      switch (frame.colour) {
        case 0:
          colour_out = "No Colour Detected";
          break;
        case 1:
          colour_out = "Red";
          break;
        case 2:
          colour_out = "Yellow";
          break;
        case 3:
          colour_out = "Blue";
          break;
        default:
          colour_out = "Error with colour detection";
      }
      sprintf(tmp, " Box %d: {%d, %d, %d, %d}, Colour: %s, Uncertainty Level: %d", i+1, frame.boxes[i][0], frame.boxes[i][1], frame.boxes[i][2], frame.boxes[i][3], colour_out, frame.colour_uncertain);
      Serial.print(tmp);
    }
    Serial.println("");
}