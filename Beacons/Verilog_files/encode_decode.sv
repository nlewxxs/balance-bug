module encode_decode(

	input logic [23:0] received0, // SMPS
	input logic [15:0] received1, //ESP32
	input logic [2:0] state,
	input logic [9:0] val,
	input logic charge_on,
	output logic [7:0] V_panel, I_panel, V_cap,
	output logic [23:0] return_data0, //SMPS
	output logic [15:0] return_data1 //ESP
	
);

	initial V_panel = 8'b0;
	initial I_panel = 8'b0;
	initial V_cap = 8'b0;

//------------------------------received from SMPS arduino------------------------------------//

	assign V_panel = received0[23:16];
	assign I_panel = received0[15:8];
	assign V_cap = received0[7:0];

//------------------send state and control value to ESP, control value to SMPS-----------------//

	assign return_data0 = {11'b0, state[2:0], val[9:0]};
	assign return_data1 = {3'b0, state[2:0], val[9:0]};

endmodule


	