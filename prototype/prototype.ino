#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include <AccelStepper.h>
#include <BluetoothSerial.h>
#include <Preferences.h>

Preferences preferences;

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

const char* ssid = "CommunityFibre10Gb_AF5A8";
const char* password = "dvasc4xppp";
String serverName = "http://192.168.1.16:8081";  // local ip of the backend host (NOT localhost)
unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< //

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> //
// ----------------- CORE 1 DEFINITIONS ----------------- //

#define HEADING_SETPOINT 0
float POSITION_SETPOINT = 0;

// Define pin connections
const int leftDirPin = 26; //A1
const int leftStepPin = 25; //A2
// Define pin connections
const int rightDirPin = 33; //A3
const int rightStepPin = 32; //A4
//4 is sda is orange, 3 is scl is yellow

const int leftMicro1 = 5; // 7 is blue - GPIO5
const int leftMicro2 = 18; // 6 is white - GPIO18
const int leftMicro3 = 19; // 5 is red - GPIO19
const int rightMicro1 = 15; // 12 is blue - GPIO15
const int rightMicro2 = 4; // 11 is white - GPIO4
const int rightMicro3 = 14; // 10 is red - GPIO14

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

// heading PID controller
float Kp_heading = 0;
float Ki_heading = 0;
float Kd_heading = 0;

// position PID controller
float Kp_position = 40;
float Ki_position = 0;
float Kd_position = 0;

// balance PID controller
float Kp_tilt = 75;
float Ki_tilt = 0;
float Kd_tilt = 0;

bool correcting1 = 0;
bool correcting2 = 0;

// balance rate PID controller
// float Kp_tiltRate = 75;
// float Ki_tiltRate = 0;
// float Kd_tiltRate = 0;

int maxSpeed = 1000;
int acceleration = 600;

float balanceCenter = 0;  // whatever tilt value is balanced
float P_bias, T_bias, Trate_bias, H_bias = 0;

float H_derivative, H_out, P_derivative, P_out, T_derivative, T_out, Trate_derivative, Trate_out;
float H_error[2], P_error[2], T_error[2], Trate_error[2] = { 0, 0 };           //stores current and previous value for derivative calculation
float H_integral[2], P_integral[2], T_integral[2], Trate_integral[2] = { 0, 0 };  //stores current and previous value for integral calculation
float iteration_time;
unsigned long oldMillis = 0;

float leftWheelDrive;
float rightWheelDrive;

