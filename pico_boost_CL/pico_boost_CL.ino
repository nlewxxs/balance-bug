/*
 * Based upon code written by Yue Zhu (yue.zhu18@imperial.ac.uk) in July 2020.
 * pin6 is PWM output at 62.5kHz.
 * duty-cycle saturation is set as 2% - 98%
 * Control frequency is set as 1kHz. 
*/
#include <SPI.h>
#include <Wire.h>
#include <INA219_WE.h>

INA219_WE ina219; // this is the instantiation of the library for the current sensor
  uint16_t toSend;
float open_loop, closed_loop; // Duty Cycles
float va,vb,vref,iL,dutyref,current_mA, vCap; // Measurement Variables
unsigned int sensorValue0,sensorValue1,sensorValue2,sensorValue3, sensorValue4;  // ADC sample values declaration
float ev=0,cv=0,ei=0,oc=0; //internal signals
float Ts=0.00005; //1 kHz control frequency. It's better to design the control period as integral multiple of switching period.
float kpv=8,kiv=15,kdv=0; // voltage pid.
float u0v,u1v,delta_uv,e0v,e1v,e2v; // u->output; e->error; 0->this time; 1->last time; 2->last last time
float kpi=0.02512,kii=39.4,kdi=0; // float kpi=0.02512,kii=39.4,kdi=0; // current pid.
float u0i,u1i,delta_ui,e0i,e1i,e2i; // Internal values for the current controller
float uv_max=4, uv_min=0; //anti-windup limitation
float ui_max=50, ui_min=0; //anti-windup limitation
float current_limit = 1.0;
boolean Boost_mode = 0;
boolean CL_mode = 0;
unsigned int loopTrigger;
unsigned int com_count=0;   // a variables to count the interrupts. Used for program debugging.
float maxInCurrent = 0.6;
int voltageTarget;
int eV;
int charging;
int storedTargetVal;

uint8_t vbScaled, iScaled, vCapScaled;
uint16_t buf1 = 0;      // fpga SPI recv buffer
uint8_t buf2 = 0;
uint8_t buf3 = 0;
int fpgaCS = 4;
int spiTransfer = 0;
int spiTrigPin = 5;
int state = 0;
int targetVal = 512;
int newTargetVal = 512;
#define capInput A6;

// Timer A CMP1 interrupt. Every 800us the program enters this interrupt. 
// This, clears the incoming interrupt flag and triggers the main loop.

ISR(TCA0_CMP1_vect){
  TCA0.SINGLE.INTFLAGS |= TCA_SINGLE_CMP1_bm; //clear interrupt flag
  loopTrigger = 1;
}

void spiTrigCall(){
  spiTransfer = 1;
}
float saturation( float sat_input, float uplim, float lowlim){ // Saturatio function
  if (sat_input > uplim) sat_input=uplim;
  else if (sat_input < lowlim ) sat_input=lowlim;
  else;
  return sat_input;
}

void pwm_modulate(float pwm_input){ // PWM function
  analogWrite(6,(int)(255-pwm_input*255)); 
}

// This is a PID controller for the voltage

float pidv( float pid_input){
  float e_integration;
  e0v = pid_input;
  e_integration = e0v;
 
  //anti-windup, if last-time pid output reaches the limitation, this time there won't be any intergrations.
  if(u1v >= uv_max) {
    e_integration = 0;
  } else if (u1v <= uv_min) {
    e_integration = 0;
  }

  delta_uv = kpv*(e0v-e1v) + kiv*Ts*e_integration + kdv/Ts*(e0v-2*e1v+e2v); //incremental PID programming avoids integrations.there is another PID program called positional PID.
  u0v = u1v + delta_uv;  //this time's control output

  //output limitation
  saturation(u0v,uv_max,uv_min);
  
  u1v = u0v; //update last time's control output
  e2v = e1v; //update last last time's error
  e1v = e0v; // update last time's error
  return u0v;
}

