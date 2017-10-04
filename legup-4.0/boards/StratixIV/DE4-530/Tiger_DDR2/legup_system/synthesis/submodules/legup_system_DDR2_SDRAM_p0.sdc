# (C) 2001-2013 Altera Corporation. All rights reserved.
# Your use of Altera Corporation's design tools, logic functions and other 
# software and tools, and its AMPP partner logic functions, and any output 
# files any of the foregoing (including device programming or simulation 
# files), and any associated documentation or information are expressly subject 
# to the terms and conditions of the Altera Program License Subscription 
# Agreement, Altera MegaCore Function License Agreement, or other applicable 
# license agreement, including, without limitation, that your use is for the 
# sole purpose of programming logic devices manufactured by Altera and sold by 
# Altera or its authorized distributors.  Please refer to the applicable 
# agreement for further details.


#####################################################################
#
# THIS IS AN AUTO-GENERATED FILE!
# -------------------------------
# If you modify this files, all your changes will be lost if you
# regenerate the core!
#
# FILE DESCRIPTION
# ----------------
# This file contains the timing constraints for the UniPHY memory
# interface.
#    * The timing parameters used by this file are assigned
#      in the legup_system_DDR2_SDRAM_p0_timing.tcl script.
#    * The helper routines are defined in legup_system_DDR2_SDRAM_p0_pin_map.tcl
#
# NOTE
# ----

set script_dir [file dirname [info script]]

source "$script_dir/legup_system_DDR2_SDRAM_p0_parameters.tcl"
source "$script_dir/legup_system_DDR2_SDRAM_p0_timing.tcl"
source "$script_dir/legup_system_DDR2_SDRAM_p0_pin_map.tcl"

load_package ddr_timing_model

set synthesis_flow 0
set sta_flow 0
set fit_flow 0
if { $::TimeQuestInfo(nameofexecutable) == "quartus_map" } {
	set synthesis_flow 1
} elseif { $::TimeQuestInfo(nameofexecutable) == "quartus_sta" } {
	set sta_flow 1
} elseif { $::TimeQuestInfo(nameofexecutable) == "quartus_fit" } {
	set fit_flow 1
}

set use_hard_read_fifo 0

####################
#                  #
# GENERAL SETTINGS #
#                  #
####################

# This is a global setting and will apply to the whole design.
# This setting is required for the memory interface to be
# properly constrained.
derive_clock_uncertainty

# Debug switch. Change to 1 to get more run-time debug information
set debug 0

# All timing requirements will be represented in nanoseconds with up to 3 decimal places of precision
set_time_format -unit ns -decimal_places 3

