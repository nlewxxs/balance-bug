#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include <AccelStepper.h>
#include <BluetoothSerial.h>
#include "Camera.h"

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> //
// ----------------- CORE 0 DEFINITIONS ----------------- //

BluetoothSerial SerialBT;

// #define ENABLE_HTTP_SERVER
#define OUTPUT_DEBUG

const uint8_t redPin = 19;
const uint8_t greenPin = 18;
const uint8_t bluePin = 5;

TaskHandle_t communication;  // task on core 0 for communication

const char* ssid = "CommunityFibre10Gb_AF5A8";
const char* password = "dvasc4xppp";
String serverName = "http://192.168.1.16:8081";  // local ip of the backend host (NOT localhost)
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

// heading PD controller
float Kp_heading = 30; 
float Ki_heading = 0;
float Kd_heading = 0; 

float Kp_position = 0;
float Ki_position = 0;
float Kd_position = 0;

int maxSpeed = 1000;
int acceleration = 600;

float Kp_tilt = 0;
float Ki_tilt = 0;
float Kd_tilt = 0;

float balanceCenter = 0;  // whatever tilt value is balanced
float P_bias, T_bias, Trate_bias, H_bias = 0;

float H_derivative, H_out, P_derivative, P_out, T_derivative, T_out, Trate_derivative, Trate_out;
float H_error[2], P_error[2], T_error[2], Trate_error[2] = { 0, 0 };            
float H_integral[2], P_integral[2], T_integral[2], Trate_integral[2] = { 0, 0 };  
float iteration_time;
unsigned long oldMillis = 0;

float leftWheelDrive;
float rightWheelDrive;

float totalDistance;
float leftDistance;
float rightDistance;

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< /

void callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param){
  if(event == ESP_SPP_SRV_OPEN_EVT){
    SerialBT.println("Client Connected");
  }
}

void setup() {

  SerialBT.begin("MZRNR");
  SerialBT.register_callback(callback);

  Wire.begin();
  Wire.setClock(400000); // 400kHz I2C clock

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  // initialize device
  setColour(170, 0, 255); // purple

  mpu.initialize();
  SerialBT.println(F("Testing device connections..."));
  SerialBT.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));

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
      setColour(0, 255, 0); // bueno, set green 
      delay(600);
      setColour(0, 0, 0); // kill

  } else {
      // ERROR!

    SerialBT.print(F("DMP Initialization failed (code "));
    SerialBT.print(devStatus);
    SerialBT.println(F(")"));
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

  // while (!dmpReady) {
  //   setColour(170, 0, 255);
  //   delay(500);
  //   setColour(0, 0, 0);
  //   delay(500);
  // }  // hang program if programming did not work

  // -------- READ FROM IMU ---------- // 

  if (mpu.dmpGetCurrentFIFOPacket(fifoBuffer)){
    mpu.dmpGetQuaternion(&q, fifoBuffer);
    mpu.dmpGetGravity(&gravity, &q);
    mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
    mpu.dmpGetGyro(&gyro, fifoBuffer);
  }

  // leftStepper.setSpeed(300);
  // leftStepper.run();

  if (ypr[1] > 0) {
    leftStepper.setSpeed(-1000);
    rightStepper.setSpeed(1000);
  } else {
    leftStepper.setSpeed(1000);
    rightStepper.setSpeed(-1000);
  }

  leftStepper.runSpeed();
  rightStepper.runSpeed();


  // float iteration_time = (millis() - oldMillis) / 1000.0;
  // oldMillis = millis();

  // // -------- PID CONTROLLER ---------- // 

  // // position control first
  // float positionReading = getPosition();  // example I used for the controller has position read from rotary encoders - this value is total distance travelled
  // P_error[1] = POSITION_SETPOINT - positionReading;
  // P_integral[1] = P_integral[0] + P_error[1] * iteration_time;  // 1 is current, 0 is old
  // P_derivative = (P_error[1] - P_error[0]) / iteration_time;
  // P_out = Kp_position * P_error[1] + Ki_position * P_integral[1] + Kd_position * P_derivative + P_bias;
  // // make this output positive to move forwards, negative to move backwards

  // P_integral[0] = P_integral[1];  // time shift integral readings after out calculated
  // P_error[0] = P_error[1];

  // // balance control
  // float tiltReading = ypr[1] * 180/M_PI;
  // T_error[1] = balanceCenter + P_out - tiltReading;             // offset the tilt reading with the distance output to allow us to manipulate the position of the robot via tilt
  // T_integral[1] = T_integral[0] + T_error[1] * iteration_time;  // 1 is current, 0 is old
  // T_derivative = (T_error[1] - T_error[0]) / iteration_time;
  // T_out = Kp_tilt * T_error[1] + Ki_tilt * T_integral[1] + Kd_tilt * T_derivative + T_bias;

  // T_integral[0] = T_integral[1];  // time shift integral readings after out calculated
  // T_error[0] = T_error[1];

  // // heading control to offset the wheels for rotation
  // float headingReading = ypr[0] * 180/M_PI;
  // H_error[1] = HEADING_SETPOINT - headingReading;
  // H_integral[1] = H_integral[0] + H_error[1] * iteration_time;  // 1 is current, 0 is old
  // H_derivative = (H_error[1] - H_error[0]) / iteration_time;
  // H_out = Kp_heading * H_error[1] + Ki_heading * H_integral[1] + Kd_heading * H_derivative + H_bias;

  // H_integral[0] = H_integral[1];  // time shift integral readings after out calculated
  // H_error[0] = H_error[1];

  // leftWheelDrive = T_out + H_out;  // voltage / pwm that will actually drive the wheels
  // rightWheelDrive = T_out - H_out;

  // leftWheelDrive = constrain(leftWheelDrive, -maxSpeed, maxSpeed);
  // rightWheelDrive = -constrain(rightWheelDrive, -maxSpeed, maxSpeed);
  
	// // Move the motor one step
  // if (leftWheelDrive > 0) {
  //   leftStepper.moveTo(leftStepper.currentPosition() + 20000);
  // } else {
  //   leftStepper.moveTo(leftStepper.currentPosition() - 20000);
  // }
  // leftStepper.setSpeed(leftWheelDrive);
	// leftStepper.run();

  // if (rightWheelDrive > 0) {
  //   rightStepper.moveTo(rightStepper.currentPosition() + 20000);
  // } else {
  //   rightStepper.moveTo(rightStepper.currentPosition() - 20000);
  // }
  // rightStepper.setSpeed(rightWheelDrive);
	// rightStepper.run();

  vTaskDelay(10);
}

