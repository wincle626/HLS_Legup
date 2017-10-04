module comm
(
    clk,
    reset,
    
    systemID,
    
    uart_read,
    uart_readdata,
    uart_readdone,
    uart_dataavail,
    uart_write,
    uart_writedata,
    uart_writespace,

    pc_module,
    pc_state,

    dbg_state,
    
    program_run_req,
    program_running,
    program_reset_req,
	external_start,
    
    mem_rd_en,
    mem_wr_en,
    mem_waitrequest,
    mem_addr,
    mem_readdata,
    mem_writedata,
    
    readback_req,
    readback_ack,
    readback_done,
    
    readback_control,
    readback_data,
    readback_regs,
    
    trigger_activate,
    trigger_deactivate,
    trigger_config_module,
    trigger_config_state,
    trigger_config_cond_en,
    trigger_config_cond_addr,
    trigger_config_cond_val,
    trigger_config_cond_opcode,
    trigger_config_cond_index,
    trigger_config_cond_and_not_or_en,
    trigger_config_cond_and_not_or,
    trigger_hit,

    trace_en_var_wr_en,
    trace_en_var_tag,
    trace_en_var_data,

    trace_en_module_wr_en,
    trace_en_module_id,
    trace_en_module_data

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

    parameter OPTION_SUPPORT_WRITES = 0;
    parameter OPTION_MEMORY_READ_AT_TRIGGER = 0;
    parameter OPTION_BREAKPOINT_NUM_CONDITIONS = 0;
    parameter OPTION_REALTIME_ENABLE_TRACE_MODULE = 0;
    parameter OPTION_REALTIME_ENABLE_TRACE_VARIABLE = 0;
    
    parameter MEMORY_DATA_BITS = -1;
    parameter MEMORY_ADDR_BITS = -1;

    parameter PC_MODULE_BITS = -1;
    parameter PC_STATE_BITS = -1;  
    
    parameter BREAKPOINT_NUM_CONDITIONS = -1;
    
    parameter READBACK_CONTROL_BITS = -1;
    parameter READBACK_DATA_BITS = -1;
    parameter READBACK_REGS_BITS = -1;
    
    localparam MEMORY_DATA_BYTES = MEMORY_DATA_BITS / 8;
    localparam MEMORY_ADDR_BYTES = MEMORY_ADDR_BITS / 8;
    
    localparam PC_MODULE_BYTES = ((PC_MODULE_BITS + 7) / 8);    // Divide, round up
    localparam PC_STATE_BYTES = ((PC_STATE_BITS + 7) / 8);  // Divide, round up         
    
    
    localparam UART_MAX_BITS1 = 100;
    localparam UART_MAX_BITS2 = READBACK_CONTROL_BITS > UART_MAX_BITS1 ? READBACK_CONTROL_BITS : UART_MAX_BITS1;
    localparam UART_MAX_BITS3 = READBACK_DATA_BITS > UART_MAX_BITS2 ? READBACK_DATA_BITS : UART_MAX_BITS2;
    localparam UART_MAX_BITS4 = READBACK_REGS_BITS > UART_MAX_BITS3 ? READBACK_REGS_BITS : UART_MAX_BITS3;
    
    localparam UART_MAX_BYTES = (UART_MAX_BITS4 + 7) / 8;
    localparam UART_MAX_BITS = UART_MAX_BYTES * 8;
    
    localparam UART_BYTE_IDX_BITS = log2(UART_MAX_BYTES);
    
    localparam BREAKPOINT_CONDITION_BITS = log2(BREAKPOINT_NUM_CONDITIONS);
    
    localparam READBACK_CONTROL_BYTES = (READBACK_CONTROL_BITS + 7) / 8;    
    localparam READBACK_DATA_BYTES = (READBACK_DATA_BITS + 7) / 8;
    localparam READBACK_REGS_BYTES = (READBACK_REGS_BITS + 7) / 8;
    
    localparam MEMORY_DATA_BYTE_IDX_BITS = log2(MEMORY_DATA_BYTES);
    
    input                                   clk;      
    input                                   reset;    
        
    input   [31:0]                          systemID;
        
    output                                  uart_read;
    input   [7:0]                           uart_readdata;
    input                                   uart_readdone;
    input                                   uart_dataavail;
    output                                  uart_write;
    output  [7:0]                           uart_writedata;
    input                                   uart_writespace;      
    
    output  [7:0]                           dbg_state;
    
    input   [PC_MODULE_BITS-1:0]            pc_module;
    input   [PC_STATE_BITS-1:0]             pc_state;
        
    output                                  program_run_req;
    input                                   program_running;
    output                                  program_reset_req;
	input									external_start;
        
    output                                  mem_rd_en;
    output                                  mem_wr_en;
    input                                   mem_waitrequest;
    output  [MEMORY_ADDR_BITS-1:0]          mem_addr;
    input   [MEMORY_DATA_BITS-1:0]          mem_readdata;
    output  [MEMORY_DATA_BITS-1:0]          mem_writedata;
        
    output                                  readback_req;
    input                                   readback_ack;
    input                                   readback_done;
    
    input   [READBACK_CONTROL_BITS-1:0]     readback_control;
    input   [READBACK_DATA_BITS-1:0]        readback_data;
    input   [READBACK_REGS_BITS-1:0]        readback_regs;
    
    output                                  trigger_activate;
    output                                  trigger_deactivate;
    output  [PC_MODULE_BITS-1:0]            trigger_config_module;
    output  [PC_STATE_BITS-1:0]             trigger_config_state;
    output                                  trigger_config_cond_en;
    output  [MEMORY_ADDR_BITS-1:0]          trigger_config_cond_addr;
    output  [MEMORY_DATA_BITS-1:0]          trigger_config_cond_val;
    output  [2:0]                           trigger_config_cond_opcode;
    output  [BREAKPOINT_CONDITION_BITS-1:0] trigger_config_cond_index;
    output                                  trigger_config_cond_and_not_or_en;
    output                                  trigger_config_cond_and_not_or;
    input                                   trigger_hit;


    output                                  trace_en_var_wr_en;
    output  [8:0]                           trace_en_var_tag;
    output                                  trace_en_var_data;

    output                                  trace_en_module_wr_en;
    output 	[PC_MODULE_BITS-1:0]            trace_en_module_id;
    output                                  trace_en_module_data;
    
    // Current limitations
    // pc_module <= 16 bits
    // pc_state <= 16 bits
    // Run cycles <= 8 bits
    
    // Potential Optimizations:
    // - Maintain local copy of write space
    
    
    
    
    localparam [7:0]
        MSG_RUN = 1, 
        MSG_RESET = 2, 
        MSG_RUN_CYCLES = 3, 
        MSG_PC_REQ = 4, 
        MSG_PC_DATA = 5, 
        MSG_SET_BREAKPOINT = 6, 
        MSG_CLR_BREAKPOINT = 7,
        MSG_READ_MEM_REQ = 8, 
        MSG_READ_MEM_DATA = 9,
        MSG_REFRESH_NOTICE = 10, 
        MSG_READBACK_REQ = 11, 
        MSG_READBACK_CONTROL = 12, 
        MSG_READBACK_DATA = 13, 
        MSG_READBACK_REGS = 14, 
        MSG_READBACK_DONE = 15,
        MSG_SYSTEM_ID_REQ = 16,
        MSG_SYSTEM_ID_DATA = 17,
        MSG_WRITE_MEM_REQ = 18,
        MSG_TRACE_VAR_ENABLE = 19,
        MSG_TRACE_VAR_DISABLE = 20,
        MSG_TRACE_MODULE_ENABLE = 21,
        MSG_TRACE_MODULE_DISABLE = 22;
    
    localparam STATES = 100;
    localparam STATE_BITS = log2(STATES-1);
    localparam [(STATE_BITS-1):0] 
        S_START = 0, 
        S_LISTENING = 1, 
        S_PROCESS_MSG = 2, 
        S_INVALID_MSG = 3,         
        S_HOLD = 4, 
                
        S_UART_RECV = 5,
        S_UART_RECV_CHECK = 6,
        S_UART_WAIT_FOR_DATA_AVAIL = 7,
        S_UART_READ_BYTE = 8,
        S_UART_WAIT_FOR_READ = 9,
        S_UART_READ_BYTE_DONE = 10,
        S_UART_SEND = 11,
        S_UART_SEND_CHECK = 12,
        S_UART_WAIT_FOR_WRITESPACE = 13,
        S_UART_WRITE_BYTE = 14,
        S_UART_WRITE_BYTE_DONE = 15,

        S_MSG_RUN = 22, 
        S_MSG_RESET = 23,
        
        S_MSG_PC_REQ = 24,
        S_PC_SEND_MODULE_ID = 25,
        S_PC_SEND_STATE = 26,
        
        S_MSG_RUN_FOR_CYCLES = 27,
        S_MSG_RUN_FOR_CYCLES_SAVE = 28,
        
        S_MSG_SET_BREAKPOINT = 29,
        S_SET_BREAKPOINT_RECV_MODULE = 30,
        S_SET_BREAKPOINT_RECV_STATE = 31, 
        S_SET_BREAKPOINT_UPDATE_PC = 32,
        S_SET_BREAKPOINT_RECV_NUM_CONDITIONS = 33,
        S_SET_BREAKPOINT_SAVE_NUM_CONDITIONS = 34,
        S_SET_BREAKPOINT_RECV_AND_OR = 35,
        S_SET_BREAKPOINT_CONFIG_AND_OR = 36,
        S_SET_BREAKPOINT_RECV_COND_ADDR = 37,
        S_SET_BREAKPOINT_RECV_COND_DATA = 38,
        S_SET_BREAKPOINT_RECV_COND_OPCODE = 39, 
        S_SET_BREAKPOINT_COND_DONE = 40,
        S_CLR_BREAKPOINT = 42,
        
        S_READ_MEM = 43,
        S_READ_MEM_SAVE_SIZE = 44,
        S_READ_MEM_SAVE_ADDR = 45,
        S_READ_MEM_REQUEST_DATA = 46,
        S_READ_MEM_SEND = 47,
        
        S_WRITE_MEM_RECV_ADDR = 48,
        S_WRITE_MEM_RECV_DATA = 49,
        S_WRITE_MEM_PERFORM_WRITE = 50,
        
        S_SEND_REFRESH_NOTICE = 51,
        
        S_MSG_READBACK_REQ = 52,
        S_READBACK_CONTROL_REQ = 53,
        S_READBACK_CONTROL_SEND_MSG = 54,
        S_READBACK_CONTROL_SEND_DATA = 55,
        S_READBACK_CONTROL_DROP_REQ = 56,
        S_READBACK_CONTROL_DONE = 57,
        S_READBACK_DATA_REQ = 58,
        S_READBACK_DATA_SEND_MSG = 59,
        S_READBACK_DATA_SEND_DATA = 60,
        S_READBACK_DATA_DROP_REQ = 61,
        S_READBACK_DATA_DONE = 62,
        S_READBACK_REGS_REQ = 63,
        S_READBACK_REGS_SEND_MSG = 64,
        S_READBACK_REGS_SEND_DATA = 65,
        S_READBACK_REGS_DROP_REQ = 66,
        S_READBACK_REGS_DONE = 67,
        S_READBACK_DONE = 68,
        
        S_MSG_SYSTEM_ID_REQ = 69,

        S_TRACE_VAR_ENABLE_RECV_TAG = 70,
        S_TRACE_VAR_ENABLE_PERFORM_WRITE = 71,

        S_TRACE_VAR_DISABLE_RECV_TAG = 72,
        S_TRACE_VAR_DISABLE_PERFORM_WRITE = 73,

        S_TRACE_MODULE_ENABLE_RECV_TAG = 74,
        S_TRACE_MODULE_ENABLE_PERFORM_WRITE = 75,

        S_TRACE_MODULE_DISABLE_RECV_TAG = 76,
        S_TRACE_MODULE_DISABLE_PERFORM_WRITE = 77;

    
    /* Wires */
    wire [7:0] write_space; 
    wire [7:0] write_pc_byte_data;
    wire [MEMORY_ADDR_BITS-1:0] readback_addr_a;
    wire [MEMORY_ADDR_BITS-1:0] readback_addr_b;
    wire [MEMORY_DATA_BITS-1:0] readback_data_a;
    wire [MEMORY_DATA_BITS-1:0] readback_data_b;
    wire [7:0] readback_addr_data_byte_data_a;
    wire [7:0] readback_addr_data_byte_data_b;
    wire [PC_MODULE_BITS-1:0] readback_module;
    wire [PC_STATE_BITS-1:0] readback_state;
    wire [7:0] readback_pc_byte_data;
    wire readback_valid_a;
    wire [1:0] readback_size_a;
    wire readback_valid_b;
    wire [1:0] readback_size_b;
    wire breakpoint_hit;
        
    /* Combinational signals */
    reg program_reset_req_c;    
    reg program_run_req_c;
    reg [7:0] uart_byte_to_send_c;
    
    /////////////////////////////// REGISTERS ////////////////////////////////
    
    // Uart
    reg [UART_MAX_BITS-1:0] uart_data_received_r;
    reg [UART_MAX_BITS-1:0] uart_data_to_send_r;
    reg [UART_BYTE_IDX_BITS-1:0] uart_bytes_to_transfer_r;
    reg [UART_BYTE_IDX_BITS-1:0] uart_bytes_transfered_r;
    reg [STATE_BITS-1:0] state_after_uart;
    
    // Trigger
    reg [BREAKPOINT_CONDITION_BITS-1:0] trigger_cond_idx_r;
    reg [PC_MODULE_BITS-1:0] trigger_module_r;
    reg [MEMORY_ADDR_BITS-1:0] trigger_addr_r;
    reg [MEMORY_DATA_BITS-1:0] trigger_data_r;
        
    reg [7:0] run_for_cycles_r;
    reg program_running_r;
    reg [(MEMORY_ADDR_BITS-1):0] mem_addr_r;
    
    reg [MEMORY_DATA_BYTE_IDX_BITS-1:0] mem_read_num_bytes_r;      
    
    reg [(MEMORY_DATA_BITS-1):0] mem_readdata_r;
    
    reg breakpoint_stall;

    reg trace_var_data_c;
    reg trace_module_data_c;
    
    
    //reg [PC_STATE_BITS-1:0] breakpoint_state_r;
    
    
    reg send_refresh_notice_r;
    
    reg run_indefinitely;
        
    reg [(STATE_BITS-1):0] state;
    
    reg trace_mode_readback_r;
    
    reg readback_req_r;
    
    reg [BREAKPOINT_CONDITION_BITS-1:0] breakpoint_conditions_expected_r;
    reg [BREAKPOINT_CONDITION_BITS-1:0] breakpoint_conditions_received_r;
    
    /* Output assignments */
    assign uart_write = (state == S_UART_WRITE_BYTE);
    assign uart_read = (state == S_UART_READ_BYTE);
    assign uart_writedata = uart_byte_to_send_c;
    
    assign mem_rd_en = (state == S_READ_MEM_REQUEST_DATA);  
    assign mem_wr_en = (state == S_WRITE_MEM_PERFORM_WRITE);


    //// for trace variable outputs


    assign trace_en_var_wr_en = (state == S_TRACE_VAR_ENABLE_PERFORM_WRITE || state == S_TRACE_VAR_DISABLE_PERFORM_WRITE);
    assign trace_en_var_tag = uart_data_received_r[8:0];
    assign trace_en_var_data = trace_var_data_c;
	assign trace_en_module_wr_en = ( state == S_TRACE_MODULE_ENABLE_PERFORM_WRITE || state == S_TRACE_MODULE_DISABLE_PERFORM_WRITE);
	assign trace_en_module_id = uart_data_received_r[PC_MODULE_BITS-1:0];
	assign trace_en_module_data = trace_module_data_c;


    assign mem_addr = mem_addr_r;
    assign mem_writedata = uart_data_received_r[MEMORY_DATA_BITS-1:0];
    
    assign program_reset_req = program_reset_req_c; 
    assign readback_req = readback_req_r;
    assign program_run_req = program_run_req_c;
    
    assign trigger_config_module = trigger_module_r;
    assign trigger_config_state = uart_data_received_r[PC_STATE_BITS-1:0];
    assign trigger_config_cond_addr = trigger_addr_r;
    assign trigger_config_cond_val = trigger_data_r;
    assign trigger_config_cond_opcode = uart_data_received_r[2:0];
    assign trigger_activate = (state == S_SET_BREAKPOINT_UPDATE_PC);
    assign trigger_deactivate = (state == S_CLR_BREAKPOINT);
    assign trigger_config_cond_en = (state == S_SET_BREAKPOINT_COND_DONE);
    assign trigger_config_cond_index = breakpoint_conditions_received_r;
    assign trigger_config_cond_and_not_or_en = (state == S_SET_BREAKPOINT_CONFIG_AND_OR);
    assign trigger_config_cond_and_not_or = uart_data_received_r[0];
    
    /* Assignments */   
    assign dbg_state = state;
    assign breakpoint_hit = trigger_hit & ~breakpoint_stall;
    
    /* State Machine */
    always @ (posedge clk or posedge reset) begin
        if (reset) 
            state <= S_START;
        else 
            case (state)
                S_START:
                    state <= S_LISTENING;
                
                /* Uart Receive */
                S_UART_RECV:
                    state <= S_UART_RECV_CHECK;                 
                S_UART_RECV_CHECK:
                    if (uart_bytes_transfered_r == uart_bytes_to_transfer_r) 
                        state <= state_after_uart;
                    else 
                        state <= S_UART_WAIT_FOR_DATA_AVAIL;                        
                S_UART_WAIT_FOR_DATA_AVAIL:
                    if (uart_dataavail)                
                        state <= S_UART_READ_BYTE; 
                S_UART_READ_BYTE:
                    state <= S_UART_WAIT_FOR_READ;
                S_UART_WAIT_FOR_READ:
                    if (uart_readdone)
                        state <= S_UART_READ_BYTE_DONE;
                S_UART_READ_BYTE_DONE:
                    state <= S_UART_RECV_CHECK;

                
                /* Uart Send */
                S_UART_SEND:
                    state <= S_UART_SEND_CHECK;
                S_UART_SEND_CHECK:
                    if (uart_bytes_transfered_r == uart_bytes_to_transfer_r)
                        state <= state_after_uart;
                    else    
                        state <= S_UART_WAIT_FOR_WRITESPACE;
                S_UART_WAIT_FOR_WRITESPACE:
                    if (uart_writespace)
                        state <= S_UART_WRITE_BYTE;
                S_UART_WRITE_BYTE:                                
                    state <= S_UART_WRITE_BYTE_DONE;                
                S_UART_WRITE_BYTE_DONE:
                    state <= S_UART_SEND_CHECK;
                
                ////////////////////////////////////////////////////////////
                /* Main message listen/process loop */
                ////////////////////////////////////////////////////////////
                                
                S_LISTENING: 
                    if (send_refresh_notice_r)
                        state <= S_SEND_REFRESH_NOTICE;
                    else if (uart_dataavail) begin
                        state <= S_UART_RECV;
                        uart_bytes_to_transfer_r <= 1;
                        state_after_uart <= S_PROCESS_MSG;
                    end

                S_PROCESS_MSG: begin
                    state <= S_INVALID_MSG;
                    case (uart_data_received_r[7:0])
                        MSG_RUN:
                            state <= S_MSG_RUN;
                        MSG_RESET:                              
                            state <= S_MSG_RESET;
                        MSG_PC_REQ:
                            state <= S_MSG_PC_REQ;          
                        MSG_RUN_CYCLES:
                            state <= S_MSG_RUN_FOR_CYCLES;
                        MSG_SET_BREAKPOINT:
                            state <= S_MSG_SET_BREAKPOINT;
                        MSG_CLR_BREAKPOINT:
                            state <= S_CLR_BREAKPOINT;
                        MSG_READ_MEM_REQ:
                            if (OPTION_MEMORY_READ_AT_TRIGGER)
                                state <= S_READ_MEM;        
                        MSG_WRITE_MEM_REQ:
                            if (OPTION_SUPPORT_WRITES)
                                state <= S_WRITE_MEM_RECV_ADDR;
                        MSG_READBACK_REQ:
                            state <= S_MSG_READBACK_REQ;
                        MSG_SYSTEM_ID_REQ:
                            state <= S_MSG_SYSTEM_ID_REQ;
                        MSG_TRACE_VAR_ENABLE:
                            if (OPTION_REALTIME_ENABLE_TRACE_VARIABLE)
                                state <= S_TRACE_VAR_ENABLE_RECV_TAG;
                        MSG_TRACE_VAR_DISABLE:
                            if (OPTION_REALTIME_ENABLE_TRACE_VARIABLE)
                                state <= S_TRACE_VAR_DISABLE_RECV_TAG;
                        MSG_TRACE_MODULE_ENABLE:
                            if (OPTION_REALTIME_ENABLE_TRACE_MODULE)
                                state <= S_TRACE_MODULE_ENABLE_RECV_TAG;
                        MSG_TRACE_MODULE_DISABLE:
                            if (OPTION_REALTIME_ENABLE_TRACE_MODULE)
                                state <= S_TRACE_MODULE_DISABLE_RECV_TAG;                        
                    endcase
                end
                S_INVALID_MSG:
                    state <= S_HOLD;
                S_MSG_RESET:
                    state <= S_LISTENING;
                S_MSG_RUN:              
                    state <= S_LISTENING; 

        //// code added for trace enable and disable states:
            
                S_TRACE_VAR_ENABLE_RECV_TAG:
                begin
                    state <= S_UART_RECV;
                    uart_bytes_to_transfer_r <= 2;
                    state_after_uart <= S_TRACE_VAR_ENABLE_PERFORM_WRITE;
                end

                S_TRACE_VAR_ENABLE_PERFORM_WRITE:
                begin
                    
                    state <= S_LISTENING;
                end


                S_TRACE_VAR_DISABLE_RECV_TAG:
                begin
                    state <= S_UART_RECV;
                    uart_bytes_to_transfer_r <= 2;
                    state_after_uart <= S_TRACE_VAR_DISABLE_PERFORM_WRITE;
                end


                S_TRACE_VAR_DISABLE_PERFORM_WRITE:
                begin
                    
                    state <= S_LISTENING;
                end


        ///////////****************************************/////////////////////////////////// 
                
                ////////////////////////////////////////////////////////////
                ////////////////////////////////////////////////////////////
                ////////////////////////////////////////////////////////////
                /* 
                Everything below this are handlers for different types of
                messages.  They should only call the high-level Uart states,
                S_UART_SEND and S_UART_RECV.  Upon completion, all message
                handlers should return to S_LISTENING.
                
                S_UART_SEND Template:
                ---------------------
                state <= S_UART_SEND;
                state_after_uart <= ;
                uart_bytes_to_transfer_r <= ;
                uart_data_to_send_r <= ;
                
                S_UART_RECV Template:
                ---------------------
                state <= S_UART_RECV;
                state_after_uart <= ;
                uart_bytes_to_transfer_r <= ;
                
                (Data is stored in uart_data_received_r)
                
                */
                
                /* Message: Send PC */
                S_MSG_PC_REQ:
                begin
                    state <= S_UART_SEND;
                    state_after_uart <= S_PC_SEND_MODULE_ID;
                    uart_bytes_to_transfer_r <= 1;
                    uart_data_to_send_r <= MSG_PC_DATA;
                end
                S_PC_SEND_MODULE_ID:
                begin
                    state <= S_UART_SEND;
                    state_after_uart <= S_PC_SEND_STATE;
                    uart_bytes_to_transfer_r <= PC_MODULE_BYTES[UART_BYTE_IDX_BITS-1:0];
                    uart_data_to_send_r <= {8'b0, pc_module};
                end                
                S_PC_SEND_STATE:
                begin
                    state <= S_UART_SEND;
                    state_after_uart <= S_LISTENING;
                    uart_bytes_to_transfer_r <= PC_STATE_BYTES[UART_BYTE_IDX_BITS-1:0];
                    uart_data_to_send_r <= {8'b0, pc_state};
                end
                
                /* Run for # cycles */
                S_MSG_RUN_FOR_CYCLES:
                begin
                    state <= S_UART_RECV;
                    state_after_uart <= S_MSG_RUN_FOR_CYCLES_SAVE;
                    uart_bytes_to_transfer_r <= 1;                    
                end
                S_MSG_RUN_FOR_CYCLES_SAVE:
                    state <= S_LISTENING;
                    
                //////////////////////// BREAKPOINT ///////////////////////////
                S_MSG_SET_BREAKPOINT:
                begin
                    state <= S_SET_BREAKPOINT_RECV_MODULE;
                end
                
                // Module
                S_SET_BREAKPOINT_RECV_MODULE:
                begin                   
                    state <= S_UART_RECV;
                    uart_bytes_to_transfer_r <= PC_MODULE_BYTES[UART_BYTE_IDX_BITS-1:0];
                    state_after_uart <= S_SET_BREAKPOINT_RECV_STATE;
                end
                
                // State
                S_SET_BREAKPOINT_RECV_STATE:
                begin
                    state <= S_UART_RECV;
                    uart_bytes_to_transfer_r <= PC_STATE_BYTES[UART_BYTE_IDX_BITS-1:0];
                    state_after_uart <= S_SET_BREAKPOINT_UPDATE_PC;
                end
                
                S_SET_BREAKPOINT_UPDATE_PC:
                begin
                    if (OPTION_BREAKPOINT_NUM_CONDITIONS > 0)
                        state <= S_SET_BREAKPOINT_RECV_NUM_CONDITIONS;
                    else
                        state <= S_LISTENING;
                end
                
                // Get Num Conditions
                S_SET_BREAKPOINT_RECV_NUM_CONDITIONS:
                begin
                    state <= S_UART_RECV;
                    uart_bytes_to_transfer_r <= 1;
                    state_after_uart <= S_SET_BREAKPOINT_SAVE_NUM_CONDITIONS;
                end
                S_SET_BREAKPOINT_SAVE_NUM_CONDITIONS:
                begin
                    state <= S_SET_BREAKPOINT_RECV_AND_OR;
                end
                
                // Get AND/OR
                S_SET_BREAKPOINT_RECV_AND_OR:
                begin
                    if (breakpoint_conditions_expected_r == 0) begin
                        state <= S_LISTENING;
                    end
                    else begin
                        state <= S_UART_RECV;
                        uart_bytes_to_transfer_r <= 1;
                        state_after_uart <= S_SET_BREAKPOINT_CONFIG_AND_OR;
                    end
                end
                S_SET_BREAKPOINT_CONFIG_AND_OR:
                begin
                    state <= S_SET_BREAKPOINT_RECV_COND_ADDR;
                end
                
                // Condition                
                S_SET_BREAKPOINT_RECV_COND_ADDR:
                begin
                    state <= S_UART_RECV;
                    uart_bytes_to_transfer_r <= MEMORY_ADDR_BYTES[UART_BYTE_IDX_BITS-1:0];
                    state_after_uart <= S_SET_BREAKPOINT_RECV_COND_DATA;
                end             
                S_SET_BREAKPOINT_RECV_COND_DATA:
                begin
                    state <= S_UART_RECV;
                    uart_bytes_to_transfer_r <= MEMORY_DATA_BYTES[UART_BYTE_IDX_BITS-1:0];
                    state_after_uart <= S_SET_BREAKPOINT_RECV_COND_OPCODE;                  
                end
                S_SET_BREAKPOINT_RECV_COND_OPCODE:
                begin
                    state <= S_UART_RECV;
                    uart_bytes_to_transfer_r <= 1;
                    state_after_uart <= S_SET_BREAKPOINT_COND_DONE; 
                end
                S_SET_BREAKPOINT_COND_DONE:
                begin
                    if (OPTION_BREAKPOINT_NUM_CONDITIONS == 0)
                        state <= S_LISTENING;
                    else if (breakpoint_conditions_received_r == breakpoint_conditions_expected_r) begin
                        state <= S_LISTENING;
                    end
                    else begin
                        state <= S_SET_BREAKPOINT_RECV_COND_ADDR;
                    end
                end
                
                /* Clr breakpoint */
                S_CLR_BREAKPOINT:
                begin
                    state <= S_LISTENING;
                end
                
                //////////////////////// READ MEMORY /////////////////
                S_READ_MEM:
                begin
                    state <= S_UART_RECV;
                    state_after_uart <= S_READ_MEM_SAVE_SIZE;
                    uart_bytes_to_transfer_r <= 1;                    
                end
                S_READ_MEM_SAVE_SIZE:
                begin
                    state <= S_UART_RECV;
                    uart_bytes_to_transfer_r <= MEMORY_ADDR_BYTES[UART_BYTE_IDX_BITS-1:0];
                    state_after_uart <= S_READ_MEM_SAVE_ADDR;
                end
                S_READ_MEM_SAVE_ADDR:
                begin
                    state <= S_READ_MEM_REQUEST_DATA;
                end             
                S_READ_MEM_REQUEST_DATA:
                begin
                    if (mem_waitrequest == 1'b0)
                        state <= S_READ_MEM_SEND;                        
                end
                S_READ_MEM_SEND:
                begin
                    state <= S_UART_SEND;
                    state_after_uart <= S_LISTENING;
                    uart_bytes_to_transfer_r <= mem_read_num_bytes_r + 1'b1;
                    uart_data_to_send_r <= { mem_readdata_r,MSG_READ_MEM_DATA};        
                end
                
                //////////////////////// WRITE MEMORY /////////////////////////                
                S_WRITE_MEM_RECV_ADDR:
                begin
                    state <= S_UART_RECV;
                    uart_bytes_to_transfer_r <= MEMORY_ADDR_BYTES[UART_BYTE_IDX_BITS-1:0];
                    state_after_uart <= S_WRITE_MEM_RECV_DATA;
                end
                S_WRITE_MEM_RECV_DATA:
                begin
                    state <= S_UART_RECV;
                    uart_bytes_to_transfer_r <= MEMORY_DATA_BYTES[UART_BYTE_IDX_BITS-1:0];
                    state_after_uart <= S_WRITE_MEM_PERFORM_WRITE;
                end
                S_WRITE_MEM_PERFORM_WRITE:
                begin
                    if (~mem_waitrequest)
                        state <= S_LISTENING;
                end        
                
                /* Refresh Notice */
                S_SEND_REFRESH_NOTICE:
                begin
                    state <= S_UART_SEND;
                    state_after_uart <= S_LISTENING;
                    uart_bytes_to_transfer_r <= 1;
                    uart_data_to_send_r <= MSG_REFRESH_NOTICE;                    
                end                
    
                ///////////////////////// READBACK ////////////////////////                
                S_MSG_READBACK_REQ: begin
                    state <= S_READBACK_CONTROL_REQ;
                end
                S_READBACK_CONTROL_REQ: begin
                    if (readback_ack) begin
                        state <= S_READBACK_CONTROL_SEND_MSG;
                    end else if (readback_done) begin
                        state <= S_READBACK_CONTROL_DONE;
                    end
                end
                S_READBACK_CONTROL_SEND_MSG: begin
                    state <= S_UART_SEND;
                    uart_bytes_to_transfer_r <= 1;
                    uart_data_to_send_r <= MSG_READBACK_CONTROL;
                    state_after_uart <= S_READBACK_CONTROL_SEND_DATA;
                end
                S_READBACK_CONTROL_SEND_DATA: begin
                    state <= S_UART_SEND;
                    uart_bytes_to_transfer_r <= READBACK_CONTROL_BYTES[UART_BYTE_IDX_BITS-1:0];
                    uart_data_to_send_r <= {8'b0, readback_control};
                    state_after_uart <= S_READBACK_CONTROL_DROP_REQ;
                end
                S_READBACK_CONTROL_DROP_REQ: begin
                    if (~readback_ack)
                        state <= S_READBACK_CONTROL_REQ;
                end
                S_READBACK_CONTROL_DONE: begin
                    state <= S_READBACK_DATA_REQ;
                end   
                
                S_READBACK_DATA_REQ: begin
                    if (readback_ack) begin
                        state <= S_READBACK_DATA_SEND_MSG;
                    end else if (readback_done) begin
                        state <= S_READBACK_DATA_DONE;
                    end
                end
                S_READBACK_DATA_SEND_MSG: begin
                    state <= S_UART_SEND;
                    uart_bytes_to_transfer_r <= 1;
                    uart_data_to_send_r <= MSG_READBACK_DATA;
                    state_after_uart <= S_READBACK_DATA_SEND_DATA;
                end
                S_READBACK_DATA_SEND_DATA: begin
                    state <= S_UART_SEND;
                    uart_bytes_to_transfer_r <= READBACK_DATA_BYTES[UART_BYTE_IDX_BITS-1:0];
                    uart_data_to_send_r <= readback_data;
                    state_after_uart <= S_READBACK_DATA_DROP_REQ;
                end
                S_READBACK_DATA_DROP_REQ: begin
                    if (~readback_ack)
                        state <= S_READBACK_DATA_REQ;
                end
                S_READBACK_DATA_DONE: begin
                    state <= S_READBACK_REGS_REQ;
                end
                
                 S_READBACK_REGS_REQ: begin
                    if (readback_ack) begin
                        state <= S_READBACK_REGS_SEND_MSG;
                    end else if (readback_done) begin
                        state <= S_READBACK_REGS_DONE;
                    end
                end
                S_READBACK_REGS_SEND_MSG: begin
                    state <= S_UART_SEND;
                    uart_bytes_to_transfer_r <= 1;
                    uart_data_to_send_r <= MSG_READBACK_REGS;
                    state_after_uart <= S_READBACK_REGS_SEND_DATA;
                end
                S_READBACK_REGS_SEND_DATA: begin
                    state <= S_UART_SEND;
                    uart_bytes_to_transfer_r <= READBACK_REGS_BYTES[UART_BYTE_IDX_BITS-1:0];
                    uart_data_to_send_r <= readback_regs;
                    state_after_uart <= S_READBACK_REGS_DROP_REQ;
                end
                S_READBACK_REGS_DROP_REQ: begin
                    if (~readback_ack)
                        state <= S_READBACK_REGS_REQ;
                end
                S_READBACK_REGS_DONE: begin
                    state <= S_READBACK_DONE;
                end
                
                S_READBACK_DONE: begin
                    state <= S_UART_SEND;
                    uart_bytes_to_transfer_r <= 1;
                    uart_data_to_send_r <= MSG_READBACK_DONE;
                    state_after_uart <= S_LISTENING;                    
                end

                /* System ID */
                S_MSG_SYSTEM_ID_REQ: begin
                    state <= S_UART_SEND;
                    uart_bytes_to_transfer_r <= 5;
                    uart_data_to_send_r <= {systemID, MSG_SYSTEM_ID_DATA};
                    state_after_uart <= S_LISTENING;
                end
                
             	//// code added for trace module and disable states:

		S_TRACE_MODULE_ENABLE_RECV_TAG:
		begin
			state <= S_UART_RECV;
    			uart_bytes_to_transfer_r <= 2;
    			state_after_uart <= S_TRACE_MODULE_ENABLE_PERFORM_WRITE;
		end

		S_TRACE_MODULE_ENABLE_PERFORM_WRITE:
		begin
					
			state <= S_LISTENING;
		end


		S_TRACE_MODULE_DISABLE_RECV_TAG:
		begin
			state <= S_UART_RECV;
    			uart_bytes_to_transfer_r <= 2;
    			state_after_uart <= S_TRACE_MODULE_DISABLE_PERFORM_WRITE;
		end


		S_TRACE_MODULE_DISABLE_PERFORM_WRITE:
		begin
					
			state <= S_LISTENING;
		end
		  
            endcase             
    end
    
    /* uart_bytes_received */
    always @ (posedge clk) begin
        case(state)
            S_UART_RECV:
                uart_bytes_transfered_r <= 0;
            S_UART_SEND:
                uart_bytes_transfered_r <= 0;
            S_UART_READ_BYTE_DONE:
                uart_bytes_transfered_r <= uart_bytes_transfered_r + 1'b1;
            S_UART_WRITE_BYTE_DONE:
                uart_bytes_transfered_r <= uart_bytes_transfered_r + 1'b1;
        endcase
    end
    
    /* uart_data_received_r
    */
    genvar i;
    generate
        for (i = 0; i < UART_MAX_BYTES; i = i + 1) begin : GEN_UART_RECV_DATA
            always @ (posedge clk) begin
                if ((state == S_UART_WAIT_FOR_READ) && uart_readdone && (uart_bytes_transfered_r == i))
                begin
                    uart_data_received_r[i*8+:8] <= uart_readdata;
                end
            end
        end
    endgenerate
    
    /* uart_byte_to_send_c */
    always @ (*) begin : UART_BYTE_TO_SEND_C_BLOCK
        integer j;
        uart_byte_to_send_c = 8'b0;
        for (j = 0 ; j < UART_MAX_BYTES; j = j + 1)
        begin
            if (uart_bytes_transfered_r == j)
                uart_byte_to_send_c = uart_data_to_send_r[j*8+:8];
        end
    end
    
    /*
    mem_read_num_bytes_r
    */
    always @ (posedge clk) begin
        case (state)
            S_READ_MEM_SAVE_SIZE:
                mem_read_num_bytes_r <= uart_data_received_r[MEMORY_DATA_BYTE_IDX_BITS-1:0];
        endcase
    end
    
    /* trigger_module_r 
        trigger_addr_r
        trigger_data_r
    */
    always @ (posedge clk) begin
        case (state)
            S_SET_BREAKPOINT_RECV_STATE:
                trigger_module_r <= uart_data_received_r[PC_MODULE_BITS-1:0];
            S_SET_BREAKPOINT_RECV_COND_DATA:
                trigger_addr_r <= uart_data_received_r[MEMORY_ADDR_BITS-1:0];
            S_SET_BREAKPOINT_RECV_COND_OPCODE:
                trigger_data_r <= uart_data_received_r[MEMORY_DATA_BITS-1:0];
        endcase
    end
        
    /* breakpoint_conditions_expected_r
        breakpoint_conditions_received_r
    */
    always @ (posedge clk) begin
        case (state)
            S_SET_BREAKPOINT_SAVE_NUM_CONDITIONS:
            begin
                breakpoint_conditions_received_r <= 0;
                breakpoint_conditions_expected_r <= uart_data_received_r[BREAKPOINT_CONDITION_BITS-1:0];
            end
            S_SET_BREAKPOINT_COND_DONE:
                breakpoint_conditions_received_r <= breakpoint_conditions_received_r + 1'b1;
        endcase
    end
        
    /* readback_req_r */
    always @ (posedge clk) begin
        case(state)
            S_READBACK_CONTROL_REQ:
                readback_req_r <= 1'b1;
            S_READBACK_CONTROL_DROP_REQ:
                readback_req_r <= 1'b0;
            S_READBACK_CONTROL_DONE:
                readback_req_r <= 1'b0;

            S_READBACK_DATA_REQ:
                readback_req_r <= 1'b1;
            S_READBACK_DATA_DROP_REQ:
                readback_req_r <= 1'b0;
            S_READBACK_DATA_DONE:
                readback_req_r <= 1'b0;

            S_READBACK_REGS_REQ:
                readback_req_r <= 1'b1;
            S_READBACK_REGS_DROP_REQ:
                readback_req_r <= 1'b0;
            S_READBACK_REGS_DONE:
                readback_req_r <= 1'b0;
        endcase
    end
        


    /* run_indefinitely */
    always @ (posedge clk or posedge reset) begin
        if (reset)
            run_indefinitely <= 1'b0;
        else begin
            if (breakpoint_hit) begin
                run_indefinitely <= 1'b0;
            end else begin
                case(state)
                S_MSG_RUN:
                    run_indefinitely <= 1'b1;
                S_MSG_RESET:
                    run_indefinitely <= 1'b0;           
                endcase
				if (external_start) begin
					run_indefinitely <= 1'b1;
                end
            end
        end
    end
    
    /* run_for_cycles_r */
    always @ (posedge clk or posedge reset) begin
        if (reset)
            run_for_cycles_r <= 8'b0;
        else
            case (state)
            S_MSG_RESET:
                run_for_cycles_r <= 8'b0;
            S_MSG_RUN_FOR_CYCLES_SAVE:
                run_for_cycles_r <= uart_data_received_r[7:0];
            default:
            begin
                if ((run_for_cycles_r > 8'b0) && program_running)
                    run_for_cycles_r <= (run_for_cycles_r - 8'b1);
                if (breakpoint_hit)
                    run_for_cycles_r <= 8'b0;
            end
            endcase
    end

    /* User-circuit reset */
    always @ (*) begin
        program_reset_req_c <= 1'b0;
        if (state == S_MSG_RESET)
            program_reset_req_c <= 1'b1;
    end 
     
    
    /* Memory address capture */
    always @ (posedge clk) begin
        case(state)
            S_READ_MEM_SAVE_ADDR:
                mem_addr_r <= uart_data_received_r[(MEMORY_ADDR_BITS-1):0];
            S_WRITE_MEM_RECV_DATA:
                mem_addr_r <= uart_data_received_r[(MEMORY_ADDR_BITS-1):0];
        endcase
    end
    
    /* Data capture */
    always @ (posedge clk) begin
        case(state)         
            S_READ_MEM_REQUEST_DATA:
                if (mem_waitrequest == 1'b0)
                    mem_readdata_r <= mem_readdata;
        endcase
    end
    
    // breakpoint stall, temporarily disables the 
    // breakpoint so that the user circuit can restart
    // otherwise it would forever be stuck on the breakpoint
    always @ (posedge clk ) begin
        if (reset)
            breakpoint_stall <= 1'b0;
        else
            if (breakpoint_hit)
                breakpoint_stall <= 1'b1;
            else if (program_running)
                breakpoint_stall <= 1'b0;
    end
    
	///// for trace var data
	always @ (*) begin
		case (state)
			S_TRACE_VAR_ENABLE_PERFORM_WRITE:
				trace_var_data_c = 1'b1;
			S_TRACE_VAR_DISABLE_PERFORM_WRITE:
				trace_var_data_c = 1'b0;
            default:
                trace_var_data_c = 1'bX;
		endcase
	end

       ///// for trace module data
	always @ (*) begin
		case (state)
			S_TRACE_MODULE_ENABLE_PERFORM_WRITE:
				trace_module_data_c = 1'b1;
			S_TRACE_MODULE_DISABLE_PERFORM_WRITE:
				trace_module_data_c = 1'b0;
			default:
				trace_module_data_c = 1'bX;
		endcase
	end
    

    /* User clock enable */
    always @ (*) begin
        program_run_req_c = 1'b0;
        if (program_reset_req_c)
            /* User circuit uses synchronous reset, so we need to 
                enable the clock during a reset */
            program_run_req_c = 1'b0;
        else if (breakpoint_hit)
            program_run_req_c = 1'b0;
        else if (run_indefinitely)
            program_run_req_c = 1'b1;
            
            // If there are cycles to be run, then request to run,
            // however, if we have started running, and are at 1 cycle remaining, 
            // lower the request, since there is a 1 cycle pipeline delay.
        else if (run_for_cycles_r == 1) 
            program_run_req_c = 1'b1;           
        else if (run_for_cycles_r > 0)
            program_run_req_c = 1'b1;
    end
    
    /* program_running_r */
    always @ (posedge clk) begin        
        program_running_r = program_running;
    end
    
    /* send_refresh_notice_r */
    always @ (posedge clk) begin
        if (reset)
            send_refresh_notice_r <= 1'b0;
        else begin
            // User circuit just stopped, signal a refresh
            if (program_running == 1'b0 && program_running_r == 1'b1)
                send_refresh_notice_r <= 1'b1;
                
            // Refresh signaled, deassert
            else if (state == S_SEND_REFRESH_NOTICE)
                send_refresh_notice_r <= 1'b0;
        end
    end
        
  
endmodule

