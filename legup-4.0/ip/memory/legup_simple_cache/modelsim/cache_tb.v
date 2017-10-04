
`timescale 1ns / 1ps

module cache_tb ();

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
wire					s_waitrequest;
wire					m_read;
wire					m_write;

// Internal Registers
reg					clk;
reg					reset;

reg		[ 5: 0]	s_rom_address;
reg		[69: 0]	s_rom_data;
reg		[69: 0]	s_rom_data_last_value;

reg		[ 5: 0]	m_rom_address;
reg		[33: 0]	m_rom_data;

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
		s_rom_address <= 0;
	else if (~s_waitrequest)
		s_rom_address <= s_rom_address + 1;
end

always @(posedge clk)
begin
	s_rom_data_last_value <= s_rom_data;
end

always @(posedge clk)
begin
	if (reset)
		m_rom_address <= 0;
	else if (m_read | m_write | m_rom_data[32])
		m_rom_address <= m_rom_address + 1;
end

/*****************************************************************************
/*****************************************************************************
 *                            Combinational Logic                            *
 *****************************************************************************/

always @(*)
begin
	case (s_rom_address)
	//	s_rom_data = {write, read, byteenable, writedata, address};
	0		:	s_rom_data	<=	{1'b0, 1'b0, {68{1'b0}}} | {{$random} % 16, {$random}, {$random}};
	1		:	s_rom_data	<=	{1'b0, 1'b1, {68{1'b0}}} | {{$random} % 16, {$random}, {$random}};
	2		:	s_rom_data	<=	{1'b0, 1'b0, {68{1'b0}}} | {{$random} % 16, {$random}, s_rom_data_last_value[31:0]};
	3		:	s_rom_data	<=	{1'b0, 1'b1, {68{1'b0}}} | {{$random} % 16, {$random}, s_rom_data_last_value[31:0]};
	4		:	s_rom_data	<=	{1'b0, 1'b0, {68{1'b0}}} | {{$random} % 16, {$random}, {$random}};
	5		:	s_rom_data	<=	{1'b0, 1'b0, {68{1'b0}}} | {{$random} % 16, {$random}, {$random}};
	6		:	s_rom_data	<=	{1'b0, 1'b1, {68{1'b0}}} | {{$random} % 16, {$random}, {$random}};
	7		:	s_rom_data	<=	{1'b0, 1'b1, {68{1'b0}}} | {{$random} % 16, {$random}, {$random}};
	8		:	s_rom_data	<=	{1'b1, 1'b0, {68{1'b0}}} | {{$random} % 16, {$random}, s_rom_data_last_value[31:0]};

	default	:	s_rom_data	<=	{1'b0, 1'b0, {68{1'b0}}} | {{$random} % 16, {$random}, {$random}};
	endcase
end

always @(*)
begin
	case (m_rom_address)
	//	m_rom_data = {waitrequest, readdatavalid, readdata};
	0		:	m_rom_data	<=	{1'b1, 1'b0, {$random}};
	1		:	m_rom_data	<=	{1'b1, 1'b0, {$random}};
	2		:	m_rom_data	<=	{1'b1, 1'b0, {$random}};
	3		:	m_rom_data	<=	{1'b0, 1'b0, {$random}};
	4		:	m_rom_data	<=	{1'b0, 1'b1, {$random}};
	5		:	m_rom_data	<=	{1'b1, 1'b0, {$random}};
	6		:	m_rom_data	<=	{1'b0, 1'b0, {$random}};
	7		:	m_rom_data	<=	{1'b0, 1'b1, {$random}};
	8		:	m_rom_data	<=	{1'b0, 1'b0, {$random}};
	9		:	m_rom_data	<=	{1'b0, 1'b1, {$random}};
	10		:	m_rom_data	<=	{1'b1, 1'b0, {$random}};
	11		:	m_rom_data	<=	{1'b1, 1'b0, {$random}};
	12		:	m_rom_data	<=	{1'b0, 1'b0, {$random}};
	13		:	m_rom_data	<=	{1'b1, 1'b0, {$random}};

	default	:	m_rom_data	<=	{1'b1, 1'b0, {$random}};
	endcase
end

/*****************************************************************************
 *                              Internal Modules                             *
 *****************************************************************************/

legup_simple_cache DUT (
	// Inputs
	.clk								(clk),
	.reset							(reset),

	.avs_cache_address			(s_rom_data[31:0]),
	.avs_cache_byteenable		(s_rom_data[67:64]),
	.avs_cache_read				(s_rom_data[68]),
	.avs_cache_write				(s_rom_data[69]),
	.avs_cache_writedata			(s_rom_data[63:32]),
	.avs_cache_readdata			(),
	.avs_cache_readdatavalid	(),
	.avs_cache_waitrequest		(s_waitrequest),

	.avm_cache_readdata			(m_rom_data[31:0]),
	.avm_cache_readdatavalid	(m_rom_data[32]),
	.avm_cache_waitrequest		(m_rom_data[33]),
	.avm_cache_address			(),
	.avm_cache_byteenable		(),
	.avm_cache_read				(m_read),
	.avm_cache_write				(m_write),
	.avm_cache_writedata			()
);

endmodule

