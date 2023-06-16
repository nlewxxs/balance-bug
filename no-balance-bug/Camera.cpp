#include <Arduino.h>
#include "Camera.h"
#include <HardwareSerial.h>
#include <Wire.h>

// UART setup 
const int8_t tx = 17;
const int8_t rx = 16;
const uint8_t baudrate = 115200;
const uint8_t nios_serialport = 2;  // using UART2
HardwareSerial SerialPortNios(nios_serialport); 
uint64_t uart_rx;  // will be passed by reference

// Boxes
boxCoordinates Boxes;
bool boxesRequested = false; 

// temp stuff >>>>>>>>>>>>>>>>>>>
uint8_t counter = 0;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

void read32bits(uint64_t *rx){
  // Serial.print("Seeing Rx");
  // Serial.print("Reading: ");
  // Serial.println(*rx);
  // char tmp[64];
  *rx = (SerialPortNios.read() | (SerialPortNios.read() << 8) | (SerialPortNios.read() << 16) | (SerialPortNios.read() << 24));
  // Serial.println("Processed Rx, continue");
  // sprintf(tmp, "%.8X", uart_rx);
  // Serial.println(tmp);
}

Matrix Camera::getBoxMatrix(){
  Matrix boxMatrix;
  boxMatrix.boxes[4][0] = Boxes.block_five.x_min; boxMatrix.boxes[4][1] = Boxes.block_five.y_min; boxMatrix.boxes[4][2] = Boxes.block_five.x_max; boxMatrix.boxes[4][3] = Boxes.block_five.y_max;
  boxMatrix.boxes[5][0] = Boxes.block_six.x_min; boxMatrix.boxes[5][1] = Boxes.block_six.y_min; boxMatrix.boxes[5][2] = Boxes.block_six.x_max; boxMatrix.boxes[5][3] = Boxes.block_six.y_max;
  boxMatrix.boxes[6][0] = Boxes.block_seven.x_min; boxMatrix.boxes[6][1] = Boxes.block_seven.y_min; boxMatrix.boxes[6][2] = Boxes.block_seven.x_max; boxMatrix.boxes[6][3] = Boxes.block_seven.y_max;
  boxMatrix.boxes[7][0] = Boxes.block_eight.x_min; boxMatrix.boxes[7][1] = Boxes.block_eight.y_min; boxMatrix.boxes[7][2] = Boxes.block_eight.x_max; boxMatrix.boxes[7][3] = Boxes.block_eight.y_max;
  boxMatrix.boxes[8][0] = Boxes.block_nine.x_min; boxMatrix.boxes[8][1] = Boxes.block_nine.y_min; boxMatrix.boxes[8][2] = Boxes.block_nine.x_max; boxMatrix.boxes[8][3] = Boxes.block_nine.y_max;
  boxMatrix.boxes[9][0] = Boxes.block_ten.x_min; boxMatrix.boxes[9][1] = Boxes.block_ten.y_min; boxMatrix.boxes[9][2] = Boxes.block_ten.x_max; boxMatrix.boxes[9][3] = Boxes.block_ten.y_max;
  boxMatrix.boxes[10][0] = Boxes.block_eleven.x_min; boxMatrix.boxes[10][1] = Boxes.block_eleven.y_min; boxMatrix.boxes[10][2] = Boxes.block_eleven.x_max; boxMatrix.boxes[10][3] = Boxes.block_eleven.y_max;
  boxMatrix.boxes[11][0] = Boxes.block_twelve.x_min; boxMatrix.boxes[11][1] = Boxes.block_twelve.y_min; boxMatrix.boxes[11][2] = Boxes.block_twelve.x_max; boxMatrix.boxes[11][3] = Boxes.block_twelve.y_max;
  boxMatrix.boxes[12][0] = Boxes.block_thirteen.x_min; boxMatrix.boxes[12][1] = Boxes.block_thirteen.y_min; boxMatrix.boxes[12][2] = Boxes.block_thirteen.x_max; boxMatrix.boxes[12][3] = Boxes.block_thirteen.y_max;
  boxMatrix.boxes[13][0] = Boxes.block_fourteen.x_min; boxMatrix.boxes[13][1] = Boxes.block_fourteen.y_min; boxMatrix.boxes[13][2] = Boxes.block_fourteen.x_max; boxMatrix.boxes[13][3] = Boxes.block_fourteen.y_max;
  boxMatrix.boxes[14][0] = Boxes.block_fifteen.x_min; boxMatrix.boxes[14][1] = Boxes.block_fifteen.y_min; boxMatrix.boxes[14][2] = Boxes.block_fifteen.x_max; boxMatrix.boxes[14][3] = Boxes.block_fifteen.y_max;
  boxMatrix.boxes[15][0] = Boxes.block_sixteen.x_min; boxMatrix.boxes[15][1] = Boxes.block_sixteen.y_min; boxMatrix.boxes[15][2] = Boxes.block_sixteen.x_max; boxMatrix.boxes[15][3] = Boxes.block_sixteen.y_max;
  boxMatrix.boxes[16][0] = Boxes.colour_coords.x_min; boxMatrix.boxes[16][1] = Boxes.colour_coords.y_min; boxMatrix.boxes[16][2] = Boxes.colour_coords.x_max; boxMatrix.boxes[16][3] = Boxes.colour_coords.y_max;
  boxMatrix.colour = Boxes.colour;
  boxMatrix.colour_uncertain = Boxes.colour_uncertain;
  
  return boxMatrix;
}

