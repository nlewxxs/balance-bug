#include <Wire.h>
#include "mpu6050.h"
#include <WiFi.h>
#include <HTTPClient.h>

TaskHandle_t communication;  // task on core 2 for communications - I am not setting the task on core 1 as this is default and I want the controller to have as much space as it needs

const char* ssid = "CommunityFibre10Gb_AF5A8";
const char* password = "dvasc4xppp";

//Your Domain name with URL path or IP address with path
String serverName = "http://192.168.1.16:8081";  // local ip of the backend host (NOT localhost)

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)
unsigned long timerDelay = 5000;




// heading PD controller
double Kp_heading = 0.8;
double Ki_heading = 0;
double Kd_heading = 0.2;

// position PD controller
double Kp_position = 0.0005;
double Ki_position = 0;
double Kd_position = 0.75;

// balance PID controller
double Kp_tilt = 5;        //175
double Ki_tilt = 0.75;     //5
double Kd_tilt = 1;        //8.5
double balanceCenter = 0;  // whatever tilt value is balanced
double P_bias, T_bias, H_bias = 0;

double H_derivative, H_out, P_derivative, P_out, T_derivative, T_out;
double H_error[2], P_error[2], T_error[2] = { 0, 0 };           //stores current and previous value for derivative calculation
double H_integral[2], P_integral[2], T_integral[2] = { 0, 0 };  //stores current and previous value for integral calculation
double iteration_time;
unsigned long oldMillis = 0;

mpu6050 mpu = mpu6050();

const byte led_gpio = 32;

void setup() {
  Serial.begin(9600);
  mpu.init();

  mpu.calibrate();

  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
    communicationCode, /* Task function. */
    "communication",   /* name of task. */
    20000,             /* Stack size of task */
    NULL,              /* parameter of the task */
    1,                 /* priority of the task */
    &communication,    /* Task handle to keep track of created task */
    0);                /* pin task to core 0 -- by default we pin to core 1 */
  delay(500);
}

void loop() {
  // Serial.println(xPortGetCoreID());
  mpu.update();
  double iteration_time = (millis() - oldMillis) / 1000.0;
  oldMillis = millis();

  // position control first
  double positionReading = getPosition();  // example I used for the controller has position read from rotary encoders - this value is total distance travelled
  P_error[1] = getPosSetpoint() - positionReading;
  P_integral[1] = P_integral[0] + P_error[1] * iteration_time;  // 1 is current, 0 is old
  P_derivative = (P_error[1] - P_error[0]) / iteration_time;
  P_out = Kp_position * P_error[1] + Ki_position * P_integral[1] + Kd_position * P_derivative + P_bias;
  // make this output positive to move forwards, negative to move backwards

  P_integral[0] = P_integral[1];  // time shift integral readings after out calculated
  P_error[0] = P_error[1];

  // balance control
  double tiltReading = mpu.getPitch();
  T_error[1] = balanceCenter + P_out - tiltReading;             // offset the tilt reading with the distance output to allow us to manipulate the position of the robot via tilt
  T_integral[1] = T_integral[0] + T_error[1] * iteration_time;  // 1 is current, 0 is old
  T_derivative = (T_error[1] - T_error[0]) / iteration_time;
  T_out = Kp_tilt * T_error[1] + Ki_tilt * T_integral[1] + Kd_tilt * T_derivative + T_bias;

  T_integral[0] = T_integral[1];  // time shift integral readings after out calculated
  T_error[0] = T_error[1];

  // heading control to offset the wheels for rotation
  double headingReading = mpu.getYaw();
  H_error[1] = getHeadingSetpoint() - headingReading;
  H_integral[1] = H_integral[0] + H_error[1] * iteration_time;  // 1 is current, 0 is old
  H_derivative = (H_error[1] - H_error[0]) / iteration_time;
  H_out = Kp_heading * H_error[1] + Ki_heading * H_integral[1] + Kd_heading * H_derivative + H_bias;

  H_integral[0] = H_integral[1];  // time shift integral readings after out calculated
  H_error[0] = H_error[1];

  double leftWheelDrive = T_out - H_out;  // voltage / pwm that will actually drive the wheels
  double rightWheelDrive = T_out + H_out;

  Serial.print("Wheel inputs: ");
  Serial.print(leftWheelDrive);
  Serial.print(" / ");
  Serial.println(rightWheelDrive);
}

double getPosition() {  //unsure of how this reading will work - needs to be 1D (as in just x)
  // could potentially have position just be a relative thing i.e. move forwards 1 / backwards 1 rather than move to position 23?
  return 0;
}

double getPosSetpoint() {
  // output from the imaging program to tell us where to be
  return 0;
}

double getHeadingSetpoint() {
  // output from the imaging program to tell us where to face
  return 0;
}



void communicationCode(void* pvParameters) {
  // Serial.println(xPortGetCoreID());
  // void communicationCode() {

  // wifi setup
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");

  // looping code - this takes up entirety of cpu time along with controller so NEEDS the delay to allow idle tasks to execute
  for (;;) {
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
    vTaskDelay(5000); //delay important to allow idle tasks to execute else processor reboots
  }
}
