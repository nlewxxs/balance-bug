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
    char colour_out [128];
    switch (frame.colour) {
      case 0:
        sprintf(colour_out, "No Colour Detected");
        break;
      case 1:
        sprintf(colour_out, "Red");
        break;
      case 2:
        sprintf(colour_out, "Yellow");
        break;
      case 4:
        sprintf(colour_out, "Blue");
        break;
      default:
        sprintf(colour_out, "Error with colour detection");
    }
    char tmp2[128];
    // for (int i = 4; i < 16; i++){
    //   char tmp[256];
      
    //   sprintf(tmp, " Box %d: {%d, %d, %d, %d}", i+1, frame.boxes[i][0], frame.boxes[i][1], frame.boxes[i][2], frame.boxes[i][3]);
    //   Serial.print(tmp);
    // }
    sprintf(tmp2, ", Colour: %s, Uncertainty Level: %d", colour_out, frame.colour_uncertain);
    Serial.print(tmp2);
    Serial.println("");
}