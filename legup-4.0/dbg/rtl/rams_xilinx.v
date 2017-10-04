module ram_sp (
    clk,
    wr_en,
    addr,    
    data,
    q
);
    function integer log2;
        input [31:0] value;
        for (log2=0; value>0; log2=log2+1)
        value = value>>1;
    endfunction
    
    parameter WIDTH = -1;
    parameter DEPTH = -1;
    
    localparam ADDR_WIDTH = log2(DEPTH-1);

    input                       clk;
    input     [ADDR_WIDTH-1:0]  addr;
    input     [WIDTH-1:0]       data;
    input                       wr_en;    
    output reg[WIDTH-1:0]       q;

    reg       [WIDTH-1:0]       RAM [DEPTH-1:0];
    
    always @ (posedge clk) begin
        if (wr_en)
            RAM[addr] <= data;
        else 
            q <= RAM[addr];
    end
    
endmodule

module ram_dp (
    clk,
    wr_en_a,
    wr_en_b,
    addr_a,
    data_a,
    addr_b,
    data_b,
    q_a,
    q_b
);
    function integer log2;
        input [31:0] value;
        for (log2=0; value>0; log2=log2+1)
        value = value>>1;
    endfunction
    
    parameter WIDTH = -1;
    parameter DEPTH = -1;
    
    localparam ADDR_WIDTH = log2(DEPTH-1);

    input                       clk;
    input   [ADDR_WIDTH-1:0]    addr_a;
    input   [ADDR_WIDTH-1:0]    addr_b;
    input   [WIDTH-1:0]         data_a;
    input   [WIDTH-1:0]         data_b;
    input                       wr_en_a;
    input                       wr_en_b;
    output reg [WIDTH-1:0]         q_a;
    output reg [WIDTH-1:0]         q_b;
    
    reg       [WIDTH-1:0]       RAM [DEPTH-1:0];

    always @ (posedge clk) begin
        if (wr_en_a)
            RAM[addr_a] <= data_a;
        else 
            q_a <= RAM[addr_a];

        if (wr_en_b)
            RAM[addr_b] <= data_b;
        else
            q_b <= RAM[addr_b];        
    end
endmodule