# Determine if entity names are on
set entity_names_on [ legup_system_DDR2_SDRAM_p0_are_entity_names_on ]

	##################
	#                #
	# QUERIED TIMING #
	#                #
	##################

	set io_standard "$::GLOBAL_legup_system_DDR2_SDRAM_p0_io_standard CLASS I"

	# This is the peak-to-peak jitter on the whole read capture path
	set DQSpathjitter [expr [get_io_standard_node_delay -dst DQDQS_JITTER -io_standard $io_standard -parameters [list IO $::GLOBAL_legup_system_DDR2_SDRAM_p0_io_interface_type] -in_fitter]/1000.0]
	set DQSpathjitter_setup_prop [expr [get_io_standard_node_delay -dst DQDQS_JITTER_DIVISION -io_standard $io_standard -parameters [list IO $::GLOBAL_legup_system_DDR2_SDRAM_p0_io_interface_type] -in_fitter]/100.0]

	set tJITper [expr [get_io_standard_node_delay -dst MEM_CK_PERIOD_JITTER -io_standard $io_standard  -parameters [list IO $::GLOBAL_legup_system_DDR2_SDRAM_p0_io_interface_type] -in_fitter -period $t(CK)]/2000.0]

	##################
	#                #
	# DERIVED TIMING #
	#                #
	##################

	# These parameters are used to make constraints more readeable

	# Half of memory clock cycle
	set half_period [ legup_system_DDR2_SDRAM_p0_round_3dp [ expr $t(CK) / 2.0 ] ]

	# Half of reference clock
	set ref_half_period [ legup_system_DDR2_SDRAM_p0_round_3dp [ expr $t(refCK) / 2.0 ] ]

	# Minimum delay on data output pins
	set t(wru_output_min_delay_external) [expr $t(DH) + $board(intra_DQS_group_skew) + $ISI(DQ)/2 + $ISI(DQS)/2 - $board(DQ_DQS_skew)]
	set t(wru_output_min_delay_internal) [expr $t(WL_DCD) + $t(WL_JITTER) + $t(WL_PSE) + $SSN(rel_pullin_o)]
	set data_output_min_delay [ legup_system_DDR2_SDRAM_p0_round_3dp [ expr - $t(wru_output_min_delay_external) - $t(wru_output_min_delay_internal)]]

	# Maximum delay on data output pins
	set t(wru_output_max_delay_external) [expr $t(DS) + $board(intra_DQS_group_skew) + $ISI(DQ)/2 + $ISI(DQS)/2 + $board(DQ_DQS_skew)]
	set t(wru_output_max_delay_internal) [expr $t(WL_DCD) + $t(WL_JITTER) + $t(WL_PSE) + $SSN(rel_pushout_o)]
	set data_output_max_delay [ legup_system_DDR2_SDRAM_p0_round_3dp [ expr $t(wru_output_max_delay_external) + $t(wru_output_max_delay_internal)]]

	# Maximum delay on data input pins
	set t(rdu_input_max_delay_external) [expr $t(DQSQ) + $board(intra_DQS_group_skew) + $board(DQ_DQS_skew)]
	set t(rdu_input_max_delay_internal) [expr $DQSpathjitter*$DQSpathjitter_setup_prop + $SSN(rel_pushout_i)]
	set data_input_max_delay [ legup_system_DDR2_SDRAM_p0_round_3dp [ expr $t(rdu_input_max_delay_external) + $t(rdu_input_max_delay_internal) ]]

	# Minimum delay on data input pins
	set t(rdu_input_min_delay_external) [expr $board(intra_DQS_group_skew) - $board(DQ_DQS_skew)]
	set t(rdu_input_min_delay_internal) [expr $t(DCD) + $DQSpathjitter*(1.0-$DQSpathjitter_setup_prop) + $SSN(rel_pullin_i)]
	set data_input_min_delay [ legup_system_DDR2_SDRAM_p0_round_3dp [ expr - $t(rdu_input_min_delay_external) - $t(rdu_input_min_delay_internal) ]]

	# Minimum delay on address and command paths
	set ac_min_delay [ legup_system_DDR2_SDRAM_p0_round_3dp [ expr - $t(IH) -$fpga(tPLL_JITTER) - $fpga(tPLL_PSERR) - $board(intra_addr_ctrl_skew) + $board(addresscmd_CK_skew) - $ISI(addresscmd_hold) ]]

	# Maximum delay on address and command paths
	set ac_max_delay [ legup_system_DDR2_SDRAM_p0_round_3dp [ expr $t(IS) +$fpga(tPLL_JITTER) + $fpga(tPLL_PSERR) + $board(intra_addr_ctrl_skew) + $board(addresscmd_CK_skew) + $ISI(addresscmd_setup) ]]

if { $debug } {
	post_message -type info "SDC: Computed Parameters:"
	post_message -type info "SDC: --------------------"
	post_message -type info "SDC: half_period: $half_period"
	post_message -type info "SDC: data_output_min_delay: $data_output_min_delay"
	post_message -type info "SDC: data_output_max_delay: $data_output_max_delay"
	post_message -type info "SDC: data_input_min_delay: $data_input_min_delay"
	post_message -type info "SDC: data_input_max_delay: $data_input_max_delay"
	post_message -type info "SDC: ac_min_delay: $ac_min_delay"
	post_message -type info "SDC: ac_max_delay: $ac_max_delay"

		post_message -type info "SDC: Using Timing Models: Micro"
}

# This is the main call to the netlist traversal routines
# that will automatically find all pins and registers required
# to apply timing constraints.
# During the fitter, the routines will be called only once
# and cached data will be used in all subsequent calls.
if { ! [ info exists legup_system_DDR2_SDRAM_p0_sdc_cache ] } {
	set legup_system_DDR2_SDRAM_p0_sdc_cache 1
	legup_system_DDR2_SDRAM_p0_initialize_ddr_db legup_system_DDR2_SDRAM_p0_ddr_db
} else {
	if { $debug } {
		post_message -type info "SDC: reusing cached DDR DB"
	}
}

