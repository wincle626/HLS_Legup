module process (
  input clk,
  input go,
  output req,
  input wait_req
);

reg cur_state = 1'b0;

always @(posedge clk)
begin
  $display(cur_state);
  case (cur_state)
    1'b0:
      cur_state <= go;
    1'b1:
      cur_state <= wait_req;
  endcase
end

assign req = (cur_state == 1'b1);

endmodule
