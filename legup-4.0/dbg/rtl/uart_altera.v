/******************************************************************************
 * License Agreement                                                          *
 *                                                                            *
 * Copyright (c) 1991-2012 Altera Corporation, San Jose, California, USA.     *
 * All rights reserved.                                                       *
 *                                                                            *
 * Any megafunction design, and related net list (encrypted or decrypted),    *
 *  support information, device programming or simulation file, and any other *
 *  associated documentation or information provided by Altera or a partner   *
 *  under Altera's Megafunction Partnership Program may be used only to       *
 *  program PLD devices (but not masked PLD devices) from Altera.  Any other  *
 *  use of such megafunction design, net list, support information, device    *
 *  programming or simulation file, or any other related documentation or     *
 *  information is prohibited for any other purpose, including, but not       *
 *  limited to modification, reverse engineering, de-compiling, or use with   *
 *  any other silicon devices, unless such use is explicitly licensed under   *
 *  a separate agreement with Altera or a megafunction partner.  Title to     *
 *  the intellectual property, including patents, copyrights, trademarks,     *
 *  trade secrets, or maskworks, embodied in any such megafunction design,    *
 *  net list, support information, device programming or simulation file, or  *
 *  any other related documentation or information provided by Altera or a    *
 *  megafunction partner, remains with Altera, the megafunction partner, or   *
 *  their respective licensors.  No other licenses, including any licenses    *
 *  needed under any third party's intellectual property, are provided herein.*
 *  Copying or modifying any file, or portion thereof, to which this notice   *
 *  is attached violates this copyright.                                      *
 *                                                                            *
 * THIS FILE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR    *
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   *
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    *
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER *
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING    *
 * FROM, OUT OF OR IN CONNECTION WITH THIS FILE OR THE USE OR OTHER DEALINGS  *
 * IN THIS FILE.                                                              *
 *                                                                            *
 * This agreement shall be governed in all respects by the laws of the State  *
 *  of California and by the laws of the United States of America.            *
 *                                                                            *
 ******************************************************************************/

/******************************************************************************
 *                                                                            *
 * This module reads and writes data to the RS232 connector on Altera's       *
 *  DE-series Development and Education Boards.                               *
 *                                                                            *
 ******************************************************************************/

module rs232 (
	// Inputs
	clk,
	reset,
	
	address,
	chipselect,
	byteenable,
	read,
	write,
	writedata,

	UART_RXD,

	// Bidirectionals

	// Outputs
	irq,
	readdata,

	UART_TXD
);

/*****************************************************************************
 *                           Parameter Declarations                          *
 *****************************************************************************/

parameter CW							= 9;		// Baud counter width
parameter BAUD_TICK_COUNT			= 434;
parameter HALF_BAUD_TICK_COUNT	= 217;

parameter TDW							= 10;		// Total data width
parameter DW							= 8;		// Data width
parameter ODD_PARITY					= 1'b0;

/*****************************************************************************
 *                             Port Declarations                             *
 *****************************************************************************/
// Inputs
input						clk;
input						reset;

input						address;
input						chipselect;
input			[ 3: 0]	byteenable;
input						read;
input						write;
input			[31: 0]	writedata;

input						UART_RXD;

// Bidirectionals

// Outputs
output reg				irq;
output reg	[31: 0]	readdata;

output					UART_TXD;

/*****************************************************************************
 *                           Constant Declarations                           *
 *****************************************************************************/

/*****************************************************************************
 *                 Internal Wires and Registers Declarations                 *
 *****************************************************************************/

// Internal Wires
wire						read_fifo_read_en;
wire			[ 7: 0]	read_available;
wire						read_data_valid;
wire		[(DW-1):0]	read_data;

wire						parity_error;

wire			[ 7: 0]	write_space;

// Internal Registers
reg						read_interrupt_en;
reg						write_interrupt_en;

reg						read_interrupt;
reg						write_interrupt;

reg						write_fifo_write_en;
reg		[(DW-1):0]	data_to_uart;

// State Machine Registers

/*****************************************************************************
 *                         Finite State Machine(s)                           *
 *****************************************************************************/


/*****************************************************************************
 *                             Sequential Logic                              *
 *****************************************************************************/

always @(posedge clk)
begin
	if (reset)
		irq <= 1'b0;
	else
		irq <= write_interrupt | read_interrupt;
end

