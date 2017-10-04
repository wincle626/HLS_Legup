module hlsd (
    clk,
    reset,
    
    start,
    finish,
    program_clk,
    program_reset,
    
    pc_module,
    pc_state,

    // Memory controller interface
    memory_controller_enable_a,
    memory_controller_enable_b,
    memory_controller_address_a,
    memory_controller_address_b,
    memory_controller_write_enable_a,
    memory_controller_write_enable_b,
    memory_controller_in_a,
    memory_controller_in_b, 
    memory_controller_out_a,
    memory_controller_out_b,
    memory_controller_size_a,
    memory_controller_size_b,   
    
    // Main interface   
    main_enable_a,
    main_enable_b,
    main_address_a,
    main_address_b, 
    main_write_enable_a,
    main_write_enable_b,
    main_in_a,
    main_in_b,
    main_out_a,
    main_out_b,
    main_size_a,
    main_size_b,

    // Register tracing
    regs_trace,
    regs_trace_wr_en_a,
    regs_trace_wr_en_b,
    
    // UART
    uart_rx,
    uart_tx,
    
    // Debug
    hlsd_state,
    external_start
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

    parameter FPGA_VENDOR = "Altera";
    parameter OPTION_SUPPORT_WRITES = 0;
    parameter OPTION_BREAKPOINT_NUM_CONDITIONS = 0;
    parameter OPTION_MEMORY_READ_AT_TRIGGER = 1;
    parameter OPTION_MEMORY_READ_THEN_RESUME = 1;
    parameter OPTION_REALTIME_ENABLE_TRACE_MODULE = 0;
    parameter OPTION_TRACE_REGS = 0;
    parameter OPTION_TRACE_REGS_DUAL_PORTED = 0;
    
    // Not supported right now
    parameter OPTION_REALTIME_ENABLE_TRACE_VARIABLE = 0;
    

    // LegUp memory configuration
    parameter MEMORY_CONTROLLER_ADDR_SIZE = 32;
    parameter MEMORY_CONTROLLER_DATA_SIZE = 64;
    parameter MEM_TAG_BITS = 9;
    parameter MEM_OFFSET_BITS = 23;
    
    // SystemID will be set by parent module
    // This is a unique 32-bit ID generated upon synthesis,
    // used to ensure synchronization between the 
    // hardware and the debug database.
    parameter [31:0] SYSTEM_ID = 0; 
      
    parameter CONTROL_BUFFER_DEPTH = -1;
    parameter DATA_BUFFER_DEPTH = -1;
    parameter REGS_BUFFER_DEPTH = -1;

    
    parameter PC_MODULE_BITS = 8;
    parameter PC_STATE_BITS = 8;

    parameter REGS_WIDTH = 1;

    localparam TRIGGER_CONDITION_BITS = log2(OPTION_BREAKPOINT_NUM_CONDITIONS-1);

    // Calculated values for memory bus trace buffer        
    localparam SEQUENCE_BITS = 6;
    localparam CONTROL_BUFFER_WIDTH = PC_MODULE_BITS + PC_STATE_BITS + SEQUENCE_BITS; // 16 + 16 + 6 = 38
    localparam CONTROL_BUFFER_ADDR_WIDTH = log2(CONTROL_BUFFER_DEPTH-1); // 9 bits
    
    localparam DATA_BUFFER_WIDTH = 2 + MEMORY_CONTROLLER_ADDR_SIZE + MEMORY_CONTROLLER_DATA_SIZE; // 2 + 9 + 6 + 32 + 64 = 112
    //localparam DATA_BUFFER_ADDR_WIDTH = log2(DATA_BUFFER_DEPTH-1); // 9 bits
    
    function integer getRegsBufferWidth;
        input [31:0] regsWidth;
        if (OPTION_TRACE_REGS_DUAL_PORTED)
            getRegsBufferWidth = (regsWidth + 1) / 2;
        else
            getRegsBufferWidth = regsWidth;
    endfunction

    localparam REGS_BUFFER_WIDTH = getRegsBufferWidth(REGS_WIDTH);
    
    input clk;
    input reset;
    output program_clk;
    output program_reset;
    
    output start;
    input finish;
    input external_start;

    input [PC_MODULE_BITS-1:0] pc_module;
    input [PC_STATE_BITS-1:0] pc_state;

    /* Memory controller interface */
    output memory_controller_enable_a;
    output memory_controller_enable_b;
    output [MEMORY_CONTROLLER_ADDR_SIZE-1:0] memory_controller_address_a;
    output [MEMORY_CONTROLLER_ADDR_SIZE-1:0] memory_controller_address_b;
    output memory_controller_write_enable_a;
    output memory_controller_write_enable_b;
    output [MEMORY_CONTROLLER_DATA_SIZE-1:0] memory_controller_in_a;
    output [MEMORY_CONTROLLER_DATA_SIZE-1:0] memory_controller_in_b;
    input [MEMORY_CONTROLLER_DATA_SIZE-1:0] memory_controller_out_a;
    input [MEMORY_CONTROLLER_DATA_SIZE-1:0] memory_controller_out_b;
    output [1:0] memory_controller_size_a;
    output [1:0] memory_controller_size_b;

    /* Main interface */
    input main_enable_a;
    input main_enable_b;
    input [MEMORY_CONTROLLER_ADDR_SIZE-1:0] main_address_a;
    input [MEMORY_CONTROLLER_ADDR_SIZE-1:0] main_address_b;
    input main_write_enable_a;
    input main_write_enable_b;
    input [MEMORY_CONTROLLER_DATA_SIZE-1:0] main_in_a;
    input [MEMORY_CONTROLLER_DATA_SIZE-1:0] main_in_b;
    output [MEMORY_CONTROLLER_DATA_SIZE-1:0] main_out_a;
    output [MEMORY_CONTROLLER_DATA_SIZE-1:0] main_out_b;
    input [1:0] main_size_a;
    input [1:0] main_size_b;
    
    /* regs trace */
    input   [REGS_WIDTH-1:0]                    regs_trace;
    input                                       regs_trace_wr_en_a;
    input                                       regs_trace_wr_en_b;

    /* Control interface */
    wire hlsd_mem_rd_en;
    wire hlsd_mem_wr_en;
    wire hlsd_mem_waitrequest;
    wire [MEMORY_CONTROLLER_ADDR_SIZE-1:0] hlsd_mem_addr;
    wire [MEMORY_CONTROLLER_DATA_SIZE-1:0] hlsd_mem_readdata;
    wire [MEMORY_CONTROLLER_DATA_SIZE-1:0] hlsd_mem_writedata;

    /* UART */
    input uart_rx;
    output uart_tx;

    /* Debug */
    output [7:0] hlsd_state;

    /* Wires */
    wire                                            program_reset_req;
    wire                                            program_run_req;
                                                
    wire                                            readback_req;
    wire                                            readback_ack;
    wire                                            readback_done;
    
    wire    [CONTROL_BUFFER_WIDTH-1:0]              readback_control;
    wire    [DATA_BUFFER_WIDTH-1:0]                 readback_data;
    wire    [REGS_BUFFER_WIDTH-1:0]                 readback_regs;
    
    wire                                            uart_read;
    wire    [7:0]                                   uart_readdata;
    wire                                            uart_readdone;
    wire                                            uart_dataavail;
    wire                                            uart_write;
    wire    [7:0]                                   uart_writedata;
    wire                                            uart_writespace;        
   
    wire    [PC_MODULE_BITS-1:0]                    trigger_config_module;
    wire    [PC_STATE_BITS-1:0]                     trigger_config_state;
    wire                                            trigger_config_cond_en;
    wire    [MEMORY_CONTROLLER_ADDR_SIZE-1:0]       trigger_config_cond_addr;
    wire    [MEMORY_CONTROLLER_DATA_SIZE-1:0]       trigger_config_cond_val;
    wire    [2:0]                                   trigger_config_cond_opcode;
    wire    [TRIGGER_CONDITION_BITS-1:0]            trigger_config_cond_index;
    wire                                            trigger_hit;
    wire                                            trigger_config_cond_and_not_or;
    wire                                            trigger_config_cond_and_not_or_en;

    wire                                            trigger_activate;
    wire                                            trigger_deactivate;

    wire                                            trace_en_var_wr_en;
    wire    [8:0]                                   trace_en_var_tag;
    wire                                            trace_en_var_data;

    wire                                            trace_en_module_wr_en;
    wire    [PC_MODULE_BITS-1:0]                    trace_en_module_id;
    wire                                            trace_en_module_data;
    wire                                            program_running;

    /* Registers */
    reg     [PC_MODULE_BITS-1:0]                    pc_module_r;
    reg     [PC_STATE_BITS-1:0]                     pc_state_r;
    reg                                             program_running_r;

    /* Combinationals */
    
    
    reg [MEMORY_CONTROLLER_ADDR_SIZE-1:0] main_address_a_r;
    reg [MEMORY_CONTROLLER_ADDR_SIZE-1:0] main_address_b_r;
    
    /* Assignments */
    assign program_reset = program_reset_req | reset;
    assign program_running = program_run_req;
    
    always @ (posedge clk) begin
        pc_module_r <= pc_module;
        pc_state_r <= pc_state;
        program_running_r <= program_running;
    end
    
    ///////// Reset after power-on ////////////
    reg reset_after_power_on = 1;
    wire reset_with_poweron = reset | reset_after_power_on;
    
    always @ (posedge clk) 
    begin
        if (reset_after_power_on)
            reset_after_power_on <= 1'b0;
    end
    
    uart_control
        #(
            .FPGA_VENDOR(FPGA_VENDOR)
        ) 
        uart_control_inst (
        .clk(clk),
        .reset(reset_with_poweron),
        .read(uart_read), 
        .readdata(uart_readdata),
        .readdone(uart_readdone),
        .dataavail(uart_dataavail),
        .write(uart_write),
        .writedata(uart_writedata),
        .writespace(uart_writespace),
        .RX(uart_rx),
        .TX(uart_tx)
    );

    comm
        #(
            .OPTION_SUPPORT_WRITES(OPTION_SUPPORT_WRITES),
            .OPTION_MEMORY_READ_AT_TRIGGER(OPTION_MEMORY_READ_AT_TRIGGER),
            .OPTION_BREAKPOINT_NUM_CONDITIONS(OPTION_BREAKPOINT_NUM_CONDITIONS),
            .OPTION_REALTIME_ENABLE_TRACE_MODULE(OPTION_REALTIME_ENABLE_TRACE_MODULE),
            .OPTION_REALTIME_ENABLE_TRACE_VARIABLE(OPTION_REALTIME_ENABLE_TRACE_VARIABLE),
            
            .MEMORY_ADDR_BITS(MEMORY_CONTROLLER_ADDR_SIZE),
            .MEMORY_DATA_BITS(MEMORY_CONTROLLER_DATA_SIZE),
            .PC_MODULE_BITS(PC_MODULE_BITS),
            .PC_STATE_BITS(PC_STATE_BITS),
            .READBACK_CONTROL_BITS(CONTROL_BUFFER_WIDTH),
            .READBACK_DATA_BITS(DATA_BUFFER_WIDTH),
            .READBACK_REGS_BITS(REGS_BUFFER_WIDTH)
        )
        comm_inst (
        .clk                               (clk), 
        .reset                             (reset_with_poweron),
        .systemID                          (SYSTEM_ID),
        .dbg_state                         (hlsd_state),
        .pc_module                         (pc_module_r),
        .pc_state                          (pc_state_r),
        .program_run_req                   (program_run_req),
        .program_running                   (program_running),
        .program_reset_req                 (program_reset_req),
		.external_start					   (external_start),

        .mem_rd_en                         (hlsd_mem_rd_en),
        .mem_waitrequest                   (hlsd_mem_waitrequest),
        .mem_addr                          (hlsd_mem_addr),
        .mem_readdata                      (hlsd_mem_readdata),
        .mem_wr_en                         (hlsd_mem_wr_en),
        .mem_writedata                     (hlsd_mem_writedata),
        
        .readback_req                      (readback_req),
        .readback_ack                      (readback_ack),
        .readback_done                     (readback_done),
        .readback_control                  (readback_control),
        .readback_data                     (readback_data),
        .readback_regs                     (readback_regs),
        
        .trigger_activate                  (trigger_activate),                  
        .trigger_deactivate                (trigger_deactivate),                   
        .trigger_config_module             (trigger_config_module),             
        .trigger_config_state              (trigger_config_state),              
        .trigger_config_cond_en            (trigger_config_cond_en),            
        .trigger_config_cond_addr          (trigger_config_cond_addr),          
        .trigger_config_cond_val           (trigger_config_cond_val),           
        .trigger_config_cond_opcode        (trigger_config_cond_opcode),        
        .trigger_config_cond_index         (trigger_config_cond_index),         
        .trigger_config_cond_and_not_or_en (trigger_config_cond_and_not_or_en), 
        .trigger_config_cond_and_not_or    (trigger_config_cond_and_not_or),    
        .trigger_hit                       (trigger_hit),                       
                
        .uart_read                         (uart_read),
        .uart_readdata                     (uart_readdata),
        .uart_readdone                     (uart_readdone),
        .uart_dataavail                    (uart_dataavail),
        .uart_write                        (uart_write),
        .uart_writedata                    (uart_writedata),
        .uart_writespace                   (uart_writespace),

        .trace_en_var_wr_en                (trace_en_var_wr_en),
        .trace_en_var_tag                  (trace_en_var_tag),
        .trace_en_var_data                 (trace_en_var_data),
        .trace_en_module_wr_en             (trace_en_module_wr_en),
        .trace_en_module_id                (trace_en_module_id),
        .trace_en_module_data              (trace_en_module_data)
    );
           
    memory_supervisor 
        #(
            .OPTION_MEMORY_READ_AT_TRIGGER(OPTION_MEMORY_READ_AT_TRIGGER),
            .OPTION_MEMORY_READ_THEN_RESUME(OPTION_MEMORY_READ_THEN_RESUME),
            .MEMORY_CONTROLLER_ADDR_SIZE(MEMORY_CONTROLLER_ADDR_SIZE),
            .MEMORY_CONTROLLER_DATA_SIZE(MEMORY_CONTROLLER_DATA_SIZE)
        )
        
        memory_supervisor_inst (
        .clk(clk),
        .reset(program_reset),
        .program_running(program_running),
        .main_enable_a(main_enable_a),
        .main_enable_b(main_enable_b),
        .main_address_a(main_address_a),
        .main_address_b(main_address_b),
        .main_write_enable_a(main_write_enable_a),
        .main_write_enable_b(main_write_enable_b),
        .main_in_a(main_in_a),
        .main_in_b(main_in_b),
        .main_size_a(main_size_a),
        .main_size_b(main_size_b),
        .main_out_a(main_out_a),
        .main_out_b(main_out_b),
        .hlsd_mem_rd_en(hlsd_mem_rd_en),
        .hlsd_mem_wr_en(hlsd_mem_wr_en),
        .hlsd_mem_addr(hlsd_mem_addr),
        .hlsd_mem_waitrequest(hlsd_mem_waitrequest),
        .hlsd_mem_readdata(hlsd_mem_readdata),
        .hlsd_mem_writedata(hlsd_mem_writedata),        
        .memory_controller_enable_a(memory_controller_enable_a),
        .memory_controller_enable_b(memory_controller_enable_b),
        .memory_controller_address_a(memory_controller_address_a),
        .memory_controller_address_b(memory_controller_address_b),
        .memory_controller_write_enable_a(memory_controller_write_enable_a),
        .memory_controller_write_enable_b(memory_controller_write_enable_b),
        .memory_controller_in_a(memory_controller_in_a),
        .memory_controller_in_b(memory_controller_in_b),
        .memory_controller_size_a(memory_controller_size_a),
        .memory_controller_size_b(memory_controller_size_b),
        .memory_controller_out_a(memory_controller_out_a),
        .memory_controller_out_b(memory_controller_out_b)
    );

    startfinish 
        startfinish_inst(
            .clk(program_clk),
            .reset(program_reset),
            .start(start),
            .finish(finish)
        );
        
        
    reg [31:0] main_address_a_masked;
    reg [31:0] main_address_b_masked;
    
    always @ (*) begin : ADDRESS_MASKING
        integer i;
    
        main_address_a_masked = 32'b0;
        main_address_b_masked = 32'b0;
        
        
        for (i = 0; i < MEM_OFFSET_BITS; i=i+1)  begin
            main_address_a_masked[i] = main_address_a[i];
            main_address_b_masked[i] = main_address_b[i];
        end
        for (i = 0; i < MEM_TAG_BITS; i=i+1) begin
            main_address_a_masked[23+i] = main_address_a[23+i];
            main_address_b_masked[23+i] = main_address_b[23+i];
        end 
    end
        
    trigger 
        #(
            .PC_MODULE_BITS(PC_MODULE_BITS),
            .PC_STATE_BITS(PC_STATE_BITS),
            .NUM_CONDITIONS(OPTION_BREAKPOINT_NUM_CONDITIONS)
        )
        trigger_inst(
        .clk(clk),
        .reset(reset),  
        .config_activate(trigger_activate),
        .config_deactivate(trigger_deactivate),
        .config_module(trigger_config_module),
        .config_state(trigger_config_state),

        .config_cond_en(trigger_config_cond_en),
        .config_cond_addr(trigger_config_cond_addr),
        .config_cond_val(trigger_config_cond_val),
        .config_cond_opcode(trigger_config_cond_opcode),
        .config_cond_index(trigger_config_cond_index),
    
        .config_cond_and_not_or_en(trigger_config_cond_and_not_or_en),
        .config_cond_and_not_or(trigger_config_cond_and_not_or),

        .pc_module(pc_module_r),
        .pc_state(pc_state_r),
        
        .main_wr_en_a(main_write_enable_a),
        .main_addr_a(main_address_a_masked),
        .main_writedata_a(main_in_a),
    
        .main_wr_en_b(main_write_enable_b),
        .main_addr_b(main_address_b_masked),
        .main_writedata_b(main_in_b),
    
        .breakpoint_hit(trigger_hit)
    );                
        
    reg [REGS_BUFFER_WIDTH-1:0] regs_trace_a;
    reg [REGS_BUFFER_WIDTH-1:0] regs_trace_b;
    generate
        if (OPTION_TRACE_REGS_DUAL_PORTED) begin
            always @ (*) begin
                regs_trace_a <= regs_trace[REGS_BUFFER_WIDTH-1:0];
                regs_trace_b <= regs_trace[REGS_WIDTH-1:REGS_BUFFER_WIDTH];
            end
        end else begin
            always @ (*) begin
                regs_trace_a <= regs_trace;
                regs_trace_b <= 0;
            end
        end
    endgenerate

    trace 
        #(
            .OPTION_TRACE_REGS(OPTION_TRACE_REGS),            
            .OPTION_REALTIME_ENABLE_TRACE_MODULE(OPTION_REALTIME_ENABLE_TRACE_MODULE),
            .OPTION_REALTIME_ENABLE_TRACE_VARIABLE(OPTION_REALTIME_ENABLE_TRACE_VARIABLE),

            .MEMORY_DATA_WIDTH(MEMORY_CONTROLLER_DATA_SIZE),    
            .MEMORY_ADDR_WIDTH(MEMORY_CONTROLLER_ADDR_SIZE),
            
            .PC_MODULE_BITS(PC_MODULE_BITS),
            .PC_STATE_BITS(PC_STATE_BITS),            
            
            .CONTROL_BUFFER_WIDTH(CONTROL_BUFFER_WIDTH),
            .CONTROL_BUFFER_DEPTH(CONTROL_BUFFER_DEPTH),
            .DATA_BUFFER_WIDTH(DATA_BUFFER_WIDTH),
            .DATA_BUFFER_DEPTH(DATA_BUFFER_DEPTH),
            .SEQUENCE_BITS(SEQUENCE_BITS),

            .REGS_BUFFER_WIDTH(REGS_BUFFER_WIDTH),
            .REGS_BUFFER_DEPTH(REGS_BUFFER_DEPTH)
        )   
        trace_inst (        
        .clk(clk),
        .reset(program_reset),  
        .pc_module_r1(pc_module_r),
        .pc_state_r1(pc_state_r),        
        .program_running(program_running),
        .program_running_r1(program_running_r),
        .readback_req(readback_req),
        .readback_ack(readback_ack),    
        .readback_done(readback_done),
        .readback_control(readback_control),
        .readback_data(readback_data),
        .readback_regs(readback_regs),
        .main_wr_en_a(main_write_enable_a),     
        .main_wr_en_b(main_write_enable_b),
        .main_addr_a(main_address_a_masked),
        .main_addr_b(main_address_b_masked),
        .main_size_a(main_size_a),  
        .main_size_b(main_size_b),
        .main_data_a(main_in_a),    
        .main_data_b(main_in_b),  
        
        .regs_muxed_a(regs_trace_a),
        .regs_muxed_b(regs_trace_b),
        .regs_valid_a(regs_trace_wr_en_a),
        .regs_valid_b(regs_trace_wr_en_b),
        
        .trace_en_var_wr_en(trace_en_var_wr_en),
        .trace_en_var_tag(trace_en_var_tag),
        .trace_en_var_data(trace_en_var_data),
        .trace_en_module_wr_en(trace_en_module_wr_en),
        .trace_en_module_id(trace_en_module_id),
        .trace_en_module_data(trace_en_module_data)
    );

    clockbuf    clockbuf_inst (
      .ena ( program_running | program_reset ),  // User circuit is a synchronous reset, so we need to enable the program clock during reset.
      .inclk ( clk ),
      .outclk ( program_clk )
    );
    
