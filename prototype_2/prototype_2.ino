#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include <AccelStepper.h>
#include <BluetoothSerial.h>
#include "Controller.h"
// #include <Preferences.h>

hw_timer_t *motorTimer = NULL;

Controller controller;
// Preferences preferences;

BluetoothSerial SerialBT;

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! please run 'make menuconfig' to enable it
#endif

#define btTuning

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

bool update = false;

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< /

void IRAM_ATTR updateController(){
  // Serial.println("5");
  // Serial.println("6");
  update = true;
}

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
  // Serial.println("1");
  motorTimer = timerBegin(0, 80, true);
  // Serial.println("2");
  timerAttachInterrupt(motorTimer, &updateController, true);
  // Serial.println("3");
  timerAlarmWrite(motorTimer, 1000, true);
  // Serial.println("4");
  timerAlarmEnable(motorTimer); //Just Enable
  // Serial.println("5");
  // #ifdef btTuning
  // controller.updateValues("Pp1", preferences.getFloat("Pp1"));
  // controller.updateValues("Pp2", preferences.getFloat("Pp2"));
  // controller.updateValues("P1", preferences.getFloat("P1"));
  // controller.updateValues("P2", preferences.getFloat("P2"));
  // controller.updateValues("I1", preferences.getFloat("I1"));
  // controller.updateValues("I2", preferences.getFloat("I2"));
  // controller.updateValues("D1", preferences.getFloat("D1"));
  // controller.updateValues("D2", preferences.getFloat("D2"));
  // controller.updateValues("B1", preferences.getFloat("B1"));
  // controller.updateValues("B2", preferences.getFloat("B2"));
  // controller.updateValues("B3", preferences.getFloat("B3"));
  // controller.updateValues("S1", preferences.getInt("S1"));
  // controller.updateValues("S2", preferences.getInt("S2"));
  // controller.updateValues("S2", preferences.getInt("S2"));
  // #endif

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

  if (update) {
    update = false;
    float tiltReading = -ypr[1] * 180/M_PI;
    float headingReading = ypr[0] * 180/M_PI;
    float pitchRate = gyro.y;
    controller.update(tiltReading, pitchRate, headingReading);
  }

  // controller.update(tiltReading, headingReading);

  delayMicroseconds(1000);
}