// This is a PID controller for the current

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
  
  sensorValue0 = analogRead(A0); //sample Vb
  sensorValue2 = analogRead(A2); //sample Vref
  sensorValue3 = analogRead(A3); //sample Va
  sensorValue4 = analogRead(A6);  // sample Vcap
  current_mA = ina219.getCurrent_mA(); // sample the inductor current (via the sensor chip)

  // Process the values so they are a bit more usable/readable
  // The analogRead process gives a value between 0 and 1023 
  // representing a voltage between 0 and the analogue reference which is 4.096V
  vb = sensorValue0 * (12400/2400) * (4.096 / 1023.0); // Convert the Vb sensor reading to volts
  vref = sensorValue2 * (4.096 / 1023.0); // Convert the Vref sensor reading to volts
  va = sensorValue3 * (12400/2400) *  (4.096 / 1023.0); // Convert the Va sensor reading to volts
  vCap = sensorValue4 * (5.5) * (4.096 / 1023);  // convert v cap to volts
  // The inductor current is in mA from the sensor so we need to convert to amps.
  // We want to treat it as an input current in the Boost, so its also inverted
  // For open loop control the duty cycle reference is calculated from the sensor
  // differently from the Vref, this time scaled between zero and 1.
  // The boost duty cycle needs to be saturated with a 0.33 minimum to prevent high output voltages
  
  if (Boost_mode == 1){
    iL = -current_mA/1000.0;
    dutyref = saturation(sensorValue2 * (1.0 / 1023.0),0.99,0.13);
  }else{
    iL = current_mA/1000.0;
    dutyref = sensorValue2 * (1.0 / 1023.0);
  }
  
}

void spiFPGATransfer(){
  //Serial.println("SPI Transfer");
  
  //delay(10);   
  // does 24 bit transfer for sending panel voltage, panel current, capacitor voltage to FPGA
  //Serial.println("SPI Transfer");
  // first panel voltage
  sampling();
  vbScaled = vb/6 * 255; // 8 bit value, its position between 0 and 256 is the voltage proportion between 0 and 6
  //buf1 = SPI.transfer(vbScaled);
  // next panel current
  iScaled = -1*iL/0.8 * 255;   // 8 bit value, prooprtional to current between 0 and 0.8A
  vCapScaled = vCap/18 * 255;   // 8 bit value, scaled proportionally to 0 to 18V
  //buf2 = SPI.transfer(iScaled);
  Serial.print("IL: ");
  Serial.print(iL);
  Serial.print("\t\tVb Scaled:  ");
  Serial.print(vbScaled);
  Serial.print("\t\tI_panel Scaled: ");
  Serial.println(iScaled);

  digitalWrite(fpgaCS, LOW); // SPI is active-low

  toSend = (vbScaled*256 + iScaled);
  buf1 = SPI.transfer16(toSend);
  //buf1 = SPI.transfer16(43690);
  // finally capacitor voltage
  buf2 = SPI.transfer(vCapScaled);
  //buf2 = SPI.transfer(170);
  // Serial.print("\t\t");
  // Serial.println(buf3);

  // Serial.print("First ten bits: ");
  // Serial.println(buf1*4 + buf2/64);
  // Serial.print("Last ten bits:  ");
  // Serial.println((buf2 % 4) * 256 + buf3);
  //buf now only contains the 8LSB which is the set point

  digitalWrite(4, HIGH); // stop FPGA sending
  delay(10);

  Serial.print("sent vcap:  ");
  Serial.print(vCapScaled);
  Serial.print("\t\t sent power:  ");
  Serial.print(vbScaled*256 + iScaled);

  //Serial.print("Bufs:  ");
  //Serial.print(buf1);
  //Serial.print("\t\t");
  //Serial.println(buf2);

  // decode
  // first conduct the error check, the last seven bits should be a specific pattern
  Serial.print("\t\tstop code:  ");
  Serial.print((buf2&127));
  Serial.print("\t\tstate");
  Serial.println((((buf1&3584)>>9)));
  if((buf2 & 127) == 101){
    // test correct, continue
    state = (buf1 & 3584) >> 9; // extract just the state pin to say whether charging cap or not
    targetVal = ((buf1 & 511) * 2) + (buf2>>7); 

  } else {
    Serial.println("Bad SPI");
  }

  // state = (buf1 & 28) >> 2; // extract just the state pin to say whether charging cap or not
  // newTargetVal = ((buf1 % 4) * 256) + buf2;
  // if (targetVal < 50){
  //   targetVal = storedTargetVal;
  // }
  
  // if ((newTargetVal > targetVal/2) && (newTargetVal < targetVal*2)){
  //   // remove anomolous values where the data has been shifted in transfer
  //   targetVal = newTargetVal;
  //   if ((state == 0) || (state == 4)){
  //     // idle
  //     Serial.print("State0");
  //     //Serial.println(state);
  //     storedTargetVal = targetVal;

  //     targetVal = 0;      
  //   }
  // }
  //Serial.println(state);
  //Serial.println(newTargetVal);
  Serial.print("Target Value: ");
  Serial.print(targetVal);
  Serial.print("\t\t State: ");
  Serial.println(state);
  
}

float getVCap(){
   sensorValue4 = analogRead(A6);  // sample Vcap
   vCap =  (sensorValue4 * (5.5) * (4.09 / 1023));  // convert v cap to volts
   Serial.println(vCap);
   return vCap;
}

