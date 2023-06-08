# Hardware changes facilitating UART comms!
Couple pin mapping changes. We need to use the second set of UART pins to avoid collisions. 
```.uart_0_rx_tx_rxd	(ARDUINO_IO[8]),
.uart_0_rx_tx_txd	(ARDUINO_IO[9])```
> These changes are made in lines 181 and 182 of the top .v file.
In order for the regular RX and TX pins to not suck up the uart signal, we need to unassign them in the pin planner.  


# Bounding Box Output
Each output is 32 segment

| Output (32bit segments)     | Description |
| ----------- | ----------- |
| 0           | 0          |
| 1           | NB          |
| 2           | one_x_min, one_y_min, two_x_min[10:1]          |
| 3           | two_x_min[0], two_y_min, three_x_min, three_y_min[10:2]          |
| 4           | three_y_min[1:0], four_x_min, four_y_min, five_x_min[10:3]          |
| 5           | five_x_min[2:0], five_y_min, six_x_min, six_y_min[10:4]          |
| 6           | six_y_min[3:0], seven_x_min, seven_y_min, eight_x_min[10:5]          |
| 7           | eight_x_min[4:0], eight_y_min, nine_x_min, nine_y_min[10:6]          |
| 8           | nine_y_min[5:0], ten_x_min, ten_y_min, eleven_x_min[10:7]          |
| 9           | eleven_x_min[6:0], eleven_y_min, twelve_x_min, twelve_y_min[10:8]          |
| 10          | twelve_y_min[7:0], thirteen_x_min, thirteen_y_min, fourteen_x_min[10:9]          |
| 11          | fourteen_x_min[8:0], fourteen_y_min, fifteen_x_min, fifteen_y_min[10]          |
| 12          | fifteen_y_min[9:0], sixteen_x_min, sixteen_y_min          |
| 13          | one_x_max, one_y_max, two_x_max[10:1]          |
| 14          | two_x_max[0], two_y_max, three_x_max, three_y_max[10:2]          |
| 15          | three_y_max[1:0], four_x_max, four_y_max, five_x_max[10:3]          |
| 16          | five_x_max[2:0], five_y_max, six_x_max, six_y_max[10:4]          |
| 18          | six_y_max[3:0], seven_x_max, seven_y_max, eight_x_max[10:5]          |
| 19          | eight_x_max[4:0], eight_y_max, nine_x_max, nine_y_max[10:6]          |
| 20          | nine_y_max[5:0], ten_x_max, ten_y_max, eleven_x_max[10:7]          |
| 21          | eleven_x_max[6:0], eleven_y_max, twelve_x_max, twelve_y_max[10:8]          |
| 22          | twelve_y_max[7:0], thirteen_x_max, thirteen_y_max, fourteen_x_max[10:9]          |
| 23          | fourteen_x_max[8:0], fourteen_y_max, fifteen_x_max, fifteen_y_max[10]          |
| 24          | fifteen_y_max[9:0], sixteen_x_max, sixteen_y_max |


