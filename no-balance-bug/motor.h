#ifndef Motor_h
#define Motor_h

#include "Esp.h"


class Motor {
  public: 
    int dir;
    int step;
    int speed;
    void init();
    void setSpeed();
  private:
    bool initialised;
    int counter;
    int target;
    void setBackwards(bool side);
    void setSpeed(bool side, int desiredSpeed);
    void setForward(bool);
    // void onStepTimer();
    hw_timer_t *step_timer;
};

#endif

