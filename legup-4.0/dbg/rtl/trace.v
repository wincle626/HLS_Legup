module trace (
    clk,
    reset,
    
    pc_module_r1,
    pc_state_r1,
    
    program_running,
    program_running_r1,
    
    readback_req,
    readback_ack,
    readback_done,
    
    main_wr_en_a,
    main_addr_a,
    main_size_a,    
    main_data_a,
    
    main_wr_en_b,
    main_addr_b,
    main_size_b,
    main_data_b,
    
    regs_muxed_a,
    regs_muxed_b,
    regs_valid_a,
    regs_valid_b,

    readback_control,
    readback_data,
    readback_regs,

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

    parameter OPTION_TRACE_REGS = 0;
    parameter OPTION_REALTIME_ENABLE_TRACE_MODULE = 0;
    
    // Not supported right now
    parameter OPTION_REALTIME_ENABLE_TRACE_VARIABLE = 0;
    localparam OPTION_REALTIME_ENABLE_TRACE = OPTION_REALTIME_ENABLE_TRACE_MODULE || OPTION_REALTIME_ENABLE_TRACE_VARIABLE;
    
    parameter MEMORY_DATA_WIDTH = -1;
    parameter MEMORY_ADDR_WIDTH = -1;
    
    parameter PC_MODULE_BITS = -1;
    parameter PC_STATE_BITS = -1;
    
    parameter CONTROL_BUFFER_WIDTH = -1;
    parameter CONTROL_BUFFER_DEPTH = -1;
    parameter DATA_BUFFER_WIDTH = -1;
    parameter DATA_BUFFER_DEPTH = -1;
   
    parameter REGS_BUFFER_WIDTH = -1;
    parameter REGS_BUFFER_DEPTH = -1;
    
    parameter SEQUENCE_BITS = -1; 

    localparam CONTROL_BUFFER_ADDR_BITS = log2(CONTROL_BUFFER_DEPTH-1);
    localparam DATA_BUFFER_ADDR_BITS = log2(DATA_BUFFER_DEPTH-1);
    localparam REGS_BUFFER_ADDR_BITS = log2(REGS_BUFFER_DEPTH-1);

    
    localparam STATES = 23;
    localparam STATE_BITS = log2(STATES-1);
    localparam [STATE_BITS-1:0] 
        S_FILL = 0, 
        S_CONTROL_START = 1,
        S_CONTROL_WAIT_FOR_REQ = 2, 
        S_CONTROL_READ = 3, 
        S_CONTROL_WAIT_FOR_READ = 4, 
        S_CONTROL_ACK = 5,
        S_CONTROL_READ_DONE = 6,
        S_CONTROL_DONE = 7,
        S_DATA_START = 8,
        S_DATA_WAIT_FOR_REQ = 9,
        S_DATA_READ = 10,
        S_DATA_WAIT_FOR_READ = 11,
        S_DATA_ACK = 12,
        S_DATA_READ_DONE = 13,
        S_DATA_DONE = 14,
        S_REGS_START = 15,
        S_REGS_WAIT_FOR_REQ = 16,
        S_REGS_READ = 17,
        S_REGS_WAIT_FOR_READ = 18,
        S_REGS_ACK = 19,
        S_REGS_READ_DONE = 20,
        S_REGS_DONE = 21,
        S_READBACK_DONE = 22;

    input                                       clk;
    input                                       reset;
            
    input   [PC_MODULE_BITS-1:0]                pc_module_r1;
    input   [PC_STATE_BITS-1:0]                 pc_state_r1;
            
    input                                       program_running;
    input                                       program_running_r1;
            
    input                                       readback_req;
    output                                      readback_ack;
    output                                      readback_done;
            
    input                                       main_wr_en_a;
    input   [MEMORY_ADDR_WIDTH-1:0]             main_addr_a;
    input   [1:0]                               main_size_a;
    input   [MEMORY_DATA_WIDTH-1:0]             main_data_a;
            
    input                                       main_wr_en_b;
    input   [MEMORY_ADDR_WIDTH-1:0]             main_addr_b;
    input   [1:0]                               main_size_b;
    input   [MEMORY_DATA_WIDTH-1:0]             main_data_b;

    input   [REGS_BUFFER_WIDTH-1:0]             regs_muxed_a;
    input   [REGS_BUFFER_WIDTH-1:0]             regs_muxed_b;
    input                                       regs_valid_a;
    input                                       regs_valid_b;

    input                                       trace_en_var_wr_en;
    input   [8:0]                               trace_en_var_tag;
    input                                       trace_en_var_data;

    input                                       trace_en_module_wr_en;
    input   [PC_MODULE_BITS-1:0]                trace_en_module_id;
    input                                       trace_en_module_data;
    
        
    output [CONTROL_BUFFER_WIDTH-1:0]           readback_control;
    output [DATA_BUFFER_WIDTH-1:0]              readback_data;
    output [REGS_BUFFER_WIDTH-1:0]              readback_regs;
          
    
    /* Registers */ 
    reg     [STATE_BITS-1:0]                    state;
        
    reg     [CONTROL_BUFFER_ADDR_BITS-1:0]      control_addr_r;
    reg                                         control_filled_r;
    reg     [SEQUENCE_BITS-1:0]                 sequence_cnt_r;
    reg     [PC_STATE_BITS-1:0]                 pc_state_sequence_start_r;

    reg     [DATA_BUFFER_ADDR_BITS-1:0]         data_addr_a_r;    
    reg                                         data_filled_r;
    reg     [1:0]                               data_lines_written_r;
    
    reg     [REGS_BUFFER_ADDR_BITS-1:0]         regs_addr_a_r;
    reg                                         regs_filled_r;
        
    //reg     [MAX_BUFFER_ADDR_BITS-1:0]          readback_remaining_r;
    reg     [CONTROL_BUFFER_ADDR_BITS-1:0]      control_readback_remaining_r;
    reg     [DATA_BUFFER_ADDR_BITS-1:0]         data_readback_remaining_r;
    reg     [REGS_BUFFER_ADDR_BITS-1:0]         regs_readback_remaining_r;

    reg     [PC_MODULE_BITS-1:0]                pc_module_r;
    reg     [PC_STATE_BITS-1:0]                 pc_state_r;
    
    
     
     /* Combinational Signals */        
    reg     [CONTROL_BUFFER_ADDR_BITS-1:0]      control_addr_c;
    reg     [CONTROL_BUFFER_ADDR_BITS-1:0]      control_addr_c_plus1_c;
    reg     [CONTROL_BUFFER_ADDR_BITS-1:0]      control_addr_r_plus1_c;
    reg     [CONTROL_BUFFER_ADDR_BITS-1:0]      control_addr_r_minus1_c;
    reg     [CONTROL_BUFFER_WIDTH-1:0]          control_writedata_c;
    reg                                         control_is_sequential_c;
    reg                                         control_rewrite_c;
    
    reg     [DATA_BUFFER_ADDR_BITS-1:0]         data_addr_a_c;
    reg     [DATA_BUFFER_ADDR_BITS-1:0]         data_addr_b_c;
    reg     [DATA_BUFFER_ADDR_BITS-1:0]         data_addr_a_r_minus1_c;
    reg     [DATA_BUFFER_ADDR_BITS-1:0]         data_addr_a_r_minus2_c;
    reg     [DATA_BUFFER_ADDR_BITS-1:0]         data_addr_a_c_plus1_c;
    reg     [DATA_BUFFER_ADDR_BITS-1:0]         data_addr_a_c_plus2_c;
    reg     [DATA_BUFFER_WIDTH-1:0]             data_writedata_a_c;
    reg     [DATA_BUFFER_WIDTH-1:0]             data_writedata_b_c;
    reg                                         data_wr_en_a_c;
    reg                                         data_wr_en_b_c;
    reg     [1:0]                               data_lines_written_c;
    
    reg     [REGS_BUFFER_ADDR_BITS-1:0]         regs_addr_b_c;
    reg     [REGS_BUFFER_ADDR_BITS-1:0]         regs_addr_a_r_plus1_c;
    reg     [REGS_BUFFER_ADDR_BITS-1:0]         regs_addr_a_r_plus2_c;
    reg     [REGS_BUFFER_WIDTH-1:0]             regs_writedata_c;
    
    reg                                         trace_module_mem_q_r;   

    reg     [8:0]                               trace_en_var_addr_a_c;    
    reg     [PC_MODULE_BITS-1:0]                trace_en_module_addr;
     
    reg     [PC_MODULE_BITS-1:0]                pc_module_last;
    reg     [PC_STATE_BITS-1:0]                 pc_state_last;     

     /* Wires */
    wire                                        control_wr_en;
    wire    [CONTROL_BUFFER_WIDTH-1:0]          control_readdata;
    
    wire    [DATA_BUFFER_WIDTH-1:0]             data_readdata;
    
    
    wire    [REGS_BUFFER_WIDTH-1:0]             regs_readdata;
    
    wire    [8:0]                               trace_en_var_addr_b;
    wire                                        trace_en_var_out_a;
    wire                                        trace_en_var_out_b;
    wire                                        trace_en_module_out;

    wire                                        regs_wr_en_a;
    wire                                        regs_wr_en_b;
     

    always @ (posedge clk) begin
        if (program_running_r1) begin
            pc_module_last <= pc_module_r1;
            pc_state_last <= pc_state_r1;
        end
    end 
    
    /* State machine */
    always @ (posedge clk) begin
        if (reset) begin
            state <= S_FILL;
        end
        else begin
            case(state)
                S_FILL: begin
                    if (readback_req == 1'b1) begin
                        state <= S_CONTROL_START;
                    end
                end
                S_CONTROL_START: begin
                    state <= S_CONTROL_WAIT_FOR_REQ;
                end
                S_CONTROL_WAIT_FOR_REQ: begin                   
                    if (readback_req) begin
                        if (control_readback_remaining_r == 0) begin
                            state <= S_CONTROL_DONE;
                        end 
                        else begin
                            state <= S_CONTROL_READ;    
                        end
                    end
                end
                S_CONTROL_READ: begin
                    state <= S_CONTROL_WAIT_FOR_READ;
                end
                S_CONTROL_WAIT_FOR_READ: begin
                    state <= S_CONTROL_ACK;
                end
                S_CONTROL_ACK: begin                    
                    if (~readback_req) begin
                        state <= S_CONTROL_READ_DONE;
                    end
                end
                S_CONTROL_READ_DONE: begin
                    state <= S_CONTROL_WAIT_FOR_REQ;
                end
                S_CONTROL_DONE: begin
                    state <= S_DATA_START;
                end
                
                S_DATA_START: begin
                    state <= S_DATA_WAIT_FOR_REQ;
                end
                S_DATA_WAIT_FOR_REQ: begin
                    if (readback_req) begin
                        if (data_readback_remaining_r == 0) begin
                            state <= S_DATA_DONE;
                        end 
                        else begin
                            state <= S_DATA_READ;    
                        end
                    end
                end
                S_DATA_READ: begin
                    state <= S_DATA_WAIT_FOR_READ;
                end
                S_DATA_WAIT_FOR_READ: begin
                    state <= S_DATA_ACK;
                end
                S_DATA_ACK: begin
                    if (~readback_req) begin
                        state <= S_DATA_READ_DONE;
                    end
                end
                S_DATA_READ_DONE: begin
                    state <= S_DATA_WAIT_FOR_REQ;
                end
                S_DATA_DONE: begin
                    state <= S_REGS_START;
                end
                
                S_REGS_START: begin                    
                    state <= S_REGS_WAIT_FOR_REQ;                    
                end
                S_REGS_WAIT_FOR_REQ: begin
                    if (readback_req) begin
                        if (regs_readback_remaining_r == 0) begin
                            state <= S_REGS_DONE;
                        end 
                        else begin
                            state <= S_REGS_READ;    
                        end
                    end
                end
                S_REGS_READ: begin
                    state <= S_REGS_WAIT_FOR_READ;
                end
                S_REGS_WAIT_FOR_READ: begin
                    state <= S_REGS_ACK;
                end 
                S_REGS_ACK: begin
                    if (~readback_req) begin
                        state <= S_REGS_READ_DONE;
                    end
                end
                S_REGS_READ_DONE: begin
                    state <= S_REGS_WAIT_FOR_REQ;
                end
                S_REGS_DONE: begin
                    state <= S_READBACK_DONE;
                end
                S_READBACK_DONE: begin
                    // need this extra state to give the comm module a cycle to respone to the 
                    // done signal, and lower the req signal.  Otherwise we will go to S_FILL
                    // and the req signal will still be high, initiating another readback pass.
                    state <= S_FILL;                     
                end
            endcase
        end         
    end
    
    //////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////// READBACK ///////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////
    /* readback_remaining_r */
    always @ (posedge clk) begin
        case (state)
            S_CONTROL_START: begin
                if (control_filled_r)
                    control_readback_remaining_r <= CONTROL_BUFFER_DEPTH[CONTROL_BUFFER_ADDR_BITS-1:0];
                else
                    control_readback_remaining_r <= control_addr_r;
            end
            S_CONTROL_READ_DONE: begin
                control_readback_remaining_r <= control_readback_remaining_r - 1'b1;
            end
            S_DATA_START: begin
                if (data_filled_r)
                    data_readback_remaining_r <= DATA_BUFFER_DEPTH[DATA_BUFFER_ADDR_BITS-1:0];
                else
                    data_readback_remaining_r <= data_addr_a_r;
            end
            S_DATA_READ_DONE: begin                    
                data_readback_remaining_r <= data_readback_remaining_r - 1'b1;
            end
            S_REGS_START: begin
                if (OPTION_TRACE_REGS == 0)
                    regs_readback_remaining_r <= 0;
                else if (regs_filled_r)
                    regs_readback_remaining_r <= REGS_BUFFER_DEPTH[REGS_BUFFER_ADDR_BITS-1:0];
                else
                    regs_readback_remaining_r <= regs_addr_a_r;
            end
            S_REGS_READ_DONE: begin
                regs_readback_remaining_r <= regs_readback_remaining_r - 1'b1;
            end
        endcase
    end
    
    assign readback_control = control_readdata;
    assign readback_data = data_readdata;
    assign readback_regs = regs_readdata;     
    assign readback_done = (state == S_DATA_DONE || state == S_CONTROL_DONE || state == S_REGS_DONE);
    assign readback_ack = (state == S_CONTROL_ACK || state == S_DATA_ACK || state == S_REGS_ACK); 
    
    //////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////// TRACING CONTROL DATA ///////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////
    ram_sp 
        #(.DEPTH(CONTROL_BUFFER_DEPTH),
          .WIDTH(CONTROL_BUFFER_WIDTH)
        )
    ram_trace_ctrl(
        .addr(control_addr_c),
        .clk(clk),
        .data(control_writedata_c),
        .wr_en(control_wr_en),
        .q(control_readdata)
    );
    
    assign control_wr_en = (state == S_FILL && program_running_r1);         
    
    /* control_filled_r */
    always @ (posedge clk) begin
        if (reset) begin
            control_filled_r <= 1'b0;
        end
        else if ((control_addr_r == CONTROL_BUFFER_DEPTH-1) && control_wr_en) begin
            control_filled_r <= 1'b1;
        end
    end
    
    always @ (posedge clk) begin
        if (reset) begin
            control_addr_r <= 0;
            sequence_cnt_r <= 0;  
            pc_state_sequence_start_r <= 0;
        end else begin
            
            /* control_addr_r */
            case (state)
                S_FILL: 
                    if (control_wr_en && !control_rewrite_c) begin
                        control_addr_r <= control_addr_c_plus1_c;
					end
                S_CONTROL_START:
                    if (~control_filled_r)
                        control_addr_r <= 0;
                    //else
                    //    control_addr_r <= control_addr_c_plus1_c;
                S_CONTROL_READ_DONE:
                    //if (control_readback_remaining_r > 0)
                        control_addr_r <= control_addr_r_plus1_c;
            endcase
            
            if (program_running_r1) begin
                if (~control_is_sequential_c) begin
                    sequence_cnt_r <= 1; 
                    pc_state_sequence_start_r <= pc_state_r1;
                end else begin
                    sequence_cnt_r <= sequence_cnt_r + 1'b1;
                end
            end
        end
    end
            
    always @ (*) begin
        control_rewrite_c = ~trace_en_module_out || control_is_sequential_c;
        
        /* control_addr_c */
        if (state == S_FILL && control_rewrite_c)
            // Rewrite last entry
            control_addr_c = control_addr_r_minus1_c;
        else
            control_addr_c = control_addr_r;
    
        /* control_writedata_c */
        if (~control_is_sequential_c) begin
            control_writedata_c = {pc_module_r1, pc_state_r1, {SEQUENCE_BITS{1'b0}}};
        end else begin
            control_writedata_c = {pc_module_r1, pc_state_sequence_start_r, sequence_cnt_r};
        end
    
        /* control_addr_c_plus1_c */
        if (control_is_sequential_c || control_rewrite_c)
            control_addr_c_plus1_c = control_addr_c;
        else if (control_addr_c == CONTROL_BUFFER_DEPTH-1) 
            control_addr_c_plus1_c = 0;
        else 
            control_addr_c_plus1_c = control_addr_c + 1'b1;
    
        if (control_addr_r == 0)
            control_addr_r_minus1_c = CONTROL_BUFFER_DEPTH[CONTROL_BUFFER_ADDR_BITS-1:0] - 1'b1;
        else
            control_addr_r_minus1_c = control_addr_r - 1'b1;
    
        if (control_addr_r == CONTROL_BUFFER_DEPTH-1)
            control_addr_r_plus1_c = 0;
        else
            control_addr_r_plus1_c = control_addr_r + 1'b1;
    end

    always @ (*) begin
        /* control_is_sequential_c */
        if (sequence_cnt_r == {SEQUENCE_BITS{1'b1}}) 
            control_is_sequential_c = 1'b0;
        else if ((pc_module_r1 == pc_module_last) && (pc_state_r1 == (pc_state_last + 1))) 
            control_is_sequential_c = 1'b1;     
        else
            control_is_sequential_c = 1'b0;
    end

    //////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////// TRACING MEMORY DATA ////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////
    ram_dp
        #(.DEPTH(DATA_BUFFER_DEPTH),
          .WIDTH(DATA_BUFFER_WIDTH)
        )  
    ram_trace_mem(
        .addr_a(data_addr_a_c),
        .addr_b(data_addr_b_c),
        .clk(clk),
        .data_a(data_writedata_a_c),
        .data_b(data_writedata_b_c),
        .wr_en_a(data_wr_en_a_c),
        .wr_en_b(data_wr_en_b_c),
        .q_a(data_readdata),
        .q_b()
    );
    
    always @ (posedge clk) begin
        if (reset) begin
            data_addr_a_r <= 0;
            data_filled_r <= 1'b0;
        end else begin
            case (state)
                S_FILL:
                    if (data_wr_en_a_c && data_wr_en_b_c)
                        data_addr_a_r <= data_addr_a_c_plus2_c;
                    else if (data_wr_en_a_c || data_wr_en_b_c)
                        data_addr_a_r <= data_addr_a_c_plus1_c;
                S_DATA_START:
                    if (~data_filled_r)
                        data_addr_a_r <= 0;
                S_DATA_READ_DONE:
                    data_addr_a_r <= data_addr_a_c_plus1_c;
            endcase
            
            if ((data_addr_a_r == DATA_BUFFER_DEPTH-1) && data_wr_en_a_c)
                data_filled_r <= 1'b1;
            else if ((data_addr_a_r == DATA_BUFFER_DEPTH-2) && data_wr_en_a_c && data_wr_en_b_c)
                data_filled_r <= 1'b1;
                
            data_lines_written_r <= data_lines_written_c;
        end
    end
    
    always @ (*) begin
        data_wr_en_a_c = 1'b0;
        data_wr_en_b_c = 1'b0;
        data_writedata_a_c = {DATA_BUFFER_WIDTH{1'bx}};
        data_writedata_b_c = {DATA_BUFFER_WIDTH{1'bx}};
        data_lines_written_c = 0;
        
        if (program_running) begin
            if (main_wr_en_a && main_wr_en_b) begin
                data_wr_en_a_c = 1'b1;
                data_wr_en_b_c = 1'b1;
                data_writedata_a_c = {main_size_a, main_addr_a, main_data_a};            
                data_writedata_b_c = {main_size_b, main_addr_b, main_data_b};            
                data_lines_written_c = 2;
            end

            else if (main_wr_en_a) begin
                data_wr_en_a_c = 1'b1;
                data_writedata_a_c = {main_size_a, main_addr_a, main_data_a};                               
                data_lines_written_c = 1;
            end

            else if (main_wr_en_b) begin
                data_wr_en_b_c = 1'b1;
                data_writedata_b_c = {main_size_b, main_addr_b, main_data_b};              
                data_lines_written_c = 1;
            end            
        end
        
        
        if (data_addr_a_c == DATA_BUFFER_DEPTH-1) 
            data_addr_a_c_plus1_c = 0;
        else 
            data_addr_a_c_plus1_c = data_addr_a_c + 1'b1;
            
        if (data_addr_a_c_plus1_c == DATA_BUFFER_DEPTH-1)
            data_addr_a_c_plus2_c = 0;
        else
            data_addr_a_c_plus2_c = data_addr_a_c_plus1_c + 1'b1;
        
        if (data_addr_a_r == 0)
            data_addr_a_r_minus1_c = DATA_BUFFER_DEPTH[DATA_BUFFER_ADDR_BITS-1:0] - 1'b1;
        else
            data_addr_a_r_minus1_c = data_addr_a_r - 1'b1;
    
        if (data_addr_a_r_minus1_c == 0)
            data_addr_a_r_minus2_c = DATA_BUFFER_DEPTH[DATA_BUFFER_ADDR_BITS-1:0] - 1'b1;
        else
            data_addr_a_r_minus2_c = data_addr_a_r_minus1_c - 1'b1;
            
        /* data_addr_a_c */
        if (!trace_en_module_out)
            // We weren't supposed to trace that module, so roll back
            if (data_lines_written_r == 2)
                data_addr_a_c = data_addr_a_r_minus2_c;
            else if (data_lines_written_r == 1)
                data_addr_a_c = data_addr_a_r_minus1_c;
            else
                data_addr_a_c = data_addr_a_r;
        else 
            data_addr_a_c = data_addr_a_r;
            
        if (~data_wr_en_a_c && data_wr_en_b_c)
            data_addr_b_c = data_addr_a_c;
        else
            data_addr_b_c = data_addr_a_c_plus1_c;
    end
    
    //////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////// TRACING REGISTER DATA //////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////
    generate // OPTION_TRACE_REGS

        assign regs_wr_en_a = regs_valid_a;
        assign regs_wr_en_b = regs_valid_b;

        if (OPTION_TRACE_REGS) begin
            ram_dp
                #(  .WIDTH(REGS_BUFFER_WIDTH),
                    .DEPTH(REGS_BUFFER_DEPTH)
                )
            ram_trace_regs(
                .addr_a(regs_addr_a_r),
                .addr_b(regs_addr_b_c),
                .clk(clk),
                .data_a(regs_muxed_a),
                .data_b(regs_muxed_b),
                .wr_en_a(regs_wr_en_a),
                .wr_en_b(regs_wr_en_b),
                .q_a(regs_readdata),
                .q_b()
            );
            
        end else begin
            assign regs_readdata = 0;
        end
        
        always @ (posedge clk) begin
            if (reset) begin
                regs_addr_a_r <= 0;
                regs_filled_r <= 1'b0;
            end else begin
                case (state)
                    S_FILL:
                        // We use program_running_r1 to check if the circuit was running last cycle
                        // because the register tracing is done with a 1 cycle delay, since
                        // all inputs to the trace scheduler are registered.
                        if (regs_wr_en_b && program_running_r1)
                            regs_addr_a_r <= regs_addr_a_r_plus2_c;
                        else if (regs_wr_en_a && program_running_r1)
                            regs_addr_a_r <= regs_addr_a_r_plus1_c;
                    S_REGS_START:
                        if (~regs_filled_r)
                            regs_addr_a_r <= 0;
                    S_REGS_READ_DONE:
                        regs_addr_a_r <= regs_addr_a_r_plus1_c;
                endcase
        
                if ((regs_addr_a_r == REGS_BUFFER_DEPTH-1) && regs_wr_en_a)
                    regs_filled_r <= 1'b1;
                else if ((regs_addr_a_r == REGS_BUFFER_DEPTH-2) && regs_wr_en_b)
                    regs_filled_r <= 1'b1;
            end
        end
        
        always @ (*) begin
            regs_addr_b_c = regs_addr_a_r_plus1_c;
            regs_addr_a_r_plus1_c = regs_addr_a_r + 1'b1;
            regs_addr_a_r_plus2_c = regs_addr_a_r + 2'd2;
            if (regs_addr_a_r == REGS_BUFFER_DEPTH-1) begin
                regs_addr_a_r_plus1_c = 0;
                regs_addr_a_r_plus2_c = 1;
            end else if (regs_addr_a_r == REGS_BUFFER_DEPTH-2) begin
                regs_addr_a_r_plus2_c = 0;                
            end                
        end
    endgenerate // OPTION_TRACE_REGS
    
    //////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////// TRACE CONFIG MODULES ///////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////
    generate 
        if (OPTION_REALTIME_ENABLE_TRACE_MODULE) begin            
            ram_sp 
            #( 
                .DEPTH ( 1 << (PC_MODULE_BITS) ), 
                .WIDTH (1) 
            )
            ram_trace_en_instance (
                .clk(clk),
                .wr_en (trace_en_module_wr_en),
                .addr (trace_en_module_addr),
                .data (trace_en_module_data),
                .q (trace_en_module_out)
            );
            
            always @ (*) begin
                if (trace_en_module_wr_en) 
                    trace_en_module_addr = trace_en_module_id;
                else
                    trace_en_module_addr = pc_module_r1;
            end
        end else begin
            assign trace_en_module_out = 1'b1;
        end
    endgenerate
    
    //////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////// TRACE CONFIG VARIABLES /////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////
    generate 
        if (OPTION_REALTIME_ENABLE_TRACE_VARIABLE) begin
            ram_dp
            #(
                .WIDTH(1),
                .DEPTH(1 << 9)
            )
            ram_trace_en_var (
                .clk(clk),
                .wr_en_a(trace_en_var_wr_en),
                .wr_en_b( 1'b0),
                .addr_a(trace_en_var_addr_a_c),
                .addr_b(trace_en_var_addr_b),
                .data_a(trace_en_var_data),
                .data_b(1'bx),
                .q_a(trace_en_var_out_a),
                .q_b(trace_en_var_out_b)
            );
            
            assign trace_en_var_addr_b = main_addr_b[31:23];
            
            always @ (*) begin
                if (trace_en_var_wr_en) begin
                    trace_en_var_addr_a_c = trace_en_var_tag;
                end
                else begin
                    trace_en_var_addr_a_c = main_addr_a[31:23];
                end     
            end 
            
        end else begin
            //assign trace_en_var_out_a = 1'b1;
            //assign trace_en_var_out_b = 1'b1;
        end
    endgenerate
endmodule

