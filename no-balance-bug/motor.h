#ifndef Motor_h
#define Motor_h

#include "Esp.h"


class Motor {
  public: 
    static int leftDir;
    static int leftStep;
    static int rightDir;
    static int rightStep;
    static int leftSpeed;
    static int rightSpeed;
    static void init();
    static void setSpeed();
  private:
    static bool initialised;
    static int leftCounter;
    static int leftTarget;
    static int rightTarget;
    static int rightCounter;
    static void IRAM_ATTR onStepTimer();
    static void setBackwards(bool side);
    static void setSpeed(bool side, int desiredSpeed);
    static void setForward(bool);
    // void onStepTimer();
    static hw_timer_t *step_timer;
};

#endif

