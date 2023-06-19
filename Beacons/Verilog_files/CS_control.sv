module CS_control (
	
	input logic clk, en_10, sent,
	output logic CS, 
	output logic [1:0] state_num

);
	
	initial CS = 1'b1;
	
	typedef enum {s0, s1, s2} state;
	
	state current_state, next_state;
	
	always_ff @(posedge clk) begin
		current_state <= next_state; 
	end
	
	initial current_state = s0;
	
	always_comb begin
		case(current_state)
			s0 : begin // 1st part of cycle
					  CS = 1'b1;
					  if(en_10)
						  next_state = s1;
				     else
						  next_state = s0;
				  end
			s1 : begin
					  CS = 1'b0;
					  if(sent)
						  next_state = s2;
					  else if(~en_10)
						  next_state = s0;
					  else
						  next_state = s1;
				  end
				
			s2 : begin
					  CS = 1'b1;
				     if(~en_10)
					     next_state = s0;
					  else 
					     next_state = s2;
				  end
			
		endcase
	end		
endmodule
		
			
			


		
			
			
	
	
	
	
