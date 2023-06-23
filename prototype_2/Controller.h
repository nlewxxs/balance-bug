#ifndef control_h
#define control_h

#include <Arduino.h>
#include <AccelStepper.h>
#include <Preferences.h>

class Controller {
  public:
    void setup();
    void update(float pitch, float pitchRate, float heading);
    float getDistance();
    float getLinearVelocity();
    float getLeftOutput();
    float getRightOutput();
    void updateValues(String param, float val);
    void updateValues(String param, int val);
    void updateSpeed();
    float getValue(String param);
    void moveForwards();
    void stopMoving();
    void rotate(float amount);
    float getVOut();
};


#endif