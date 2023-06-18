#include <map>
#include <stack>
#include <vector>
#include <Wire.h>
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include <AccelStepper.h>
#include <BluetoothSerial.h>
#include "Camera.h"
#include "Classify.h"
#include "Traversal.h"
#include "Communicate.h"

// MACROS
// #define ENABLE_YAW_OUTPUT
#define ENABLE_HTTP_SERVER
// #define ENABLE_BLUETOOTH
// #define ENABLE_CAMERA
// #define ENABLE_CLASSIFICATION
// #define ENABLE_MOTORS
// #define ENABLE_TRIANGULATE
// //#define XDIST 100
// // #define YDIST 100
// // #define ERRMARGIN 5

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> //
// ----------------- CORE 0 DEFINITIONS ----------------- //

BluetoothSerial SerialBT;
Camera D8M;

const uint8_t redPin = 19;
const uint8_t greenPin = 18;
const uint8_t bluePin = 5;
String bugId =  "MazEERunnEEr";

TaskHandle_t communication;  // task on core 0 for communication

Communicate communicate;

#define CHECK_NEW_SESSION_TIMEOUT 20


// const char* ssid = "Ben";
// const char* password = "begforit";
// String serverName = "http://90.196.3.86:8081";  // local ip of the backend host (NOT localhost)
unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< //

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> //
// ----------------- CORE 1 DEFINITIONS ----------------- //

float HEADING_SETPOINT = 0;
float POSITION_SETPOINT = 0;

// Define pin connections
const int leftDirPin = 32; //A4
const int leftStepPin = 33; //A3
// Define pin connections
const int rightDirPin = 25; //A2
const int rightStepPin = 26; //A1
//D2 D3 for 23 and 22 (sda and scl)

// Define motor interface type
#define motorInterfaceType 1
#define stepsPerRevolution 200

// Creates an instance
AccelStepper leftStepper(motorInterfaceType, leftStepPin, leftDirPin);
AccelStepper rightStepper(motorInterfaceType, rightStepPin, rightDirPin);

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
int maxSpeed = 1000;
int acceleration = 600;

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

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  // initialize device
  setColour(170, 0, 255); // purple
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
      setColour(0, 255, 0); // bueno, set green 
      delay(600);
      setColour(0, 0, 0); // kill
  } else {
      // ERROR!
    debugOutput((F("DMP Initialization failed (code ")));
    (devStatus);
    debugOutput(F(")"));
    setColour(255, 0, 0);
  }

  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
    communicationCode, // Task function.
    "communication",   // name of task.
    20000,             // Stack size of task 
    NULL,              // parameter of the task, not 
    1,                 // priority of the task 
    &communication,    // Task handle to keep track of created task 
    0);                // pin task to core 0 -- by default we pin to core 1 
  
	leftStepper.setMaxSpeed(maxSpeed);
	leftStepper.setAcceleration(acceleration);
	rightStepper.setMaxSpeed(maxSpeed);
	rightStepper.setAcceleration(acceleration);

  leftStepper.setSpeed(-2000);
  rightStepper.setSpeed(2000);
  
  delay(1000);
}

void loop() {

  // -------- READ FROM IMU ---------- // 

  if (mpu.dmpGetCurrentFIFOPacket(fifoBuffer)){
    mpu.dmpGetQuaternion(&q, fifoBuffer);
    mpu.dmpGetGravity(&gravity, &q);
    mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
    mpu.dmpGetGyro(&gyro, fifoBuffer);
  }

  #ifdef ENABLE_MOTORS
    if (ypr[1] > 0) {
      leftStepper.setSpeed(-1000);
      rightStepper.setSpeed(1000);
    } else {
      leftStepper.setSpeed(1000);
      rightStepper.setSpeed(-1000);
    }

    leftStepper.runSpeed();
    rightStepper.runSpeed();
    
  #endif
  #ifdef ENABLE_CAMERA
    D8M.update();
    Matrix frame = D8M.getBoxMatrix();
    char tmp[64];

    debugOutput(ypr[0], false);
    debugOutput(",", false);
  
    for (int i = 0; i < 11; i++){
      sprintf(tmp, "%d,%d,%d,%d,", frame.boxes[i][0], frame.boxes[i][1], frame.boxes[i][2], frame.boxes[i][3]);
      debugOutput(tmp, false);
    }
    sprintf(tmp, "%d,%d,%d,%d", frame.boxes[12][0], frame.boxes[12][1], frame.boxes[12][2], frame.boxes[12][3]);
    debugOutput(tmp);

    Image newImage;   

    newImage.classify(frame.boxes);
    debugOutput("Classification:");
    newImage.debugInfo();

  #endif

  vTaskDelay(10);
}

