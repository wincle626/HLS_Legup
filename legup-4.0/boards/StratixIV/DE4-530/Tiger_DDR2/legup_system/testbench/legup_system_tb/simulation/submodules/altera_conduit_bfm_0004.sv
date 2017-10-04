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
// output_name:                                       altera_conduit_bfm_0004
// role:width:direction:                              local_init_done:1:input,local_cal_success:1:input,local_cal_fail:1:input
// 0
//-----------------------------------------------------------------------------
`timescale 1 ns / 1 ns

module altera_conduit_bfm_0004
(
   sig_local_init_done,
   sig_local_cal_success,
   sig_local_cal_fail
);

   //--------------------------------------------------------------------------
   // =head1 PINS 
   // =head2 User defined interface
   //--------------------------------------------------------------------------
   input sig_local_init_done;
   input sig_local_cal_success;
   input sig_local_cal_fail;

   // synthesis translate_off
   import verbosity_pkg::*;
   
   typedef logic ROLE_local_init_done_t;
   typedef logic ROLE_local_cal_success_t;
   typedef logic ROLE_local_cal_fail_t;

   logic [0 : 0] local_init_done_in;
   logic [0 : 0] local_init_done_local;
   logic [0 : 0] local_cal_success_in;
   logic [0 : 0] local_cal_success_local;
   logic [0 : 0] local_cal_fail_in;
   logic [0 : 0] local_cal_fail_local;

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
   
   event signal_input_local_init_done_change;
   event signal_input_local_cal_success_change;
   event signal_input_local_cal_fail_change;
   
   function automatic string get_version();  // public
      // Return BFM version string. For example, version 9.1 sp1 is "9.1sp1" 
      string ret_version = "13.1";
      return ret_version;
   endfunction

   // -------------------------------------------------------
   // local_init_done
   // -------------------------------------------------------
   function automatic ROLE_local_init_done_t get_local_init_done();
   
      // Gets the local_init_done input value.
      $sformat(message, "%m: called get_local_init_done");
      print(VERBOSITY_DEBUG, message);
      return local_init_done_in;
      
   endfunction

   // -------------------------------------------------------
   // local_cal_success
   // -------------------------------------------------------
   function automatic ROLE_local_cal_success_t get_local_cal_success();
   
      // Gets the local_cal_success input value.
      $sformat(message, "%m: called get_local_cal_success");
      print(VERBOSITY_DEBUG, message);
      return local_cal_success_in;
      
   endfunction

   // -------------------------------------------------------
   // local_cal_fail
   // -------------------------------------------------------
   function automatic ROLE_local_cal_fail_t get_local_cal_fail();
   
      // Gets the local_cal_fail input value.
      $sformat(message, "%m: called get_local_cal_fail");
      print(VERBOSITY_DEBUG, message);
      return local_cal_fail_in;
      
   endfunction

   assign local_init_done_in = sig_local_init_done;
   assign local_cal_success_in = sig_local_cal_success;
   assign local_cal_fail_in = sig_local_cal_fail;


   always @(local_init_done_in) begin
      if (local_init_done_local != local_init_done_in)
         -> signal_input_local_init_done_change;
      local_init_done_local = local_init_done_in;
   end
   
   always @(local_cal_success_in) begin
      if (local_cal_success_local != local_cal_success_in)
         -> signal_input_local_cal_success_change;
      local_cal_success_local = local_cal_success_in;
   end
   
   always @(local_cal_fail_in) begin
      if (local_cal_fail_local != local_cal_fail_in)
         -> signal_input_local_cal_fail_change;
      local_cal_fail_local = local_cal_fail_in;
   end
   


// synthesis translate_on

endmodule

