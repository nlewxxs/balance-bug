#ifndef mpu_h
#define mpu_h

#include <Arduino.h>
#include <Wire.h>

class mpu6050 {
    public: 
        void init();
        void update();
        void calibrate();
        float getPitch();
        float getRoll();
        float getYaw();
        float getRawXAccel();
        float getRawYAccel();
        float getRawXGyro();
        float getRawYGyro();
        float getRawZGyro();
};

#endif
