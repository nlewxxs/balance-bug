#include <map>
#include <stack>
#include <vector>
#include <Wire.h>
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include <BluetoothSerial.h>
#include "Camera.h"
#include "Classify.h"
// #include "Traversal.h" // for the time being
#include "Communicate.h"
#include "SimpleTraversal.h"
#include "Controller.h"

// MACROS
// #define ENABLE_YAW_OUTPUT
#define ENABLE_HTTP_SERVER
#define ENABLE_BLUETOOTH
// #define ENABLE_CAMERA  
#define ENABLE_TRAVERSAL
#define ENABLE_MOTORS
#define ENABLE_CAMERA
// #define ENABLE_TRIANGULATE
// //#define XDIST 100
// // #define YDIST 100
// // #define ERRMARGIN 5

// how much we move forwards before taking action in centimetres
#define MOVE_THEN_DIR_OFFSET 30.0 

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> //
// ----------------- CORE 0 DEFINITIONS ----------------- //

BluetoothSerial SerialBT;
Camera D8M;
Controller controller;

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! please run 'make menuconfig' to enable it
#endif

SimpleTraversal traversal;
classifyElement classification;
String bugId =  "MazEERunnEEr";

TaskHandle_t communication;  // task on core 0 for communication
Communicate communicate;

#define CHECK_NEW_SESSION_TIMEOUT 20

char* ssid = "JJ's Galaxy S22+";
char* password = "sbsx6554";
char* serverId = "http://54.152.2.20:8081";

// String serverName = "http://90.196.3.86:8081";  // local ip of the backend host (NOT localhost)
// unsigned long lastTime = 0;
// unsigned long timerDelay = 5000;

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< //

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> //
// ----------------- CORE 1 DEFINITIONS ----------------- //

MPU6050 mpu;
bool dmpReady = false;  
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

Quaternion q;           // [w, x, y, z]         quaternion container
VectorFloat gravity;    // [x, y, z]            gravity vector
VectorInt16 gyro;
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector
bool moved = false;
bool decisionMade = false;
bool movingRight = false;
bool movingLeft = false;

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< /

template <typename T> 
void debugOutput(T o, bool newline = true) {
  #ifdef ENABLE_BLUETOOTH
    if (newline) { SerialBT.println(o); } else { SerialBT.print(o); }
  #else 
    if (newline) { Serial.println(o); } else { Serial.print(o); }
  #endif
}

int n_overflows = 0;  // keeps track of how many times we've crossed that boundary so far.
float yawHistory[2] = {0, 0};

void setup() {

  #ifdef ENABLE_CAMERA
    D8M.init();
  #endif

  #ifdef ENABLE_BLUETOOTH
    SerialBT.begin("MZRNR");
  #else 
    Serial.begin(115200);
  #endif

  Wire.begin();
  Wire.setClock(400000); // 400kHz I2C clock

  // initialize device
  mpu.initialize();
  debugOutput("Testing device connections...");
  debugOutput(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));

  // load and configure the DMP
  debugOutput("Initializing DMP...");
  devStatus = mpu.dmpInitialize();

  // mpu offsets
  mpu.setXGyroOffset(220);
  mpu.setYGyroOffset(76);
  mpu.setZGyroOffset(-85);
  mpu.setZAccelOffset(1788);

  // make sure it worked (returns 0 if so)
  if (devStatus == 0) {
      // Calibration Time: generate offsets and calibrate our MPU6050
      mpu.CalibrateAccel(6);
      mpu.CalibrateGyro(6);
      mpu.PrintActiveOffsets();
      // turn on the DMP, now that it's ready

      debugOutput("Enabling DMP...");

      mpu.setDMPEnabled(true);
      dmpReady = true;

  } else {
      // ERROR!
    debugOutput((F("DMP Initialization failed (code ")));
    (devStatus);
    debugOutput(F(")"));
  }

  controller.setup();

  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
    communicationCode, // Task function.
    "communication",   // name of task.
    20000,             // Stack size of task 
    NULL,              // parameter of the task, not 
    1,                 // priority of the task 
    &communication,    // Task handle to keep track of created task 
    0);                // pin task to core 0 -- by default we pin to core 1 
  
  delay(500);
}


