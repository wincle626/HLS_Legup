
`timescale 1ns / 1ps

module div_testbench ();

/*****************************************************************************
 *                           Parameter Declarations                          *
 *****************************************************************************/

parameter CLOCK_PERIOD = 20;

/*****************************************************************************
 *                             Port Declarations                             *
 *****************************************************************************/
// Inputs

// Bidirectionals

// Outputs

/*****************************************************************************
 *                 Internal Wires and Registers Declarations                 *
 *****************************************************************************/

// Internal Wires
wire					run;

wire					my_done;
wire					steve_done;

wire		[63: 0]	my_result;
wire		[63: 0]	steve_result;
wire		[63: 0]	lpm_result;

// Internal Registers
reg					clk;
reg					reset;
reg					clken;

reg		[ 5: 0]	rom_address;
reg		[16: 0]	rom_data;

reg		[31: 0]	operandA;
reg		[31: 0]	operandB;

reg					my_error;
reg					steve_error;

// State Machine Registers

/*****************************************************************************
 *                         Finite State Machine(s)                           *
 *****************************************************************************/


/*****************************************************************************
 *                             Sequential Logic                              *
 *****************************************************************************/

initial begin
		reset <= 1'b1;
	#20	reset <= 1'b0;
		clken <= 1'b1;
end

initial
begin
	clk <= 1'b0;
end

always
begin : Clock_Generator
	#((CLOCK_PERIOD) / 2) clk = ~clk;
end

always @(posedge clk)
begin
	if (reset)
		rom_address <= 0;
	else if (~run | steve_done)
		rom_address <= rom_address + 1;
end

always @(posedge clk)
begin
	if (reset | (steve_done & run))
	begin
		operandA <= $random;
		operandB[31:16] <= 0;
		operandB[15:0] <= $random;
	end
end

always @(posedge clk)
begin
	if (reset)
		my_error <= 1'b0;
	else if (run & my_done & (my_result != lpm_result))
		my_error <= 1'b1;
end

always @(posedge clk)
begin
	if (reset)
		steve_error <= 1'b0;
	else if (run & steve_done & (steve_result != lpm_result))
		steve_error <= 1'b1;
end

/*****************************************************************************
 *                            Combinational Logic                            *
 *****************************************************************************/

assign run		= rom_data[0];

always @(*)
begin
	case (rom_address)
	//	rom_data = {serial_data, startofpacket, endofpacket, valid, channel, data};
	0		:	rom_data	<=	{1'b0};
	1		:	rom_data	<=	{1'b0};
	2		:	rom_data	<=	{1'b0};
	3		:	rom_data	<=	{1'b1};
	4		:	rom_data	<=	{1'b0};
	5		:	rom_data	<=	{1'b0};
	6		:	rom_data	<=	{1'b1};
	7		:	rom_data	<=	{1'b0};
	8		:	rom_data	<=	{1'b0};
	9		:	rom_data	<=	{1'b1};
	10		:	rom_data	<=	{1'b0};
	11		:	rom_data	<=	{1'b0};
	12		:	rom_data	<=	{1'b1};
	13		:	rom_data	<=	{1'b0};
	14		:	rom_data	<=	{1'b0};
	15		:	rom_data	<=	{1'b1};
	16		:	rom_data	<=	{1'b0};
	17		:	rom_data	<=	{1'b0};
	18		:	rom_data	<=	{1'b1};

	default	:	rom_data	<=	{1'b0};
	endcase
end

/*****************************************************************************
 *                              Internal Modules                             *
 *****************************************************************************/


SerialDivider MY_DUT (
	.clk			(clk), 
	.clken			(clken), 
	.resetn		(~reset), 
	.go			(~reset & run),
	.dividend	(operandA),
	.divisor		(operandB),

	.remainder	(my_result[63:32]),
	.quotient	(my_result[31: 0]),
	.done			(my_done)
);
defparam
	MY_DUT.n			= 32,
	MY_DUT.log2n	= 5;

divider_steve DUT (
	.Clock	(clk), 
	.Resetn	(~reset), 
	.s			(~reset & run),
	.LA		(~reset & ~run),
	.EB		(~reset & ~run),
	.DataA	(operandA),
	.DataB	(operandB),

	.R			(steve_result[63:32]),
	.Q			(steve_result[31: 0]),
	.Done		(steve_done)
);
defparam
	DUT.n		= 32,
	DUT.logn	= 5;

lpm_divide	LPM_DIVIDE_component (
	.clock		(clk),
	.denom		(operandB),
	.numer		(operandA),
	.remain		(lpm_result[63:32]),
	.quotient	(lpm_result[31: 0]),
	.aclr			(1'b0),
	.clken		(1'b1));
defparam
	LPM_DIVIDE_component.lpm_drepresentation = "UNSIGNED",
	LPM_DIVIDE_component.lpm_hint = "LPM_REMAINDERPOSITIVE=TRUE",
	LPM_DIVIDE_component.lpm_nrepresentation = "UNSIGNED",
	LPM_DIVIDE_component.lpm_pipeline = 1,
	LPM_DIVIDE_component.lpm_type = "LPM_DIVIDE",
	LPM_DIVIDE_component.lpm_widthd = 32,
	LPM_DIVIDE_component.lpm_widthn = 32;

endmodule

