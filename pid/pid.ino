#include <Wire.h>

const int mpu_addy = 0x68;  // designated mpu_addy I2C address
float AccX, AccY, AccZ;
float GyroX, GyroY, GyroZ;  
float accAngleX, accAngleY, gyroAngleX, gyroAngleY, gyroAngleZ;
float roll, pitch, yaw;
float AccErrorX, AccErrorY, GyroErrorX, GyroErrorY, GyroErrorZ;
float elapsedTime, currentTime, previousTime;

int calibrate = 0;  // calibration counter 

void setup() {

  Serial.begin(19200);
  Wire.begin();                      
  Wire.beginTransmission(mpu_addy);  
  Wire.write(0x6B);                  // Talk to the register 6B
  Wire.write(0x00);                  // placing 0 in the PWR_MGMT_1 register (wakes mpu)
  Wire.endTransmission(true);        //end the transmission

  Serial.println("la mpu esta hablando con el monitor serial!!!");

  //-------------------- PUT PID SETUP HERE ----------------------//



  //--------------------------------------------------------------//


  /*
  // Configure Accelerometer Sensitivity - Full Scale Range (default +/- 2g)
  Wire.beginTransmission(mpu_addy);
  Wire.write(0x1C);                  //Talk to the ACCEL_CONFIG register (1C hex)
  Wire.write(0x10);                  //Set the register bits as 00010000 (+/- 8g full scale range)
  Wire.endTransmission(true);
  // Configure Gyro Sensitivity - Full Scale Range (default +/- 250deg/s)
  Wire.beginTransmission(mpu_addy);
  Wire.write(0x1B);                   // Talk to the GYRO_CONFIG register (1B hex)
  Wire.write(0x10);                   // Set the register bits as 00010000 (1000deg/s full scale)
  Wire.endTransmission(true);
  delay(20);
  */
  // Call this function if you need to get the IMU error values for your module

  calculate_IMU_error();
  delay(20);

}

void loop() {
  
  // reading accelerometer data
  Wire.beginTransmission(mpu_addy);
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
  
  // Print the values on the serial monitor
  Serial.print(roll);
  Serial.print("/");
  Serial.print(pitch);
  Serial.print("/");
  Serial.println(yaw);

  //-------------------- PUT PID LOOP HERE ----------------------//



  //--------------------------------------------------------------//

}

void calculate_IMU_error() {

  while (calibrate < 200) {

    Wire.beginTransmission(mpu_addy);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(mpu_addy, 6, true);

    AccX = (Wire.read() << 8 | Wire.read()) / 16384.0 ;
    AccY = (Wire.read() << 8 | Wire.read()) / 16384.0 ;
    AccZ = (Wire.read() << 8 | Wire.read()) / 16384.0 ;

    // Sum all readings
    AccErrorX = AccErrorX + ((atan((AccY) / sqrt(pow((AccX), 2) + pow((AccZ), 2))) * 180 / PI));
    AccErrorY = AccErrorY + ((atan(-1 * (AccX) / sqrt(pow((AccY), 2) + pow((AccZ), 2))) * 180 / PI));
    calibrate++;
  }
  
  //average out over 200 cycles above
  AccErrorX = AccErrorX / calibrate;
  AccErrorY = AccErrorY / calibrate;
  calibrate = 0;
  
  while (calibrate < 200) {

    Wire.beginTransmission(mpu_addy);
    Wire.write(0x43);
    Wire.endTransmission(false);
    Wire.requestFrom(mpu_addy, 6, true);

    GyroX = Wire.read() << 8 | Wire.read();
    GyroY = Wire.read() << 8 | Wire.read();
    GyroZ = Wire.read() << 8 | Wire.read();

    // Sum all readings
    GyroErrorX = GyroErrorX + (GyroX / 131.0);
    GyroErrorY = GyroErrorY + (GyroY / 131.0);
    GyroErrorZ = GyroErrorZ + (GyroZ / 131.0);
    calibrate++;
  }

  GyroErrorX = GyroErrorX / calibrate;
  GyroErrorY = GyroErrorY / calibrate;
  GyroErrorZ = GyroErrorZ / calibrate;

  // Serial.print("AccErrorX: ");
  // Serial.println(AccErrorX);
  // Serial.print("AccErrorY: ");
  // Serial.println(AccErrorY);
  // Serial.print("GyroErrorX: ");
  // Serial.println(GyroErrorX);
  // Serial.print("GyroErrorY: ");
  // Serial.println(GyroErrorY);
  // Serial.print("GyroErrorZ: ");
  // Serial.println(GyroErrorZ);
}