//Using Two Simple Priority Arbiters with a Mask - scalable
//author: dongjun_luo@hotmail.com
module round_robin_arbiter (
	rst_an,
	clk,
	req_in,
	grant_comb,
	waitrequest
);

parameter N = 2;

input		rst_an;
input		clk;
input	[N-1:0]	req_in;
input waitrequest;
output	[N-1:0]	grant_comb;

wire [N-1:0] req;
reg	[N-1:0]	rotate_ptr;
//reg	rotate_ptr;
wire	[N-1:0]	mask_req;
wire	[N-1:0]	mask_grant;
wire	[N-1:0]	grant_comb;
reg	[N-1:0]	grant;
//reg	[N-1:0]	grant_final;
wire		no_mask_req;
wire	[N-1:0] nomask_grant;
wire		update_ptr;
genvar i;

assign req[N-1:0] = req_in[N-1:0];
// rotate pointer update logic
assign update_ptr = |grant_comb[N-1:0];
always @ (posedge clk)
begin
	if (rst_an) begin
		//rotate_ptr[N-1:0] <= {N{1'b1}};
		rotate_ptr[0] <= 1'b1;
		rotate_ptr[1] <= 1'b1;
	end
	else if (update_ptr && !waitrequest)
//	else if (update_ptr)
	begin
		// note: N must be at least 2
		rotate_ptr[0] <= grant_comb[N-1];
		rotate_ptr[1] <= grant_comb[N-1] | grant_comb[0];
	end
end

/*
always @ (posedge clk or negedge rst_an)
begin
	if (!rst_an) begin
		rotate_ptr <= 1'b0;
	end
	else if (update_ptr && !waitrequest)
//	else if (update_ptr)
	begin
		// note: N must be at least 2
		rotate_ptr[0] <= grant_comb[N-1];
		rotate_ptr[1] <= grant_comb[N-1] | grant_comb[0];
	end
end

always @ (*) begin
	case (rotate_ptr)
		2'b00: begin
			if (req[0]) grant_comb = 2'b01;
			else if (req[1]) grant_comb = 2'b10;
			else grant_comb = 2'b00;
		end
		2'b01 : begin
			if (req[1]) grant_comb = 2'b10;
			else if (req[0]) grant_comb = 2'b01;
			else grant = 2'b00;
		end
	endcase
end
*/

generate
	for (i=2;i<N;i=i+1) begin : rotateptr
		always @ (posedge clk or negedge rst_an)
		begin
			if (!rst_an)
				rotate_ptr[i] <= 1'b1;
			else if (update_ptr && !waitrequest)
				rotate_ptr[i] <= grant[N-1] | (|grant[i-1:0]);
		end
	end
endgenerate

// mask grant generation logic
assign mask_req[N-1:0] = req[N-1:0] & rotate_ptr[N-1:0];

assign mask_grant[0] = mask_req[0];
generate 
	for (i=1;i<N;i=i+1) begin : maskgrant
		assign mask_grant[i] = (~|mask_req[i-1:0]) & mask_req[i];
	end
endgenerate

// non-mask grant generation logic
assign nomask_grant[0] = req[0];
generate
	for (i=1;i<N;i=i+1) begin : nomaskgrant
		assign nomask_grant[i] = (~|req[i-1:0]) & req[i];
	end
endgenerate

// grant generation logic
assign no_mask_req = ~|mask_req[N-1:0];
assign grant_comb[N-1:0] = mask_grant[N-1:0] | (nomask_grant[N-1:0] & {N{no_mask_req}});

always @ (posedge clk or negedge rst_an)
begin
	if (!rst_an)	grant[N-1:0] <= {N{1'b0}};
//	else		grant[N-1:0] <= grant_comb[N-1:0] & ~grant[N-1:0];
	else		grant[N-1:0] <= grant_comb[N-1:0];
end
/*

always @ (posedge clk or negedge rst_an)
begin
	if (!rst_an)
	begin
		grant_final <= 4'd0;
	end
	else
	begin
		case(grant_comb) 
			4'd1 : grant_final <= 4'd1;
			4'd2 : grant_final <= 4'd2;
			4'd4 : grant_final <= 4'd4; 
			4'd8 : grant_final <= 4'd8;
		endcase
	end
end

/*
always @(*)
begin
	if (!rst_an)  grant_final[N-1:0] <= {N{1'b0}};
	else if (grant[i]) grant_final[i] <= 1'b1;
	else if ()
end*/
endmodule
