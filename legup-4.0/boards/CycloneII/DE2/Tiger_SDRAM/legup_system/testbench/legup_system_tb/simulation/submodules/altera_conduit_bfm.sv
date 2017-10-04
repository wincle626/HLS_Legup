// (C) 2001-2013 Altera Corporation. All rights reserved.
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
// role:width:direction:                              start:1:input,end:1:input
// true
//-----------------------------------------------------------------------------
`timescale 1 ns / 1 ns

module altera_conduit_bfm
(
   clk,
   reset,
   reset_n,
   sig_start,
   sig_end
);

   //--------------------------------------------------------------------------
   // =head1 PINS 
   // =head2 User defined interface
   //--------------------------------------------------------------------------
   input clk;
   input reset;
   input reset_n;
   input sig_start;
   input sig_end;

   // synthesis translate_off
   import verbosity_pkg::*;
   
   typedef logic ROLE_start_t;
   typedef logic ROLE_end_t;

   logic [0 : 0] start_in;
   logic [0 : 0] start_local;
   logic [0 : 0] end_in;
   logic [0 : 0] end_local;

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
   event signal_input_start_change;
   event signal_input_end_change;
   
   function automatic string get_version();  // public
      // Return BFM version string. For example, version 9.1 sp1 is "9.1sp1" 
      string ret_version = "13.0";
      return ret_version;
   endfunction

   // -------------------------------------------------------
   // start
   // -------------------------------------------------------
   function automatic ROLE_start_t get_start();
   
      // Gets the start input value.
      $sformat(message, "%m: called get_start");
      print(VERBOSITY_DEBUG, message);
      return start_in;
      
   endfunction

   // -------------------------------------------------------
   // end
   // -------------------------------------------------------
   function automatic ROLE_end_t get_end();
   
      // Gets the end input value.
      $sformat(message, "%m: called get_end");
      print(VERBOSITY_DEBUG, message);
      return end_in;
      
   endfunction

   always @(posedge clk) begin
      start_in <= sig_start;
      end_in <= sig_end;
   end
   

   always @(posedge reset or negedge reset_n) begin
      -> signal_reset_asserted;
   end

   always @(start_in) begin
      if (start_local != start_in)
         -> signal_input_start_change;
      start_local = start_in;
   end
   
   always @(end_in) begin
      if (end_local != end_in)
         -> signal_input_end_change;
      end_local = end_in;
   end
   


// synthesis translate_on

endmodule