uint8_t Camera::getColours(){
  return 0; 
}

void updateCoordinates(boxCoordinates *coords){
  // called straight after NB start bit detected. 
  uint16_t tmpMin;
  uint16_t tmpMax;
  uint16_t colTmp;

  //remove the whitespace from the second MSG fifo
  read32bits(&uart_rx);
  //start processing bits
  read32bits(&uart_rx);
  coords->colour = (uart_rx & 0b11100000000000000000000000000000) >> 29;
  coords->colour_uncertain = (uart_rx & 0b00010000000000000000000000000000) >> 28;
  coords->colour_coords.x_min = (uart_rx & 0b00001111111111100000000000000000) >> 17;
  colTmp = (uart_rx & 0b00000000000000011111111100000000) << 6;
  tmpMin =                       (uart_rx & 0b00000000000000000000000011111111) << 3;


  read32bits(&uart_rx);
  coords->colour_coords.y_min = colTmp + (uart_rx & 0b11000000000000000000000000000000) >> 30;
  coords->colour_coords.x_max = (uart_rx & 0b00111111111110000000000000000000) >> 19;
  coords->colour_coords.y_max = (uart_rx & 0b00000000000001111111111100000000) >> 8;  
  tmpMax =                       (uart_rx & 0b00000000000000000000000011111111) << 3;

  read32bits(&uart_rx);
  coords->block_five.x_min =  tmpMin + ((uart_rx & 0b11100000000000000000000000000000) >> 29);
  coords->block_five.y_min =  (uart_rx & 0b00011111111111000000000000000000) >> 18;
  coords->block_six.x_min =   (uart_rx & 0b00000000000000111111111110000000) >> 7;
  tmpMin =                       (uart_rx & 0b00000000000000000000000001111111) << 4;

  read32bits(&uart_rx);
    coords->block_five.x_max =  tmpMax + ((uart_rx & 0b11100000000000000000000000000000) >> 29);
  coords->block_five.y_max =  (uart_rx & 0b00011111111111000000000000000000) >> 18;
  coords->block_six.x_max =   (uart_rx & 0b00000000000000111111111110000000) >> 7;
  tmpMax =                       (uart_rx & 0b00000000000000000000000001111111) << 4;

  read32bits(&uart_rx);
  coords->block_six.y_min =   tmpMin + ((uart_rx & 0b11110000000000000000000000000000) >> 28);
  coords->block_seven.x_min = (uart_rx & 0b00001111111111100000000000000000) >> 17;
  coords->block_seven.y_min = (uart_rx & 0b00000000000000011111111111000000) >> 6;
  tmpMin =                       (uart_rx & 0b00000000000000000000000000111111) << 5;
  
  read32bits(&uart_rx);
  coords->block_six.y_max =   tmpMax + ((uart_rx & 0b11110000000000000000000000000000) >> 28);
  coords->block_seven.x_max = (uart_rx & 0b00001111111111100000000000000000) >> 17;
  coords->block_seven.y_max = (uart_rx & 0b00000000000000011111111111000000) >> 6;
  tmpMax =                       (uart_rx & 0b00000000000000000000000000111111) << 5;

  read32bits(&uart_rx);
  coords->block_eight.x_min = tmpMin + ((uart_rx & 0b11111000000000000000000000000000) >> 27);
  coords->block_eight.y_min = (uart_rx & 0b00000111111111110000000000000000) >> 16;
  coords->block_nine.x_min =  (uart_rx & 0b00000000000000001111111111100000) >> 5;
  tmpMin =                       (uart_rx & 0b00000000000000000000000000011111) << 6;

  read32bits(&uart_rx);
  coords->block_eight.x_max = tmpMax + ((uart_rx & 0b11111000000000000000000000000000) >> 27);
  coords->block_eight.y_max = (uart_rx & 0b00000111111111110000000000000000) >> 16;
  coords->block_nine.x_max =  (uart_rx & 0b00000000000000001111111111100000) >> 5;
  tmpMax =                       (uart_rx & 0b00000000000000000000000000011111) << 6;

  read32bits(&uart_rx);
  coords->block_nine.y_min =  tmpMin + ((uart_rx & 0b11111100000000000000000000000000) >> 26);
  coords->block_ten.x_min =   (uart_rx & 0b00000011111111111000000000000000) >> 15;
  coords->block_ten.y_min =   (uart_rx & 0b00000000000000000111111111110000) >> 4;
  tmpMin =                       (uart_rx & 0b00000000000000000000000000001111) << 7;

  read32bits(&uart_rx);
  coords->block_nine.y_max =  tmpMax + ((uart_rx & 0b11111100000000000000000000000000) >> 26);
  coords->block_ten.x_max =   (uart_rx & 0b00000011111111111000000000000000) >> 15;
  coords->block_ten.y_max =   (uart_rx & 0b00000000000000000111111111110000) >> 4;
  tmpMax =                       (uart_rx & 0b00000000000000000000000000001111) << 7;

  read32bits(&uart_rx);
  coords->block_eleven.x_min =  tmpMin + ((uart_rx & 0b11111110000000000000000000000000) >> 25);
  coords->block_eleven.y_min =  (uart_rx & 0b00000001111111111100000000000000) >> 14;
  coords->block_twelve.x_min =  (uart_rx & 0b00000000000000000011111111111000) >> 3;
  tmpMin =                         (uart_rx & 0b00000000000000000000000000000111) << 8;

  read32bits(&uart_rx);
  coords->block_eleven.x_max =  tmpMax + ((uart_rx & 0b11111110000000000000000000000000) >> 25);
  coords->block_eleven.y_max =  (uart_rx & 0b00000001111111111100000000000000) >> 14;
  coords->block_twelve.x_max =  (uart_rx & 0b00000000000000000011111111111000) >> 3;
  tmpMax =                         (uart_rx & 0b00000000000000000000000000000111) << 8;

  read32bits(&uart_rx);
  coords->block_twelve.y_min =    tmpMin + ((uart_rx & 0b11111111000000000000000000000000) >> 24);
  coords->block_thirteen.x_min =  (uart_rx & 0b00000000111111111110000000000000) >> 13;
  coords->block_thirteen.y_min =  (uart_rx & 0b00000000000000000001111111111100) >> 2;
  tmpMin =                           (uart_rx & 0b00000000000000000000000000000011) << 9;

  read32bits(&uart_rx);
  coords->block_twelve.y_max =    tmpMax + ((uart_rx & 0b11111111000000000000000000000000) >> 24);
  coords->block_thirteen.x_max =  (uart_rx & 0b00000000111111111110000000000000) >> 13;
  coords->block_thirteen.y_max =  (uart_rx & 0b00000000000000000001111111111100) >> 2;
  tmpMax =                           (uart_rx & 0b00000000000000000000000000000011) << 9;

  read32bits(&uart_rx);
  coords->block_fourteen.x_min =  tmpMin + ((uart_rx & 0b11111111100000000000000000000000) >> 23);
  coords->block_fourteen.y_min =  (uart_rx & 0b00000000011111111111000000000000) >> 12;
  coords->block_fifteen.x_min =   (uart_rx & 0b00000000000000000000111111111110) >> 1;
  tmpMin =                           (uart_rx & 0b00000000000000000000000000000001) << 10;

  read32bits(&uart_rx); 
  coords->block_fourteen.x_max =  tmpMax + ((uart_rx & 0b11111111100000000000000000000000) >> 23);
  coords->block_fourteen.y_max =  (uart_rx & 0b00000000011111111111000000000000) >> 12;
  coords->block_fifteen.x_max =   (uart_rx & 0b00000000000000000000111111111110) >> 1;
  tmpMax =                           (uart_rx & 0b00000000000000000000000000000001) << 10;




  read32bits(&uart_rx);
  coords->block_fifteen.y_min =   tmpMin + ((uart_rx & 0b11111111110000000000000000000000) >> 22);
  coords->block_sixteen.x_min =   (uart_rx & 0b00000000001111111111100000000000) >> 11;
  coords->block_sixteen.y_min =   (uart_rx & 0b00000000000000000000011111111111);


  read32bits(&uart_rx);
  coords->block_fifteen.y_max =   tmpMax + ((uart_rx & 0b11111111110000000000000000000000) >> 22);
  coords->block_sixteen.x_max =   (uart_rx & 0b00000000001111111111100000000000) >> 11;
  coords->block_sixteen.y_max =   (uart_rx & 0b00000000000000000000011111111111);
}