void setColour(uint8_t r, uint8_t g, uint8_t b){
  analogWrite(redPin, r);
  analogWrite(greenPin, g);
  analogWrite(bluePin, b);
}

// float getPosition() {  //unsure of how this reading will work - needs to be 1D (as in just x)
//   // could potentially have position just be a relative thing i.e. move forwards 1 / backwards 1 rather than move to position 23?

//   leftDistance = leftStepper.currentPosition()/200.0 * 0.175 * M_PI;
//   rightDistance = rightStepper.currentPosition()/200.0 * 0.175 * M_PI;
  
//   totalDistance = (leftDistance + rightDistance) / 2.0;
//   return totalDistance;
// }

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
    wifi setup
    WiFi.begin(ssid, password);
    Serial.println("Connecting");

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print("Attempting WiFi connection...");
    }

    Serial.println("");
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
  #endif

  // looping code - this takes up entirety of cpu time along with controller so NEEDS the delay to allow idle tasks to execute
  for (;;) {

    // -------- OUTPUTS ---------- //
    SerialBT.print("Yaw: ");
    SerialBT.print(ypr[0]);
    SerialBT.print("\t L: ");
    SerialBT.print(leftWheelDrive);
    SerialBT.print("\t R: ");
    SerialBT.println(rightWheelDrive);

    #ifdef ENABLE_HTTP_SERVER
      if (millis() - lastTime > 5000) {
        //Check WiFi connection status
        if (WiFi.status() == WL_CONNECTED) {
          // WiFiClient client;
          HTTPClient http;
          // String serverPath = serverName + "/Nodes/Add?SessionId=1&NodeId=3&XCoord=72&YCoord=56";
          String serverPath = serverName + "/Edges/Add?SessionId=1&NodeId=2&EdgeNodeId=3&Distance=34.2&Angle=72.0";
          // Serial.println(serverPath);
          // Your Domain name with URL path or IP address with path
          http.begin(serverPath.c_str());
          // HTTP GET request
          int httpResponseCode = http.GET();

          if (httpResponseCode > 0) {
            Serial.print("HTTP Response code: ");
            Serial.println(httpResponseCode);  // HTTP response code e.g. 200
            String payload = http.getString();
            Serial.println(payload);  // HTTP response package e..g JSON object
          } else {
            Serial.print("Error code: ");
            Serial.println(httpResponseCode);
          }

          // Free resources
          http.end();
        } else {
          Serial.println("WiFi Disconnected");
        }
        // this delay is not actually necessary as the time waiting for http request is enough for idle tasks to run ?
        // vTaskDelay(5000); //delay important to allow idle tasks to execute else processor reboots
        lastTime = millis();
      }
    #endif
    vTaskDelay(100);
  }
}