/*
 * Based upon code written by Yue Zhu (yue.zhu18@imperial.ac.uk) in July 2020.
 * pin6 is PWM output at 62.5kHz.
 * duty-cycle saturation is set as 2% - 98%
 * Control frequency is set as 1kHz. 

 * modified now to be used just as buck for pico w by Jacob L
*/

#include <RPi_Pico_TimerInterrupt.h>
#include <WiFi.h>
#include<WiFiUDP.h>
//#include <rp2040.h>
float closed_loop = 0; // Duty Cycles
float vin,vout,vret,dutyref,current_ma, loadCurrent; // Measurement Variables
unsigned int sensorValue0,sensorValue1,sensorValue2;  // ADC sample values declaration
float ev=0,cv=0,ei=0,oc=0; //internal signals
float Ts=0.001; //1 kHz control frequency. It's better to design the control period as integral multiple of switching period.
float kpv=0.05024,kiv=15.78,kdv=0; // voltage pid.
float u0v,u1v,delta_uv,e0v,e1v,e2v; // u->output; e->error; 0->this time; 1->last time; 2->last last time
float kpi=0.005,kii=35.95,kdi=0.0; // float kpi=0.02512,kii=39.4,kdi=0; // current pid.
float u0i,u1i,delta_ui,e0i,e1i,e2i; // Internal values for the current controller
float uv_max=4, uv_min=0; //anti-windup limitation
float ui_max=50, ui_min=0; //anti-windup limitation
float current_limit = 0.350;
unsigned int loopTrigger;
unsigned int com_count=0;   // a variables to count the interrupts. Used for program debugging.
int counter = 0;


// setpoint, held constant for now
// between 0 and 1023 as a proportion of maximum current
float setPoint = 100;
float range = 1024;

int P0 = A0; //GP26 or Pin 31
int P1 = A1; //GP27 or Pin 32
int P2 = A2; //GP28 or Pin 34
int pwmIn = 16;
int gateEnable = 1;

int pwmSignal;

//wifi
const char* ssid = "Mi 9T Pro";
const char* password = "randompass";
// const char* ssid = "WIN-I3U26E5IBMR 2736";
// const char* password = "361Q7{0b";
int keyIndex = 0;            // your network key Index number (needed only for WEP)


unsigned int localPort = 2390;      // local port to listen on

char packetBuffer[255]; //buffer to hold incoming packet
char  ReplyBuffer[4] = "";       // a string to send back

WiFiUDP Udp;

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
  if(digitalRead(pwmIn)){
    analogWrite(0,(int)(pwm_input*255)); 
  }
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

float getCurrent(){
  analogReadResolution(12);
  vret = ((analogRead(P0)*100)*33/1027); // Convert GPIO26/ADC0 to the actual voltage across the current sense in mV
  current_ma = vret/1.02;  // convert vret to current in mili amps
  return current_ma;
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
  // start up delay
  // 0 for yellow, 1 second for red, two seconds for blue
  delay(1000);
  //Initialize serial and wait for port to open
  Serial.begin(115200);
  pinMode(15, OUTPUT);
  digitalWrite(15, 1);
  Serial.println("Connecting to WiFi");
  //WiFi.config(ip, gateway, subnet);
  WiFi.begin(ssid, password);
 
  // Print periods on monitor while establishing connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    delay(500);
    counter ++;
    if (counter == 5){
      // if after 10 second wifi isnt connected, retry
      counter = 0;
      Serial.println("Connection failed, restarting");
      WiFi.begin(ssid, password);
    }
  }

  // PWM setup
  analogWriteFreq(100000);
  analogWriteRange(255);
  pwm_modulate(0.1);
  Serial.println("Starting"); 

  // Connection established
  digitalWrite(15, 0);
  Serial.println("");
  Serial.print("Pico W is connected to WiFi network ");
  Serial.println(WiFi.SSID());
 
  // Print IP Address
  Serial.print("Assigned IP Address: ");
  Serial.println(WiFi.localIP());

  // Serial.println("\nStarting connection to server...");
  // // if you get a connection, report back via serial
  // if (!Udp.begin(localPort)){
  //   Serial.println("UDP Failed to start");
  // }

  Udp.begin(2390);

  delay(500);
  noInterrupts(); //disable all interrupts
  pinMode(P0, INPUT);
  pinMode(P1, INPUT);
  pinMode(P2, INPUT);
  pinMode(13, OUTPUT);  //Pin13 is used to time the loops of the controller

  // gate enable
  pinMode(pwmIn, INPUT);
  pinMode((pwmIn+1), INPUT);
  pinMode(gateEnable, OUTPUT);
  // Control loop timer setup
  controlTimer0.attachInterruptInterval(CONTROL_LOOP_TIMER_INTERVAL_US, controlLoopRunCallback);
  interrupts();  //enable interrupts.  
}

 void loop() {
  // pwm enable shizzz
  pwmSignal = digitalRead(pwmIn);
  digitalWrite(gateEnable, pwmSignal);    // pwm controlled
  //digitalWrite(gateEnable, 1);              // constant on
  // //loopTrigger = 0;
  if(loopTrigger && pwmSignal) { // This loop is triggered, it wont run unless there is an interrupt
    
    digitalWrite(13, HIGH);   // set pin 13. Pin13 shows the time consumed by each control cycle. It's used for debugging.
    
    // Sample all of the measurements and check which control mode we are in
    sampling();    
    // Closed Loop Buck
    // The closed loop path has a voltage controller cascaded with a current controller. The voltage controller
    // creates a current demand based upon the voltage error. This demand is saturated to give current limiting.
    // The current loop then gives a duty cycle demand based upon the error between demanded current and measured
    // current
    current_limit = 90; // Yellow
    //current_limit = 100; // blue
    // cuurrent_limit = 100; // red
    // only using current controller for now
    cv = setPoint/range * current_limit;
    cv = saturation(cv, current_limit, 0);
    ei=cv - current_ma; //current error
    //ei = saturation(ei, 90, -90);
    delta_ui=pidi(ei);  //current pid
    delta_ui = delta_ui * 0.02;
    closed_loop = delta_ui;
    closed_loop=saturation(closed_loop,0.486,0.01);  //duty_cycle saturation
    pwm_modulate(closed_loop); //pwm modulation
   

    com_count++;              //used for debugging.
    if (com_count >= 100) {  //send out data every second.
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
  // //Serial.println("Test");

  // check if packet
  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remoteIp = Udp.remoteIP();
    Serial.print(remoteIp);
    Serial.print(", port ");
    Serial.println(Udp.remotePort());
    int len = Udp.read(packetBuffer, 255);
    int val[2]; 
    Serial.println(len);
    for(byte i=0; i<len; i++){
      Serial.print("packetBuffer[");
      Serial.print(i);
      Serial.print("] = ");
      val[i] = packetBuffer[i];
      Serial.println((val[i]));
      //Serial.println((int) strtol(packetBuffer[i], 0, 16));
    }
    Serial.print("\nVal:  ");
    Serial.println((val[1]*256) + val[0]);
    setPoint = (val[1] * 256) + val[0];
  }
}

/*end of the program.*/




