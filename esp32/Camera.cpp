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
uint32_t uart_rx;  // will be passed by reference

// Boxes
boxCoordinates Boxes;
bool boxesRequested = false; 

// temp stuff >>>>>>>>>>>>>>>>>>>
uint8_t counter = 0;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

void read32bits(uint32_t *rx){
  // Serial.print("Seeing Rx");
  // Serial.print("Reading: ");
  // Serial.println(*rx);
  // char tmp[32];
  *rx = (SerialPortNios.read() | (SerialPortNios.read() << 8) | (SerialPortNios.read() << 16) | (SerialPortNios.read() << 24));
  // Serial.println("Processed Rx, continue");
  // sprintf(tmp, "%.8X", uart_rx);
  // Serial.println(tmp);
}

//TODO CHECK FOR FFFFFFFF AND IF SO IGNORE IT!

Matrix Camera::getBoxMatrix(){
  Matrix boxMatrix;
  // boxMatrix.boxes[0][0] = Boxes.block_one.x_min; boxMatrix.boxes[0][1] = Boxes.block_one.y_min; boxMatrix.boxes[0][2] = Boxes.block_one.x_max; boxMatrix.boxes[0][3] = Boxes.block_one.y_max;
  // boxMatrix.boxes[1][0] = Boxes.block_two.x_min; boxMatrix.boxes[1][1] = Boxes.block_two.y_min; boxMatrix.boxes[1][2] = Boxes.block_two.x_max; boxMatrix.boxes[1][3] = Boxes.block_two.y_max;
  // boxMatrix.boxes[2][0] = Boxes.block_three.x_min; boxMatrix.boxes[2][1] = Boxes.block_three.y_min; boxMatrix.boxes[2][2] = Boxes.block_three.x_max; boxMatrix.boxes[2][3] = Boxes.block_three.y_max;
  // boxMatrix.boxes[3][0] = Boxes.block_four.x_min; boxMatrix.boxes[3][1] = Boxes.block_four.y_min; boxMatrix.boxes[3][2] = Boxes.block_four.x_max; boxMatrix.boxes[3][3] = Boxes.block_four.y_max;
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
  return boxMatrix;
}

uint8_t Camera::getColours(){
  return 0; 
}

