#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include <AccelStepper.h>
#include <BluetoothSerial.h>
#include "Controller.h"

Controller controller;

BluetoothSerial SerialBT;

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! please run 'make menuconfig' to enable it
#endif

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> //
// ----------------- CORE 0 DEFINITIONS ----------------- //

// #define ENABLE_HTTP_SERVER
#define OUTPUT_DEBUG

// const uint8_t redPin = 19;
// const uint8_t greenPin = 18;
// const uint8_t bluePin = 5;

TaskHandle_t communication;  // task on core 0 for communication

const char* ssid = "";
const char* password = "";
String serverName = "http://192.168.1.16:8081";  // local ip of the backend host (NOT localhost)
unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< //

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

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< /

void setup() {

  Serial.begin(115200);
  // while (!Serial);    // hang until serial connection established
  SerialBT.begin("shrektastic");
  // while (!SerialBT);    // hang until serial connection established

  Wire.begin();
  Wire.setClock(400000); // 400kHz I2C clock
  
  // pinMode(redPin, OUTPUT);
  // pinMode(greenPin, OUTPUT);
  // pinMode(bluePin, OUTPUT);

  // initialize device
  // setColour(170, 0, 255); // purple

  // initialize device
  mpu.initialize();
  SerialBT.println(F("Testing device connections..."));
  SerialBT.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));
  // Serial.println(F("Testing device connections..."));
  // Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));

  // load and configure the DMP
  SerialBT.println(F("Initializing DMP..."));
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
      SerialBT.println(F("Enabling DMP..."));
      mpu.setDMPEnabled(true);
      dmpReady = true;
      // setColour(0, 255, 0); // bueno, set green 
      // delay(600);
      // setColour(0, 0, 0); // kill
  } else {
      // ERROR!
      // 1 = initial memory load failed
      // 2 = DMP configuration updates failed
      // (if it's going to break, usually the code will be 1)
      SerialBT.print(F("DMP Initialization failed (code "));
      SerialBT.print(devStatus);
      SerialBT.println(F(")"));
      // setColour(255, 0, 0);
      // Serial.print(F("DMP Initialization failed (code "));
      // Serial.print(devStatus);
      // Serial.println(F(")"));
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

  // -------- PID CONTROLLER ---------- // 
  float tiltReading = ypr[1] * 180/M_PI;
  float headingReading = ypr[0] * 180/M_PI;

  controller.update(tiltReading, headingReading);

  delayMicroseconds(1000);
}

void communicationCode(void* pvParameters) {
  // -------- OUTPUTS ---------- //
  for (;;) {
    float L = controller.getLeftOutput();
    float R = controller.getRightOutput()
    ;
    SerialBT.print("L: ");
    SerialBT.print(L);
    SerialBT.print(", R: ");
    SerialBT.print(R);
    SerialBT.print(", Pit: ");
    SerialBT.print(ypr[1] * 180/M_PI);
    SerialBT.print(", Dis: ");
    SerialBT.print(controller.getDistance());
    // SerialBT.print(", I: ");
    // SerialBT.println(T_integral[1]);
    SerialBT.print(", GY: ");
    SerialBT.println(gyro.y);

    //bluetooth tuning code
    // if (SerialBT.available()) {
    //   controller.updateValues();
    // }
    vTaskDelay(100);
  }
}

// void communicationCode(void* pvParameters) {
//   // Serial.println(xPortGetCoreID());

//   #ifdef ENABLE_HTTP_SERVER
//     wifi setup
//     WiFi.begin(ssid, password);
//     Serial.println("Connecting");

//     while (WiFi.status() != WL_CONNECTED) {
//       delay(500);
//       Serial.print("Attempting WiFi connection...");
//     }

//     Serial.println("");
//     Serial.print("Connected to WiFi network with IP Address: ");
//     Serial.println(WiFi.localIP());
//     Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
//   #endif

//     // looping code - this takes up entirety of cpu time along with controller so NEEDS the delay to allow idle tasks to execute
//   for (;;) {
//     #ifdef ENABLE_HTTP_SERVER
//       if (millis() - lastTime > 5000) {
//         //Check WiFi connection status
//         if (WiFi.status() == WL_CONNECTED) {
//           // WiFiClient client;
//           HTTPClient http;
//           // String serverPath = serverName + "/Nodes/Add?SessionId=1&NodeId=3&XCoord=72&YCoord=56";
//           String serverPath = serverName + "/Edges/Add?SessionId=1&NodeId=2&EdgeNodeId=3&Distance=34.2&Angle=72.0";
//           // Serial.println(serverPath);
//           // Your Domain name with URL path or IP address with path
//           http.begin(serverPath.c_str());
//           // HTTP GET request
//           int httpResponseCode = http.GET();

//           if (httpResponseCode > 0) {
//             Serial.print("HTTP Response code: ");
//             Serial.println(httpResponseCode);  // HTTP response code e.g. 200
//             String payload = http.getString();
//             Serial.println(payload);  // HTTP response package e..g JSON object
//           } else {
//             Serial.print("Error code: ");
//             Serial.println(httpResponseCode);
//           }

//           // Free resources
//           http.end();
//         } else {
//           Serial.println("WiFi Disconnected");
//         }
//         // this delay is not actually necessary as the time waiting for http request is enough for idle tasks to run ?
//         // vTaskDelay(5000); //delay important to allow idle tasks to execute else processor reboots
//         lastTime = millis();
//       }
//     #endif
//     vTaskDelay(20);
//   }
// }