float totalDistance;
float leftDistance;
float rightDistance;

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

  pinMode(leftMicro1, OUTPUT);
  pinMode(leftMicro2, OUTPUT);
  pinMode(leftMicro3, OUTPUT);
  pinMode(rightMicro1, OUTPUT);
  pinMode(rightMicro2, OUTPUT);
  pinMode(rightMicro3, OUTPUT);
  
  digitalWrite(leftMicro1 , LOW);
  digitalWrite(leftMicro2 , LOW);
  digitalWrite(leftMicro3 , LOW);
  digitalWrite(rightMicro1, LOW);
  digitalWrite(rightMicro2, LOW);
  digitalWrite(rightMicro3, LOW);

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

  preferences.begin("bastard", false);

  // Kp_tilt = preferences.getFloat("P", 0);
  // Ki_tilt = preferences.getFloat("I", 0);
  // Kd_tilt = preferences.getFloat("D", 0);
  // Kp_position = preferences.getFloat("Pp", 0);
  // Ki_position = preferences.getFloat("Ii", 0);
  // Kd_position = preferences.getFloat("Dd", 0);
  // acceleration = preferences.getInt("A", 0);
  // maxSpeed = preferences.getInt("S", 0);

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
	// leftStepper.setAcceleration(acceleration);
	leftStepper.setMinPulseWidth(20);
	rightStepper.setMaxSpeed(maxSpeed);
	// rightStepper.setAcceleration(acceleration);
	rightStepper.setMinPulseWidth(20);
  
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

  float iteration_time = (millis() - oldMillis) / 1000.0;
  oldMillis = millis();

  // -------- PID CONTROLLER ---------- // 
  float tiltReading = ypr[1] * 180/M_PI;

  if (abs(tiltReading) < preferences.getFloat("B1", 0)) {
    Kp_tilt = preferences.getFloat("P1", 0);
    Ki_tilt = preferences.getFloat("I1", 0);
    Kd_tilt = preferences.getFloat("D1", 0);
    Kp_position = preferences.getFloat("Pp1", 0);
    Ki_position = preferences.getFloat("Ii1", 0);
    Kd_position = preferences.getFloat("Dd1", 0);
    maxSpeed = preferences.getInt("S1", 0);
    digitalWrite(leftMicro1 , HIGH);
    digitalWrite(leftMicro2 , HIGH);
    digitalWrite(leftMicro3 , LOW);
    digitalWrite(rightMicro1, HIGH);
    digitalWrite(rightMicro2, HIGH);
    digitalWrite(rightMicro3, LOW);
  } else {
    Kp_tilt = preferences.getFloat("P2", 0);
    Ki_tilt = preferences.getFloat("I2", 0);
    Kd_tilt = preferences.getFloat("D2", 0);
    Kp_position = preferences.getFloat("Pp2", 0);
    Ki_position = preferences.getFloat("Ii2", 0);
    Kd_position = preferences.getFloat("Dd2", 0);
    maxSpeed = preferences.getInt("S2", 0);
    digitalWrite(leftMicro1 , LOW);
    digitalWrite(leftMicro2 , LOW);
    digitalWrite(leftMicro3 , LOW);
    digitalWrite(rightMicro1, LOW);
    digitalWrite(rightMicro2, LOW);
    digitalWrite(rightMicro3, LOW);
  }

  // position control first
  float positionReading = getDistance();  // example I used for the controller has position read from rotary encoders - this value is total distance travelled
  P_error[1] = POSITION_SETPOINT - positionReading;
  P_integral[1] = P_integral[0] + P_error[1] * iteration_time;  // 1 is current, 0 is old
  P_derivative = (P_error[1] - P_error[0]) / iteration_time;
  P_out = Kp_position * P_error[1] + Ki_position * P_integral[1] + Kd_position * P_derivative + P_bias;
  // make this output positive to move forwards, negative to move backwards

  P_integral[0] = P_integral[1];  // time shift integral readings after out calculated
  P_error[0] = P_error[1];

  // balance control
  T_error[1] = balanceCenter + P_out - tiltReading;             // offset the tilt reading with the distance output to allow us to manipulate the position of the robot via tilt
  T_integral[1] = T_integral[0] + T_error[1] * iteration_time;  // 1 is current, 0 is old
  T_derivative = (T_error[1] - T_error[0]) / iteration_time;
  T_out = Kp_tilt * T_error[1] + Ki_tilt * T_integral[1] + Kd_tilt * T_derivative + T_bias;

  T_integral[0] = T_integral[1];  // time shift integral readings after out calculated
  T_error[0] = T_error[1];
  
  // balance rate control
  // int tiltRateReading = gyro.y;
  // Trate_error[1] = T_out - tiltRateReading; // tilt rate setpoint is output of the tilt controller - this involves actually thinking about real world tilt rate maybe?
  //                                       // units in rad/s?
  // Trate_integral[1] = Trate_integral[0] + Trate_error[1] * iteration_time;  // 1 is current, 0 is old
  // Trate_derivative = (Trate_error[1] - Trate_error[0]) / iteration_time;
  // Trate_out = Kp_tiltRate * Trate_error[1] + Ki_tiltRate * Trate_integral[1] + Kd_tiltRate * Trate_derivative + Trate_bias;

  // Trate_integral[0] = Trate_integral[1];  // time shift integral readings after out calculated
  // Trate_error[0] = Trate_error[1];

  // heading control to offset the wheels for rotation
  float headingReading = ypr[0] * 180/M_PI;
  H_error[1] = HEADING_SETPOINT - headingReading;
  H_integral[1] = H_integral[0] + H_error[1] * iteration_time;  // 1 is current, 0 is old
  H_derivative = (H_error[1] - H_error[0]) / iteration_time;
  H_out = Kp_heading * H_error[1] + Ki_heading * H_integral[1] + Kd_heading * H_derivative + H_bias;

  H_integral[0] = H_integral[1];  // time shift integral readings after out calculated
  H_error[0] = H_error[1];

  leftWheelDrive = T_out + H_out;  // voltage / pwm that will actually drive the wheels
  rightWheelDrive = T_out - H_out;

  leftWheelDrive = constrain(leftWheelDrive, -maxSpeed, maxSpeed);
  rightWheelDrive = -constrain(rightWheelDrive, -maxSpeed, maxSpeed);

  // WHIP
  if (tiltReading > preferences.getFloat("B2", 0) || (correcting1 && tiltReading > preferences.getFloat("B3", 0))) {
    int speed = preferences.getInt("S3", 0);
	  leftStepper.setMaxSpeed(speed);
	  rightStepper.setMaxSpeed(speed);
    leftWheelDrive = -speed;
    rightWheelDrive = speed;
    correcting1 = true;
  } else if (tiltReading < -preferences.getFloat("B2", 0) || (correcting2 && tiltReading < -preferences.getFloat("B3", 0))) {
    int speed = preferences.getInt("S3", 0);
	  leftStepper.setMaxSpeed(speed);
	  rightStepper.setMaxSpeed(speed);
    leftWheelDrive = speed;
    rightWheelDrive = -speed;
    correcting2 = true;
  }

  if (correcting1 && !(tiltReading > preferences.getFloat("B3", 0))) {
    correcting1 = false;
  }
  if (correcting2 && !(tiltReading < -preferences.getFloat("B3", 0))) {
    correcting2 = false;
  }

  leftStepper.setSpeed(leftWheelDrive);
	leftStepper.runSpeed();
  rightStepper.setSpeed(rightWheelDrive);
	rightStepper.runSpeed();

  delayMicroseconds(1000);
  // vTaskDelay(10);
}

