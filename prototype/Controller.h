#ifndef control_h
#define control_h

#include <Arduino.h>
#include <AccelStepper.h>
#include <Preferences.h>

class Controller {
  public:
    void setup();
    void update(float pitch, float heading);
    float getDistance();
    float getLeftOutput();
    float getRightOutput();
    void updateValues(String param, float val);
    void updateValues(String param, int val);
    void updateSpeed();
    float getValue(String param);
};


#endif