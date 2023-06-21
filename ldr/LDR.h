#ifndef LDR_h
#define LDR_h

#include <Arduino.h>
#define LDR_THRESHOLD 800

class LDR {
  public:
    uint8_t pin;
    void init(uint8_t _pin);
    bool isWall();
    int getReading();
  private:
    int reading;
    void update();
};

#endif