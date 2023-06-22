int n_overflows = 0;  // keeps track of how many times we've crossed that boundary so far.
float rawReadings[2] = {-170, 170};
float processedReading; // we store the signs of the last 2 readings. 

void setup(){
  Serial.begin(115200);
  calculateAdjustment();
  Serial.println(applyAdjustment(170.0));
}

void loop(){
}

void calculateAdjustment(){
  if ((rawReadings[0] < -150) && (rawReadings[1] > 150)) { // crossed over from -180 to 180;
    n_overflows -= 1; // we need to subtract 360
  } else if ((rawReadings[0] > 150) && (rawReadings[1] < -150)) { // crossed from 180 to -180
    n_overflows += 1; // we need to add 360
  }
}

float applyAdjustment(float raw){
  return raw + ((float) (n_overflows * 360.0));
}