void loop() {

  // -------- READ FROM IMU ---------- // 

  if (mpu.dmpGetCurrentFIFOPacket(fifoBuffer)){
    mpu.dmpGetQuaternion(&q, fifoBuffer);
    mpu.dmpGetGravity(&gravity, &q);
    mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
    mpu.dmpGetGyro(&gyro, fifoBuffer);
    yawHistory[0] = yawHistory[1];
    yawHistory[1] = ypr[0] * 180/M_PI;
    calculateAdjustment();
    ypr[0] = applyAdjustment(yawHistory[1]) / (180/M_PI);
  }

  #ifdef ENABLE_TRAVERSAL
  if (!controller.getMoving()) {
    // SerialBT.print("clear: ");
    // SerialBT.println(classification.isClear);
    if (movingRight && !classification.isPath) {
      rotate(5);
      for (int g = 0; g < 10; g++){
        D8M.update();
      }
      Matrix frame = D8M.getBoxMatrix();
      char tmp[128];

      Image newImage;   
      classification = newImage.classify(frame.boxes);
      // newImage.debugInfo();
      debugOutput(newImage.printInfo());
    } else if (movingLeft && !classification.isPath) {
      rotate(-5);
      for (int g = 0; g < 10; g++){
        D8M.update();
      }
      Matrix frame = D8M.getBoxMatrix();
      char tmp[128];

      Image newImage;   
      classification = newImage.classify(frame.boxes);
      newImage.debugInfo();
    } else if (decisionMade) {
      switch (traversal.getDecision()){
        debugOutput(traversal.getDecision());
        case Stationary:
          break;
        case Forward:
          move(0.1);
          break;
        case Left:
          rotate(-5);
          break;
        case Right:
          rotate(5);
          break;
        case Backwards:
          move(-0.1);
          break;
        case MoveThenLeft:
          moveDir(0.5, false);
          break;
        case MoveThenRight:
          moveDir(0.5, true);
          break;
        case DeadEnd:
          moveDir(0, true);
        default:
          break;
      }
      decisionMade = false;
    }
    if (movingRight && classification.isPath) {
      movingRight = false;
      // move(0.5);
    }
    if (movingLeft && classification.isPath) {
      movingLeft = false;
      // move(0.5);
    }
  }
  controller.update(ypr[0] * 180/M_PI);
  #endif

  vTaskDelay(10);
}

void move(float amount) {
  controller.updatePositionSetpoint(controller.getDistance() + amount);
}

void rotate(float amount) {
  float rotationSet = ypr[0] * 180/M_PI + amount;
  controller.updateHeadingSetpoint(rotationSet);
}

void moveDir(float distance, bool isRight) {
  controller.updatePositionSetpoint(controller.getDistance() + distance);
  float rotationSet = ypr[0] * 180/M_PI + (isRight ? 20 : -20);
  controller.setNextHeadingSetpoint(rotationSet);
  if (isRight) {
    movingRight = true;
  } else {
    movingLeft = true;
  }
}

// float convertYaw(float yaw){
//   while (yaw < 0.0){
//     yaw = yaw + 360.0;
//   }
//   while (yaw > 360.0){
//     yaw = yaw - 360.0;
//   }
//   return yaw;
// }

