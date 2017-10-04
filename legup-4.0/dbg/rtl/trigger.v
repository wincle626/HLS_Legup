module trigger (

	clk,
	reset,
	
	config_activate,
	config_deactivate,
	
	config_module,
	config_state,

	config_cond_en,
	config_cond_addr,
	config_cond_val,
	config_cond_opcode,
	config_cond_index,
	
	config_cond_and_not_or_en,
	config_cond_and_not_or,

	pc_module,
	pc_state,
	
	main_wr_en_a,
	main_addr_a,
	main_writedata_a,
	
	main_wr_en_b,
	main_addr_b,
	main_writedata_b,
	
	breakpoint_hit
);

    function integer log2;
        input [31:0] value;
        if (value < 0) begin
           log2 = 0;
        end else begin
            for (log2=0; value>0; log2=log2+1)
            value = value>>1;
        end
    endfunction


	parameter MEMORY_CONTROLLER_ADDR_SIZE = 32;
	parameter MEMORY_CONTROLLER_DATA_SIZE = 64;
	parameter PC_MODULE_BITS = 16;
	parameter PC_STATE_BITS = 16;
	parameter NUM_CONDITIONS = 1;
	
	localparam INDEX_BITS = log2(NUM_CONDITIONS-1);
	
	input clk;
	input reset;
	
	input config_activate;
	input config_deactivate;
	
	input [PC_MODULE_BITS-1:0] config_module;
	input [PC_STATE_BITS-1:0] config_state;
	
	input config_cond_en;
	input [MEMORY_CONTROLLER_ADDR_SIZE-1:0] config_cond_addr;
	input [MEMORY_CONTROLLER_DATA_SIZE-1:0] config_cond_val;
	input [2:0] config_cond_opcode;
	input [INDEX_BITS-1:0] config_cond_index;
	
	input config_cond_and_not_or_en;
	input config_cond_and_not_or;
	
	input [PC_MODULE_BITS-1:0] pc_module;
	input [PC_STATE_BITS-1:0] pc_state;
	
	input main_wr_en_a;
	input [MEMORY_CONTROLLER_ADDR_SIZE-1:0] main_addr_a;
	input [MEMORY_CONTROLLER_DATA_SIZE-1:0] main_writedata_a;
	
	input main_wr_en_b;
	input [MEMORY_CONTROLLER_ADDR_SIZE-1:0] main_addr_b;
	input [MEMORY_CONTROLLER_DATA_SIZE-1:0] main_writedata_b;
	
	output breakpoint_hit;
	
	/* Regs */    
	reg active;
	reg [PC_MODULE_BITS-1:0] trigger_module;
	reg [PC_STATE_BITS-1:0] trigger_state;
	reg [MEMORY_CONTROLLER_ADDR_SIZE-1:0] trigger_addr [NUM_CONDITIONS-1:0];
	reg [MEMORY_CONTROLLER_DATA_SIZE-1:0] trigger_val [NUM_CONDITIONS-1:0];
	reg [2:0] trigger_opcode [NUM_CONDITIONS-1:0];
	reg trigger_and_not_or;
	
	reg [NUM_CONDITIONS-1:0] satisfied ;
	reg addr_hit_a [NUM_CONDITIONS-1:0];
	reg addr_hit_b [NUM_CONDITIONS-1:0];
	wire [NUM_CONDITIONS-1:0]inequality_satisfied_a ;
	wire  [NUM_CONDITIONS-1:0]inequality_satisfied_b;
	
	/* Wires */
	wire conditions_met_and;
	wire conditions_met_or;
	
        
	/* Activate/Deactivate */
	always @ (posedge clk) begin
		if (reset) begin
			active <= 1'b0;
		end else if (config_activate) begin
			active <= 1'b1;
            trigger_module <= config_module;
			trigger_state <= config_state;
        end	else if (config_deactivate) begin
			active <= 1'b0;
        end
	end	
	
	/* Conditions and/or */
	always @ (posedge clk) begin
		if (config_cond_and_not_or_en)
			trigger_and_not_or <= config_cond_and_not_or;			
	end
	
	/* Conditions */
	genvar i;
	generate
		for (i = 0; i < NUM_CONDITIONS; i=i+1) begin : TRIGGER_CONFIG
		
			// Condition configure
			always @ (posedge clk) begin
				if (reset) begin
					trigger_opcode[i] <= 3'b0;
				end
				else if (config_activate) begin
					trigger_opcode[i] <= 3'b0;
				end
				else if (config_cond_en)  begin
					if (config_cond_index == i) begin
						trigger_addr[i] <= config_cond_addr;
						trigger_val[i] <= config_cond_val;
						trigger_opcode[i] <= config_cond_opcode;
					end
				end
			end
			
			// Addr Hit
			always @ (*) begin
				if (main_wr_en_a == 1'b1 && main_addr_a == trigger_addr[i])
					addr_hit_a[i] <= 1'b1;
				else
					addr_hit_a[i] <= 1'b0;
					
				if (main_wr_en_b == 1'b1 && main_addr_b == trigger_addr[i])
					addr_hit_b[i] <= 1'b1;
				else
					addr_hit_b[i] <= 1'b0;
			end
			
			// Inequality
			trigger_inequality trigger_ineq_a(
				.trigger_data(trigger_val[i]),
				.write_data(main_writedata_a),
				.opcode(trigger_opcode[i]),
				.valid(inequality_satisfied_a[i])
			);
			
			trigger_inequality trigger_ineq_b(
				.trigger_data(trigger_val[i]),
				.write_data(main_writedata_b),
				.opcode(trigger_opcode[i]),
				.valid(inequality_satisfied_b[i])
			);
			
			// Condition Satisfied
			always @ (posedge clk) begin
				if (reset) 
					satisfied[i] <= 1'b1;
				else 	
					if (config_activate) begin
						satisfied[i] <= 1'b1;
					end
					else if (addr_hit_a[i]) 
						if (inequality_satisfied_a[i]) 
							satisfied[i] <= 1'b1;
						else
							satisfied[i] <= 1'b0;
					else if (addr_hit_b[i])
						if (inequality_satisfied_b[i])
							satisfied[i] <= 1'b1;
						else
							satisfied[i] <= 1'b0;
				
			end
		end
	endgenerate
	
	/* Breakpoint_hit */
	generate 
		if (NUM_CONDITIONS > 0) begin
			assign conditions_met_and = &satisfied;
			assign conditions_met_or = |satisfied;
			assign breakpoint_hit = active && 
				(pc_module == trigger_module) &&
				(pc_state == trigger_state) && 
				((trigger_and_not_or == 1'b1 && conditions_met_and) || (trigger_and_not_or == 1'b0 && conditions_met_or));
		end else begin
			assign breakpoint_hit = active && (pc_module == trigger_module) && (pc_state == trigger_state);
		end
	endgenerate
	
endmodule

module trigger_inequality(
	trigger_data,
	write_data,
	opcode,
	valid
);

	parameter DATA_WIDTH = 64;

	input [DATA_WIDTH-1:0] trigger_data;
	input [DATA_WIDTH-1:0] write_data;
	input [2:0] opcode;

	output reg valid;

	always @ (*) begin
		case (opcode)
			0: 
				valid = 1'b1;
			1:
				valid = (write_data == trigger_data);
			2:
				valid = (write_data != trigger_data);
			3:
				valid = (write_data >= trigger_data);
			4:
				valid = (write_data > trigger_data);
			5:
				valid = (write_data <= trigger_data);
			6:
				valid = (write_data < trigger_data);
			default:
				valid = 1'b0;
		endcase
	end

endmodule
