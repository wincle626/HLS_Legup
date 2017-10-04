`timescale 1 ns / 1 ns
module main_tb ();
    reg  clk;
    reg  reset;
    reg  start;
    wire  finish;
    wire [1:0] KEY;
    wire [31:0] ret;

    assign KEY[0] = reset;
    assign KEY[1] = start;

    de2 de2_inst (clk, KEY, ret);

    initial
        clk = 0;
    always @(clk)
        clk <= #10 ~clk;

    initial begin
    start <= 1;
    @(negedge clk);
    reset <= 0;
    @(negedge clk);
    reset <= 1;
    @(negedge clk);
    @(negedge clk);
    @(negedge clk);
    @(negedge clk);
    start = 0;

    end

    initial
        $monitor("At t=%t %d", $time, ret);
endmodule

module de2 (CLOCK_50, KEY, return_val_reg);
    input CLOCK_50;
    wire [7:0] LEDG;
    input [1:0] KEY;
    wire [15:0] SW;

    wire clk = CLOCK_50;
    wire reset = ~KEY[0];
    wire go = ~KEY[1];

    wire  start;
    wire [31:0] return_val;
    wire  finish;

    output reg [31:0] return_val_reg;

    top top_inst (
        .clk (clk),
        .reset (reset),
        .start (start),
        .finish (finish),
        .return_val (return_val)
    );


    parameter s_WAIT = 3'b001, s_START = 3'b010, s_EXE = 3'b011,
                s_DONE = 3'b100;

    // state registers
    reg [3:0] y_Q, Y_D;

    assign LEDG[3:0] = y_Q;

    // next state
    always @(*)
    begin
        case (y_Q)
            s_WAIT: if (go) Y_D = s_START; else Y_D = y_Q;

            s_START: Y_D = s_EXE;

            s_EXE: if (!finish) Y_D = s_EXE; else Y_D = s_DONE;

            s_DONE: Y_D = s_DONE;

            default: Y_D = 3'bxxx;
        endcase
    end

    // current state
    always @(posedge clk)
    begin
        if (reset) // synchronous clear
            y_Q <= s_WAIT;
        else
            y_Q <= Y_D;
    end

    always @(posedge clk)
        if (y_Q == s_EXE && finish)
            return_val_reg <= return_val;
        else if (y_Q == s_DONE)
            return_val_reg <= return_val_reg;
        else
            return_val_reg <= 0;


    assign start = (y_Q == s_START);
endmodule