void communicationCode(void* pvParameters) {
  // Serial.println(xPortGetCoreID());
  
  #ifdef ENABLE_HTTP_SERVER
    //wifi setup
  // if (!communicate.getInitialised()){
  //   communicate.init("", "", "http://90.196.3.86:8081", bugId, CHECK_NEW_SESSION_TIMEOUT);
  // }
    traversal.init(ssid, password, serverId, "MazEERunnEEr", CHECK_NEW_SESSION_TIMEOUT);
    debugOutput("traversal init");
    
  #endif

  // looping code - this takes up entirety of cpu time along with controller so NEEDS the delay to allow idle tasks to execute
  for (;;) {
      // -------- OUTPUTS ---------- //

    // #ifdef ENABLE_YAW_OUTPUT
    //   debugOutput("Yaw: ", false);
    //   debugOutput(ypr[0], false);
    // #endif
    #ifdef ENABLE_HTTP_SERVER
      if (!traversal.communicate.getInSession()){
        traversal.communicate.ping();
        traversal.communicate.checkNewSession();
        debugOutput("stuck in pinging");
        vTaskDelay(100);
      } else if(!traversal.communicate.getStatusMapSetup()){
        traversal.communicate.setUpMap();
        debugOutput("traversal state 2");
      }
      else{
        Serial.println("All set up");
      }
    #endif

    #ifdef ENABLE_TRAVERSAL
      if(!controller.getMoving() && !movingRight && !movingLeft) {
        for (int g = 0; g < 10; g++){
          D8M.update();
        }
        Matrix frame = D8M.getBoxMatrix();
        char tmp[128];
      
        // for (int i = 0; i < 11; i++){
        //   sprintf(tmp, "%d,%d,%d,%d,", frame.boxes[i][0], frame.boxes[i][1], frame.boxes[i][2], frame.boxes[i][3]);
        //   debugOutput(tmp, false);
        // }
        // sprintf(tmp, "%d,%d,%d,%d", frame.boxes[12][0], frame.boxes[12][1], frame.boxes[12][2], frame.boxes[12][3]);
        // debugOutput(tmp);

        Image newImage;   
        classification = newImage.classify(frame.boxes);
        newImage.debugInfo();
        debugOutput(newImage.printInfo());
      
        traversal.makeDecision(classification.isEnd, classification.isNode, 
                        classification.isPath, classification.isClear, 
                        classification.leftWall, classification.rightWall, 
                        classification.leftTurn, classification.rightTurn);
        decisionMade = true;
        debugOutput("made a decision: ");
        debugOutput(traversal.getDecision());

        char newtmp[128];
        sprintf(newtmp, "Box index 6: %d, %d, %d, %d \t Box index 7: %d, %d, %d, %d", frame.boxes[6][0], frame.boxes[6][1], frame.boxes[6][2], frame.boxes[6][3],
                      frame.boxes[7][0], frame.boxes[7][1], frame.boxes[7][2], frame.boxes[7][3]);
        SerialBT.println(newtmp);

        if (SerialBT.available()) {
          String test = SerialBT.readString();
          if (test.substring(0,1) == "F") {
            move(test.substring(1,test.length()-1).toFloat());
          } else if (test.substring(0,1) == "L") {
            rotate(-test.substring(1,test.length()-1).toFloat());
          } else if (test.substring(0,1) == "R") {
            rotate(test.substring(1,test.length()-1).toFloat());
          } else if (test.substring(0,1) == "B") {
            move(-test.substring(1,test.length()-1).toFloat());
          } else if (test.substring(0,2) == "ML") {
            moveDir(0.1, false);
          } else if (test.substring(0,2) == "MR") {
            moveDir(0.1, true);
          }
        }
      }
    #endif

    // SerialBT.print("RRR");
    // SerialBT.print(controller.getDistance());
    // SerialBT.print(", ");
    // SerialBT.print(controller.getPositionSetpoint());
    // SerialBT.print(", ");
    // SerialBT.print(ypr[0] * 180/M_PI);
    // SerialBT.print(", ");
    // SerialBT.print(controller.getLeftOutput());
    // SerialBT.print(", HS:");
    // SerialBT.print(controller.getHeadingSetpoint());
    //SerialBT.print(", im:");
    //SerialBT.println(newImage.printInfo());

    vTaskDelay(100);
  }
}

void calculateAdjustment(){
  if ((yawHistory[0] < -150) && (yawHistory[1] > 150)) { // crossed over from -180 to 180;
    n_overflows -= 1; // we need to subtract 360
  } else if ((yawHistory[0] > 150) && (yawHistory[1] < -150)) { // crossed from 180 to -180
    n_overflows += 1; // we need to add 360
  }
}

float applyAdjustment(float raw){
  return raw + (n_overflows * 360.0);
}