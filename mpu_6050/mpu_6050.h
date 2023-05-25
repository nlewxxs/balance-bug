#ifndef mpu_h
#define mpu_h

#include <Arduino.h>
#include <Wire.h>

class mpu6050 {
    public: 
        void update();
        void calibrate();
        uint8_t calibration_length;
    private: 
        double AccErrorX;
        double AccErrorY;
        double GyroErrorX;
        double GyroErrorY;
        double GyroErrorZ;
        void getPitch();
        void getRoll();
        void getYaw();
        void getRawXAccel();
        void getRawYAccel();
        void getRawXGyro();
        void getRawYGyro();
        void getRawZGyro();
};

#endif