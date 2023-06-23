#ifndef control_h
#define control_h

#include <Arduino.h>
#include <AccelStepper.h>

class Controller {
  public:
    void setup();
    void update(float heading);
    void updatePositionSetpoint(float setpoint);
    void updateHeadingSetpoint(float setpoint);
    void setNextHeadingSetpoint(float setpoint);
    float getDistance();
    float getLeftOutput();
    float getRightOutput();
    float getHeadingSetpoint();
    float getPositionSetpoint();
    bool getMoving();
};


#endif