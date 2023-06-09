// GPIO for SPI pins
int csP = 1;
int sckP = 2;
int sdiP = 3;

int data = 0;
int readSDI = 0;

const int numBits = 16;
int currentBit = 0;
int readData[numBits];

void csFallCallback(){
  Serial.println("CS Fall");
  data = 1;
}

void sckSDIReadCallback(){
  Serial.println("SCK Rise");
  readSDI = 1;
}

void setup() {
  // put your setup code here, to run once:
  pinMode(csP, INPUT);
  pinMode(sckP, INPUT);
  pinMode(sdiP, INPUT);

  // CS falling interrupt
  attachInterrupt(digitalPinToInterrupt(csP), csFallCallback, FALLING);
  attachInterrupt(digitalPinToInterrupt(sckP), sckSDIReadCallback, RISING);
  Serial.begin(115200);
}

void loop() {
  // waits for data to be being sent
  // data is set high via interrupt when CS falls
  if (data){
    // read the incoming data formatted accoridng to the SPI cheung module
    Serial.println("Data");
    currentBit = 0;
    while (currentBit < numBits){
      // waits for sdiRead to be set high by interrupt with SCK rising
      if (readSDI){
        readData[currentBit] = digitalRead(sdiP);
        readSDI = 0;
        Serial.print("Bit:  ");
        Serial.println(currentBit);
        currentBit++;
      }
    }

    for (int i=0; i<numBits; i++){
      Serial.print(readData[i]);
    }
    Serial.println();
    data = 0;  
  }

}
