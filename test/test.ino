#include "Classify.h"

Image myimage;
int input_grid[12][4] = {{0,0,0,0}, {270,120,283,137}, {0,0,0,0}, {0,0,0,0}, {4,254,159,359}, {160,246,319,287}, {320,243,479,359}, {480,250,637,338}, {4,360,84,437}, {0,0,0,0}, {399,360,478,476}, {313,0,315,54}};
// ,,,,,,,,,,,

void setup(){
  Serial.begin(115200);
  myimage.classify(input_grid);
  myimage.debugInfo();
}

void loop(){

}