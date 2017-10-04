module uart_control (
	clk,
	reset,
	read, 
	readdata,
	readdone,
	dataavail,
	write,
	writedata,
	writespace,
	RX,
	TX
);

	// Reading data from uart:
	// dataavail: indicates whether data is available to be read
	// When data is available:
	// - Assert read for 1 cycle
	// - Once data is available, readdone will be asserted for 1 cycle, and
	//   readdata will contain the data byte during the same cycle.  This can
	//   occur the cycle immediately following read being asserted, or later.
	//
	// Writign data to uart:
	// writespace: indicates whether data can be sent
	// To send data:
	//  - Assert write for 1 cycle
	//  - Provide data on writedata during that cycle


	input clk;
	input reset;
	input read;
	output [7:0] readdata;
	output readdone;
	output dataavail;
	input write;
	input [7:0] writedata;
	output writespace;
	output TX;
	input RX;

    function integer log2;
        input [31:0] value;
        for (log2=0; value>0; log2=log2+1)
        value = value>>1;
    endfunction

	
	parameter FPGA_VENDOR = "Altera";

	reg dataavail_r;
	reg [7:0] readdata_r;
	reg readdone_r;	    
	reg writespace_r;
	    
	assign readdone = readdone_r;
	assign dataavail = dataavail_r;
	assign readdata = readdata_r;
	assign writespace = writespace_r;

	generate
	
	// Xilinx
	if (FPGA_VENDOR == "Altera") begin

		localparam [0:0] UART_DATA = 0, UART_CONTROL = 1;

		reg uart_addr_c;
		reg uart_read_c;
		reg uart_write_c;
		reg [31:0] uart_writedata_c;
		wire [31:0] uart_readdata;
		reg data_is_waiting_to_send_r;
		reg [7:0] data_to_send_r;

		wire uart_dataavail = uart_readdata[15]; // (data_reg)
		wire [7:0] uart_writespace = uart_readdata[23:16]; // (control_reg)

	  	localparam STATES = 10;
	    localparam STATE_BITS = log2(STATES-1);
	    localparam [(STATE_BITS-1):0] 
	        S_RESET = 0, 
	        S_START = 1, 
	        S_WRITE_DATA = 2,
	        S_WRITE_DATA2 = 3,
	        S_WRITE_DATA3 = 4, 
	        S_WRITE_DATA4 = 5,
	        S_READ_DATA = 6,
	        S_READ_DATA2 = 7,
	        S_READ_CONTROL = 8,
	        S_READ_CONTROL2 = 9;

	    reg [STATE_BITS-1:0] state;

		rs232 uart_inst(			
			.clk(clk),
			.reset(reset),			
			.address(uart_addr_c),
			.chipselect(1'b1),
			.byteenable(4'b1),
			.read(uart_read_c),
			.write(uart_write_c),
			.writedata(uart_writedata_c),			
			.irq(),
			.readdata(uart_readdata),
			.UART_RXD(RX),
			.UART_TXD(TX)
		);

		always @ (posedge clk) begin
			if (reset)
				state <= S_RESET;
			else begin
				case (state)
				S_RESET:
					state <= S_START;
				S_START: begin
					if (data_is_waiting_to_send_r)
						state <= S_WRITE_DATA;
					else if (writespace_r == 0)
						state <= S_READ_CONTROL;
					else if (dataavail_r == 0)
						state <= S_READ_DATA;
				end

				S_WRITE_DATA:
					state <= S_WRITE_DATA2;
				S_WRITE_DATA2:
					// This extra state is needed because after writing to the uart
					// it takes a few cycles before you can accurately read the 
					// space available.  If you read too fast after a write you won't
					// observe the decrement of the available FIFO space.
					state <= S_START;
				S_WRITE_DATA3:
					state <= S_WRITE_DATA4;
				S_WRITE_DATA4:
					state <= S_START;

				S_READ_CONTROL:
					state <= S_READ_CONTROL2;
				S_READ_CONTROL2:
					state <= S_START;

				S_READ_DATA:
					state <= S_READ_DATA2;
				S_READ_DATA2:
					state <= S_START;

				default:
					state <= S_RESET;
				endcase
			end
		end

		always @  (posedge clk) begin
			if (reset) begin
				dataavail_r <= 1'b0;
				data_is_waiting_to_send_r <= 1'b0;
				readdone_r <= 1'b0;
				writespace_r <= 1'b0;
			end else begin
				if (state == S_READ_DATA2 && uart_dataavail) begin
					dataavail_r <= 1'b1;
					readdata_r <= uart_readdata[7:0];
				end else if (read)
					dataavail_r <= 1'b0;

				if (write)
					writespace_r <= 1'b0;
				else if ((state == S_READ_CONTROL2) && (~data_is_waiting_to_send_r))
					writespace_r <= (uart_writespace > 0);
				
				if (read)
					readdone_r <= 1'b1;
				else begin
					readdone_r <= 1'b0;
				end

				if (state == S_WRITE_DATA)
					data_is_waiting_to_send_r <= 1'b0;
				else if (write) begin
					data_is_waiting_to_send_r <= 1'b1;
					data_to_send_r <= writedata;
				end

			end
		end

		always @ (*) begin
			uart_writedata_c = 32'b0;
			uart_addr_c = UART_DATA;
			uart_write_c = 1'b0;
			uart_read_c = 1'b0;
			case (state)
				S_READ_DATA:
					uart_read_c = 1'b1;
				S_READ_CONTROL: begin
					uart_addr_c = UART_CONTROL;
					uart_read_c = 1'b1;
				end
				S_WRITE_DATA: begin
					uart_write_c = 1'b1;
					uart_writedata_c[7:0] = data_to_send_r;
				end
				default: begin
					
				end
			endcase
		end


	end else if (FPGA_VENDOR == "Xilinx") begin	
	    



		localparam UART_RX_REG = 0, UART_TX_REG = 1, UART_STATUS_REG = 2, UART_CTRL_REG = 3;

	  	localparam STATES = 4;
	    localparam STATE_BITS = log2(STATES-1);
	    localparam [(STATE_BITS-1):0] 
	        S_RESET = 0, 
	        S_READ_STATUS = 1, 
	        S_WRITE = 2, 
	        S_READ_DATA = 3;

	    reg [STATE_BITS-1:0] state;

	    wire [7:0] uart_writedata;
	    wire [7:0] uart_readdata;
	    reg [0:3] uart_rdce;
	    reg [0:3] uart_wrce;


	    assign uart_writedata = writedata;

		uartlite_core 
			#(
				.C_S_AXI_ACLK_FREQ_HZ(66000000),
				.C_BAUDRATE(115200)
			)
			uart_inst (
	                .Clk            (clk),
	                .Reset          (reset),
	                .bus2ip_data    (uart_writedata),
	                .bus2ip_rdce    (uart_rdce),
	                .bus2ip_wrce    (uart_wrce),
	                .bus2ip_cs      (1'b1),
	                .ip2bus_rdack   (),
	                .ip2bus_wrack   (),
	                .SIn_DBus       (uart_readdata),
	                .RX             (RX),
	                .TX             (TX),
	                .Interrupt      ()
	        );  

		always @ (posedge clk) begin
			if (reset)
				state <= S_RESET;
			else begin
				case (state)
				S_RESET:
					state <= S_READ_STATUS;
				S_READ_STATUS:
					if (write)
						state <= S_WRITE;
					else if (read)
						state <= S_READ_DATA;
				S_WRITE:
					state <= S_READ_STATUS;
				S_READ_DATA:
					state <= S_READ_STATUS;
				default:
					state <= S_RESET;
				endcase
			end
		end

		always @ (posedge clk) begin
			case (state)
			S_READ_STATUS: begin
				dataavail_r <= uart_readdata[0];
				writespace_r <= ~uart_readdata[3];
				if (read)
					readdone_r <= 1'b0;
			end
			S_READ_DATA: begin
				readdata_r <= uart_readdata;
				readdone_r <= 1'b1;
			end
			S_WRITE: 
				// Assume no write space until we check again
				writespace_r <= 1'b0;
			endcase
		end

		always @ (*) begin
			uart_rdce <= 4'b0000;
			uart_wrce <= 4'b0000;
			case (state)
			S_READ_STATUS: 
				uart_rdce[UART_STATUS_REG] <= 1'b1;
			S_READ_DATA:
				uart_rdce[UART_RX_REG] <= 1'b1;
			S_WRITE:
				uart_wrce[UART_TX_REG] <= 1'b1;
			endcase
		end

	end else begin
		non_existing_module illegal_parameter();
	end
	endgenerate
endmodule