void updateCoordinates(boxCoordinates *coords){
  // called straight after NB start bit detected. 
  uint16_t tmp;

  // ---------------------------- MIN VALUES ---------------------------------- //

  // read32bits(&uart_rx);     // [31:21] = one_x_min[10:0], [20:10] = one_y_min[10:0], [9:0] = two_x_min[10:1]
  // coords->block_one.x_min =   (uart_rx & 0b11111111111000000000000000000000) >> 21;
  // coords->block_one.y_min =   (uart_rx & 0b00000000000111111111110000000000) >> 10;
  // tmp =                       (uart_rx & 0b00000000000000000000001111111111) << 1; 
  
  // read32bits(&uart_rx);     // two_x_min[0], two_y_min, three_x_min, three_y_min[10:2]
  // coords->block_two.x_min =   tmp + ((uart_rx & 0b10000000000000000000000000000000) >> 31);
  // coords->block_two.y_min =   (uart_rx & 0b01111111111100000000000000000000) >> 20;
  // coords->block_three.x_min = (uart_rx & 0b00000000000011111111111000000000) >> 9;
  // tmp =                       (uart_rx & 0b00000000000000000000000111111111) << 2;
  read32bits(&uart_rx);
  
  // do{
  //   read32bits(&uart_rx);
  // }
  // while(uart_rx == 0xFFFFFFFF);
  // read32bits(&uart_rx);     // three_y_min[1:0], four_x_min, four_y_min, five_x_min[10:3]
  
  // coords->block_three.y_min = tmp + ((uart_rx & 0b11000000000000000000000000000000) >> 30);
  // coords->block_four.x_min =  (uart_rx & 0b00111111111110000000000000000000) >> 19;
  // coords->block_four.y_min =  (uart_rx & 0b00000000000001111111111100000000) >> 8;
  tmp =                       (uart_rx & 0b00000000000000000000000011111111) << 3;
read32bits(&uart_rx);
  
  // read32bits(&uart_rx);     // five_x_min[2:0], five_y_min, six_x_min, six_y_min[10:4]
  coords->block_five.x_min =  tmp + ((uart_rx & 0b11100000000000000000000000000000) >> 29);
  coords->block_five.y_min =  (uart_rx & 0b00011111111111000000000000000000) >> 18;
  coords->block_six.x_min =   (uart_rx & 0b00000000000000111111111110000000) >> 7;
  tmp =                       (uart_rx & 0b00000000000000000000000001111111) << 4;

  read32bits(&uart_rx);
  

  // read32bits(&uart_rx);     // six_y_min[3:0], seven_x_min, seven_y_min, eight_x_min[10:5]
  coords->block_six.y_min =   tmp + ((uart_rx & 0b11110000000000000000000000000000) >> 28);
  coords->block_seven.x_min = (uart_rx & 0b00001111111111100000000000000000) >> 17;
  coords->block_seven.y_min = (uart_rx & 0b00000000000000011111111111000000) >> 6;
  tmp =                       (uart_rx & 0b00000000000000000000000000111111) << 5;
read32bits(&uart_rx);
  
  // read32bits(&uart_rx);     // eight_x_min[4:0], eight_y_min, nine_x_min, nine_y_min[10:6]
  coords->block_eight.x_min = tmp + ((uart_rx & 0b11111000000000000000000000000000) >> 27);
  coords->block_eight.y_min = (uart_rx & 0b00000111111111110000000000000000) >> 16;
  coords->block_nine.x_min =  (uart_rx & 0b00000000000000001111111111100000) >> 5;
  tmp =                       (uart_rx & 0b00000000000000000000000000011111) << 6;
read32bits(&uart_rx);
  
  // read32bits(&uart_rx);     // nine_y_min[5:0], ten_x_min, ten_y_min, eleven_x_min[10:7]
  coords->block_nine.y_min =  tmp + ((uart_rx & 0b11111100000000000000000000000000) >> 26);
  coords->block_ten.x_min =   (uart_rx & 0b00000011111111111000000000000000) >> 15;
  coords->block_ten.y_min =   (uart_rx & 0b00000000000000000111111111110000) >> 4;
  tmp =                       (uart_rx & 0b00000000000000000000000000001111) << 7;
read32bits(&uart_rx);
  
  // read32bits(&uart_rx);     // eleven_x_min[6:0], eleven_y_min, twelve_x_min, twelve_y_min[10:8]
  coords->block_eleven.x_min =  tmp + ((uart_rx & 0b11111110000000000000000000000000) >> 25);
  coords->block_eleven.y_min =  (uart_rx & 0b00000001111111111100000000000000) >> 14;
  coords->block_twelve.x_min =  (uart_rx & 0b00000000000000000011111111111000) >> 3;
  tmp =                         (uart_rx & 0b00000000000000000000000000000111) << 8;
read32bits(&uart_rx);
  
  // read32bits(&uart_rx);     // twelve_y_min[7:0], thirteen_x_min, thirteen_y_min, fourteen_x_min[10:9]
  coords->block_twelve.y_min =    tmp + ((uart_rx & 0b11111111000000000000000000000000) >> 24);
  coords->block_thirteen.x_min =  (uart_rx & 0b00000000111111111110000000000000) >> 13;
  coords->block_thirteen.y_min =  (uart_rx & 0b00000000000000000001111111111100) >> 2;
  tmp =                           (uart_rx & 0b00000000000000000000000000000011) << 9;
read32bits(&uart_rx);
  
  // read32bits(&uart_rx);     // fourteen_x_min[8:0], fourteen_y_min, fifteen_x_min, fifteen_y_min[10]
  coords->block_fourteen.x_min =  tmp + ((uart_rx & 0b11111111100000000000000000000000) >> 23);
  coords->block_fourteen.y_min =  (uart_rx & 0b00000000011111111111000000000000) >> 12;
  coords->block_fifteen.x_min =   (uart_rx & 0b00000000000000000000111111111110) >> 1;
  tmp =                           (uart_rx & 0b00000000000000000000000000000001) << 10;
read32bits(&uart_rx);
  // Serial.print("Saw: ");
  // Serial.println(uart_rx);
  // read32bits(&uart_rx);     // fifteen_y_min[9:0], sixteen_x_min, sixteen_y_min
  coords->block_fifteen.y_min =   tmp + ((uart_rx & 0b11111111110000000000000000000000) >> 22);
  coords->block_sixteen.x_min =   (uart_rx & 0b00000000001111111111100000000000) >> 11;
  coords->block_sixteen.y_min =   (uart_rx & 0b00000000000000000000011111111111);

  // ---------------------------- MAX VALUES ---------------------------------- //

  // read32bits(&uart_rx);     // one_x_max, one_y_max, two_x_max[10:1]
  // coords->block_one.x_max =   (uart_rx & 0b11111111111000000000000000000000) >> 21;
  // coords->block_one.y_max =   (uart_rx & 0b00000000000111111111110000000000) >> 10;
  // tmp =                       (uart_rx & 0b00000000000000000000001111111111) << 1; 
  
  // read32bits(&uart_rx);     // two_x_max[0], two_y_max, three_x_max, three_y_max[10:2]
  // coords->block_two.x_max =   tmp + ((uart_rx & 0b10000000000000000000000000000000) >> 31);
  // coords->block_two.y_max =   (uart_rx & 0b01111111111100000000000000000000) >> 20;
  // coords->block_three.x_max = (uart_rx & 0b00000000000011111111111000000000) >> 9;
  // tmp =                       (uart_rx & 0b00000000000000000000000111111111) << 2;
read32bits(&uart_rx);
  
  // read32bits(&uart_rx);     // three_y_max[1:0], four_x_max, four_y_max, five_x_max[10:3]
  // coords->block_three.y_max = tmp + ((uart_rx & 0b11000000000000000000000000000000) >> 30);
  // coords->block_four.x_max =  (uart_rx & 0b00111111111110000000000000000000) >> 19;
  // coords->block_four.y_max =  (uart_rx & 0b00000000000001111111111100000000) >> 8;
  tmp =                       (uart_rx & 0b00000000000000000000000011111111) << 3;
read32bits(&uart_rx);
  
  // read32bits(&uart_rx);     // five_x_max[2:0], five_y_max, six_x_max, six_y_max[10:4]
  coords->block_five.x_max =  tmp + ((uart_rx & 0b11100000000000000000000000000000) >> 29);
  coords->block_five.y_max =  (uart_rx & 0b00011111111111000000000000000000) >> 18;
  coords->block_six.x_max =   (uart_rx & 0b00000000000000111111111110000000) >> 7;
  tmp =                       (uart_rx & 0b00000000000000000000000001111111) << 4;
read32bits(&uart_rx);
  
  // read32bits(&uart_rx);     // six_y_max[3:0], seven_x_max, seven_y_max, eight_x_max[10:5]
  coords->block_six.y_max =   tmp + ((uart_rx & 0b11110000000000000000000000000000) >> 28);
  coords->block_seven.x_max = (uart_rx & 0b00001111111111100000000000000000) >> 17;
  coords->block_seven.y_max = (uart_rx & 0b00000000000000011111111111000000) >> 6;
  tmp =                       (uart_rx & 0b00000000000000000000000000111111) << 5;
read32bits(&uart_rx);
  
  // read32bits(&uart_rx);     // eight_x_max[4:0], eight_y_max, nine_x_max, nine_y_max[10:6]
  coords->block_eight.x_max = tmp + ((uart_rx & 0b11111000000000000000000000000000) >> 27);
  coords->block_eight.y_max = (uart_rx & 0b00000111111111110000000000000000) >> 16;
  coords->block_nine.x_max =  (uart_rx & 0b00000000000000001111111111100000) >> 5;
  tmp =                       (uart_rx & 0b00000000000000000000000000011111) << 6;
read32bits(&uart_rx);
  
  // read32bits(&uart_rx);     // nine_y_max[5:0], ten_x_max, ten_y_max, eleven_x_max[10:7]
  coords->block_nine.y_max =  tmp + ((uart_rx & 0b11111100000000000000000000000000) >> 26);
  coords->block_ten.x_max =   (uart_rx & 0b00000011111111111000000000000000) >> 15;
  coords->block_ten.y_max =   (uart_rx & 0b00000000000000000111111111110000) >> 4;
  tmp =                       (uart_rx & 0b00000000000000000000000000001111) << 7;
read32bits(&uart_rx);
  
  // read32bits(&uart_rx);     // eleven_x_max[6:0], eleven_y_max, twelve_x_max, twelve_y_max[10:8]
  coords->block_eleven.x_max =  tmp + ((uart_rx & 0b11111110000000000000000000000000) >> 25);
  coords->block_eleven.y_max =  (uart_rx & 0b00000001111111111100000000000000) >> 14;
  coords->block_twelve.x_max =  (uart_rx & 0b00000000000000000011111111111000) >> 3;
  tmp =                         (uart_rx & 0b00000000000000000000000000000111) << 8;
read32bits(&uart_rx);
  
  // read32bits(&uart_rx);     // twelve_y_max[7:0], thirteen_x_max, thirteen_y_max, fourteen_x_max[10:9]
  coords->block_twelve.y_max =    tmp + ((uart_rx & 0b11111111000000000000000000000000) >> 24);
  coords->block_thirteen.x_max =  (uart_rx & 0b00000000111111111110000000000000) >> 13;
  coords->block_thirteen.y_max =  (uart_rx & 0b00000000000000000001111111111100) >> 2;
  tmp =                           (uart_rx & 0b00000000000000000000000000000011) << 9;
read32bits(&uart_rx);
  
  // read32bits(&uart_rx);     // fourteen_x_max[8:0], fourteen_y_max, fifteen_x_max, fifteen_y_max[10]
  coords->block_fourteen.x_max =  tmp + ((uart_rx & 0b11111111100000000000000000000000) >> 23);
  coords->block_fourteen.y_max =  (uart_rx & 0b00000000011111111111000000000000) >> 12;
  coords->block_fifteen.x_max =   (uart_rx & 0b00000000000000000000111111111110) >> 1;
  tmp =                           (uart_rx & 0b00000000000000000000000000000001) << 10;
read32bits(&uart_rx);
  
  // Serial.println("GOT: ");
  // char tmp2[32];
  // sprintf(tmp2, "%.8X", uart_rx);
  // Serial.println(tmp2);
  // read32bits(&uart_rx);     // fifteen_y_max[9:0], sixteen_x_max, sixteen_y_max
  coords->block_fifteen.y_max =   tmp + ((uart_rx & 0b11111111110000000000000000000000) >> 22);
  coords->block_sixteen.x_max =   (uart_rx & 0b00000000001111111111100000000000) >> 11;
  coords->block_sixteen.y_max =   (uart_rx & 0b00000000000000000000011111111111);

  //TODO DO SOMETHING WITH THE END BIT IF REQUIRED  
  // read32bits(&uart_rx);

  // }
  // while(uart_rx != 0x00454E44);
  //TODO ADD A METHOD OF SKIPPING IF A COUNTER REACHES A MINIMUM VALUE TO CATCH ERROR IN THE TRANFER!!!!!, should do automatically though but test
  //try implemented a start and end bit to look for in valid readings maybe, this could work but would require the buffer to be entirely moved around, test with potentially faster framerate but could lead to buffer overflowing, this definitely needs tuning, but higher is potentially better as it wont write if there is too little space to write to, but we need to ensure that the buffer is never empty in this case
  //maybe add a switch, to write some values in one frame and some in another
  //this would take time
}

