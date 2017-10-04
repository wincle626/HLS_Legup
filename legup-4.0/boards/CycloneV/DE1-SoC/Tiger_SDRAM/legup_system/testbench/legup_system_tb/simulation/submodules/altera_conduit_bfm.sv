// (C) 2001-2015 Altera Corporation. All rights reserved.
// Your use of Altera Corporation's design tools, logic functions and other 
// software and tools, and its AMPP partner logic functions, and any output 
// files any of the foregoing (including device programming or simulation 
// files), and any associated documentation or information are expressly subject 
// to the terms and conditions of the Altera Program License Subscription 
// Agreement, Altera MegaCore Function License Agreement, or other applicable 
// license agreement, including, without limitation, that your use is for the 
// sole purpose of programming logic devices manufactured by Altera and sold by 
// Altera or its authorized distributors.  Please refer to the applicable 
// agreement for further details.


// $Id: //acds/main/ip/sopc/components/verification/altera_tristate_conduit_bfm/altera_tristate_conduit_bfm.sv.terp#7 $
// $Revision: #7 $
// $Date: 2010/08/05 $
// $Author: klong $
//-----------------------------------------------------------------------------
// =head1 NAME
// altera_conduit_bfm
// =head1 SYNOPSIS
// Bus Functional Model (BFM) for a Standard Conduit BFM
//-----------------------------------------------------------------------------
// =head1 DESCRIPTION
// This is a Bus Functional Model (BFM) for a Standard Conduit Master.
// This BFM sampled the input/bidirection port value or driving user's value to 
// output ports when user call the API.  
// This BFM's HDL is been generated through terp file in Qsys/SOPC Builder.
// Generation parameters:
// output_name:                                       altera_conduit_bfm
// role:width:direction:                              select:3:output,lights:18:input
// 1
//-----------------------------------------------------------------------------
`timescale 1 ps / 1 ps

module altera_conduit_bfm
(
   clk,
   reset,
   reset_n,
   sig_select,
   sig_lights
);

   //--------------------------------------------------------------------------
   // =head1 PINS 
   // =head2 User defined interface
   //--------------------------------------------------------------------------
   input clk;
   input reset;
   input reset_n;
   output [2 : 0] sig_select;
   input [17 : 0] sig_lights;

   // synthesis translate_off
   import verbosity_pkg::*;
   
   typedef logic [2 : 0] ROLE_select_t;
   typedef logic [17 : 0] ROLE_lights_t;

   reg [2 : 0] sig_select_temp;
   reg [2 : 0] sig_select_out;
   logic [17 : 0] sig_lights_in;
   logic [17 : 0] sig_lights_local;

   //--------------------------------------------------------------------------
   // =head1 Public Methods API
   // =pod
   // This section describes the public methods in the application programming
   // interface (API). The application program interface provides methods for 
   // a testbench which instantiates, controls and queries state in this BFM 
   // component. Test programs must only use these public access methods and 
   // events to communicate with this BFM component. The API and module pins
   // are the only interfaces of this component that are guaranteed to be
   // stable. The API will be maintained for the life of the product. 
   // While we cannot prevent a test program from directly accessing internal
   // tasks, functions, or data private to the BFM, there is no guarantee that
   // these will be present in the future. In fact, it is best for the user
   // to assume that the underlying implementation of this component can 
   // and will change.
   // =cut
   //--------------------------------------------------------------------------
   
   event signal_reset_asserted;
   event signal_input_lights_change;
   
   function automatic string get_version();  // public
      // Return BFM version string. For example, version 9.1 sp1 is "9.1sp1" 
      string ret_version = "15.0";
      return ret_version;
   endfunction

   // -------------------------------------------------------
   // select
   // -------------------------------------------------------

   function automatic void set_select (
      ROLE_select_t new_value
   );
      // Drive the new value to select.
      
      $sformat(message, "%m: method called arg0 %0d", new_value); 
      print(VERBOSITY_DEBUG, message);
      
      sig_select_temp = new_value;
   endfunction

   // -------------------------------------------------------
   // lights
   // -------------------------------------------------------
   function automatic ROLE_lights_t get_lights();
   
      // Gets the lights input value.
      $sformat(message, "%m: called get_lights");
      print(VERBOSITY_DEBUG, message);
      return sig_lights_in;
      
   endfunction

   always @(posedge clk) begin
      sig_select_out <= sig_select_temp;
      sig_lights_in <= sig_lights;
   end
   
   assign sig_select = sig_select_out;

   always @(posedge reset or negedge reset_n) begin
      -> signal_reset_asserted;
   end

   always @(sig_lights_in) begin
      if (sig_lights_local != sig_lights_in)
         -> signal_input_lights_change;
      sig_lights_local = sig_lights_in;
   end
   


// synthesis translate_on

endmodule

