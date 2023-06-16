#ifndef Motor_h
#define Motor_h

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
    void onStepTimer();
    hw_timer_t *step_timer;
};

#endif