# If multiple instances of this core are present in the
# design they will all be constrained through the
# following loop
set instances [ array names legup_system_DDR2_SDRAM_p0_ddr_db ]
foreach { inst } $instances {
	if { [ info exists pins ] } {
		# Clean-up stale content
		unset pins
	}
	array set pins $legup_system_DDR2_SDRAM_p0_ddr_db($inst)

	set prefix $inst
	if { $entity_names_on } {
		set prefix [ string map "| |*:" $inst ]
		set prefix "*:$prefix"
	}

	#####################################################
	#                                                   #
	# Transfer the pin names to more readable variables #
	#                                                   #
	#####################################################

	set dqs_pins $pins(dqs_pins)
	set dqsn_pins $pins(dqsn_pins)
	set q_groups [ list ]
	foreach { q_group } $pins(q_groups) {
		set q_group $q_group
		lappend q_groups $q_group
	}
	set all_dq_pins [ join [ join $q_groups ] ]

	set ck_pins $pins(ck_pins)
	set ckn_pins $pins(ckn_pins)
	set add_pins $pins(add_pins)
	set ba_pins $pins(ba_pins)
	set cmd_pins $pins(cmd_pins)
	set ac_pins [ concat $add_pins $ba_pins $cmd_pins ]
	set dm_pins $pins(dm_pins)
	set all_dq_dm_pins [ concat $all_dq_pins $dm_pins ]

	set pll_ref_clock $pins(pll_ref_clock)
	set pll_afi_clock $pins(pll_afi_clock)
	set pll_ac_clock $pins(pll_ac_clock)
	set pll_ck_clock $pins(pll_ck_clock)
	set pll_write_clock $pins(pll_write_clock)
	set pll_avl_clock $pins(pll_avl_clock)
	set pll_config_clock $pins(pll_config_clock)

	set dqs_in_clocks $pins(dqs_in_clocks)
	set dqs_out_clocks $pins(dqs_out_clocks)
	set dqsn_out_clocks $pins(dqsn_out_clocks)
	set leveling_pins $pins(leveling_pins)

	set afi_reset_reg $pins(afi_reset_reg)
	set seq_reset_reg $pins(seq_reset_reg)
	set sync_reg $pins(sync_reg)
	set read_capture_ddio $pins(read_capture_ddio)
	set fifo_wraddress_reg $pins(fifo_wraddress_reg)
	set fifo_rdaddress_reg $pins(fifo_rdaddress_reg)
	set fifo_wrdata_reg $pins(fifo_wrdata_reg)
	set fifo_rddata_reg $pins(fifo_rddata_reg)
	set valid_fifo_wrdata_reg $pins(valid_fifo_wrdata_reg)
	set valid_fifo_rddata_reg $pins(valid_fifo_rddata_reg)

	##################
	#                #
	# QUERIED TIMING #
	#                #
	##################

	# Phase Jitter on DQS paths. This parameter is queried at run time
	set fpga(tDQS_PHASE_JITTER) [ expr [ get_integer_node_delay -integer $::GLOBAL_legup_system_DDR2_SDRAM_p0_dqs_delay_chain_length -parameters {IO MAX HIGH} -src DQS_PHASE_JITTER -in_fitter ] / 1000.0 ]

	# Phase Error on DQS paths. This parameter is queried at run time
	set fpga(tDQS_PSERR) [ expr [ get_integer_node_delay -integer $::GLOBAL_legup_system_DDR2_SDRAM_p0_dqs_delay_chain_length -parameters {IO MAX HIGH} -src DQS_PSERR -in_fitter ] / 1000.0 ]

		# Correct input min/max delay for queried parameters
		set t(rdu_input_min_delay_external) [expr $t(rdu_input_min_delay_external) + ($t(CK)/2.0 - $t(QH_time))]
		set t(rdu_input_min_delay_internal) [expr $t(rdu_input_min_delay_internal) + $fpga(tDQS_PSERR) + $tJITper]
		set t(rdu_input_max_delay_external) [expr $t(rdu_input_max_delay_external)]
		set t(rdu_input_max_delay_internal) [expr $t(rdu_input_max_delay_internal) + $fpga(tDQS_PSERR)]

		set final_data_input_max_delay [ legup_system_DDR2_SDRAM_p0_round_3dp [ expr $data_input_max_delay + $fpga(tDQS_PSERR) ]]
		set final_data_input_min_delay [ legup_system_DDR2_SDRAM_p0_round_3dp [ expr $data_input_min_delay - $t(CK) / 2.0 + $t(QH_time) - $fpga(tDQS_PSERR) - $tJITper]]


	if { $debug } {
		post_message -type info "SDC: Jitter Parameters"
		post_message -type info "SDC: -----------------"
		post_message -type info "SDC:    DQS Phase: $::GLOBAL_legup_system_DDR2_SDRAM_p0_dqs_delay_chain_length"
		post_message -type info "SDC:    fpga(tDQS_PHASE_JITTER): $fpga(tDQS_PHASE_JITTER)"
		post_message -type info "SDC:    fpga(tDQS_PSERR): $fpga(tDQS_PSERR)"
		post_message -type info "SDC:    t(QH_time): $t(QH_time)"
		post_message -type info "SDC:"
		post_message -type info "SDC: Derived Parameters:"
		post_message -type info "SDC: -----------------"
		post_message -type info "SDC:    Corrected data_input_max_delay: $final_data_input_max_delay"
		post_message -type info "SDC:    Corrected data_input_min_delay: $final_data_input_min_delay"
		post_message -type info "SDC: -----------------"
	}

	# ----------------------- #
	# -                     - #
	# --- REFERENCE CLOCK --- #
	# -                     - #
	# ----------------------- #

	# This is the reference clock used by the PLL to derive any other clock in the core
	if { [get_collection_size [get_clocks -nowarn $pll_ref_clock]] > 0 } { remove_clock $pll_ref_clock }
	create_clock -period $t(refCK) -waveform [ list 0 $ref_half_period ] $pll_ref_clock

	# ------------------ #
	# -                - #
	# --- PLL CLOCKS --- #
	# -                - #
	# ------------------ #

	# PLL Clock Names
	set local_pll_afi_clk [legup_system_DDR2_SDRAM_p0_get_clock_name_from_pin_name_pre_vseries $pll_afi_clock "afi_clk"]
	set local_pll_mem_clk [legup_system_DDR2_SDRAM_p0_get_clock_name_from_pin_name_pre_vseries $pll_ck_clock "mem_clk"]
	set local_pll_addr_cmd_clk [legup_system_DDR2_SDRAM_p0_get_clock_name_from_pin_name_pre_vseries $pll_ac_clock "addr_cmd_clk"]
	set local_pll_write_clk [legup_system_DDR2_SDRAM_p0_get_clock_name_from_pin_name_pre_vseries $pll_write_clock "write_clk"]
	set local_pll_avl_clock [legup_system_DDR2_SDRAM_p0_get_clock_name_from_pin_name_pre_vseries $pll_avl_clock "avl_clk"]
	set local_pll_config_clock [legup_system_DDR2_SDRAM_p0_get_clock_name_from_pin_name_pre_vseries $pll_config_clock "config_clk"]

	if { [get_collection_size [get_clocks -nowarn "$local_pll_afi_clk"]] > 0 } { remove_clock "$local_pll_afi_clk" }
	if { [get_collection_size [get_clocks -nowarn "$local_pll_mem_clk"]] > 0 } { remove_clock "$local_pll_mem_clk" }
	if { [get_collection_size [get_clocks -nowarn "$local_pll_addr_cmd_clk"]] > 0 } { remove_clock "$local_pll_addr_cmd_clk" }
	if { [get_collection_size [get_clocks -nowarn "$local_pll_write_clk"]] > 0 } { remove_clock "$local_pll_write_clk" }
	if { [get_collection_size [get_clocks -nowarn "$local_pll_avl_clock"]] > 0 } { remove_clock "$local_pll_avl_clock" }
	if { [get_collection_size [get_clocks -nowarn "$local_pll_config_clock"]] > 0 } { remove_clock "$local_pll_config_clock" }

	# W A R N I N G !
	# The PLL parameters are statically defined in the legup_system_DDR2_SDRAM_p0_parameters.tcl
	# file at generation time!
	# To ensure timing constraints and timing reports are correct, when you make
	# any changes to the PLL component using the MegaWizard Plug-In,
	# apply those changes to the PLL parameters in the legup_system_DDR2_SDRAM_p0_parameters.tcl

	set pll_i 5
	create_generated_clock -add -name "$local_pll_afi_clk" -source $pll_ref_clock -multiply_by $::GLOBAL_legup_system_DDR2_SDRAM_p0_pll_mult(0) -divide_by $::GLOBAL_legup_system_DDR2_SDRAM_p0_pll_div(0) -phase $::GLOBAL_legup_system_DDR2_SDRAM_p0_pll_phase(0) $pll_afi_clock
	if {[string compare -nocase $pll_afi_clock $pll_ck_clock]==0} {
		set local_pll_mem_clk $local_pll_afi_clk
	} else {
		create_generated_clock -add -name "$local_pll_mem_clk" -source $pll_ref_clock -multiply_by $::GLOBAL_legup_system_DDR2_SDRAM_p0_pll_mult(1) -divide_by $::GLOBAL_legup_system_DDR2_SDRAM_p0_pll_div(1) -phase $::GLOBAL_legup_system_DDR2_SDRAM_p0_pll_phase(1) $pll_ck_clock
	}
	create_generated_clock -add -name "$local_pll_addr_cmd_clk" -source $pll_ref_clock -multiply_by $::GLOBAL_legup_system_DDR2_SDRAM_p0_pll_mult(3) -divide_by $::GLOBAL_legup_system_DDR2_SDRAM_p0_pll_div(3) -phase $::GLOBAL_legup_system_DDR2_SDRAM_p0_pll_phase(3) $pll_ac_clock
	set write_clk_phase $::GLOBAL_legup_system_DDR2_SDRAM_p0_pll_phase(2)
	create_generated_clock -add -name "$local_pll_write_clk" -source $pll_ref_clock -multiply_by $::GLOBAL_legup_system_DDR2_SDRAM_p0_pll_mult(2) -divide_by $::GLOBAL_legup_system_DDR2_SDRAM_p0_pll_div(2) -phase $write_clk_phase $pll_write_clock
	set avl_pll_clk_i $pll_i
	if {[string compare -nocase $pll_afi_clock $pll_avl_clock]==0} {
		set local_pll_avl_clock $local_pll_afi_clk
	} else {
		create_generated_clock -add -name "$local_pll_avl_clock" -source $pll_ref_clock -multiply_by $::GLOBAL_legup_system_DDR2_SDRAM_p0_pll_mult($pll_i) -divide_by $::GLOBAL_legup_system_DDR2_SDRAM_p0_pll_div($pll_i) -phase $::GLOBAL_legup_system_DDR2_SDRAM_p0_pll_phase($pll_i) $pll_avl_clock
	}
	incr pll_i
	create_generated_clock -add -name "$local_pll_config_clock" -source $pll_ref_clock -multiply_by $::GLOBAL_legup_system_DDR2_SDRAM_p0_pll_mult($pll_i) -divide_by $::GLOBAL_legup_system_DDR2_SDRAM_p0_pll_div($pll_i) -phase $::GLOBAL_legup_system_DDR2_SDRAM_p0_pll_phase($pll_i) $pll_config_clock
	incr pll_i

	if {$fit_flow} {
		if {[string compare -nocase $pll_avl_clock $pll_afi_clock] != 0} {
			set_clock_uncertainty -from [get_clocks $local_pll_avl_clock] -to [get_clocks $local_pll_afi_clk] -add -hold 0.100
			set_clock_uncertainty -from [get_clocks $local_pll_afi_clk] -to [get_clocks $local_pll_avl_clock] -add -hold 0.050
		}
	}



	# -------------------- #
	# -                  - #
	# --- SYSTEM CLOCK --- #
	# -                  - #
	# -------------------- #

	# This is the CK clock
	foreach { ck_pin } $ck_pins {
		create_generated_clock -multiply_by 1 -source $pll_ck_clock -master_clock "$local_pll_mem_clk" $ck_pin -name $ck_pin
	}

	# This is the CK#clock
	foreach { ckn_pin } $ckn_pins {
		create_generated_clock -multiply_by 1 -invert -source $pll_ck_clock -master_clock "$local_pll_mem_clk" $ckn_pin -name $ckn_pin
	}
	
	# ------------------- #
	# -                 - #
	# --- READ CLOCKS --- #
	# -                 - #
	# ------------------- #


	foreach dqs_in_clock_struct $dqs_in_clocks {
		array set dqs_in_clock $dqs_in_clock_struct
		# This is the DQS clock for Read Capture analysis (micro model)
		create_clock -period $t(CK) -waveform [ list 0 $half_period ] $dqs_in_clock(dqs_pin) -name $dqs_in_clock(dqs_pin)_IN -add

		# DIV clock is generated on the output of the clock divider.
		# This clock is created using DQS as the source. However, in the netlist there's an inverter
		# between the two clocks. In order to create the right waveform, the clock is divided and shifted by 90
		# degrees.
		create_generated_clock -name $dqs_in_clock(div_name) -source $dqs_in_clock(dqs_pin) -divide_by 2 -phase 90 -offset 0.001 $dqs_in_clock(div_pin) -master $dqs_in_clock(dqs_pin)_IN

			# Clock Uncertainty is accounted for by the ...pathjitter parameters
			set_clock_uncertainty -from [ get_clocks $dqs_in_clock(dqs_pin)_IN ] 0
	}

	# ----------------------- #
	# -                     - #
	# --- LEVELING CLOCKS --- #
	# -                     - #
	# ----------------------- #

	create_generated_clock -add -name "${inst}|legup_system_DDR2_SDRAM_p0_leveling_clk" -master_clock [get_clocks $local_pll_write_clk] -source $pll_write_clock -phase $::GLOBAL_legup_system_DDR2_SDRAM_p0_leveling_capture_phase [ get_pins $leveling_pins ]

	# -------------------- #
	# -                  - #
	# --- WRITE CLOCKS --- #
	# -                  - #
	# -------------------- #

	# This is the DQS clock for Data Write analysis (micro model)
	foreach dqs_out_clock_struct $dqs_out_clocks {
		array set dqs_out_clock $dqs_out_clock_struct
		# Set DQS phase to the ideal 90 degrees and let the calibration scripts take care of
		# properly adjust the margins
		create_generated_clock -multiply_by 1 -master_clock [get_clocks "${inst}|legup_system_DDR2_SDRAM_p0_leveling_clk"] -source $dqs_out_clock(src) -phase 90 $dqs_out_clock(dst) -name $dqs_out_clock(dst)_OUT -add

			# Clock Uncertainty is accounted for by the ...pathjitter parameters
			set_clock_uncertainty -from [get_clocks ${inst}|legup_system_DDR2_SDRAM_p0_leveling_clk] -to [ get_clocks $dqs_out_clock(dst)_OUT ] 0
	}

	# This is the DQS#clock for Data Write analysis (micro model)
	foreach dqsn_out_clock_struct $dqsn_out_clocks {
		array set dqsn_out_clock $dqsn_out_clock_struct
		# Set DQS#phase to the ideal 90 degrees and let the calibration scripts take care of
		# properly adjust the margins
		create_generated_clock -multiply_by 1 -master_clock [get_clocks "${inst}|legup_system_DDR2_SDRAM_p0_leveling_clk"] -source $dqsn_out_clock(src) -phase 90 $dqsn_out_clock(dst) -name $dqsn_out_clock(dst)_OUT -add

			# Clock Uncertainty is accounted for by the ...pathjitter parameters
			set_clock_uncertainty -from [get_clocks ${inst}|legup_system_DDR2_SDRAM_p0_leveling_clk] -to [ get_clocks $dqsn_out_clock(dst)_OUT ] 0
	}


	##################
	#                #
	# READ DATA PATH #
	#                #
	##################

		##################
		#                #
		# (Micro Model)  #
		#                #
		##################

		foreach { dqs_pin } $dqs_pins { dq_pins } $q_groups {
			foreach { dq_pin } $dq_pins {
				set_max_delay -from [get_ports $dq_pin] -to $read_capture_ddio 0
				set_min_delay -from [get_ports $dq_pin] -to $read_capture_ddio [expr 0-$half_period]

				# Specifies the maximum delay difference between the DQ pin and the DQS pin:
				set_input_delay -max $final_data_input_max_delay -clock [get_clocks ${dqs_pin}_IN ] [get_ports $dq_pin] -add_delay

				# Specifies the minimum delay difference between the DQ pin and the DQS pin:
				set_input_delay -min $final_data_input_min_delay -clock [get_clocks ${dqs_pin}_IN ] [get_ports $dq_pin] -add_delay
			}
		}

	# Constraint to increase resynchronization timing margin
	if {(($::quartus(nameofexecutable) eq "quartus_fit"))} {
		set_max_delay -from $pins(fifo_rdaddress_reg) -to $pins(fifo_rddata_reg) [legup_system_DDR2_SDRAM_p0_round_3dp [expr $t(CK)/2.0]]
		set_min_delay -from $pins(fifo_rdaddress_reg) -to $pins(fifo_rddata_reg) 0
	}

	###################
	#                 #
	# WRITE DATA PATH #
	#                 #
	###################

		###################
		#                 #
		# (Micro Model)   #
		#                 #
		###################

		foreach { dqs_pin } $dqs_pins { dq_pins } $q_groups {
			foreach { dq_pin } $dq_pins {
				# Specifies the minimum delay difference between the DQS pin and the DQ pins:
				set_output_delay -min $data_output_min_delay -clock [get_clocks ${dqs_pin}_OUT ] [get_ports $dq_pin] -add_delay

				# Specifies the maximum delay difference between the DQS pin and the DQ pins:
				set_output_delay -max $data_output_max_delay -clock [get_clocks ${dqs_pin}_OUT ] [get_ports $dq_pin] -add_delay
			}
		}

		foreach { dqsn_pin } $dqsn_pins { dq_pins } $q_groups {
			foreach { dq_pin } $dq_pins {
				# Specifies the minimum delay difference between the DQS#pin and the DQ pins:
				set_output_delay -min $data_output_min_delay -clock [get_clocks ${dqsn_pin}_OUT ] [get_ports $dq_pin] -add_delay

				# Specifies the maximum delay difference between the DQS#pin and the DQ pins:
				set_output_delay -max $data_output_max_delay -clock [get_clocks ${dqsn_pin}_OUT ] [get_ports $dq_pin] -add_delay
			}
		}

		foreach dqs_out_clock_struct $dqs_out_clocks {
			array set dqs_out_clock $dqs_out_clock_struct

			if { [string length $dqs_out_clock(dm_pin)] > 0 } {
				# Specifies the minimum delay difference between the DQS and the DM pins:
				set_output_delay -min $data_output_min_delay -clock [get_clocks $dqs_out_clock(dst)_OUT ] [get_ports $dqs_out_clock(dm_pin)] -add_delay

				# Specifies the maximum delay difference between the DQS and the DM pins:
				set_output_delay -max $data_output_max_delay -clock [get_clocks $dqs_out_clock(dst)_OUT ] [get_ports $dqs_out_clock(dm_pin)] -add_delay
			}
		}

		foreach dqsn_out_clock_struct $dqsn_out_clocks {
			array set dqsn_out_clock $dqsn_out_clock_struct

			if { [string length $dqsn_out_clock(dm_pin)] > 0 } {
				# Specifies the minimum delay difference between the DQS and the DM pins:
				set_output_delay -min $data_output_min_delay -clock [get_clocks $dqsn_out_clock(dst)_OUT ] [get_ports $dqsn_out_clock(dm_pin)] -add_delay

				# Specifies the maximum delay difference between the DQS and the DM pins:
				set_output_delay -max $data_output_max_delay -clock [get_clocks $dqsn_out_clock(dst)_OUT ] [get_ports $dqsn_out_clock(dm_pin)] -add_delay
			}
		}


	############
	#          #
	# A/C PATH #
	#          #
	############

	foreach { ck_pin } $ck_pins {
		# Specifies the minimum delay difference between the DQS pin and the address/control pins:
		set_output_delay -min $ac_min_delay -clock [get_clocks $ck_pin] [ get_ports $ac_pins ] -add_delay

		# Specifies the maximum delay difference between the DQS pin and the address/control pins:
		set_output_delay -max $ac_max_delay -clock [get_clocks $ck_pin] [ get_ports $ac_pins ] -add_delay
	}


	##########################
	#                        #
	# MULTICYCLE CONSTRAINTS #
	#                        #
	##########################


	set_multicycle_path -from [get_registers ${prefix}|*s0|*sequencer_phy_mgr_inst|phy_mux_sel] -to [remove_from_collection [get_keepers *] [get_registers -nowarn [list ${prefix}|*s0|*sequencer_phy_mgr_inst|phy_mux_sel ${prefix}|*p0|*altdq_dqs2_inst|oct_*reg*]]] -setup 3
	set_multicycle_path -from [get_registers ${prefix}|*s0|*sequencer_phy_mgr_inst|phy_mux_sel] -to [remove_from_collection [get_keepers *] [get_registers -nowarn [list ${prefix}|*s0|*sequencer_phy_mgr_inst|phy_mux_sel ${prefix}|*p0|*altdq_dqs2_inst|oct_*reg*]]] -hold 2

	if { [get_collection_size [get_registers -nowarn ${prefix}|*p0|*umemphy|*uio_pads|*uaddr_cmd_pads|*clock_gen[*].umem_ck_pad|*]] > 0 } {
		set_multicycle_path -to [get_registers ${prefix}|*p0|*umemphy|*uio_pads|*uaddr_cmd_pads|*clock_gen[*].umem_ck_pad|*] -end -setup 4
		set_multicycle_path -to [get_registers ${prefix}|*p0|*umemphy|*uio_pads|*uaddr_cmd_pads|*clock_gen[*].umem_ck_pad|*] -end -hold 4
	}
	
	# Set multicycle path between address command clock and mem clock when COMMAND_PHASE is > 90 degrees or < -90 degrees
	if {(270 - $::GLOBAL_legup_system_DDR2_SDRAM_p0_pll_phase(3)) > 90 || ($::GLOBAL_legup_system_DDR2_SDRAM_p0_pll_phase(3) - 270) > 90} {
		foreach { ck_pin } $ck_pins {
			set_multicycle_path -from [get_clocks $local_pll_addr_cmd_clk] -to [get_clocks $ck_pin] -setup -end 2
			set_multicycle_path -from [get_clocks $local_pll_addr_cmd_clk] -to [get_clocks $ck_pin] -hold -start 1
		}
	}


	##########################
	#                        #
	# FALSE PATH CONSTRAINTS #
	#                        #
	##########################

	# Cut calibrated paths from DQS enable clock to DQS
	set_false_path -from $pins(dqs_enable_ctrl_reg) -to $pins(dqs_enable_reg)
	set_false_path -from $pins(dqs_enable_ctrl_extend_reg) -to $pins(dqs_enable_reg)


	# Cut paths for memory clocks to avoid unconstrained warnings
	foreach { pin } [concat $dqs_pins $dqsn_pins $ck_pins $ckn_pins] {
		set_false_path -to [get_ports $pin]
	}

	foreach dqs_in_clock_struct $dqs_in_clocks dqsn_out_clock_struct $dqsn_out_clocks {
		array set dqs_in_clock $dqs_in_clock_struct
		array set dqsn_out_clock $dqsn_out_clock_struct

		set_clock_groups -physically_exclusive	-group "$dqs_in_clock(dqs_pin)_IN $dqs_in_clock(div_name)" -group "$dqs_in_clock(dqs_pin)_OUT $dqsn_out_clock(dst)_OUT"

		# Cut paths between AFI Clock and Div Clock
		set_false_path -from [ get_clocks $local_pll_afi_clk ] -to [ get_clocks $dqs_in_clock(div_name) ]
		if { [get_collection_size [get_clocks -nowarn  $pll_afi_clock]] > 0 } {
			set_false_path -from [ get_clocks $pll_afi_clock ] -to [ get_clocks $dqs_in_clock(div_name) ]
		}


		# Cut reset path to clock divider (reset signal controlled by the sequencer)
		set_false_path -from [ get_clocks $local_pll_afi_clk ] -to $dqs_in_clock(div_pin)
		if { [get_collection_size [get_clocks -nowarn  $pll_afi_clock]] > 0 } {
			set_false_path -from [ get_clocks $pll_afi_clock ] -to $dqs_in_clock(div_pin)
		}


		# Cut reset path from sequencer to the clock divider
		set_false_path -from $seq_reset_reg -to $dqs_in_clock(div_pin)
	}

	# This is a register based memory operating as an asynchronous FIFO
	# therefore there is no timing path between the write and read side
	if { ! $use_hard_read_fifo || ! $synthesis_flow } {
		set_false_path -from [get_registers $fifo_wrdata_reg] -to [get_registers $fifo_rddata_reg]
	}



	# The paths between DQS_ENA_CLK and DQS_IN are calibrated, so they must not be analyzed
	set_false_path -from [get_clocks $local_pll_write_clk] -to [get_clocks {*_IN}]



	set tCK_AFI [ expr $t(CK) * 2.0 ]
	
	set capture_reg ${prefix}*read_data_out*

	# Add clock (DQS) uncertainty applied from the DDIO registers to registers in the core
	set_max_delay -from [get_registers $capture_reg] -to $fifo_wrdata_reg -0.05
	set_min_delay -from [get_registers $capture_reg] -to $fifo_wrdata_reg [ legup_system_DDR2_SDRAM_p0_round_3dp [expr -$t(CK) + 0.20 ]]

}

	if {(($::quartus(nameofexecutable) ne "quartus_fit") && ($::quartus(nameofexecutable) ne "quartus_map"))} {
		set dqs_clocks [legup_system_DDR2_SDRAM_p0_get_all_instances_dqs_pins legup_system_DDR2_SDRAM_p0_ddr_db]
		# Leave clocks active when in debug mode
		if {[llength $dqs_clocks] > 0 && !$debug} {
			post_sdc_message info "Setting DQS clocks as inactive; use Report DDR to timing analyze DQS clocks"
			set_active_clocks [remove_from_collection [get_active_clocks] [get_clocks $dqs_clocks]]
		}
	}



######################
#                    #
# REPORT DDR COMMAND #
#                    #
######################

add_ddr_report_command "source [list [file join [file dirname [info script]] ${::GLOBAL_legup_system_DDR2_SDRAM_p0_corename}_report_timing.tcl]]"

