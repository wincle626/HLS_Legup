module arbiter_tb ();
  parameter N = 4;

  reg clk;
  reg reset;
  reg [N-1:0] go;

  wire [N-1:0] req;
  wire [N-1:0] grant;

genvar i;

generate
  for (i=0; i < N; i=i+1) begin : proc_loop
    process P(clk, go[i], req[i], !grant[i]);
  end
endgenerate

round_robin_arbiter #(N) arb(reset, clk, req, grant, 1'b0);

initial
    clk <= 0;
always @(clk)
    clk <= #10 ~clk;

always @(posedge clk)
  $display("clk posedge");

initial begin
reset <= 1'b1;
go <= {N{1'b0}};
#5;
@(negedge clk);
reset <= 1'b0;
@(negedge clk);
go <= {N{1'b1}};
@(negedge clk);
$display("starting");
go <= {N{1'b0}};
end

endmodule
