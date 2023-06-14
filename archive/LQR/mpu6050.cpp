#include "Arduino.h"
#include "mpu6050.h"
#include "Wire.h"

#define calibration_span 200

double AccX, AccY, AccZ;
double AccErrorX, AccErrorY = 0;

double GyroX, GyroY, GyroZ;
double GyroErrorX, GyroErrorY, GyroErrorZ = 0;

int8_t gyrox1, gyrox2, gyroy1, gyroy2, gyroz1, gyroz2;
int8_t accx1, accx2, accy1, accy2, accz1, accz2;

double pitch, roll, yaw;

const int mpu_addr = 0x68;

unsigned long previousTime, currentTime, elapsedTimeTemp = 0;
double elapsedTime;

double accAngleX, accAngleY;
double gyroAngleX, gyroAngleY;

void mpu6050::init(){
    Wire.begin();
    Wire.setBufferSize(128);
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

        accx1 = Wire.read();
        accx2 = Wire.read();
        accy1 = Wire.read();
        accy2 = Wire.read();
        accz1 = Wire.read();
        accz2 = Wire.read();

        AccX = (double) (((accx1 << 8) | accx2) / 16384.0);
        AccY = (double) (((accy1 << 8) | accy2) / 16384.0);
        AccZ = (double) (((accz1 << 8) | accz2) / 16384.0);

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
        
        gyrox1 = Wire.read(); 
        gyrox2 = Wire.read();
        gyroy1 = Wire.read();
        gyroy2 = Wire.read();
        gyroz1 = Wire.read();
        gyroz2 = Wire.read();

        GyroX = (double) (((gyrox1 << 8) | gyrox2) / 131.0);
        GyroY = (double) (((gyroy1 << 8) | gyroy2) / 131.0);
        GyroZ = (double) (((gyroz1 << 8) | gyroz2) / 131.0);

        GyroErrorX = GyroX + GyroErrorX; 
        GyroErrorY = GyroY + GyroErrorY; 
        GyroErrorZ = GyroZ + GyroErrorZ;
        c++;
    }

    GyroErrorX = GyroErrorX / calibration_span;
    GyroErrorY = GyroErrorY / calibration_span;
    GyroErrorZ = GyroErrorZ / calibration_span;
    // c++;
}

void mpu6050::update(){

    Wire.beginTransmission(mpu_addr);
    Wire.write(0x3B);                     
    Wire.endTransmission(false);          
    Wire.requestFrom(mpu_addr, 6, 1);

    accx1 = Wire.read();
    accx2 = Wire.read();
    accy1 = Wire.read();
    accy2 = Wire.read();
    accz1 = Wire.read();
    accz2 = Wire.read();

    AccX = (double) (((accx1 << 8) | accx2) / 16384.0);
    AccY = (double) (((accy1 << 8) | accy2) / 16384.0);
    AccZ = (double) (((accz1 << 8) | accz2) / 16384.0);

    // Calculating Roll and Pitch from the accelerometer data
    accAngleX = (atan(AccY / sqrt(pow(AccX, 2) + pow(AccZ, 2))) * 180 / PI) - AccErrorX;
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
    gyrox1 = Wire.read(); 
    gyrox2 = Wire.read();
    gyroy1 = Wire.read();
    gyroy2 = Wire.read();
    gyroz1 = Wire.read();
    gyroz2 = Wire.read();

    GyroX = (double) (((gyrox1 << 8) | gyrox2) / 131.0);
    GyroY = (double) (((gyroy1 << 8) | gyroy2) / 131.0);
    GyroZ = (double) (((gyroz1 << 8) | gyroz2) / 131.0);

    // Correct the outputs with the calculated error values
    GyroX = GyroX - GyroErrorX; 
    GyroY = GyroY - GyroErrorY; 
    GyroZ = GyroZ - GyroErrorZ; 

    gyroAngleX = gyroAngleX + GyroX * elapsedTime; 
    gyroAngleY = gyroAngleY + GyroY * elapsedTime;
    yaw = (double) yaw + (double) (GyroZ * elapsedTime);

    // Complementary filter
    roll = 0.98 * (double) gyroAngleX + 0.02 * (double) accAngleX;
    pitch = 0.98 * (double) gyroAngleY + 0.02 * (double)accAngleY;
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