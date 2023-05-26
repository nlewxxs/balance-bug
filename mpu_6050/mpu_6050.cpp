#include "Arduino.h"
#include "mpu_6050.h"
#include "Wire.h"

#define mpu_addr 0x68
#define calibration_span 200

double AccX, AccY, AccZ;
double AccErrorX, AccErrorY = 0;

double GyroX, GyroY, GyroZ;
double GyroErrorX, GyroErrorY, GyroErrorZ = 0;

double pitch, roll, yaw;

mpu6050::mpu6050(){
    Wire.begin();
    Wire.beginTransmission(mpu_addr);
    Wire.write(0x6B) // writing to PWR_MGMT_1 register
    Wire.write(0x00); // set to low (wakes mpu)
    Wire.endTransmission(true);
}

void mpu6050::calibrate(){

    while (int c = calibration_span; c < 200; c++){

        Wire.beginTransmission(mpu_addr);
        Wire.write(0x3B);
        Wire.endTransmission(false);
        Wire.requestFrom(mpu_addr, 6, true);

        AccX = ((Wire.read() << 8) | Wire.read()) / 16384.0;
        AccY = ((Wire.read() << 8) | Wire.read()) / 16384.0;
        AccZ = ((Wire.read() << 8) | Wire.read()) / 16384.0;

        AccErrorX = AccErrorX + ((atan((AccY) / sqrt(pow((AccX), 2) + pow((AccZ), 2))) * 180 / PI));
        AccErrorY = AccErrorY + ((atan(-1 * (AccX) / sqrt(pow((AccY), 2) + pow((AccZ), 2))) * 180 / PI));

    }

    AccErrorX = AccErrorX / calibration_span;
    AccErrorY = AccErrorY / calibration_span;

    while (int c = calibration_span; c < 200; c++){
        
        Wire.beginTransmission(mpu_addr);
        Wire.write(0x43);
        Wire.endTransmission(false);
        Wire.requestFrom(mpu_addr, 6, true);

        GyroX = Wire.read() << 8 | Wire.read();
        GyroY = Wire.read() << 8 | Wire.read();
        GyroZ = Wire.read() << 8 | Wire.read();

        GyroErrorX = GyroErrorX + (GyroX / 131.0);
        GyroErrorY = GyroErrorY + (GyroY / 131.0);
        GyroErrorZ = GyroErrorZ + (GyroZ / 131.0);
    }

    GyroErrorX = GyroErrorX / calibration_span;
    GyroErrorY = GyroErrorY / calibration_span;
    GyroErrorZ = GyroErrorZ / calibration_span;
}

void mpu6050::update(){

    double accAngleX, accAngleY;

    Wire.beginTransmission(mpu_addr);
    Wire.write(0x3B);                     
    Wire.endTransmission(false);          
    Wire.requestFrom(mpu_addy, 6, true);

    // normalising
    // for a range of +-2g, we need to divide the raw values by 16384, according to the datasheet
    AccX = (Wire.read() << 8 | Wire.read()) / 16384.0; // X-axis value
    AccY = (Wire.read() << 8 | Wire.read()) / 16384.0; // Y-axis value
    AccZ = (Wire.read() << 8 | Wire.read()) / 16384.0; // Z-axis value

    // Calculating Roll and Pitch from the accelerometer data
    accAngleX = (atan(AccY / sqrt(pow(AccX, 2) + pow(AccZ, 2))) * 180 / PI) - AccErrorX;
    accAngleY = (atan(-1 * AccX / sqrt(pow(AccY, 2) + pow(AccZ, 2))) * 180 / PI) - AccErrorY;

    // reading gyro data 
    previousTime = currentTime;                         
    currentTime = millis();                             
    elapsedTime = (currentTime - previousTime) / 1000;  

    Wire.beginTransmission(mpu_addy);
    Wire.write(0x43); 
    Wire.endTransmission(false);
    Wire.requestFrom(mpu_addy, 6, true); 

    // For a 250deg/s range we have to divide first the raw value by 131.0, according to the datasheet
    GyroX = (Wire.read() << 8 | Wire.read()) / 131.0; 
    GyroY = (Wire.read() << 8 | Wire.read()) / 131.0;
    GyroZ = (Wire.read() << 8 | Wire.read()) / 131.0;

    // Correct the outputs with the calculated error values
    GyroX = GyroX - GyroErrorX; 
    GyroY = GyroY - GyroErrorY; 
    GyroZ = GyroZ - GyroErrorZ; 

    gyroAngleX = gyroAngleX + GyroX * elapsedTime; 
    gyroAngleY = gyroAngleY + GyroY * elapsedTime;
    yaw =  yaw + GyroZ * elapsedTime;

    // Complementary filter
    roll = 0.98 * gyroAngleX + 0.02 * accAngleX;
    pitch = 0.98 * gyroAngleY + 0.02 * accAngleY;
}

double mpu6050::getPitch(){
    return pitch;
}

double mpu6050::getRoll(){
    return roll;
}

double mpu6050::getYaw(){
    return yaw;
}

double mpu6050::getRawXAccel(){
    return AccX;
}

double mpu6050::getRawYAccel(){
    return AccY;
}

double mpu6050::getRawXGyro(){
    return GyroX;
}

double mpu6050::getRawYGyro(){
    return GyroY;
}

double mpu6050::getRawZGyro(){
    return GyroZ;
}
