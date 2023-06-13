#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
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

TaskHandle_t communication;  // task on core 0 for communication

const char* ssid = "CommunityFibre10Gb_AF5A8";
const char* password = "dvasc4xppp";
String serverName = "http://192.168.1.16:8081";  // local ip of the backend host (NOT localhost)
unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

int sckdelay = 0.001;  // arbitrary number tbh can tweak this
int fpga_cs = 4;  // fpga "chip select" - selects the FPGA as the slave
int buf = 0;  // recv buffer

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< //

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> //
// ----------------- CORE 1 DEFINITIONS ----------------- //

#define HEADING_SETPOINT 0
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
float Kp_heading = 0; //0.8
float Ki_heading = 0;
float Kd_heading = 0; //0.2

// position PD controller -- Kp = -0.00123, Ki = -1.88e-06, Kd = -0.202
// float Kp_position = 12;
float Kp_position = 40;
float Ki_position = 0;
float Kd_position = 0;
// float Kp_position = 40;
// float Ki_position = 0;
// float Kd_position = 0.1;
// float Kp_position = -0.00123;
// float Ki_position = -0.00000186;
// float Kd_position = -0.202;
// float Kp_position = 0.0005;
// float Ki_position = 0;
// float Kd_position = 0.75;

int maxSpeed = 1000;
int acceleration = 600;


// balance PID controller -- Kp = 18.3, Ki = 26.3, Kd = 1.88
// float Kp_tilt = 13.5;  //13.5      //175 // 16 //2min17s
// float Ki_tilt = 0;     //5
// float Kd_tilt = 0.0012;  //0.0012      //8.5 // 0.0013
float Kp_tilt = 75;  //13.5      //175 // 16 //2min17s
float Ki_tilt = 0;     //5
float Kd_tilt = 0;  //0.0012      //8.5 // 0.0013
// float Kp_tilt = 13.5;      //175 // 16 //2min17s
// float Ki_tilt = 0;     //5
// float Kd_tilt = 0.0012;      //8.5 // 0.0013
// float Kp_tilt = 43;  //13.5      //175 // 16 //2min17s
// float Ki_tilt = 14.4;     //5
// float Kd_tilt = 0.02;  //0.0012      //8.5 // 0.0013
// float Kp_tilt = 2;        //175
// float Ki_tilt = 0.0075;     //5
// float Kd_tilt = 2;        //8.5


// float Kp_tiltRate = 75;  //13.5      //175 // 16 //2min17s
// float Ki_tiltRate = 0;     //5
// float Kd_tiltRate = 0;  //0.0012      //8.5 // 0.0013

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

  // configure SPI
  // pinMode(fpga_cs, OUTPUT);
  // SPI.begin();

  // initialize device
  mpu.initialize();
  SerialBT.println(F("Testing device connections..."));
  SerialBT.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));
  Serial.println(F("Testing device connections..."));
  Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));

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

  } else {
      // ERROR!
      // 1 = initial memory load failed
      // 2 = DMP configuration updates failed
      // (if it's going to break, usually the code will be 1)
      SerialBT.print(F("DMP Initialization failed (code "));
      SerialBT.print(devStatus);
      SerialBT.println(F(")"));
  }

  preferences.begin("bastard", false);

  Kp_tilt = preferences.getFloat("P", 0);
  Ki_tilt = preferences.getFloat("I", 0);
  Kd_tilt = preferences.getFloat("D", 0);
  Kp_position = preferences.getFloat("Pp", 0);
  Ki_position = preferences.getFloat("Ii", 0);
  Kd_position = preferences.getFloat("Dd", 0);
  acceleration = preferences.getInt("A", 0);
  maxSpeed = preferences.getInt("S", 0);

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
	// leftStepper.setMinPulseWidth(50);
	rightStepper.setMaxSpeed(maxSpeed);
	rightStepper.setAcceleration(acceleration);
	// rightStepper.setMinPulseWidth(50);
  
  delay(1000);
}

