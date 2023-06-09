#include <HardwareSerial.h>

//MACROS
const int8_t tx = 17;
const int8_t rx = 16;
const uint8_t baudrate = 115200;
const uint8_t nios_serialport = 2;

struct coordinate {
  uint16_t x_min;
  uint16_t x_max;
  uint16_t y_min;
  uint16_t y_max;
};

struct boxCoordinates {

  coordinate block_one;
  coordinate block_two;
  coordinate block_three;
  coordinate block_four;
  coordinate block_five;
  coordinate block_six;
  coordinate block_seven;
  coordinate block_eight;
  coordinate block_nine;
  coordinate block_ten;
  coordinate block_eleven;
  coordinate block_twelve;
  coordinate block_thirteen;
  coordinate block_fourteen;
  coordinate block_fifteen;
  coordinate block_sixteen;

};

boxCoordinates Boxes;

HardwareSerial SerialPortNios(nios_serialport);  //if using UART2

void setup() {

  //SerialBT.begin();
  Serial.begin(115200);
  //Serial.setTimeout(100);
  Serial.println("Opened locally");
  SerialPortNios.begin(115200, SERIAL_8N1, rx, tx);

  SerialPortNios.setTimeout(10);
  delay(1000);

}

uint32_t uart_rx;

void loop() {

  if (SerialPortNios.available()) {

    read32bits(&uart_rx);
    if (uart_rx == 0x00004E42){ // NB start bit
      updateCoordinates(&Boxes);
      printCoordinates(&Boxes);
    }
  }

  delay(5);
}

void readByte(uint32_t *rx){
  *rx = SerialPortNios.read();
}

void read32bits(uint32_t *rx){
  Serial.print("Reading: ");
  char tmp[32];
  *rx = (SerialPortNios.read() | (SerialPortNios.read() << 8) | (SerialPortNios.read() << 16) | (SerialPortNios.read() << 24));
  sprintf(tmp, "%.8X", uart_rx);
  Serial.println(tmp);
}

