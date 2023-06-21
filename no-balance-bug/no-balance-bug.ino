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
// #define ENABLE_HTTP_SERVER
<<<<<<< Updated upstream
#define ENABLE_BLUETOOTH
// #define ENABLE_CAMERA
// #define ENABLE_TRAVERSAL
#define ENABLE_MOTORS
=======
// #define ENABLE_BLUETOOTH
#define ENABLE_CAMERA
// #define ENABLE_MOTORS
>>>>>>> Stashed changes
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

SimpleTraversal traversal;
String bugId =  "MazEERunnEEr";

TaskHandle_t communication;  // task on core 0 for communication
Communicate communicate;

#define CHECK_NEW_SESSION_TIMEOUT 20

char* ssid = "Ben";
char* password = "test1234";
char* serverId = "http://90.196.3.86:8081";
char* serverId = "http://44.203.160.76:8081";

// String serverName = "http://90.196.3.86:8081";  // local ip of the backend host (NOT localhost)
unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

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

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< /

template <typename T> 
void debugOutput(T o, bool newline = true) {
  #ifdef ENABLE_BLUETOOTH
    if (newline) { SerialBT.println(o); } else { SerialBT.print(o); }
  #else 
    if (newline) { Serial.println(o); } else { Serial.print(o); }
  #endif
}

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
  }

  #ifdef ENABLE_CAMERA
    D8M.update();
    Matrix frame = D8M.getBoxMatrix();
    char tmp[128];
  
    for (int i = 0; i < 11; i++){
      sprintf(tmp, "%d,%d,%d,%d,", frame.boxes[i][0], frame.boxes[i][1], frame.boxes[i][2], frame.boxes[i][3]);
      debugOutput(tmp, false);
    }
    sprintf(tmp, "%d,%d,%d,%d", frame.boxes[12][0], frame.boxes[12][1], frame.boxes[12][2], frame.boxes[12][3]);
    debugOutput(tmp);

    // Image newImage;   
    // classifyElement classification = newImage.classify(frame.boxes);
    // newImage.debugInfo();

    #ifdef ENABLE_TRAVERSAL
    if (!controller.getMoving()) {
      traversal.makeDecision(classification.isEnd, classification.isNode, 
                          classification.isPath, classification.isClear, 
                          classification.leftWall, classification.rightWall, 
                          classification.leftTurn, classification.rightTurn);
      switch (traversal.getDecision()){
        case Stationary:
          break;
        case Forward:
          move(0.05);
          break;
        case Left:
          rotate(5);
          break;
        case Right:
          rotate(-5);
          break;
        case Backwards:
          move(-0.05);
          break;
        case MoveThenLeft:
          move(0.05);   
          break;
        case MoveThenRight:
          move(0.05);
          break;
        default:
          break;
      }
    }
    controller.update(-ypr[0]*180/M_PI);
    move();
    #endif
  #endif

  vTaskDelay(10);
}

int getDistance(){
  return 400; // arbitrary, testing purposes
}

void move(float amount) {
  controller.updatePositionSetpoint(amount);
}

void rotate(float amount) {
  controller.updateHeadingSetpoint(amount);
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

void communicationCode(void* pvParameters) {
  // Serial.println(xPortGetCoreID());

  #ifdef ENABLE_HTTP_SERVER
    //wifi setup
  // if (!communicate.getInitialised()){
  //   communicate.init("", "", "http://90.196.3.86:8081", bugId, CHECK_NEW_SESSION_TIMEOUT);
  // }
    traversal.init(ssid, password, serverId, "MazEERunnEEr", CHECK_NEW_SESSION_TIMEOUT);
    
  #endif

  // looping code - this takes up entirety of cpu time along with controller so NEEDS the delay to allow idle tasks to execute
  for (;;) {
    

    // -------- OUTPUTS ---------- //

    #ifdef ENABLE_YAW_OUTPUT
      debugOutput("Yaw: ", false);
      debugOutput(ypr[0], false);
    #endif
    #ifdef ENABLE_HTTP_SERVER
      if (!traversal.communicate.getInSession()){
        traversal.communicate.ping();
        traversal.communicate.checkNewSession();
        vTaskDelay(100);
      } else if(!traversal.communicate.getStatusMapSetup()){
        traversal.communicate.setUpMap();
      }
      else{
        Serial.println("All set up");
      }
    #endif
    SerialBT.print(controller.getDistance());
    SerialBT.print(", ");
    SerialBT.print(-ypr[0]*180/M_PI);
    SerialBT.print(", ");
    SerialBT.println(controller.getLeftOutput());

    vTaskDelay(100);
  }
}