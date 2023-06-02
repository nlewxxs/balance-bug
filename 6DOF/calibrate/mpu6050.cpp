#include "Arduino.h"
#include "mpu6050.h"
#include "Wire.h"

#define calibration_span 800

float AccX, AccY, AccZ;
float AccErrorX, AccErrorY = 0;

float GyroX, GyroY, GyroZ;
float GyroErrorX, GyroErrorY, GyroErrorZ = 0;

float pitch, roll, yaw;

const int mpu_addr = 0x68;

unsigned long previousTime, currentTime, elapsedTimeTemp = 0;
float elapsedTime;

float accAngleX, accAngleY;
float gyroAngleX, gyroAngleY;

void mpu6050::init(){
    Wire.begin();
    Wire.beginTransmission(mpu_addr);
    Wire.write(0x6B); // writing to PWR_MGMT_1 register
    Wire.write(0x00); // set to low (wakes mpu)
    Wire.endTransmission(true);
}

void mpu6050::calibrate() {

    int c = 0;

    while (c < calibration_span) {

        Wire.beginTransmission(mpu_addr);
        Wire.write(0x3B);
        Wire.endTransmission(false);
        Wire.requestFrom(mpu_addr, 6, 1);

        AccX = ((((int8_t) Wire.read()) << 8) | (int8_t) Wire.read()) / 16384.0;
        AccY = ((((int8_t) Wire.read()) << 8) | (int8_t) Wire.read()) / 16384.0;
        AccZ = ((((int8_t) Wire.read()) << 8) | (int8_t) Wire.read()) / 16384.0;

        AccErrorX = AccErrorX + ((atan((AccY) / sqrt(pow((AccX), 2) + pow((AccZ), 2))) * 180 / PI));
        AccErrorY = AccErrorY + ((atan(-1 * (AccX) / sqrt(pow((AccY), 2) + pow((AccZ), 2))) * 180 / PI));
        c++;
    }

    AccErrorX = AccErrorX / calibration_span;
    AccErrorY = AccErrorY / calibration_span;
    c = 0;

    while (c < calibration_span) {
        
        Wire.beginTransmission(mpu_addr);
        Wire.write(0x43);
        Wire.endTransmission(false);
        Wire.requestFrom(mpu_addr, 6, 1);

        GyroX = ((((int8_t) Wire.read()) << 8) | (int8_t) Wire.read()) / 131.0;
        GyroY = ((((int8_t) Wire.read()) << 8) | (int8_t) Wire.read()) / 131.0;
        GyroZ = ((((int8_t) Wire.read()) << 8) | (int8_t) Wire.read()) / 131.0;

        GyroErrorX = GyroX + GyroErrorX; 
        GyroErrorY = GyroY + GyroErrorY; 
        GyroErrorZ = GyroZ + GyroErrorZ;
        c++;
    }

    GyroErrorX = GyroErrorX / calibration_span;
    GyroErrorY = GyroErrorY / calibration_span;
    GyroErrorZ = GyroErrorZ / calibration_span;
}

void mpu6050::update(){

    Wire.beginTransmission(mpu_addr);
    Wire.write(0x3B);                     
    Wire.endTransmission(false);          
    Wire.requestFrom(mpu_addr, 6, 1);

    Wire.read();
    Wire.read();
    // AccX = ((((int8_t) Wire.read()) << 8) | (int8_t) Wire.read()) / 16384.0;
    AccY = ((((int8_t) Wire.read()) << 8) | (int8_t) Wire.read()) / 16384.0;
    AccZ = ((((int8_t) Wire.read()) << 8) | (int8_t) Wire.read()) / 16384.0;

    // Calculating Roll and Pitch from the accelerometer data
    // accAngleX = (atan(AccY / sqrt(pow(AccX, 2) + pow(AccZ, 2))) * 180 / PI) - AccErrorX;
    accAngleY = (atan(-1 * AccX / sqrt(pow(AccY, 2) + pow(AccZ, 2))) * 180 / PI) - AccErrorY;

    // reading gyro data 
    previousTime = currentTime;
    currentTime = millis();
    // elapsedTime = (currentTime - previousTime) / 1000;
    elapsedTimeTemp = (currentTime - previousTime);
    elapsedTime = elapsedTimeTemp / 1000.0;

    Wire.beginTransmission(mpu_addr);
    Wire.write(0x43); 
    Wire.endTransmission(false);
    Wire.requestFrom(mpu_addr, 6, 1); 

    // For a 250deg/s range we have to divide first the raw value by 131.0, according to the datasheet

    // Wire.read();
    // Wire.read();
    GyroX = ((((int8_t) Wire.read()) << 8) | (int8_t) Wire.read()) / 131.0;
    GyroY = ((((int8_t) Wire.read()) << 8) | (int8_t) Wire.read()) / 131.0;
    GyroZ = ((((int8_t) Wire.read()) << 8) | (int8_t) Wire.read()) / 131.0;
    // Correct the outputs with the calculated error values
    // GyroX = GyroX - GyroErrorX; 
    // GyroY = GyroY - GyroErrorY; 
    // GyroZ = GyroZ - GyroErrorZ; 

    // gyroAngleX = gyroAngleX + GyroX * elapsedTime; 
    gyroAngleY = gyroAngleY + GyroY * elapsedTime;
    yaw = (yaw + (GyroZ * elapsedTime));

    // Complementary filter
    // roll = 0.98 * gyroAngleX + 0.02 * accAngleX;
    pitch = 0.98 * gyroAngleY + 0.02 * accAngleY;
}

float mpu6050::getPitch(){
    return pitch;
}

float mpu6050::getRoll(){
    return roll;
}

float mpu6050::getYaw(){
    return yaw;
}

float mpu6050::getRawXAccel(){
    return AccX;
}

float mpu6050::getRawYAccel(){
    return AccY;
}

float mpu6050::getRawXGyro(){
    return GyroX;
}

float mpu6050::getRawYGyro(){
    return GyroY;
}

float mpu6050::getRawZGyro(){
    return GyroZ;
}

float mpu6050::getGyroErrorX(){
  return GyroErrorX;
}

float mpu6050::getGyroErrorY(){
  return GyroErrorY;
}

float mpu6050::getGyroErrorZ(){
  return GyroErrorZ;
}