void loop() {

  if (!dmpReady) return;  // hang program if programming did not work

  // Serial.println(xPortGetCoreID());

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

  // position control first
  float positionReading = getDistance();  // example I used for the controller has position read from rotary encoders - this value is total distance travelled
  P_error[1] = POSITION_SETPOINT - positionReading;
  P_integral[1] = P_integral[0] + P_error[1] * iteration_time;  // 1 is current, 0 is old
  P_derivative = (P_error[1] - P_error[0]) / iteration_time;
  P_out = Kp_position * P_error[1] + Ki_position * P_integral[1] + Kd_position * P_derivative + P_bias;
  // make this output positive to move forwards, negative to move backwards

  P_integral[0] = P_integral[1];  // time shift integral readings after out calculated
  P_error[0] = P_error[1];

  // P_out = constrain(P_out, -15, 15);

  // balance control
  float tiltReading = ypr[1] * 180/M_PI;
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
  // leftWheelDrive = T_out;  // voltage / pwm that will actually drive the wheels
  // rightWheelDrive = T_out;

  leftWheelDrive = constrain(leftWheelDrive, -maxSpeed, maxSpeed);
  rightWheelDrive = -constrain(rightWheelDrive, -maxSpeed, maxSpeed);

  // leftWheelDrive = 200;
  // rightWheelDrive = 200;

  // if (tiltReading > 5) {
  //   leftWheelDrive = rightWheelDrive = 150;
  // } else if (tiltReading < -5) {
  //   leftWheelDrive = rightWheelDrive = -150;
  // }

  
	// Move the motor one step
  if (leftWheelDrive > 0) {
    leftStepper.moveTo(leftStepper.currentPosition() + 20000);
  } else {
    leftStepper.moveTo(leftStepper.currentPosition() - 20000);
  }
  leftStepper.setSpeed(leftWheelDrive);
	leftStepper.run();
  if (rightWheelDrive > 0) {
    rightStepper.moveTo(rightStepper.currentPosition() + 20000);
  } else {
    rightStepper.moveTo(rightStepper.currentPosition() - 20000);
  }
  rightStepper.setSpeed(rightWheelDrive);
	rightStepper.run();
	// leftStepper.run();
	// leftStepper.setSpeed(200);
	// rightStepper.run();  
	// rightStepper.setSpeed(200);

  delayMicroseconds(1000);
}

float square(float x) {
  return x * x;
}

float getDistance() {  //unsure of how this reading will work - needs to be 1D (as in just x)
  // could potentially have position just be a relative thing i.e. move forwards 1 / backwards 1 rather than move to position 23?

  leftDistance = leftStepper.currentPosition()/200.0 * 0.175 * M_PI;
  rightDistance = rightStepper.currentPosition()/200.0 * 0.175 * M_PI;

  // if ((abs(leftDistance) >= abs(rightDistance) && leftDistance >= 0) || (abs(leftDistance) < abs(rightDistance) && rightDistance >= 0)) {
  //   totalDistance = sqrt(square(leftDistance) + square(rightDistance)) / 2.0;
  // } else {
  //   totalDistance = -sqrt(square(leftDistance) + square(rightDistance)) / 2.0;
  // }
  totalDistance = (leftDistance + rightDistance) / 2.0;

  // POSITION_SETPOINT = totalDistance + 0.1;

  return totalDistance;

  // return 0;
}