void communicationCode(void* pvParameters) {
  // -------- OUTPUTS ---------- //
  for (;;) {
    float L = controller.getLeftOutput();
    float R = controller.getRightOutput();
    float LV = controller.getLinearVelocity();
    float PR = gyro.y;
    SerialBT.print("/*L: ");
    SerialBT.print(L);
    SerialBT.print(", R: ");
    SerialBT.print(R);
    SerialBT.print(", Pit: ");
    SerialBT.print(-ypr[1] * 180/M_PI);
    // SerialBT.print(", Dis: ");
    // SerialBT.print(controller.getDistance());
    SerialBT.print(", LV: ");
    SerialBT.print(LV);
    // SerialBT.print(", I: ");
    // SerialBT.println(T_integral[1]);
    SerialBT.print(", GY: ");
    SerialBT.print(gyro.y);
    SerialBT.print(", V: ");
    SerialBT.print(LV-PR);
    SerialBT.print(", Vout: ");
    SerialBT.print(controller.getVOut());
    SerialBT.println("*/");

    //bluetooth tuning code
    #ifdef btTuning
    if (SerialBT.available()) {
      String test = SerialBT.readString();
      if (test.substring(0,3) == "Pp1") {
        float Pp1 = test.substring(3,test.length()-1).toFloat();
        // preferences.putFloat("Pp1", Pp1);
        // Kp_position = preferences.getInt("Pp", 0);
        controller.updateValues("Pp1", Pp1);
        SerialBT.print("Set Pp1 to ");
        SerialBT.println(Pp1);
      } else if (test.substring(0,3) == "Pp2") {
        float Pp2 = test.substring(3,test.length()-1).toFloat();
        // preferences.putFloat("Pp2", Pp2);
        // Kp_position = preferences.getInt("Pp", 0);
        controller.updateValues("Pp2", Pp2);
        SerialBT.print("Set Pp2 to ");
        SerialBT.println(Pp2);
      } else if (test.substring(0,2) == "P1") {
        float P1 = test.substring(2,test.length()-1).toFloat();
        // preferences.putFloat("P1", P1);
        // Kp_tilt = preferences.getFloat("P", 0);
        controller.updateValues("P1", P1);
        SerialBT.print("Set P1 to ");
        SerialBT.println(P1, 4);
      } else if (test.substring(0,2) == "I1") {
        float I1 = test.substring(2,test.length()-1).toFloat();
        // preferences.putFloat("I1", I1);
        // Kp_tilt = preferences.getFloat("P", 0);
        controller.updateValues("I1", I1);
        SerialBT.print("Set I1 to ");
        SerialBT.println(I1, 4);
      } else if (test.substring(0,2) == "D1") {
        float D1 = test.substring(2,test.length()-1).toFloat();
        // preferences.putFloat("D1", D1);
        // Kp_tilt = preferences.getFloat("P", 0);
        controller.updateValues("D1", D1);
        SerialBT.print("Set D1 to ");
        SerialBT.println(D1, 4);
      } else if (test.substring(0,2) == "P2") {
        float P2 = test.substring(2,test.length()-1).toFloat();
        // preferences.putFloat("P2", P2);
        // Kp_tilt = preferences.getFloat("P", 0);
        controller.updateValues("P2", P2);
        SerialBT.print("Set P2 to ");
        SerialBT.println(P2, 4);
      } else if (test.substring(0,2) == "I2") {
        float I2 = test.substring(2,test.length()-1).toFloat();
        // preferences.putFloat("I2", I2);
        // Kp_tilt = preferences.getFloat("P", 0);
        controller.updateValues("I2", I2);
        SerialBT.print("Set I2 to ");
        SerialBT.println(I2, 4);
      } else if (test.substring(0,2) == "D2") {
        float D2 = test.substring(2,test.length()-1).toFloat();
        // preferences.putFloat("D2", D2);
        // Kp_tilt = preferences.getFloat("P", 0);
        controller.updateValues("D2", D2);
        SerialBT.print("Set D2 to ");
        SerialBT.println(D2, 4);
      } else if (test.substring(0,2) == "S1") {
        int S1 = test.substring(2,test.length()-1).toInt();
        // preferences.putInt("S1", S1);
        // maxSpeed1 = preferences.getInt("S1", 0);
        controller.updateValues("S1", S1);
        SerialBT.print("Set S1 to ");
        SerialBT.println(S1);
      } else if (test.substring(0,2) == "S2") {
        int S2 = test.substring(2,test.length()-1).toInt();
        // preferences.putInt("S2", S2);
        // maxSpeed1 = preferences.getInt("S1", 0);
        controller.updateValues("S2", S2);
        SerialBT.print("Set S2 to ");
        SerialBT.println(S2);
      } else if (test.substring(0,2) == "S3") {
        int S3 = test.substring(2,test.length()-1).toInt();
        // preferences.putInt("S3", S3);
        // maxSpeed1 = preferences.getInt("S1", 0);
        controller.updateValues("S3", S3);
        SerialBT.print("Set S3 to ");
        SerialBT.println(S3);
      } else if (test.substring(0,2) == "B1") {
        float hahha = test.substring(2,test.length()-1).toFloat();
        // preferences.putFloat("B1", hahha);
        // Kp_tilt = preferences.getFloat("P", 0);
        controller.updateValues("B1", hahha);
        SerialBT.print("Set B1 to ");
        SerialBT.println(hahha, 4);
      } else if (test.substring(0,2) == "B2") {
        float B2 = test.substring(2,test.length()-1).toFloat();
        // preferences.putFloat("B2", B2);
        // Kp_tilt = preferences.getFloat("P", 0);
        controller.updateValues("B2", B2);
        SerialBT.print("Set B2 to ");
        SerialBT.println(B2, 4);
      } else if (test.substring(0,2) == "B3") {
        float B3 = test.substring(2,test.length()-1).toFloat();
        // preferences.putFloat("B3", B3);
        // Kp_tilt = preferences.getFloat("P", 0);
        controller.updateValues("B3", B3);
        SerialBT.print("Set B3 to ");
        SerialBT.println(B3, 4);
      } else if (test.substring(0,3) == "MTO") {
        float MTO = test.substring(3,test.length()-1).toFloat();
        // preferences.putFloat("B3", B3);
        // Kp_tilt = preferences.getFloat("P", 0);
        controller.updateValues("MTO", MTO);
        SerialBT.print("Set MTO to ");
        SerialBT.println(MTO, 4);
      } else if (test.substring(0,2) == "MV") {
        float MV = test.substring(2,test.length()-1).toFloat();
        // preferences.putFloat("B3", B3);
        // Kp_tilt = preferences.getFloat("P", 0);
        controller.updateValues("MV", MV);
        SerialBT.print("Set MV to ");
        SerialBT.println(MV, 4);
      } else if (test.substring(0,1) == "F") {
        // float MTO = test.substring(3,test.length()-1).toFloat();
        // preferences.putFloat("B3", B3);
        // Kp_tilt = preferences.getFloat("P", 0);
        // controller.updateValues("MTO", MTO);
        controller.moveForwards();
        SerialBT.println("Move forwards");
      } else if (test[0] == 'C') {
        SerialBT.print("P1: ");
        SerialBT.print(controller.getValue("P1"), 4);
        SerialBT.print(" I1: ");
        SerialBT.print(controller.getValue("I1"), 4);
        SerialBT.print(" D1: ");
        SerialBT.print(controller.getValue("D1"), 7);
        SerialBT.print(" P2: ");
        SerialBT.print(controller.getValue("P2"), 4);
        SerialBT.print(" I2: ");
        SerialBT.print(controller.getValue("I2"), 4);
        SerialBT.print(" D2: ");
        SerialBT.print(controller.getValue("D2"), 7);
        SerialBT.print(" Pp1: ");
        SerialBT.print(controller.getValue("Pp1"), 4);
        SerialBT.print(" Ii1: ");
        SerialBT.print(controller.getValue("Ii1"), 4);
        SerialBT.print(" Dd1: ");
        SerialBT.print(controller.getValue("Dd1"), 4);
        SerialBT.print(" Pp2: ");
        SerialBT.print(controller.getValue("Pp2"), 4);
        SerialBT.print(" Ii2: ");
        SerialBT.print(controller.getValue("Ii2"), 4);
        SerialBT.print(" Dd2: ");
        SerialBT.print(controller.getValue("Dd2"), 4);
        SerialBT.print(" maxSpeed1: ");
        SerialBT.print(controller.getValue("S1"));
        SerialBT.print(" maxSpeed2: ");
        SerialBT.print(controller.getValue("S2"));
        SerialBT.print(" correct speed: ");
        SerialBT.print(controller.getValue("S3"));
        SerialBT.print(" B1: ");
        SerialBT.print(controller.getValue("B1"), 4);
        SerialBT.print(" B2: ");
        SerialBT.print(controller.getValue("B2"), 4);
        SerialBT.print(" B3: ");
        SerialBT.print(controller.getValue("B3"), 4);
        SerialBT.print(" MV: ");
        SerialBT.print(controller.getValue("MV"), 4);
        SerialBT.print(" MTO: ");
        SerialBT.print(controller.getValue("MTO"), 4);
      }
      #endif
    }
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