void updateCoordinates(boxCoordinates *coords){
  // called straight after NB start bit detected. 
  uint16_t tmp;

  // ---------------------------- MIN VALUES ---------------------------------- //

  read32bits(&uart_rx);     // [31:21] = one_x_min[10:0], [20:10] = one_y_min[10:0], [9:0] = two_x_min[10:1]
  coords->block_one.x_min =   (uart_rx & 0b11111111111000000000000000000000) >> 21;
  coords->block_one.y_min =   (uart_rx & 0b00000000000111111111110000000000) >> 10;
  tmp =                       (uart_rx & 0b00000000000000000000001111111111) << 1; 
  
  read32bits(&uart_rx);     // two_x_min[0], two_y_min, three_x_min, three_y_min[10:2]
  coords->block_two.x_min =   tmp + ((uart_rx & 0b10000000000000000000000000000000) >> 31);
  coords->block_two.y_min =   (uart_rx & 0b01111111111100000000000000000000) >> 20;
  coords->block_three.x_min = (uart_rx & 0b00000000000011111111111000000000) >> 9;
  tmp =                       (uart_rx & 0b00000000000000000000000111111111) << 2;

  read32bits(&uart_rx);     // three_y_min[1:0], four_x_min, four_y_min, five_x_min[10:3]
  coords->block_three.y_min = tmp + ((uart_rx & 0b11000000000000000000000000000000) >> 30);
  coords->block_four.x_min =  (uart_rx & 0b00111111111110000000000000000000) >> 19;
  coords->block_four.y_min =  (uart_rx & 0b00000000000001111111111100000000) >> 8;
  tmp =                       (uart_rx & 0b00000000000000000000000011111111) << 3;

  read32bits(&uart_rx);     // five_x_min[2:0], five_y_min, six_x_min, six_y_min[10:4]
  coords->block_five.x_min =  tmp + ((uart_rx & 0b11100000000000000000000000000000) >> 29);
  coords->block_five.y_min =  (uart_rx & 0b00011111111111000000000000000000) >> 18;
  coords->block_six.x_min =   (uart_rx & 0b00000000000000111111111110000000) >> 7;
  tmp =                       (uart_rx & 0b00000000000000000000000001111111) << 4;

  read32bits(&uart_rx);     // six_y_min[3:0], seven_x_min, seven_y_min, eight_x_min[10:5]
  coords->block_six.y_min =   tmp + ((uart_rx & 0b11110000000000000000000000000000) >> 28);
  coords->block_seven.x_min = (uart_rx & 0b00001111111111100000000000000000) >> 17;
  coords->block_seven.y_min = (uart_rx & 0b00000000000000011111111111000000) >> 6;
  tmp =                       (uart_rx & 0b00000000000000000000000000111111) << 5;

  read32bits(&uart_rx);     // eight_x_min[4:0], eight_y_min, nine_x_min, nine_y_min[10:6]
  coords->block_eight.x_min = tmp + ((uart_rx & 0b11111000000000000000000000000000) >> 27);
  coords->block_eight.y_min = (uart_rx & 0b00000111111111110000000000000000) >> 16;
  coords->block_nine.x_min =  (uart_rx & 0b00000000000000001111111111100000) >> 5;
  tmp =                       (uart_rx & 0b00000000000000000000000000011111) << 6;

  read32bits(&uart_rx);     // nine_y_min[5:0], ten_x_min, ten_y_min, eleven_x_min[10:7]
  coords->block_nine.y_min =  tmp + ((uart_rx & 0b11111100000000000000000000000000) >> 26);
  coords->block_ten.x_min =   (uart_rx & 0b00000011111111111000000000000000) >> 15;
  coords->block_ten.y_min =   (uart_rx & 0b00000000000000000111111111110000) >> 4;
  tmp =                       (uart_rx & 0b00000000000000000000000000001111) << 7;

  read32bits(&uart_rx);     // eleven_x_min[6:0], eleven_y_min, twelve_x_min, twelve_y_min[10:8]
  coords->block_eleven.x_min =  tmp + ((uart_rx & 0b11111110000000000000000000000000) >> 25);
  coords->block_eleven.y_min =  (uart_rx & 0b00000001111111111100000000000000) >> 14;
  coords->block_twelve.x_min =  (uart_rx & 0b00000000000000000011111111111000) >> 3;
  tmp =                         (uart_rx & 0b00000000000000000000000000000111) << 8;

  read32bits(&uart_rx);     // twelve_y_min[7:0], thirteen_x_min, thirteen_y_min, fourteen_x_min[10:9]
  coords->block_twelve.y_min =    tmp + ((uart_rx & 0b11111111000000000000000000000000) >> 24);
  coords->block_thirteen.x_min =  (uart_rx & 0b00000000111111111110000000000000) >> 13;
  coords->block_thirteen.y_min =  (uart_rx & 0b00000000000000000001111111111100) >> 2;
  tmp =                           (uart_rx & 0b00000000000000000000000000000011) << 9;

  read32bits(&uart_rx);     // fourteen_x_min[8:0], fourteen_y_min, fifteen_x_min, fifteen_y_min[10]
  coords->block_fourteen.x_min =  tmp + ((uart_rx & 0b11111111100000000000000000000000) >> 23);
  coords->block_fourteen.y_min =  (uart_rx & 0b00000000011111111111000000000000) >> 12;
  coords->block_fifteen.x_min =   (uart_rx & 0b00000000000000000000111111111110) >> 1;
  tmp =                           (uart_rx & 0b00000000000000000000000000000001) << 10;

  read32bits(&uart_rx);     // fifteen_y_min[9:0], sixteen_x_min, sixteen_y_min
  coords->block_fifteen.y_min =   tmp + ((uart_rx & 0b11111111110000000000000000000000) >> 22);
  coords->block_sixteen.x_min =   (uart_rx & 0b00000000001111111111100000000000) >> 11;
  coords->block_sixteen.y_min =   (uart_rx & 0b00000000000000000000011111111111);

  // ---------------------------- MAX VALUES ---------------------------------- //

  read32bits(&uart_rx);     // [31:21] = one_x_min[10:0], [20:10] = one_y_min[10:0], [9:0] = two_x_min[10:1]
  coords->block_one.x_max =   (uart_rx & 0b11111111111000000000000000000000) >> 21;
  coords->block_one.y_max =   (uart_rx & 0b00000000000111111111110000000000) >> 10;
  tmp =                       (uart_rx & 0b00000000000000000000001111111111) << 1; 
  
  read32bits(&uart_rx);     // two_x_min[0], two_y_min, three_x_min, three_y_min[10:2]
  coords->block_two.x_max =   tmp + ((uart_rx & 0b10000000000000000000000000000000) >> 31);
  coords->block_two.y_max =   (uart_rx & 0b01111111111100000000000000000000) >> 20;
  coords->block_three.x_max = (uart_rx & 0b00000000000011111111111000000000) >> 9;
  tmp =                       (uart_rx & 0b00000000000000000000000111111111) << 2;

  read32bits(&uart_rx);     // three_y_min[1:0], four_x_min, four_y_min, five_x_min[10:3]
  coords->block_three.y_max = tmp + ((uart_rx & 0b11000000000000000000000000000000) >> 30);
  coords->block_four.x_max =  (uart_rx & 0b00111111111110000000000000000000) >> 19;
  coords->block_four.y_max =  (uart_rx & 0b00000000000001111111111100000000) >> 8;
  tmp =                       (uart_rx & 0b00000000000000000000000011111111) << 3;

  read32bits(&uart_rx);     // five_x_min[2:0], five_y_min, six_x_min, six_y_min[10:4]
  coords->block_five.x_max =  tmp + ((uart_rx & 0b11100000000000000000000000000000) >> 29);
  coords->block_five.y_max =  (uart_rx & 0b00011111111111000000000000000000) >> 18;
  coords->block_six.x_max =   (uart_rx & 0b00000000000000111111111110000000) >> 7;
  tmp =                       (uart_rx & 0b00000000000000000000000001111111) << 4;

  read32bits(&uart_rx);     // six_y_min[3:0], seven_x_min, seven_y_min, eight_x_min[10:5]
  coords->block_six.y_max =   tmp + ((uart_rx & 0b11110000000000000000000000000000) >> 28);
  coords->block_seven.x_max = (uart_rx & 0b00001111111111100000000000000000) >> 17;
  coords->block_seven.y_max = (uart_rx & 0b00000000000000011111111111000000) >> 6;
  tmp =                       (uart_rx & 0b00000000000000000000000000111111) << 5;

  read32bits(&uart_rx);     // eight_x_min[4:0], eight_y_min, nine_x_min, nine_y_min[10:6]
  coords->block_eight.x_max = tmp + ((uart_rx & 0b11111000000000000000000000000000) >> 27);
  coords->block_eight.y_max = (uart_rx & 0b00000111111111110000000000000000) >> 16;
  coords->block_nine.x_max =  (uart_rx & 0b00000000000000001111111111100000) >> 5;
  tmp =                       (uart_rx & 0b00000000000000000000000000011111) << 6;

  read32bits(&uart_rx);     // nine_y_min[5:0], ten_x_min, ten_y_min, eleven_x_min[10:7]
  coords->block_nine.y_max =  tmp + ((uart_rx & 0b11111100000000000000000000000000) >> 26);
  coords->block_ten.x_max =   (uart_rx & 0b00000011111111111000000000000000) >> 15;
  coords->block_ten.y_max =   (uart_rx & 0b00000000000000000111111111110000) >> 4;
  tmp =                       (uart_rx & 0b00000000000000000000000000001111) << 7;

  read32bits(&uart_rx);     // eleven_x_min[6:0], eleven_y_min, twelve_x_min, twelve_y_min[10:8]
  coords->block_eleven.x_max =  tmp + ((uart_rx & 0b11111110000000000000000000000000) >> 25);
  coords->block_eleven.y_max =  (uart_rx & 0b00000001111111111100000000000000) >> 14;
  coords->block_twelve.x_max =  (uart_rx & 0b00000000000000000011111111111000) >> 3;
  tmp =                         (uart_rx & 0b00000000000000000000000000000111) << 8;

  read32bits(&uart_rx);     // twelve_y_min[7:0], thirteen_x_min, thirteen_y_min, fourteen_x_min[10:9]
  coords->block_twelve.y_max =    tmp + ((uart_rx & 0b11111111000000000000000000000000) >> 24);
  coords->block_thirteen.x_max =  (uart_rx & 0b00000000111111111110000000000000) >> 13;
  coords->block_thirteen.y_max =  (uart_rx & 0b00000000000000000001111111111100) >> 2;
  tmp =                           (uart_rx & 0b00000000000000000000000000000011) << 9;

  read32bits(&uart_rx);     // fourteen_x_min[8:0], fourteen_y_min, fifteen_x_min, fifteen_y_min[10]
  coords->block_fourteen.x_max =  tmp + ((uart_rx & 0b11111111100000000000000000000000) >> 23);
  coords->block_fourteen.y_max =  (uart_rx & 0b00000000011111111111000000000000) >> 12;
  coords->block_fifteen.x_max =   (uart_rx & 0b00000000000000000000111111111110) >> 1;
  tmp =                           (uart_rx & 0b00000000000000000000000000000001) << 10;

  read32bits(&uart_rx);     // fifteen_y_min[9:0], sixteen_x_min, sixteen_y_min
  coords->block_fifteen.y_max =   tmp + ((uart_rx & 0b11111111110000000000000000000000) >> 22);
  coords->block_sixteen.x_max =   (uart_rx & 0b00000000001111111111100000000000) >> 11;
  coords->block_sixteen.y_max =   (uart_rx & 0b00000000000000000000011111111111);

}

void printCoordinates(boxCoordinates *coords){
  Serial.print(" one_x_min: ");
  Serial.print(coords->block_one.x_min);
  Serial.print(" one_y_min: ");
  Serial.print(coords->block_one.y_min);
  Serial.print(" two_x_min: ");
  Serial.println(coords->block_two.x_min);
}

// char hexToCharArray(){
//   char tmp[32];

// }

// uint16_t mutilatePacket(char packet[32], uint8_t shoulder, uint8_t kneecap){
// }


