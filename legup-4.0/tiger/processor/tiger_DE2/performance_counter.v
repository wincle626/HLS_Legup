`timescale 1ns / 1ps
module performance_counter (
    // inputs:
    csi_clockreset_clk,
    csi_clockreset_resetn,
    avs_s1_address,
    avs_s1_write,
    avs_s1_writedata,

    // outputs:
    avs_s1_readdata
    );

    input            csi_clockreset_clk;
    input            csi_clockreset_resetn;
    input            avs_s1_address;
    input            avs_s1_write;
    input   [ 31: 0] avs_s1_writedata;
    output  [ 31: 0] avs_s1_readdata;
    reg     [ 31: 0] counter;
    reg              counter_en;
    wire             clk;
    wire             reset_n;
    wire             address;
    wire             write;

    assign clk = csi_clockreset_clk;
    assign reset_n = csi_clockreset_resetn;
    assign address = avs_s1_address;
    assign write = avs_s1_write;

    always @(posedge clk or negedge reset_n)
    begin
        if (reset_n == 0)
            counter_en <= 0;
        else if (address == 0 && write)
            counter_en <= 1;
        else if (address == 1 && write)
            counter_en <= 0;
    end

    always @(posedge clk or negedge reset_n)
    begin
        if (reset_n == 0)
            counter <= 0;
        else if (counter_en)
            counter <= counter + 1;
    end

    assign avs_s1_readdata = counter;

endmodule