void setup() {

  //Basic pin setups
  
  noInterrupts(); //disable all interrupts
  pinMode(13, OUTPUT);  //Pin13 is used to time the loops of the controller
  pinMode(3, INPUT_PULLUP); //Pin3 is the input from the Buck/Boost switch
  pinMode(2, INPUT_PULLUP); // Pin 2 is the input from the CL/OL switch
  analogReference(EXTERNAL); // We are using an external analogue reference for the ADC

  // setup SPI
  pinMode(fpgaCS, OUTPUT);
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV128);
  // TimerA0 initialization for control-loop interrupt.
  
  TCA0.SINGLE.PER = 1999; //
  TCA0.SINGLE.CMP1 = 1999; //
  TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV16_gc | TCA_SINGLE_ENABLE_bm; //16 prescaler, 1M.
  TCA0.SINGLE.INTCTRL = TCA_SINGLE_CMP1_bm; 

  // TimerB0 initialization for PWM output
  
  pinMode(6, OUTPUT);
  TCB0.CTRLA=TCB_CLKSEL_CLKDIV1_gc | TCB_ENABLE_bm; //62.5kHz
  analogWrite(6,120); 

  // ineterrupt for FPGA SPI transfer
  pinMode(spiTrigPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(spiTrigPin), spiTrigCall, FALLING);


  Serial.begin(115200);   //serial communication enable. Used for program debugging.
  interrupts();  //enable interrupts.
  Wire.begin(); // We need this for the i2c comms for the current sensor
  ina219.init(); // this initiates the current sensor
  Wire.setClock(700000); // set the comms speed for i2c
  Serial.println("Setup done");
  
}

 void loop() {
  // // check if fpga spi transfer
  // if (digitalRead(fpgaCS) == 0){
  //   // low so transfer
  //   spiFPGATransfer();
  // } 
  // see if supplied 10Hz clock has gone low
  if(spiTransfer){
    spiTransfer = 0;
    spiFPGATransfer();
  }
  if (vCap > 16){
    pwm_modulate(0.5); 
    //Serial.println("Va Limit reached");
    if(loopTrigger){
      loopTrigger = 0;
      sampling();
    }
  } else {
    if(loopTrigger) { // This loop is triggered, it wont run unless there is an interrupt
      
      digitalWrite(13, HIGH);   // set pin 13. Pin13 shows the time consumed by each control cycle. It's used for debugging.
      
      // Sample all of the measurements and check which control mode we are in
      sampling();

      //if (CL_mode) { //Closed Loop switch to control mode
      if  (state==2){     // checks if state is a charge state
          // The closed loop path has a voltage controller cascaded with a current controller. The voltage controller
          // creates a current demand based upon the voltage error. This demand is saturated to give current limiting.
          // The current loop then gives a duty cycle demand based upon the error between demanded current and measured
          // current
          cv = maxInCurrent * targetVal/1024;    // set point is current between 600mA and 0
          ei=iL-cv; //current error
          closed_loop=pidi(ei);  //current pid
          //closed_loop = cv;
          // change to boost
          //closed_loop = 1/(1-closed_loop);
          closed_loop=saturation(closed_loop,0.99,0.01);  //duty_cycle saturation
          pwm_modulate(closed_loop); //pwm modulation
      }else{ // Open Loop Boost path with just current controller (or when state is 1, 2, ,3, i.e. charging capacitor)
          // this is the voltage controller
          // sets to target of 9V, sufficiently high enough above 7V to prevent droops and brown outs
          voltageTarget = 9 ;
          eV= va - voltageTarget;
          closed_loop = pidv(eV);
          closed_loop = pidi(closed_loop - iL);
          //closed_loop = 1- closed_loop;
          closed_loop = saturation(closed_loop, 0.99, 0.13);
          pwm_modulate(closed_loop);
      }

      com_count++;              //used for debugging.
      if (com_count >= 500) {  //send out data every second.
        // Serial.print("CV: ");
        // Serial.print(cv);

        // Serial.print("\tVref: ");
        // Serial.print(vref);

        Serial.print("\tClosed_loop:  ");
        Serial.print(closed_loop);

        Serial.print("\tVa: ");
        Serial.print(va);
        Serial.print("\t");

        Serial.print("Vb: ");
        Serial.print(vb);
        Serial.print("\t");

        Serial.print("VCap: ");
        Serial.print(vCap);
        Serial.print("\t");

        Serial.print("Inductor Current: ");
        Serial.print(iL);
        Serial.print("\t\t");

        Serial.print("Boost Mode: ");
        Serial.print(Boost_mode);
        Serial.print("\t\t");

        Serial.print("CL Mode: ");
        Serial.print(CL_mode);
        Serial.print("\n");
        com_count = 0;
      }

      digitalWrite(13, LOW);   // reset pin13.
      loopTrigger = 0;
    }
  }
}



/*end of the program.*/