void communicationCode(void* pvParameters) {
  // -------- OUTPUTS ---------- //
  for (;;) {
    float L = leftWheelDrive;
    float R = rightWheelDrive;
    SerialBT.print("L: ");
    SerialBT.print(L);
    SerialBT.print(", R: ");
    SerialBT.println(R);
    // Serial.print("L: ");
    // Serial.print(L);
    // Serial.print(", R: ");
    // Serial.println(R);

    if (SerialBT.available()) {
      String test = SerialBT.readString();
      if (test.substring(0,2) == "Pp") {
        preferences.putInt("Pp", test.substring(2,test.length()-1).toFloat());
        Kp_position = preferences.getInt("Pp", 0);
        SerialBT.print("Set Pp to ");
        SerialBT.println(Kp_position);
      } else if (test.substring(0,2) == "Ii") {
        preferences.putInt("Ii", test.substring(2,test.length()-1).toFloat());
        Ki_position = preferences.getInt("Ii", 0);
        SerialBT.print("Set Ii to ");
        SerialBT.println(Ki_position);
      } else if (test.substring(0,2) == "Dd") {
        preferences.putInt("Dd", test.substring(2,test.length()-1).toFloat());
        Kd_position = preferences.getInt("Dd", 0);
        SerialBT.print("Set Dd to ");
        SerialBT.println(Kd_position);
      } else if (test[0] == 'P') {
        preferences.putFloat("P", test.substring(1,test.length()-1).toFloat());
        Kp_tilt = preferences.getFloat("P", 0);
        SerialBT.print("Set P to ");
        SerialBT.println(Kp_tilt);
      } else if (test[0] == 'I') {
        preferences.putFloat("I", test.substring(1,test.length()-1).toFloat());
        Ki_tilt = preferences.getFloat("I", 0);
        SerialBT.print("Set I to ");
        SerialBT.println(Ki_tilt);
      } else if (test[0] == 'D') {
        preferences.putFloat("D", test.substring(1,test.length()-1).toFloat());
        Kd_tilt = preferences.getFloat("D", 0);
        SerialBT.print("Set D to ");
        SerialBT.println(Kd_tilt);
      } else if (test[0] == 'A') {
        preferences.putInt("A", test.substring(1,test.length()-1).toInt());
        acceleration = preferences.getInt("A", 0);
        SerialBT.print("Set A to ");
        SerialBT.println(acceleration);
        leftStepper.setAcceleration(acceleration);
        rightStepper.setAcceleration(acceleration);
      } else if (test[0] == 'S') {
        preferences.putInt("S", test.substring(1,test.length()-1).toInt());
        maxSpeed = preferences.getInt("S", 0);
        SerialBT.print("Set S to ");
        SerialBT.println(maxSpeed);
        leftStepper.setMaxSpeed(maxSpeed);
        rightStepper.setMaxSpeed(maxSpeed);
      }
    }
    // SerialBT.print(" T: ");
    // SerialBT.print(T_out);
    // SerialBT.print(" H: ");
    // SerialBT.print(H_out);
    // getDistance();
    // Serial.print(" L_D: ");
    // Serial.print(leftDistance);
    // Serial.print(" R_D: ");
    // Serial.print(rightDistance);
    // SerialBT.print(" L_D: ");
    // SerialBT.print(leftDistance);
    // SerialBT.print(" R_D: ");
    // SerialBT.print(rightDistance);
    // SerialBT.print(" P_out: ");
    // SerialBT.print(P_out);
    // SerialBT.print(" Distance: ");
    // SerialBT.print(totalDistance);
    // SerialBT.print("  pitch: ");
    // SerialBT.print(ypr[1] * 180/M_PI);
    // SerialBT.print(" roll: ");
    // SerialBT.print(ypr[2] * 180/M_PI);
    // SerialBT.print(" yaw: ");
    // SerialBT.println(ypr[0] * 180/M_PI);
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
//     recvSPIbytes();
//     vTaskDelay(20);
//   }
// }

// void recvSPIbytes(){  // receives 128 bytes of SPI

//   digitalWrite(fpga_cs, LOW); // SPI is active-low
//   delay(10);

//   for (int j = 0; j < 16; j++){
//     for (int i = 0; i < 8; i++){
//       buf = SPI.transfer(0xFF);
//       // Serial.println(buf);
//     }
//     delay(10); // this can be decreased
//   }

//   digitalWrite(fpga_cs, HIGH); // stop FPGA sending
//   delay(10);
// }