always @(posedge clk)
begin
	if (reset)
		readdata <= 32'h00000000;
	else if (chipselect)
	begin
		if (address == 1'b0)
			readdata <= 
				{8'h00,
				 read_available,
				 read_data_valid,
				 5'h00,
				 parity_error,
				 1'b0,
				 read_data[(DW - 1):0]};
		else
			readdata <= 
				{8'h00,
				 write_space,
				 6'h00,
				 write_interrupt,
				 read_interrupt,
				 6'h00,
				 write_interrupt_en,
				 read_interrupt_en};
	end
end


always @(posedge clk)
begin
	if (reset)
		read_interrupt_en <= 1'b0;
	else if ((chipselect) && (write) && (address) && (byteenable[0]))
		read_interrupt_en <= writedata[0];
end

always @(posedge clk)
begin
	if (reset)
		write_interrupt_en <= 1'b0;
	else if ((chipselect) && (write) && (address) && (byteenable[0]))
		write_interrupt_en <= writedata[1];
end

always @(posedge clk)
begin
	if (reset)
		read_interrupt <= 1'b0;
	else if (read_interrupt_en == 1'b0)
		read_interrupt <= 1'b0;
	else
		read_interrupt <= (&(read_available[6:5]) | read_available[7]);
end

always @(posedge clk)
begin
	if (reset)
		write_interrupt <= 1'b0;
	else if (write_interrupt_en == 1'b0)
		write_interrupt <= 1'b0;
	else
		write_interrupt <= (&(write_space[6:5]) | write_space[7]);
end


always @(posedge clk)
begin
	if (reset)
		write_fifo_write_en <= 1'b0;
	else
		write_fifo_write_en <= 
			chipselect & write & ~address & byteenable[0];
end

always @(posedge clk)
begin
	if (reset)
		data_to_uart <= 'h0;
	else
		data_to_uart <= writedata[(DW - 1):0];
end

/*****************************************************************************
 *                            Combinational Logic                            *
 *****************************************************************************/

assign parity_error = 1'b0;

assign read_fifo_read_en = chipselect & read & ~address & byteenable[0];


/*****************************************************************************
 *                              Internal Modules                             *
 *****************************************************************************/

altera_up_rs232_in_deserializer RS232_In_Deserializer (
	// Inputs
	.clk						(clk),
	.reset					(reset),
	
	.serial_data_in		(UART_RXD),

	.receive_data_en		(read_fifo_read_en),

	// Bidirectionals

	// Outputs
	.fifo_read_available	(read_available),

	.received_data_valid	(read_data_valid),
	.received_data			(read_data)
);
defparam 
	RS232_In_Deserializer.CW							= CW,
	RS232_In_Deserializer.BAUD_TICK_COUNT			= BAUD_TICK_COUNT,
	RS232_In_Deserializer.HALF_BAUD_TICK_COUNT	= HALF_BAUD_TICK_COUNT,
	RS232_In_Deserializer.TDW							= TDW,
	RS232_In_Deserializer.DW							= (DW - 1);

altera_up_rs232_out_serializer RS232_Out_Serializer (
	// Inputs
	.clk						(clk),
	.reset					(reset),
	
	.transmit_data			(data_to_uart),
	.transmit_data_en		(write_fifo_write_en),

	// Bidirectionals

	// Outputs
	.fifo_write_space		(write_space),

	.serial_data_out		(UART_TXD)
);
defparam 
	RS232_Out_Serializer.CW							= CW,
	RS232_Out_Serializer.BAUD_TICK_COUNT		= BAUD_TICK_COUNT,
	RS232_Out_Serializer.HALF_BAUD_TICK_COUNT	= HALF_BAUD_TICK_COUNT,
	RS232_Out_Serializer.TDW						= TDW,
	RS232_Out_Serializer.DW							= (DW - 1);

endmodule




module altera_up_rs232_in_deserializer (
	// Inputs
	clk,
	reset,
	
	serial_data_in,

	receive_data_en,

	// Bidirectionals

	// Outputs
	fifo_read_available,

	received_data_valid,
	received_data
);

/*****************************************************************************
 *                           Parameter Declarations                          *
 *****************************************************************************/

parameter CW							= 9;		// Baud counter width
parameter BAUD_TICK_COUNT			= 433;
parameter HALF_BAUD_TICK_COUNT	= 216;

parameter TDW							= 11;		// Total data width
parameter DW							= 9;		// Data width

/*****************************************************************************
 *                             Port Declarations                             *
 *****************************************************************************/
// Inputs
input						clk;
input						reset;

input						serial_data_in;

input						receive_data_en;

// Bidirectionals

// Outputs
output reg	[ 7: 0]	fifo_read_available;

output					received_data_valid;
output		[DW: 0]	received_data;

/*****************************************************************************
 *                           Constant Declarations                           *
 *****************************************************************************/

/*****************************************************************************
 *                 Internal Wires and Registers Declarations                 *
 *****************************************************************************/

// Internal Wires
wire						shift_data_reg_en;
wire						all_bits_received;

wire						fifo_is_empty;
wire						fifo_is_full;
wire			[ 6: 0]	fifo_used;

// Internal Registers
reg						receiving_data;

reg		[(TDW-1):0]	data_in_shift_reg;

// State Machine Registers

/*****************************************************************************
 *                         Finite State Machine(s)                           *
 *****************************************************************************/


/*****************************************************************************
 *                             Sequential Logic                              *
 *****************************************************************************/

always @(posedge clk)
begin
	if (reset)
		fifo_read_available <= 8'h00;
	else
		fifo_read_available <= {fifo_is_full, fifo_used};
end

always @(posedge clk)
begin
	if (reset)
		receiving_data <= 1'b0;
	else if (all_bits_received)
		receiving_data <= 1'b0;
	else if (serial_data_in == 1'b0)
		receiving_data <= 1'b1;
end

always @(posedge clk)
begin
	if (reset)
		data_in_shift_reg	<= {TDW{1'b0}};
	else if (shift_data_reg_en)
		data_in_shift_reg	<= 
			{serial_data_in, data_in_shift_reg[(TDW - 1):1]};
end

/*****************************************************************************
 *                            Combinational Logic                            *
 *****************************************************************************/

// Output assignments
assign received_data_valid = ~fifo_is_empty;

// Input assignments


/*****************************************************************************
 *                              Internal Modules                             *
 *****************************************************************************/

altera_up_rs232_counters RS232_In_Counters (
	// Inputs
	.clk								(clk),
	.reset							(reset),
	
	.reset_counters				(~receiving_data),

	// Bidirectionals

	// Outputs
	.baud_clock_rising_edge		(),
	.baud_clock_falling_edge	(shift_data_reg_en),
	.all_bits_transmitted		(all_bits_received)
);
defparam 
	RS232_In_Counters.CW							= CW,
	RS232_In_Counters.BAUD_TICK_COUNT		= BAUD_TICK_COUNT,
	RS232_In_Counters.HALF_BAUD_TICK_COUNT	= HALF_BAUD_TICK_COUNT,
	RS232_In_Counters.TDW						= TDW;

altera_up_sync_fifo RS232_In_FIFO (
	// Inputs
	.clk				(clk),
	.reset			(reset),

	.write_en		(all_bits_received & ~fifo_is_full),
	.write_data		(data_in_shift_reg[(DW + 1):1]),

	.read_en			(receive_data_en & ~fifo_is_empty),
	
	// Bidirectionals

	// Outputs
	.fifo_is_empty	(fifo_is_empty),
	.fifo_is_full	(fifo_is_full),
	.words_used		(fifo_used),

	.read_data		(received_data)
);
defparam 
	RS232_In_FIFO.DW				= DW,
	RS232_In_FIFO.DATA_DEPTH	= 128,
	RS232_In_FIFO.AW				= 6;

endmodule




module altera_up_rs232_counters (
	// Inputs
	clk,
	reset,
	
	reset_counters,

	// Bidirectionals

	// Outputs
	baud_clock_rising_edge,
	baud_clock_falling_edge,
	all_bits_transmitted
);

/*****************************************************************************
 *                           Parameter Declarations                          *
 *****************************************************************************/

parameter CW							= 9;		// BAUD COUNTER WIDTH
parameter BAUD_TICK_COUNT			= 433;
parameter HALF_BAUD_TICK_COUNT	= 216;

parameter TDW							= 11;		// TOTAL DATA WIDTH

/*****************************************************************************
 *                             Port Declarations                             *
 *****************************************************************************/
// Inputs
input						clk;
input						reset;

input						reset_counters;

// Bidirectionals

// Outputs
output reg				baud_clock_rising_edge;
output reg				baud_clock_falling_edge;
output reg				all_bits_transmitted;

/*****************************************************************************
 *                           Constant Declarations                           *
 *****************************************************************************/

/*****************************************************************************
 *                 Internal Wires and Registers Declarations                 *
 *****************************************************************************/

// Internal Wires

// Internal Registers
reg		[(CW-1):0]	baud_counter;
reg			[ 3: 0]	bit_counter;

// State Machine Registers


/*****************************************************************************
 *                         Finite State Machine(s)                           *
 *****************************************************************************/


/*****************************************************************************
 *                             Sequential Logic                              *
 *****************************************************************************/

always @(posedge clk)
begin
	if (reset)
		baud_counter <= {CW{1'b0}};
	else if (reset_counters)
		baud_counter <= {CW{1'b0}};
	else if (baud_counter == BAUD_TICK_COUNT)
		baud_counter <= {CW{1'b0}};
	else
		baud_counter <= baud_counter + 1'b1;
end

always @(posedge clk)
begin
	if (reset)
		baud_clock_rising_edge <= 1'b0;
	else if (baud_counter == BAUD_TICK_COUNT)
		baud_clock_rising_edge <= 1'b1;
	else
		baud_clock_rising_edge <= 1'b0;
end

always @(posedge clk)
begin
	if (reset)
		baud_clock_falling_edge <= 1'b0;
	else if (baud_counter == HALF_BAUD_TICK_COUNT)
		baud_clock_falling_edge <= 1'b1;
	else
		baud_clock_falling_edge <= 1'b0;
end

always @(posedge clk)
begin
	if (reset)
		bit_counter <= 4'h0;
	else if (reset_counters)
		bit_counter <= 4'h0;
	else if (bit_counter == TDW)
		bit_counter <= 4'h0;
	else if (baud_counter == BAUD_TICK_COUNT)
		bit_counter <= bit_counter + 4'h1;
end

always @(posedge clk)
begin
	if (reset)
		all_bits_transmitted <= 1'b0;
	else if (bit_counter == TDW)
		all_bits_transmitted <= 1'b1;
	else
		all_bits_transmitted <= 1'b0;
end

/*****************************************************************************
 *                            Combinational Logic                            *
 *****************************************************************************/


/*****************************************************************************
 *                              Internal Modules                             *
 *****************************************************************************/


endmodule



module altera_up_sync_fifo (
	// Inputs
	clk,
	reset,

	write_en,
	write_data,

	read_en,
	
	// Bidirectionals

	// Outputs
	fifo_is_empty,
	fifo_is_full,
	words_used,

	read_data
);

/*****************************************************************************
 *                           Parameter Declarations                          *
 *****************************************************************************/

parameter	DW				= 31;		// Data width
parameter	DATA_DEPTH	= 128;
parameter	AW				= 6;		// Address width

/*****************************************************************************
 *                             Port Declarations                             *
 *****************************************************************************/

// Inputs
input						clk;
input						reset;

input						write_en;
input			[DW: 0]	write_data;

input						read_en;

// Bidirectionals

// Outputs
output					fifo_is_empty;
output					fifo_is_full;
output		[AW: 0]	words_used;

output		[DW: 0]	read_data;

/*****************************************************************************
 *                           Constant Declarations                           *
 *****************************************************************************/

/*****************************************************************************
 *                 Internal Wires and Registers Declarations                 *
 *****************************************************************************/

// Internal Wires

// Internal Registers

// State Machine Registers

/*****************************************************************************
 *                         Finite State Machine(s)                           *
 *****************************************************************************/


/*****************************************************************************
 *                             Sequential Logic                              *
 *****************************************************************************/


/*****************************************************************************
 *                            Combinational Logic                            *
 *****************************************************************************/


/*****************************************************************************
 *                              Internal Modules                             *
 *****************************************************************************/


scfifo	Sync_FIFO (
	// Inputs
	.clock			(clk),
	.sclr				(reset),

	.data				(write_data),
	.wrreq			(write_en),

	.rdreq			(read_en),

	// Bidirectionals

	// Outputs
	.empty			(fifo_is_empty),
	.full				(fifo_is_full),
	.usedw			(words_used),
	
	.q					(read_data)

	// Unused
	// synopsys translate_off
	
	,.aclr				(),
	.almost_empty	(),
	.almost_full	()
	// synopsys translate_on
);
defparam
	Sync_FIFO.add_ram_output_register	= "OFF",
	Sync_FIFO.intended_device_family		= "Cyclone II",
	Sync_FIFO.lpm_numwords					= DATA_DEPTH,
	Sync_FIFO.lpm_showahead					= "ON",
	Sync_FIFO.lpm_type						= "scfifo",
	Sync_FIFO.lpm_width						= DW + 1,
	Sync_FIFO.lpm_widthu						= AW + 1,
	Sync_FIFO.overflow_checking			= "OFF",
	Sync_FIFO.underflow_checking			= "OFF",
	Sync_FIFO.use_eab							= "ON";

endmodule




module altera_up_rs232_out_serializer (
	// Inputs
	clk,
	reset,
	
	transmit_data,
	transmit_data_en,

	// Bidirectionals

	// Outputs
	fifo_write_space,

	serial_data_out
);

/*****************************************************************************
 *                           Parameter Declarations                          *
 *****************************************************************************/

parameter CW							= 9;			// Baud counter width
parameter BAUD_TICK_COUNT			= 433;
parameter HALF_BAUD_TICK_COUNT	= 216;

parameter TDW							= 11;			// Total data width
parameter DW							= 9;			// Data width

/*****************************************************************************
 *                             Port Declarations                             *
 *****************************************************************************/
// Inputs
input						clk;
input						reset;

input			[DW: 0]	transmit_data;
input						transmit_data_en;

// Bidirectionals

// Outputs
output reg	[ 7: 0]	fifo_write_space;

output reg				serial_data_out;

/*****************************************************************************
 *                           Constant Declarations                           *
 *****************************************************************************/
 
/*****************************************************************************
 *                 Internal Wires and Registers Declarations                 *
 *****************************************************************************/

// Internal Wires
wire						shift_data_reg_en;
wire						all_bits_transmitted;

wire						read_fifo_en;

wire						fifo_is_empty;
wire						fifo_is_full;
wire			[ 6: 0]	fifo_used;

wire			[DW: 0]	data_from_fifo;

// Internal Registers
reg						transmitting_data;

reg			[DW+1:0]	data_out_shift_reg;

// State Machine Registers

/*****************************************************************************
 *                         Finite State Machine(s)                           *
 *****************************************************************************/


/*****************************************************************************
 *                             Sequential Logic                              *
 *****************************************************************************/

always @(posedge clk)
begin
	if (reset)
		fifo_write_space <= 8'h00;
	else
		fifo_write_space <= 8'h80 - {fifo_is_full, fifo_used};
end


always @(posedge clk)
begin
	if (reset)
		serial_data_out <= 1'b1;
	else
		serial_data_out <= data_out_shift_reg[0];
end

always @(posedge clk)
begin
	if (reset)
		transmitting_data <= 1'b0;
	else if (all_bits_transmitted)
		transmitting_data <= 1'b0;
	else if (fifo_is_empty == 1'b0)
		transmitting_data <= 1'b1;
end

always @(posedge clk)
begin
	if (reset)
		data_out_shift_reg	<= {(DW + 2){1'b1}};
	else if (read_fifo_en)
		data_out_shift_reg	<= {data_from_fifo, 1'b0};
	else if (shift_data_reg_en)
		data_out_shift_reg	<= 
			{1'b1, data_out_shift_reg[DW+1:1]};
end

/*****************************************************************************
 *                            Combinational Logic                            *
 *****************************************************************************/

assign read_fifo_en = 
			~transmitting_data & ~fifo_is_empty & ~all_bits_transmitted;

/*****************************************************************************
 *                              Internal Modules                             *
 *****************************************************************************/

altera_up_rs232_counters RS232_Out_Counters (
	// Inputs
	.clk								(clk),
	.reset							(reset),
	
	.reset_counters				(~transmitting_data),

	// Bidirectionals

	// Outputs
	.baud_clock_rising_edge		(shift_data_reg_en),
	.baud_clock_falling_edge	(),
	.all_bits_transmitted		(all_bits_transmitted)
);
defparam 
	RS232_Out_Counters.CW		= CW,
	RS232_Out_Counters.BAUD_TICK_COUNT			= BAUD_TICK_COUNT,
	RS232_Out_Counters.HALF_BAUD_TICK_COUNT	= HALF_BAUD_TICK_COUNT,
	RS232_Out_Counters.TDW							= TDW;

altera_up_sync_fifo RS232_Out_FIFO (
	// Inputs
	.clk				(clk),
	.reset			(reset),

	.write_en		(transmit_data_en & ~fifo_is_full),
	.write_data		(transmit_data),

	.read_en			(read_fifo_en),
	
	// Bidirectionals

	// Outputs
	.fifo_is_empty	(fifo_is_empty),
	.fifo_is_full	(fifo_is_full),
	.words_used		(fifo_used),

	.read_data		(data_from_fifo)
);
defparam 
	RS232_Out_FIFO.DW				= DW,
	RS232_Out_FIFO.DATA_DEPTH	= 128,
	RS232_Out_FIFO.AW				= 6;

endmodule

