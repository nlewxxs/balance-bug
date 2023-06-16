#include "Camera.h"
#include "I2Cdev.h"
#include "Wire.h"
#include "MPU6050_6Axis_MotionApps20.h"

MPU6050 mpu;  // default address 0x68 used

Camera D8M;
bool dmpReady = false;  // set true if DMP init was successful
// uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

Quaternion q;           // [w, x, y, z]         quaternion container
VectorFloat gravity;    // [x, y, z]            gravity vector
VectorInt16 gyro;       // [x, y, z]            angular velocity
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

void setup(){
  Serial.begin(115200);
  Serial.println("Starting camera");
  D8M.init();
  Serial.println("Started camera");

  // Wire.begin();
  // Wire.setClock(400000); // 400kHz I2C clock

  // Serial.begin(115200);
  // while (!Serial); // wait for serial monitor 

  // // initialize device
  // Serial.println(F("Initializing I2C devices..."));
  // mpu.initialize();

  // // verify connection
  // Serial.println(F("Testing device connections..."));
  // Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));

  // // load and configure the DMP
  // Serial.println(F("Initializing DMP..."));
  // devStatus = mpu.dmpInitialize();

  // // gyro offsets, I tried with these and it seems perfect. Tried the calibrate.ino sketch after and the 
  // // raw values were being consistently returned as 0
  // mpu.setXGyroOffset(220);
  // mpu.setYGyroOffset(76);
  // mpu.setZGyroOffset(-85);
  // mpu.setZAccelOffset(1788); // 1688 factory default for my test chip
  // // make sure it worked (returns 0 if so)
  // if (devStatus == 0) {
  // // Calibration Time: generate offsets and calibrate our MPU6050
  //   mpu.CalibrateAccel(6);
  //   mpu.CalibrateGyro(6);
  //   mpu.PrintActiveOffsets();
  //   // turn on the DMP, now that it's ready
  //   Serial.println(F("Enabling DMP..."));
  //   mpu.setDMPEnabled(true);
  //   dmpReady = true;
  // } else {
  //   // ERROR!
  //   // 1 = initial memory load failed
  //   // 2 = DMP configuration updates failed
  //   // (if it's going to break, usually the code will be 1)
  //   Serial.print(F("DMP Initialization failed (code "));
  //   Serial.print(devStatus);
  //   Serial.println(F(")"));
  // }
}

void loop(){

    // if (!dmpReady) return;  // hang program if programming did not work
    // // read a packet from FIFO
    // if (mpu.dmpGetCurrentFIFOPacket(fifoBuffer)) { 
    //   mpu.dmpGetQuaternion(&q, fifoBuffer);
    //   mpu.dmpGetGravity(&gravity, &q);
    //   mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
    // }

    // convertYaw(&ypr);

    // for (int i = -1000; i < 1000; i++){
    //   Serial.print("Yaw: ");
    //   Serial.println(convertYaw(i));
    // }

    D8M.update();
    Matrix frame = D8M.getBoxMatrix();
    char tmp[64];

    Serial.print(ypr[0]);
    Serial.print(",");
    
    for (int i = 4; i < 15; i++){
      sprintf(tmp, "%d,%d,%d,%d,", frame.boxes[i][0], frame.boxes[i][1], frame.boxes[i][2], frame.boxes[i][3]);
      Serial.print(tmp);
    }

    sprintf(tmp, "%d,%d,%d,%d", frame.boxes[15][0], frame.boxes[15][1], frame.boxes[15][2], frame.boxes[15][3]);
    Serial.println(tmp);

}

float convertYaw(float yaw){
  while (yaw < 0.0){
    yaw = yaw + 360.0;
  }
  while (yaw > 360.0){
    yaw = yaw - 360.0;
  }
  return yaw;
}