float getDistance() {
  leftDistance = leftStepper.currentPosition()/200.0 * 0.175 * M_PI;
  rightDistance = rightStepper.currentPosition()/200.0 * 0.175 * M_PI;

  totalDistance = (leftDistance + rightDistance) / 2.0;

  // POSITION_SETPOINT = totalDistance + 0.1;

  return totalDistance;
}

void communicationCode(void* pvParameters) {
  // -------- OUTPUTS ---------- //
  for (;;) {
    float L = leftWheelDrive;
    float R = rightWheelDrive;
    SerialBT.print("L: ");
    SerialBT.print(L);
    SerialBT.print(", R: ");
    SerialBT.print(R);
    SerialBT.print(", Pitch: ");
    SerialBT.println(ypr[1] * 180/M_PI);

    if (SerialBT.available()) {
      String test = SerialBT.readString();
      if (test.substring(0,3) == "Pp1") {
        preferences.putFloat("Pp1", test.substring(3,test.length()-1).toFloat());
        // Kp_position = preferences.getInt("Pp", 0);
        SerialBT.print("Set Pp1 to ");
        SerialBT.println(preferences.getFloat("Pp1", 0));
      } else if (test.substring(0,3) == "Ii1") {
        preferences.putFloat("Ii1", test.substring(3,test.length()-1).toFloat());
        // Ki_position = preferences.getInt("Ii", 0);
        SerialBT.print("Set Ii1 to ");
        SerialBT.println(preferences.getFloat("Ii1", 0));
      } else if (test.substring(0,3) == "Dd1") {
        preferences.putFloat("Dd1", test.substring(3,test.length()-1).toFloat());
        // Kd_position = preferences.getInt("Dd", 0);
        SerialBT.print("Set Dd1 to ");
        SerialBT.println(preferences.getFloat("Dd1", 0));
      } else if (test.substring(0,3) == "Pp2") {
        preferences.putFloat("Pp2", test.substring(3,test.length()-1).toFloat());
        // Kp_position = preferences.getInt("Pp", 0);
        SerialBT.print("Set Pp2 to ");
        SerialBT.println(preferences.getFloat("Pp2", 0));
      } else if (test.substring(0,3) == "Ii2") {
        preferences.putFloat("Ii2", test.substring(3,test.length()-1).toFloat());
        // Ki_position = preferences.getInt("Ii", 0);
        SerialBT.print("Set Ii2 to ");
        SerialBT.println(preferences.getFloat("Ii2", 0));
      } else if (test.substring(0,3) == "Dd2") {
        preferences.putFloat("Dd2", test.substring(3,test.length()-1).toFloat());
        // Kd_position = preferences.getInt("Dd", 0);
        SerialBT.print("Set Dd2 to ");
        SerialBT.println(preferences.getFloat("Dd2", 0));
      } else if (test.substring(0,2) == "P1") {
        preferences.putFloat("P1", test.substring(2,test.length()-1).toFloat());
        // Kp_tilt = preferences.getFloat("P", 0);
        SerialBT.print("Set P1 to ");
        SerialBT.println(preferences.getFloat("P1", 0), 4);
      } else if (test.substring(0,2) == "I1") {
        preferences.putFloat("I1", test.substring(2,test.length()-1).toFloat());
        // Kp_tilt = preferences.getFloat("P", 0);
        SerialBT.print("Set I1 to ");
        SerialBT.println(preferences.getFloat("I1", 0), 4);
      } else if (test.substring(0,2) == "D1") {
        preferences.putFloat("D1", test.substring(2,test.length()-1).toFloat());
        // Kp_tilt = preferences.getFloat("P", 0);
        SerialBT.print("Set D1 to ");
        SerialBT.println(preferences.getFloat("D1", 0), 4);
      } else if (test.substring(0,2) == "P2") {
        preferences.putFloat("P2", test.substring(2,test.length()-1).toFloat());
        // Kp_tilt = preferences.getFloat("P", 0);
        SerialBT.print("Set P2 to ");
        SerialBT.println(preferences.getFloat("P2", 0), 4);
      } else if (test.substring(0,2) == "I2") {
        preferences.putFloat("I2", test.substring(2,test.length()-1).toFloat());
        // Kp_tilt = preferences.getFloat("P", 0);
        SerialBT.print("Set I2 to ");
        SerialBT.println(preferences.getFloat("I2", 0), 4);
      } else if (test.substring(0,2) == "D2") {
        preferences.putFloat("D2", test.substring(2,test.length()-1).toFloat());
        // Kp_tilt = preferences.getFloat("P", 0);
        SerialBT.print("Set D2 to ");
        SerialBT.println(preferences.getFloat("D2", 0), 4);
      } else if (test.substring(0,2) == "S1") {
        preferences.putInt("S1", test.substring(2,test.length()-1).toInt());
        // maxSpeed1 = preferences.getInt("S1", 0);
        SerialBT.print("Set S1 to ");
        SerialBT.println(preferences.getInt("S1", 0));
      } else if (test.substring(0,2) == "S2") {
        preferences.putInt("S2", test.substring(2,test.length()-1).toInt());
        // maxSpeed2 = preferences.getInt("S2", 0);
        SerialBT.print("Set S2 to ");
        SerialBT.println(preferences.getInt("S2", 0));
      } else if (test.substring(0,2) == "S3") {
        preferences.putInt("S3", test.substring(2,test.length()-1).toInt());
        // maxSpeed2 = preferences.getInt("S2", 0);
        SerialBT.print("Set S3 to ");
        SerialBT.println(preferences.getInt("S3", 0));
      } else if (test.substring(0,1) == "B1") {
        preferences.putFloat("B1", test.substring(2,test.length()-1).toFloat());
        // maxSpeed2 = preferences.getInt("S2", 0);
        SerialBT.print("Set B1 to ");
        SerialBT.println(preferences.getFloat("B1", 0));
      } else if (test.substring(0,2) == "B2") {
        preferences.putFloat("B2", test.substring(2,test.length()-1).toFloat());
        // maxSpeed2 = preferences.getInt("S2", 0);
        SerialBT.print("Set B2 to ");
        SerialBT.println(preferences.getFloat("B2", 0));
      } else if (test.substring(0,2) == "B3") {
        preferences.putFloat("B3", test.substring(2,test.length()-1).toFloat());
        // maxSpeed2 = preferences.getInt("S2", 0);
        SerialBT.print("Set B3 to ");
        SerialBT.println(preferences.getFloat("B3", 0));
      } else if (test[0] == 'C') {
        SerialBT.print("P1: ");
        SerialBT.print(preferences.getFloat("P1", 0), 4);
        SerialBT.print(" I1: ");
        SerialBT.print(preferences.getFloat("I1", 0), 4);
        SerialBT.print(" D1: ");
        SerialBT.print(preferences.getFloat("D1", 0), 4);
        SerialBT.print(" P2: ");
        SerialBT.print(preferences.getFloat("P2", 0), 4);
        SerialBT.print(" I2: ");
        SerialBT.print(preferences.getFloat("I2", 0), 4);
        SerialBT.print(" D2: ");
        SerialBT.print(preferences.getFloat("D2", 0), 4);
        SerialBT.print(" Pp1: ");
        SerialBT.print(preferences.getFloat("Pp1", 0), 4);
        SerialBT.print(" Ii1: ");
        SerialBT.print(preferences.getFloat("Ii1", 0), 4);
        SerialBT.print(" Dd1: ");
        SerialBT.print(preferences.getFloat("Dd1", 0), 4);
        SerialBT.print(" Pp2: ");
        SerialBT.print(preferences.getFloat("Pp2", 0), 4);
        SerialBT.print(" Ii2: ");
        SerialBT.print(preferences.getFloat("Ii2", 0), 4);
        SerialBT.print(" Dd2: ");
        SerialBT.print(preferences.getFloat("Dd2", 0), 4);
        SerialBT.print(" maxSpeed1: ");
        SerialBT.print(preferences.getInt("S1", 0));
        SerialBT.print(" maxSpeed2: ");
        SerialBT.print(preferences.getInt("S2", 0));
        SerialBT.print(" correct speed: ");
        SerialBT.print(preferences.getInt("S3", 0));
      }
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