void printCoordinates(boxCoordinates *coords){
  Serial.print(" five_x_min: ");
  Serial.print(coords->block_five.x_min);
  Serial.print(" five_x_max: ");
  Serial.print(coords->block_five.x_max);
  Serial.print(" five_y_min: ");
  Serial.print(coords->block_five.y_min);
  Serial.print(" five_y_max: ");
  Serial.print(coords->block_five.y_max);
  Serial.print(" six_x_min: ");
  Serial.print(coords->block_six.x_min);
  Serial.print(" six_x_max: ");
  Serial.print(coords->block_six.x_max);
  Serial.print(" six_y_min: ");
  Serial.print(coords->block_six.y_min);
  Serial.print(" six_y_max: ");
  Serial.println(coords->block_six.y_max);
  Serial.print(" seven_x_min: ");
  Serial.print(coords->block_seven.x_min);
  Serial.print(" seven_x_max: ");
  Serial.print(coords->block_seven.x_max);
  Serial.print(" seven_y_min: ");
  Serial.print(coords->block_seven.y_min);
  Serial.print(" seven_y_max: ");
  Serial.print(coords->block_seven.y_max);
  Serial.print(" eight_x_min: ");
  Serial.print(coords->block_eight.x_min);
  Serial.print(" eight_x_max: ");
  Serial.print(coords->block_eight.x_max);
  Serial.print(" eight_y_min: ");
  Serial.print(coords->block_eight.y_min);
  Serial.print(" eight_y_max: ");
  Serial.println(coords->block_eight.y_max);

  Serial.print(" nine_x_min: ");
  Serial.print(coords->block_nine.x_min);
  Serial.print(" nine_x_max: ");
  Serial.print(coords->block_nine.x_max);
  Serial.print(" nine_y_min: ");
  Serial.print(coords->block_nine.y_min);
  Serial.print(" nine_y_max: ");
  Serial.print(coords->block_nine.y_max);
  Serial.print(" ten_x_min: ");
  Serial.print(coords->block_ten.x_min);
  Serial.print(" ten_x_max: ");
  Serial.print(coords->block_ten.x_max);
  Serial.print(" ten_y_min: ");
  Serial.print(coords->block_ten.y_min);
  Serial.print(" ten_y_max: ");
  Serial.println(coords->block_ten.y_max);
  Serial.print(" eleven_x_min: ");
  Serial.print(coords->block_eleven.x_min);
  Serial.print(" eleven_x_max: ");
  Serial.print(coords->block_eleven.x_max);
  Serial.print(" eleven_y_min: ");
  Serial.print(coords->block_eleven.y_min);
  Serial.print(" eleven_y_max: ");
  Serial.print(coords->block_eleven.y_max);
  Serial.print(" twelve_x_min: ");
  Serial.print(coords->block_twelve.x_min);
  Serial.print(" twelve_x_max: ");
  Serial.print(coords->block_twelve.x_max);
  Serial.print(" twelve_y_min: ");
  Serial.print(coords->block_twelve.y_min);
  Serial.print(" twelve_y_max: ");
  Serial.println(coords->block_twelve.y_max);

  Serial.print(" thirteen_x_min: ");
  Serial.print(coords->block_thirteen.x_min);
  Serial.print(" thirteen_x_max: ");
  Serial.print(coords->block_thirteen.x_max);
  Serial.print(" thirteen_y_min: ");
  Serial.print(coords->block_thirteen.y_min);
  Serial.print(" thirteen_y_max: ");
  Serial.print(coords->block_thirteen.y_max);
  Serial.print(" fourteen_x_min: ");
  Serial.print(coords->block_fourteen.x_min);
  Serial.print(" fourteen_x_max: ");
  Serial.print(coords->block_fourteen.x_max);
  Serial.print(" fourteen_y_min: ");
  Serial.print(coords->block_fourteen.y_min);
  Serial.print(" fourteen_y_max: ");
  Serial.println(coords->block_fourteen.y_max);
  Serial.print(" fifteen_x_min: ");
  Serial.print(coords->block_fifteen.x_min);
  Serial.print(" fifteen_x_max: ");
  Serial.print(coords->block_fifteen.x_max);
  Serial.print(" fifteen_y_min: ");
  Serial.print(coords->block_fifteen.y_min);
  Serial.print(" fifteen_y_max: ");
  Serial.print(coords->block_fifteen.y_max);
  Serial.print(" sixteen_x_min: ");
  Serial.print(coords->block_sixteen.x_min);
  Serial.print(" sixteen_x_max: ");
  Serial.print(coords->block_sixteen.x_max);
  Serial.print(" sixteen_y_min: ");
  Serial.print(coords->block_sixteen.y_min);
  Serial.print(" sixteen_y_max: ");
  Serial.println(coords->block_sixteen.y_max);
}

void Camera::init(){
  SerialPortNios.begin(115200, SERIAL_8N1, rx, tx);
  SerialPortNios.setTimeout(10);
  delay(1000);
}

void Camera::update(){
  if (SerialPortNios.available()) {
    do {
      read32bits(&uart_rx);
      Serial.println("Waiting");
    } while (uart_rx != 0x00004E42);
    updateCoordinates(&Boxes);
  }
}
