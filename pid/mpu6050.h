#ifndef mpu_h
#define mpu_h

#include <Arduino.h>
#include <Wire.h>

class mpu6050 {
    public: 
        void init();
        void update();
        void calibrate();
        double getPitch();
        double getRoll();
        double getYaw();
        unsigned long getElapsedTime();
        double getRawXAccel();
        double getRawYAccel();
        double getRawXGyro();
        double getRawYGyro();
        double getRawZGyro();
};

#endif