void setColour(uint8_t r, uint8_t g, uint8_t b){
  analogWrite(redPin, r);
  analogWrite(greenPin, g);
  analogWrite(bluePin, b);
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
  if (!communicate.getInitialised()){
    communicate.init("Ben", "test1234", "http://90.196.3.86:8081", bugId, CHECK_NEW_SESSION_TIMEOUT);
  }
    
  #endif

  // looping code - this takes up entirety of cpu time along with controller so NEEDS the delay to allow idle tasks to execute
  for (;;) {
    

    // -------- OUTPUTS ---------- //

    #ifdef ENABLE_YAW_OUTPUT
      debugOutput("Yaw: ", false);
      debugOutput(ypr[0], false);
    #endif
    #ifdef ENABLE_HTTP_SERVER
      if (!communicate.getInSession()){
        communicate.ping();
        communicate.checkNewSession();
        vTaskDelay(100);
      } else if(!communicate.getStatusMapSetup()){
        communicate.setUpMap();
      }
      else{
        Serial.println("All set up");
      }
      // if (millis() - lastTime > 5000) {
      //   //Check WiFi connection status
      //   if (WiFi.status() == WL_CONNECTED) {
      //     // WiFiClient client;
      //     HTTPClient http;
      //     // String serverPath = serverName + "/Nodes/Add?SessionId=1&NodeId=3&XCoord=72&YCoord=56";
      //     String serverPath = serverName + "/Edges/Add?SessionId=1&NodeId=2&EdgeNodeId=3&Distance=34.2&Angle=72.0";
      //     // Serial.println(serverPath);
      //     // Your Domain name with URL path or IP address with path
      //     http.begin(serverPath.c_str());
      //     // HTTP GET request
      //     int httpResponseCode = http.GET();

      //     if (httpResponseCode > 0) {
      //       debugOutput("HTTP Response code: ", false);
      //       debugOutput(httpResponseCode);  // HTTP response code e.g. 200
      //       String payload = http.getString();
      //       debugOutput(payload);  // HTTP response package e..g JSON object
      //     } else {
      //       debugOutput("Error code: ", false);
      //       debugOutput(httpResponseCode);
      //     }

      //     // Free resources
      //     http.end();
      //   } else {
      //     debugOutput("WiFi Disconnected");
      //   }
      //   // this delay is not actually necessary as the time waiting for http request is enough for idle tasks to run ?
      //   // vTaskDelay(5000); //delay important to allow idle tasks to execute else processor reboots
      //   lastTime = millis();
      // }
    #endif
    vTaskDelay(100);
  }
}

void spin(){  // this is a function to just spin
  float target_angle = ypr[0] + 360.0;
  leftStepper.setSpeed(200);
  rightStepper.setSpeed(200);
  while (ypr[0] < target_angle){
    leftStepper.runSpeed();
    rightStepper.runSpeed();

    #ifdef ENABLE_TRIANGULATE
    // ----------------- BENBENBEN TODO TODO HERE BEN HERE ---------------------- //
    #endif

  }
  leftStepper.setSpeed(0);
  rightStepper.setSpeed(0);
  leftStepper.runSpeed();
  rightStepper.runSpeed();
}