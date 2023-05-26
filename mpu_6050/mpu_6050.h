#ifndef mpu_h
#define mpu_h

#include <Arduino.h>
#include <Wire.h>

class mpu6050 {
    public: 
        void update();
        void calibrate();
    private: 
        double AccErrorX;
        double AccErrorY;
        double GyroErrorX;
        double GyroErrorY;
        double GyroErrorZ;
        double getPitch();
        double getRoll();
        double getYaw();
        double getRawXAccel();
        double getRawYAccel();
        double getRawXGyro();
        double getRawYGyro();
        double getRawZGyro();
};

#endif