#ifndef LDR_h
#define LDR_h

#include <Arduino.h>
#define LDR_THRESHOLD 0

class LDR {
  public:
    uint8_t pin;
    void init(uint8_t _pin);
    bool isWall();
  private:
    bool reading;
    void update();
};

#endif