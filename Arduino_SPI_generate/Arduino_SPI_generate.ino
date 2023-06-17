// GPIO for SPI pins
int csP = 1;
int sckP = 2;
int sdiP = 3;

int readSDI = 0;

hw_timer_t *sckTimer = NULL;
const int sckFreq = 100000;

const int numBits = 16;
int currentBit = 0;
int data[12] = {1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0}; // 12 bit data to send
int sckToggleCount = 0;
int sendData = 0;
int currentSck = 0;

void IRAM_ATTR onSckTimer(){
    // evey time this is called, SCK will clock
    currentSck = digitalRead(sckP);
    digitalWrite(sckP, !currentSck);
    sckToggleCount++;
    if(currentSck == 0){
      sendData = 1;
    }   
}

void setup() {
  // put your setup code here, to run once:
  pinMode(csP, OUTPUT);
  pinMode(sckP, OUTPUT);
  pinMode(sdiP, OUTPUT);

  sckTimer = timerBegin(0, 80, true);
  timerAttachInterrupt(sckTimer, &onSckTimer, true);
  timerAlarmWrite(sckTimer, (2/sckFreq), true);
  Serial.begin(115200);
}

void loop(){
  // begin data send
  // check not currently sending
  if(sckToggleCount == 0){
    // begin
    digitalWrite(csP, false);
    // enable Sck toggling with timer   
    timerAlarmEnable(sckTimer);
    while (sckToggleCount < 31){
      if (sendData){
        delayMicroseconds(4);
        digitalWrite(sdiP, data[sckToggleCount/2]);
        sendData = 0;
      }
    }
    sckToggleCount = 0;
  }
}