endmodule

module startfinish(
    input clk, 
    input reset, 
    output start, 
    input finish
);

    localparam [2:0] S_RESET = 0, S_START = 1, S_WAIT = 2, S_FINISH = 3;
    reg [2:0] cur_state;
    
    always @ (posedge clk or posedge reset) begin
        if (reset)
            cur_state <= S_RESET;
        else
            case(cur_state)
            S_RESET:
                cur_state <= S_START;
            S_START:
                cur_state <= S_WAIT;
            S_WAIT:
                if (finish)
                    cur_state <= S_FINISH;
            endcase
    end
    
    assign start = (cur_state == S_START);
endmodule


module memory_supervisor (
    clk,
    reset,
    
    program_running,
    
    main_enable_a,
    main_enable_b,
    main_address_a,
    main_address_b,
    main_write_enable_a,
    main_write_enable_b,
    main_in_a,
    main_in_b,
    main_size_a,
    main_size_b,
    main_out_a,
    main_out_b,
            
    hlsd_mem_rd_en,
    hlsd_mem_wr_en,
    hlsd_mem_addr,
    hlsd_mem_waitrequest,
    hlsd_mem_readdata,
    hlsd_mem_writedata,
            
    memory_controller_enable_a,
    memory_controller_enable_b,
    memory_controller_address_a,
    memory_controller_address_b,
    memory_controller_write_enable_a,
    memory_controller_write_enable_b,
    memory_controller_in_a,
    memory_controller_in_b,
    memory_controller_size_a,
    memory_controller_size_b,
    memory_controller_out_a,
    memory_controller_out_b
);
    function integer log2;
        input [31:0] value;
        for (log2=0; value>0; log2=log2+1)
        value = value>>1;
    endfunction

    parameter OPTION_MEMORY_READ_AT_TRIGGER = 1;
    parameter OPTION_MEMORY_READ_THEN_RESUME = 1;
    
    parameter MEMORY_CONTROLLER_ADDR_SIZE = 32;
    parameter MEMORY_CONTROLLER_DATA_SIZE = 64;

    input clk;
    input reset;
    input program_running;
    input main_enable_a;
    input main_enable_b;
    input [MEMORY_CONTROLLER_ADDR_SIZE-1:0] main_address_a;
    input [MEMORY_CONTROLLER_ADDR_SIZE-1:0] main_address_b;
    input main_write_enable_a;
    input main_write_enable_b;
    input [MEMORY_CONTROLLER_DATA_SIZE-1:0] main_in_a;
    input [MEMORY_CONTROLLER_DATA_SIZE-1:0] main_in_b;
    input [1:0] main_size_a;
    input [1:0] main_size_b;
    output reg [MEMORY_CONTROLLER_DATA_SIZE-1:0] main_out_a;
    output reg [MEMORY_CONTROLLER_DATA_SIZE-1:0] main_out_b;
            
    input hlsd_mem_rd_en;
    input hlsd_mem_wr_en;
    input [MEMORY_CONTROLLER_ADDR_SIZE-1:0] hlsd_mem_addr;
    output reg hlsd_mem_waitrequest;
    output reg [MEMORY_CONTROLLER_DATA_SIZE-1:0] hlsd_mem_readdata;
    input [MEMORY_CONTROLLER_DATA_SIZE-1:0] hlsd_mem_writedata;
            
    output reg memory_controller_enable_a;
    output reg memory_controller_enable_b;
    output reg [MEMORY_CONTROLLER_ADDR_SIZE-1:0] memory_controller_address_a;
    output reg [MEMORY_CONTROLLER_ADDR_SIZE-1:0] memory_controller_address_b;
    output reg memory_controller_write_enable_a;
    output reg memory_controller_write_enable_b;
    output reg [MEMORY_CONTROLLER_DATA_SIZE-1:0] memory_controller_in_a;
    output reg [MEMORY_CONTROLLER_DATA_SIZE-1:0] memory_controller_in_b;
    output reg [1:0] memory_controller_size_a;
    output reg [1:0] memory_controller_size_b;
    input [MEMORY_CONTROLLER_DATA_SIZE-1:0] memory_controller_out_a;
    input [MEMORY_CONTROLLER_DATA_SIZE-1:0] memory_controller_out_b;

    localparam STATES = 9;
    localparam STATE_BITS = log2(STATES-1);
    parameter [STATE_BITS-1:0]
        S_STOPPED = 0,
        S_READ_REQUEST = 1,
        S_STALL = 2,
        S_READ_DONE = 3,
        S_STARTING_1 = 4,
        S_STARTING_2 = 5,
        S_RUNNING = 6,
        S_WRITE_REQUEST = 7,
        S_WRITE_DONE = 8;
    
    /* Assignments */
    
    /* Combinationals */
    reg hlsd_controls_mem_c;
    
    /* Registers */
    reg program_running_r1;
    reg program_running_r2;
    
    reg [STATE_BITS-1:0] state;
    reg [MEMORY_CONTROLLER_ADDR_SIZE-1:0] last_read_addr_a;
    reg [1:0] last_read_size_a;
    
    reg [MEMORY_CONTROLLER_DATA_SIZE-1:0] last_read_data_a1;
    reg [MEMORY_CONTROLLER_DATA_SIZE-1:0] last_read_data_a2;
    reg [MEMORY_CONTROLLER_DATA_SIZE-1:0] last_read_data_b1;
    reg [MEMORY_CONTROLLER_DATA_SIZE-1:0] last_read_data_b2;
    

    /* State machine */
    always @ (posedge clk) begin
        if (reset)
            state <= S_STOPPED;
        else
            case(state)
                /* Steady-states */
                S_STOPPED: begin
                    if (program_running) begin
						if (OPTION_MEMORY_READ_THEN_RESUME)
                        	state <= S_STARTING_1;
						else
							state <= S_RUNNING;
                    end else if (hlsd_mem_rd_en)
                        state <= S_READ_REQUEST;
                    else if (hlsd_mem_wr_en)
                        state <= S_WRITE_REQUEST;
                end
                
 				/* Resuming program execution */
                S_STARTING_1:
                    state <= S_RUNNING;

				S_RUNNING:
                    if (~program_running)
                        state <= S_STOPPED;
                                
                     /* HLSD performing a memory write */
                
                /* HLSD performing a memory read */
                S_READ_REQUEST:                 
                    state <= S_STALL;
                S_STALL:                    
                    state <= S_READ_DONE;
                S_READ_DONE:
                    state <= S_STOPPED;
                    
                /* HLSD performing a memory write */
                S_WRITE_REQUEST:
                    state <= S_WRITE_DONE;
                S_WRITE_DONE:
                    state <= S_STOPPED;
                  
            endcase
    end
    
    /* hlsd_controls_mem_c */
    always @ (*) begin
        if ((OPTION_MEMORY_READ_AT_TRIGGER == 0) && (OPTION_MEMORY_READ_THEN_RESUME == 0)) 
            hlsd_controls_mem_c <= 1'b0;
        //else if ((state == S_STOPPED && ~program_running) || state == S_READ_REQUEST || state == S_STALL || state == S_READ_DONE || state == S_WRITE_REQUEST || state == S_WRITE_DONE)
        else if (~program_running)
            hlsd_controls_mem_c <= 1'b1;
        else 
            hlsd_controls_mem_c <= 1'b0;
    end
    
    /* Memory data out */

    generate
        if (OPTION_MEMORY_READ_THEN_RESUME) begin
            // If we support pausing and resuming, then the memory
            // output fed into main needs to replay the previous 
            // read on resume.

            always @ (posedge clk) begin
                program_running_r1 <= program_running;
                program_running_r2 <= program_running_r1;
            end
            
            /* last_read_data_* */
            always @ (posedge clk) begin
                if (program_running_r2) begin
                    last_read_data_a2 <= memory_controller_out_a;
                    last_read_data_b2 <= memory_controller_out_b;           
                    last_read_data_a1 <= last_read_data_a2;
                    last_read_data_b1 <= last_read_data_b2;
                end 
            end
            
            always @ (*) begin
                if (state == S_STOPPED && program_running) begin        
                    main_out_a = last_read_data_a1;
                    main_out_b = last_read_data_b1;
                end
                else if (state == S_STARTING_1) begin
                    main_out_a = last_read_data_a2;
                    main_out_b = last_read_data_b2;
                end 
                else if (state == S_RUNNING) begin
                    main_out_a = memory_controller_out_a;
                    main_out_b = memory_controller_out_b;
                end
                else begin
                    main_out_a = {MEMORY_CONTROLLER_DATA_SIZE{1'bx}};
                    main_out_b = {MEMORY_CONTROLLER_DATA_SIZE{1'bx}};
                end
            end
        end else begin
            always @ (*) begin
                main_out_a = memory_controller_out_a;
                main_out_b = memory_controller_out_b;
            end
        end
    endgenerate
    
    /* Outputs */
    always @ (*) begin
        if (~hlsd_controls_mem_c)
        begin
            memory_controller_enable_a = main_enable_a;
            memory_controller_enable_b = main_enable_b;
            memory_controller_address_a = main_address_a;
            memory_controller_address_b = main_address_b;
            memory_controller_write_enable_a = main_write_enable_a;
            memory_controller_write_enable_b = main_write_enable_b;
            memory_controller_in_a = main_in_a;
            memory_controller_in_b = main_in_b;
            memory_controller_size_a = main_size_a;
            memory_controller_size_b = main_size_b;
            hlsd_mem_waitrequest = hlsd_mem_rd_en | hlsd_mem_wr_en;
            hlsd_mem_readdata = 0;
        end
        else
        begin
            memory_controller_enable_a = 1'b0;
            memory_controller_enable_b = 1'b0;
            memory_controller_address_a = {MEMORY_CONTROLLER_ADDR_SIZE{1'bx}};
            memory_controller_address_b = {MEMORY_CONTROLLER_ADDR_SIZE{1'bx}};
            memory_controller_write_enable_a = 1'b0;
            memory_controller_write_enable_b = 1'b0;
            memory_controller_in_a = {MEMORY_CONTROLLER_DATA_SIZE{1'bx}};
            memory_controller_in_b = {MEMORY_CONTROLLER_DATA_SIZE{1'bx}};
            memory_controller_size_a = 2'bx;
            memory_controller_size_b = 2'bx;
            hlsd_mem_readdata = memory_controller_out_a;
            hlsd_mem_waitrequest = 1'b1;
    
            case(state)
                S_READ_REQUEST:
                begin
                    memory_controller_enable_a = 1'b1;
                    memory_controller_address_a = hlsd_mem_addr;
                end         
                S_READ_DONE:
                begin
                    hlsd_mem_waitrequest = 1'b0;
                end
                S_WRITE_REQUEST:
                begin
                    memory_controller_enable_a = 1'b1;
                    memory_controller_write_enable_a = 1'b1;
                    memory_controller_address_a = hlsd_mem_addr;
                    memory_controller_in_a = hlsd_mem_writedata;
                end
                S_WRITE_DONE:
                begin
                    hlsd_mem_waitrequest = 1'b0;
                end
                default: begin
                end
            endcase
        end
    end
endmodule


