module ztiger_control_unit(
	input clock, reset, i_stall,
	input [7:0] d_stall_bus,
	
	input [31:0] instruction,
	input [31:0] rs, rt,
		
	output stall_fetch, stall_decode, stall_execute, reset_fetch, reset_execute, reset_wb,
	output reg [31:0] control_ex, control_wb,
	output reg [7:0]  data_mux_sel,
	
	output [31:0] nextnextPC, add_result
);

	reg [31:0] instr, nextPC;	
	reg jump;	
	wire branch; 	// it will be assigned in the PC related section	
	wire jump_tmp;

	/********** 	control signals 	**********/	
	wire d_stall = d_stall_bus != 8'd0;
	wire depend  = (((({instr[31:28], instr[26]} != 5'b00000) & (instr[31:29] != 3'b001)) | jump_tmp) &   // requires rs immediately
	                 (((instr[25:21] == control_ex[4:0]) | (instr[25:21] == control_wb[4:0])) & (instr[25:21] != 5'd0))) |
	               ((instr[31:27] == 5'b00010) &                                                          // requires rt immediately
	                 (((instr[20:16] == control_ex[4:0]) | (instr[20:16] == control_wb[4:0])) & (instr[20:16] != 5'd0))) ;

	wire   stall_pc      = stall_fetch;
	assign stall_fetch   = ~reset & (d_stall | i_stall | depend);
	assign stall_decode  = ~reset &  d_stall;
	assign stall_execute = ~reset &  d_stall;					
					
	wire   reset_pc      = reset;
	assign reset_fetch   = reset | branch | jump;
	wire   reset_decode  = reset | stall_fetch;
	assign reset_execute = reset; // do not | stall_decode, because it's the same with stall_execute
	assign reset_wb      = reset | stall_execute;

	/********** 	fetch stage 		**********/
	always @ (posedge clock)
	begin
		if(stall_fetch)
			instr <= instr;	
		else if(reset_fetch)
			instr <= 32'd0;
		else 
			instr <= instruction;
	end	
	
	/**********		decode stage 		**********/

	// whether is a link instruction
	wire link = (instr[31:26] == 6'b000011) | ((instr[31:26] == 6'b000000) & (instr[5:0] == 6'b001001));	
	
	// decode for which unit to execute
	always @ (posedge clock)
	begin
		if(stall_decode)
			control_ex <= control_ex;	
		else if(reset_decode)
			control_ex <= 32'd0;	
		else begin
			control_ex[31:22] <= 10'd0;		// not used
			control_ex[21]    <= instr[31:26] == 6'b001111;                                                            // LUI
			control_ex[20]    <= instr[28];                                                                            // unsign
			control_ex[19]    <= ~instr[3];                                                                            // HI/LO access
			control_ex[18]    <= instr[29];                                                                            // loadn_store
			control_ex[17:16] <= (instr[31:26] == 6'd0) ? instr[1:0] : instr[27:26];                                   // last two bit

			// select execution unit
			control_ex[15]    <= 1'b0;                                                                                 // c0
			control_ex[14]    <= ( instr[31:26] == 6'b000000) & ( instr[5:4] == 5'b01);                                // Mult_Divider
			control_ex[13]    <= ( instr[31:26] == 6'b000000) & ( instr[5:3] == 5'b000) & (instr[15:11] != 5'd0);      // Shifter
			control_ex[12]    <= ( instr[31:30] == 2'b10);	                                                           // Memory_Access 
			control_ex[11]    <= ( instr[31:26] == 6'b001111) |   link;                                                // Direct
			control_ex[10]    <= ((instr[31:28] == 4'b0011)   & ( instr[27:26] != 2'b11)) | 
			                     ((instr[31:26] == 6'b000000) & ( instr[5:2] == 4'b1001));                             // Logic
			control_ex[9]     <= ( instr[31:27] == 5'b00101)  |
			                     ((instr[31:26] == 6'b000000) & ( instr[5:3] == 3'b101));                              // Comp
			control_ex[8]     <= ( instr[31:27] == 5'b00100)  | ((instr[31:26] == 6'b000000) & instr[5:2] == 4'b1000); // Add_Sub
			
			// decode for destination
			control_ex[5]     <= 1'b0;	// register file (reserve for floating point extention)
			control_ex[4:0]   <= ((instr[31:26] == 6'b000000) & 
			                     ((instr[4:3] == 2'b00) | (instr[5:3] == 3'b101) | ({instr[5:2], instr[0]} == 5'b01000))) ? instr[15:11] : 
			                     link ? 5'd31 : (instr[31] ^ instr[29]) ? instr[20:16] : 5'd0;	
		end
	end	
	
	// decode for data mux selection
	wire [7:0] mux_sel;
	// dataa
	assign mux_sel[7] = mux_sel[6:4] == 3'b000;                                                               // rs
	assign mux_sel[6] = ((instr[31:30] == 2'b10) | link);                                                     // add_result
	assign mux_sel[5] = ((instr[31:26] == 6'b000000) & (instr[5:2] == 4'b0000));                              // shamt
	assign mux_sel[4] = (mux_sel[6:5] == 2'b00) & (instr[25:21] == control_ex[4:0]) & (instr[25:21] != 5'd0); // data_rs
	//datab
	assign mux_sel[3] = mux_sel[2:0] == 3'b000;                                                               // rt
	assign mux_sel[2] = (instr[31:28] == 4'b0011);                                                            // ZeroImm
	assign mux_sel[1] = instr[31:28] == 4'b0010;	                                                          // SignImm
	assign mux_sel[0] = (mux_sel[2:1] == 2'b00) & (instr[20:16] == control_ex[4:0]) & (instr[20:16] != 5'd0); // data_rt
	
	always @ (posedge clock)
	begin
		if(stall_decode)
			data_mux_sel <= data_mux_sel;	
		else if(reset_decode)
			data_mux_sel <= 8'd0;	
		else
			data_mux_sel <= mux_sel;
	end		
	
	/********** 	execution stage	**********/
	always @ (posedge clock)
	begin
		if(stall_execute)
			control_wb <= control_wb;	
		else if(reset_execute)
			control_wb <= 32'd0;
		else
			control_wb <= control_ex;
	end
		
	
	/********** 	PC related(de s)	**********/

	// decoding for branch and jump instruction
	wire [5:0] branch_type;
	assign branch_type[5] = (instr[31:28] == 4'b0001)   |  (instr[31:26] == 6'b000001);                // branch_instr
	assign branch_type[4] = (instr[31:26] == 6'b000110) | ((instr[31:26] == 6'b000001) & ~instr[16]);	// branch_ltz
	assign branch_type[3] = (instr[31:26] == 6'b000111) | ((instr[31:26] == 6'b000001) & instr[16]);	// branch_gtz
	assign branch_type[2] = (instr[31:26] == 6'b000110) | ((instr[31:26] == 6'b000001) & instr[16]);	// branch_zero
	assign branch_type[1] = instr[31:26] == 6'b000101;																	// branch_ne
	assign branch_type[0] = instr[31:26] == 6'b000100; 			 													// branch_eq
	
	reg  [5:0] branch_type_buff;
	always @ (posedge clock)
	begin
		if(stall_pc)
			branch_type_buff <= branch_type_buff;	
		else if(reset_pc)
			branch_type_buff <= 6'd0;
		else
			branch_type_buff <= branch_type;
	end	
		
	assign jump_tmp = (instr[31:27] == 5'b00001) | ((instr[31:26] == 6'b000000) & instr[5:1] == 5'b00100) ;
	always @ (posedge clock)
	begin
		if(stall_pc)
			jump <= jump;	
		else if(reset_pc)
			jump <= 1'b0;
		else
			jump <= jump_tmp;
	end	

	// prepare data for deciding whether branch should taken 	
	reg [6:0] condition;
	always @ (posedge clock)
	begin
		if(stall_pc)
			condition <= condition;
		else begin
			condition[6] <= rs[31];                 // ltz
			condition[5] <= rs[31:16] == 16'd0;     // eqz
			condition[4] <= rs[15:0]  == 16'd0;			
			condition[3] <= rs[31:24] == rt[31:24]; // eq	
			condition[2] <= rs[23:16] == rt[23:16];
			condition[1] <= rs[15:8]  == rt[15:8];
			condition[0] <= rs[7:0]	  == rt[7:0];
		end
	end		
	
	// pre-calculate jump_to
	reg [31:0] jump_to;
	always @ (posedge clock)
	begin
		if(stall_pc)
			jump_to <= jump_to;
		else
			jump_to <= instr[27] ? 	{nextPC[31:28], instr[25:0], 2'b00} : rs;
	end	
		
	// adder preparation
	reg [31:0] to_add1, to_add2;
	always @ (posedge clock)
	begin
		if(stall_pc)
			to_add1 <= to_add1;
		else
			to_add1 <= branch_type[5] ? nextPC : link ? nextPC + 4 : rs;
	end

	always @ (posedge clock)
	begin
		if(stall_pc)
			to_add2 <= to_add2;
		else
			to_add2 <= branch_type[5] ? {{14{instr[15]}}, instr[15:0], 2'b00} : link ? 32'd0 : {{16{instr[15]}}, instr[15:0]};
	end	
	
	/********** 	PC related(ex s)	**********/
	assign add_result = to_add1 + to_add2;
	
	wire stall_tmp   = d_stall | depend;
	wire [1:0]  nnPC_sel = { stall_tmp | branch, stall_tmp | jump };
	wire [31:0] nextnextPC_wire = (nnPC_sel == 2'b00)? nextPC + 4 :
	                              (nnPC_sel == 2'b01)? jump_to :
	                              (nnPC_sel == 2'b10)? add_result : nextPC;	
	
	assign nextnextPC = i_stall ? nextPC : reset_pc ? 32'd0 : nextnextPC_wire;
	
	assign branch = (branch_type_buff[0] & (condition[3:0] == 4'b1111)) | 
	                (branch_type_buff[1] & (condition[3:0] != 4'b1111)) | 
						 (branch_type_buff[2] & (condition[5:4] == 2'b11))   | 
						 (branch_type_buff[3] & ~condition[6] & (condition[5:4] != 2'b11)) | (branch_type_buff[4] & condition[6]);	


	initial
		nextPC = 32'd0;

	always @(posedge clock)
	begin
		if(stall_pc)
			nextPC <= nextPC;	
		else if(reset_pc)
			nextPC <= 32'd0;
		else
			nextPC <= nextnextPC;	
	end
	
endmodule 
