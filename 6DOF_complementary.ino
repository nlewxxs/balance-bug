#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

// Timers
unsigned long timer = 0;
float timeStep = 0.007;

// pitch and roll accelerometer values
float pitch_a = 0;
float roll_a = 0;

// pitch, roll and yaw values from gyro
float pitch = 0;
float roll = 0;

// yaw is an ESTIMATE with mpu6050
// due to the absence of a magnetometer
float yaw = 0;

/* COMPLEMENTARY FILTER GAINS: 
this is how much we trust the reading from
the gyro and accelerometer respectively
*/
float gyroGain = 0.98; 
float accelGain = 0.02;

void setup(){
    Serial.begin(115200);

    //init mpu
    while (!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G)){
        Serial.println("la MPU no está hablando con el monitor serial");
        delay(500);
    }

    //calibrate the gyro, (must be at rest)
    mpu.CalibrateGyro();

    //leave commented for now
    //mpu.setThreshold();
}

void loop(){
    
    //read normalised values 
    Vector normGyro = mpu.readNormalizeGyro();
    Vector normAccel = mpu.readNormalizeAccel();

    //calculate pitch, roll, yaw from gyro
    pitch = pitch + normGyro.YAxis * timeStep;
    roll = roll + normGyroXAxis * timeStep;
    yaw = yaw + rawGyro.ZAxis * timeStep;

    // pitch and roll from accel
    pitch_a = -(atan2(normAccel.XAxis, sqrt(normAccel.YAxis*normAccel.YAxis + normAccel.ZAxis*normAccel.Zaxis))*180.0)/M_PI;
    roll_a = (atan2(normAccel.YAxis, normAccel.ZAxis)*180.0)/M_PI;

    // and now we weight these how we like
    pitch = gyroGain * (normGyro.YAxis * timeStep) + accelGain * pitch_a;
    roll = gyroGain * (normGyro.XAxis * timeStep) + accelGain * roll_a;

    // normalising the yaw
    yaw = (float(analogRead(1) - 512)) / 512;

    //output in csv format
    Serial.print(pitch);
    Serial.print(",");
    Serial.print(roll);
    Serial.print(",");
    Serial.println(yaw);
    
}