void printCoordinates(boxCoordinates *coords){
  // Serial.print(" one_x_min: ");
  // Serial.print(coords->block_one.x_min);
  // Serial.print(" one_x_max: ");
  // Serial.print(coords->block_one.x_max);
  // Serial.print(" one_y_min: ");
  // Serial.print(coords->block_one.y_min);
  // Serial.print(" one_y_max: ");
  // Serial.print(coords->block_one.y_max);
  // Serial.print(" two_x_min: ");
  // Serial.print(coords->block_two.x_min);
  // Serial.print(" two_x_max: ");
  // Serial.print(coords->block_two.x_max);
  // Serial.print(" two_y_min: ");
  // Serial.print(coords->block_two.y_min);
  // Serial.print(" two_y_max: ");
  // Serial.println(coords->block_two.y_max);
  // Serial.print(" three_x_min: ");
  // Serial.print(coords->block_three.x_min);
  // Serial.print(" three_x_max: ");
  // Serial.print(coords->block_three.x_max);
  // Serial.print(" three_y_min: ");
  // Serial.print(coords->block_three.y_min);
  // Serial.print(" three_y_max: ");
  // Serial.print(coords->block_three.y_max);
  // Serial.print(" four_x_min: ");
  // Serial.print(coords->block_four.x_min);
  // Serial.print(" four_x_max: ");
  // Serial.print(coords->block_four.x_max);
  // Serial.print(" four_y_min: ");
  // Serial.print(coords->block_four.y_min);
  // Serial.print(" four_y_max: ");
  // Serial.println(coords->block_four.y_max);

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
    } while(uart_rx != 0x00004E42);
    // if (){ // NB start bit
    Serial.println("Starting");
    updateCoordinates(&Boxes);
      // printCoordinates(&Boxes);
    // }  
  }
}

//chars are being read/overwritten before being able to leave





