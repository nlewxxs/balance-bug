
/*
 * Based upon code written by Yue Zhu (yue.zhu18@imperial.ac.uk) in July 2020.
 * pin6 is PWM output at 62.5kHz.
 * duty-cycle saturation is set as 2% - 98%
 * Control frequency is set as 1kHz. 

 * modified now to be used just as buck for pico w by Jacob L as a single current controller
 * then modified to be controllable over UDP
*/

#include <RPi_Pico_TimerInterrupt.h> 
// Include the WiFi Library
#include <WiFi.h>
#include <WiFiUDP.h>

// variables
float closed_loop = 0; // Duty Cycles
float vin,vout,vret,dutyref,current_ma, loadCurrent; // Measurement Variables
unsigned int sensorValue0,sensorValue1,sensorValue2;  // ADC sample values declaration
float cv=0,ei=0; //internal signals
float Ts=0.001; //1 kHz control frequency. It's better to design the control period as integral multiple of switching period.
float kpi=0.005,kii=35.95,kdi=0.0; // float kpi=0.02512,kii=39.4,kdi=0; // current pid.
float u0i,u1i,delta_ui,e0i,e1i,e2i; // Internal values for the current controller
float ui_max=50, ui_min=0; //anti-windup limitation
float current_limit = 0.350;
unsigned int loopTrigger;
unsigned int com_count=0;   // a variables to count the interrupts. Used for program debugging.

// setpoint, held constant for now
// between 0 and 1023 as a proportion of maximum current
float setPoint = 100;
float range = 1024;


// pins
int gateEnable = 1;
int pwmIn = 39;
int P0 = A0; //GP26 or Pin 31
int P1 = A1; //GP27 or Pin 32
int P2 = A2; //GP28 or Pin 34

// wifi setuop
int localPort = 2390;
char packetBuffer[255]; // buffer to hold incoming packets
char replyBuffer[4] = ""; // buffer for outgoing packets

WiFiUDP udp;

// Replace with your network credentials
const char* ssid = "Mi 9T Pro";
const char* password = "randompass";

// Timer 1 for control loop
#define CONTROL_LOOP_TIMER_INTERVAL_US 1000
RPI_PICO_Timer controlTimer0(0);

// Timer ISR
bool controlLoopRunCallback(struct repeating_timer *t){
  loopTrigger=1;
  return true;
}

float saturation( float sat_input, float uplim, float lowlim){ // Saturatio function
  if (sat_input > uplim) sat_input=uplim;
  else if (sat_input < lowlim ) sat_input=lowlim;
  else;
  return sat_input;
}

void pwm_modulate(float pwm_input){ // PWM function
  analogWrite(0,(int)(pwm_input*255)); 
}

// This is a PID controller for the current
//. N.B d gain is 0 so actually PI
float pidi(float pid_input){
  float e_integration;
  e0i = pid_input;
  e_integration=e0i;
  
  //anti-windup
  if(u1i >= ui_max){
    e_integration = 0;
  } else if (u1i <= ui_min) {
    e_integration = 0;
  }
  
  delta_ui = kpi*(e0i-e1i) + kii*Ts*e_integration + kdi/Ts*(e0i-2*e1i+e2i); //incremental PID programming avoids integrations.
  u0i = u1i + delta_ui;  //this time's control output

  //output limitation
  saturation(u0i,ui_max,ui_min);
  
  u1i = u0i; //update last time's control output
  e2i = e1i; //update last last time's error
  e1i = e0i; // update last time's error
  return u0i;
}

