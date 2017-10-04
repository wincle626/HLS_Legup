`include "cache_parameters.v"

module multipumping_controller_2x(
    input  wire              base_clock,
    input  wire              clock,         // multiplied *synchronous* clock
    output reg  `PHASE       phase);

reg  clear_en;

// Else simulation can't start properly
initial begin
   clear_en = `LOW;
   phase = `PHASE_0; 
end

// Set clear_en at first clock edge in low phase of base_clock.
// This happens to be the last such edge before the last clock posedge
// coinciding with the posedge of base_clock.
always @(negedge clock) begin
   clear_en <= ~base_clock; 
end

// Disable clear_en when base_clock goes high.
// This leaves time for clear_en to go low before
// the next base_clock low phase.
reg clear;

always @(*) begin
    clear <= clear_en & ~base_clock;
end

always @(posedge clock) begin
    if(clear == `HIGH) begin
        phase <= `PHASE_0;             // synchronize to base_clock
    end
    else begin
        phase <= phase + `PHASE_1;     // binary encoding!
    end
end

endmodule
