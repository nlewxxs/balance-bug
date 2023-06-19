#ifndef Camera_h
#define Camera_h

#include <Arduino.h>
#include <Wire.h>
#include <HardwareSerial.h>

struct Matrix {
    uint32_t boxes[12][4];
};

struct coordinate {
    uint16_t x_min;
    uint16_t y_min;
    uint16_t x_max;
    uint16_t y_max;
};

struct boxCoordinates {
    coordinate block_one;
    coordinate block_two;
    coordinate block_three;
    coordinate block_four;
    coordinate block_five;
    coordinate block_six;
    coordinate block_seven;
    coordinate block_eight;
    coordinate block_nine;
    coordinate block_ten;
    coordinate block_eleven;
    coordinate block_twelve;
};

class Camera {
    public: 
        void init();
        void update();
        Matrix getBoxMatrix();
        uint8_t getColours();
    private:
        bool initialised;
};

#endif