// This subroutine processes all of the analogue samples, creating the required values for the main loop
void sampling(){

  // Make the initial sampling operations for the circuit measurements
  analogReadResolution(12);
  sensorValue0 = analogRead(P0); //sample Vret
  analogReadResolution(12);
  sensorValue1 = analogRead(P1); //sample Vin, handle weird behaviour
  analogReadResolution(12);
  sensorValue2 = analogRead(P2); //sample Vout, handle weird behaviour

  // Process the values so they are a bit more usable/readable
  // The analogRead process gives a value between 0 and 1023 
  // representing a voltage between 0 and the analogue reference which is 4.096V
  
  vret = ((sensorValue0*100)*33/4095); // Convert GPIO26/ADC0 to the actual voltage across the current sense in mV
  vin = ((sensorValue1*100)*33/4095)*(1249/249); // Convert GPIO27/ADC1 to the voltage at the input in mV
  vout = ((sensorValue2*100)*33/4095)*(1249/249); // Convert GPIO28/ADC2 to the voltage at the output in mV
  current_ma = vret/1.02;  // convert vret to current in mili amps
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void setup() {
 
  // Start the Serial Monitor
  Serial.begin(115200);

  Serial.println("Serial connected and now waiting");
  delay(500);
 
  // Operate in WiFi Station mode
  WiFi.mode(WIFI_STA);

  WiFi.begin(ssid, password);
 
  // Print periods on monitor while establishing connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    delay(500);
  }
  printWifiStatus();

  // Connection established
  Serial.println("");
  Serial.print("Pico W is connected to WiFi network ");
  Serial.println(WiFi.SSID());
 
  // Print IP Address
  Serial.print("Assigned IP Address: ");
  Serial.println(WiFi.localIP());

  Serial.print("Subnet mask:  ");
  Serial.println(WiFi.subnetMask());


  udp.begin(localPort);

  delay(500);
  noInterrupts(); //disable all interrupts
  pinMode(pwmIn, INPUT);
  pinMode(gateEnable, OUTPUT);
  pinMode(13, OUTPUT);  //Pin13 is used to time the loops of the controller

  // Control loop timer setup
  controlTimer0.attachInterruptInterval(CONTROL_LOOP_TIMER_INTERVAL_US, controlLoopRunCallback);

  // PWM setup
  analogWriteFreq(100000);
  analogWriteRange(255);
  
  Serial.println("Outputting for ~43.75mA");
  pwm_modulate(0.0);
  Serial.println("Starting"); 
  interrupts();  //enable interrupts.  
 
}
 
void loop() {
  //digitalWrite(gateEnable, digitalRead(pwmIn));
  digitalWrite(gateEnable, 1);
  delay(10);

  if(loopTrigger) { // This loop is triggered, it wont run unless there is an interrupt    
    digitalWrite(13, HIGH);   // set pin 13. Pin13 shows the time consumed by each control cycle. It's used for debugging.
  
    // Sample all of the measurements and check which control mode we are in
    sampling();    
    // Closed Loop Buck
    // The closed loop path has a current controller which sets output current to setPoint
    // current limit depends on LED
    current_limit = 90; // Yellow
    // current_limit = 100; // blue
    // cuurrent_limit = 100; // red
    // only using current controller for now
    cv = setPoint/range * current_limit;
    cv = saturation(cv, current_limit, 0);
    ei=cv - current_ma; //current error
    delta_ui=pidi(ei);  //current pid
    delta_ui = delta_ui * 0.02;
    closed_loop = delta_ui;
    closed_loop=saturation(closed_loop,0.486,0.01);  //duty_cycle saturation
    pwm_modulate(closed_loop); //pwm modulation
    com_count++;              //used for debugging.
    if (com_count >= 1000) {  //send out data every second.
      Serial.print("CV Setpoint:  ");
      Serial.print(cv);
      Serial.print("\nei: ");
      Serial.print(ei);

      Serial.print("\tControl Output: ");
      Serial.print(delta_ui);

      Serial.print("\tPWM:  ");
      Serial.print(closed_loop);

      Serial.print("\nVin: ");
      Serial.print(sensorValue1);
      Serial.print("\t");
      Serial.print(vin);
      Serial.print("\t");

      Serial.print("Vout: ");
      Serial.print(sensorValue2);
      Serial.print("\t");
      Serial.print(vout);
      Serial.print("\t");

      Serial.print("Output Current: ");
      Serial.print(sensorValue0);
      Serial.print("\t");
      Serial.print(current_ma);
      Serial.print("\n");

      // Print IP Address
      Serial.print("Assigned IP Address: ");
      Serial.println(WiFi.localIP());

      com_count = 0;
    }

    digitalWrite(13, LOW);   // reset pin13.
    loopTrigger = 0;
  }
 
  // Print IP Address
  int packetSize = udp.parsePacket();
  if (packetSize) {
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remoteIp = udp.remoteIP();
    Serial.print(remoteIp);
    Serial.print(", port ");
    Serial.println(udp.remotePort());
    int len = udp.read(packetBuffer, 255);
    int val[2]; 
    Serial.println(len);
    for(byte i=0; i<len; i++){
      Serial.print("packetBuffer[");
      Serial.print(i);
      Serial.print("] = 0x");
      val[i] = packetBuffer[i];
      Serial.println((val[i]));
      //Serial.println((int) strtol(packetBuffer[i], 0, 16));
    }
    Serial.print("\nVal:  ");
    Serial.println((val[0]));
    setPoint = val[0];
   }
}