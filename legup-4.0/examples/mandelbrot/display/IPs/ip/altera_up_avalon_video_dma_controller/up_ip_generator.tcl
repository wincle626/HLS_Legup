# +----------------------------------------------------------------------------+
# | License Agreement                                                          |
# |                                                                            |
# | Copyright (c) 1991-2013 Altera Corporation, San Jose, California, USA.     |
# | All rights reserved.                                                       |
# |                                                                            |
# | Any megafunction design, and related net list (encrypted or decrypted),    |
# |  support information, device programming or simulation file, and any other |
# |  associated documentation or information provided by Altera or a partner   |
# |  under Altera's Megafunction Partnership Program may be used only to       |
# |  program PLD devices (but not masked PLD devices) from Altera.  Any other  |
# |  use of such megafunction design, net list, support information, device    |
# |  programming or simulation file, or any other related documentation or     |
# |  information is prohibited for any other purpose, including, but not       |
# |  limited to modification, reverse engineering, de-compiling, or use with   |
# |  any other silicon devices, unless such use is explicitly licensed under   |
# |  a separate agreement with Altera or a megafunction partner.  Title to     |
# |  the intellectual property, including patents, copyrights, trademarks,     |
# |  trade secrets, or maskworks, embodied in any such megafunction design,    |
# |  net list, support information, device programming or simulation file, or  |
# |  any other related documentation or information provided by Altera or a    |
# |  megafunction partner, remains with Altera, the megafunction partner, or   |
# |  their respective licensors.  No other licenses, including any licenses    |
# |  needed under any third party's intellectual property, are provided herein.|
# |  Copying or modifying any file, or portion thereof, to which this notice   |
# |  is attached violates this copyright.                                      |
# |                                                                            |
# | THIS FILE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR    |
# | IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   |
# | FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    |
# | THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER |
# | LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING    |
# | FROM, OUT OF OR IN CONNECTION WITH THIS FILE OR THE USE OR OTHER DEALINGS  |
# | IN THIS FILE.                                                              |
# |                                                                            |
# | This agreement shall be governed in all respects by the laws of the State  |
# |  of California and by the laws of the United States of America.            |
# |                                                                            |
# +----------------------------------------------------------------------------+

# +----------------------------------------------------------------------------+
# | Created by the Altera University Program                                   |
# |  for use with the Altera University Program IP Cores                       |
# |                                                                            |
# | Version: 1.4                                                               |
# |                                                                            |
# +----------------------------------------------------------------------------+

proc alt_up_generate {dest_path src_path old_module_name new_module_name params_str sections_str} {
#	send_message info "Starting UP Generation from $src_path to $dest_path"

	set src_file [ open "$src_path" "r" ]
	set src_module [ read $src_file ]
	close $src_file

	set dest_module [ string trim $src_module ]
	set dest_module [ alt_up_modify_sections $src_module $sections_str ]
	set dest_module [ alt_up_modify_module_name $dest_module $old_module_name $new_module_name ]
	set dest_module [ alt_up_modify_params_verilog $dest_module $params_str ]
	set dest_module [ alt_up_modify_params_vhdl $dest_module $params_str ]
	set dest_module [ string trim $dest_module ]
	set dest_module "$dest_module\n"

	set dest_file [ open "$dest_path" "w" ]
	puts $dest_file $dest_module
	close $dest_file
}

proc alt_up_modify_module_name {module_str old_module_name new_module_name} {
	set module_list [ split $module_str "\n" ]

	foreach line $module_list {
		if { [ string match [ format "*%s*" $old_module_name ] $line ] } {
#			append dest_module "\n" [ string map {$old_module_name $new_module_name} $line ]

			if { [ string match "module*" $line ] } {
				append dest_module "\n" "module $new_module_name ("
			} elseif { [ string match "ENTITY*" $line ] } {
				append dest_module "\n" "ENTITY $new_module_name IS"
			} elseif { [ string match "END*" $line ] } {
				append dest_module "\n" "END $new_module_name;"
			} elseif { [ string match "ARCHITECTURE*" $line ] } {
				append dest_module "\n" "ARCHITECTURE Behaviour OF $new_module_name IS"
			} else {
				append dest_module "\n" $line
			}
		} else {
			append dest_module "\n" $line
		}
	}

	return $dest_module
}

proc alt_up_modify_params_verilog {module_str params_str} {
	set params_list [ split $params_str ":;" ]
	set module_list [ split $module_str "\n" ]

	foreach line $module_list {
		if { [ string match "parameter*" $line ] || [ string match "localparam*" $line ] } {
			set param [ split $line "=;" ]
			set param_name [ string trim [ join [ split [ lindex $param 0 ] "localparameter" ] "" ] ]
			
			append dest_module "\n" [ lindex $param 0 ]
			if { [ string match "*$param_name:*" $params_str ] } {
				append dest_module "= " [ string map $params_list $param_name ]
			} else {
				append dest_module "=" [ lindex $param 1 ]
			}
			if { [ llength $param ] == 3 } {
				append dest_module ";" [ lindex $param 2 ]
			}
		} else {
			append dest_module "\n" $line
		}
	}

	return $dest_module
}

proc alt_up_modify_params_vhdl {module_str params_str} {
	set params_list [ split $params_str ":;" ]
	set module_list [ split $module_str "\n" ]

	set is_entity_section 0
	set is_generic_section 0

	foreach line $module_list {
		if { $is_entity_section } {
			if { $is_generic_section } {
				# Check for the end of the generic section
				if { [ string match ");*" $line ] } {
					set is_generic_section 0
				}
				
			# Check for the start of the generic section
			} elseif { [ string match "GENERIC *" $line ] } {
				set is_generic_section 1

			# Check for the end of the entity section
			} elseif { [ string match "END *" $line ] } {
				set is_entity_section 0
			}
			
		# Check for the start of the entity section
		} elseif { [ string match "ENTITY *" $line ] } {
			set is_entity_section 1
		}


		if { $is_entity_section && $is_generic_section } {
			if { [ string match "*:=*" $line ] } {
				set param [ split $line ":=" ]
				set param_name [ string trim [ lindex $param 0 ] ]

				if { [ string match "*$param_name:*" $params_str ] } {
					append dest_module "\n" [ lindex $param 0 ] ":" [ lindex $param 1 ] ":= "

					if { [ string match "*;*" [ lindex $param 3 ] ] } {
						set param [ split [ lindex $param 3 ] ";" ]
						append dest_module [ alt_up_convert_verilog_parameter_value_to_vhdl [ string map $params_list $param_name ] ] ";" [ lindex $param 1 ]
					} elseif { [ string match "*--*" [ lindex $param 3 ] ] } {
						set param [ split [ lindex $param 3 ] "-" ]
						append dest_module [ alt_up_convert_verilog_parameter_value_to_vhdl [ string map $params_list $param_name ] ] "\t-- " [ lindex $param 2 ]
					} else {
						append dest_module [ alt_up_convert_verilog_parameter_value_to_vhdl [ string map $params_list $param_name ] ]
					}
					
				} else {
					append dest_module "\n" $line
				}
			} else {
				append dest_module "\n" $line
			}

		} elseif { [ string match "\tCONSTANT*" $line ] } {
			set param [ split $line ":=;" ]
			set param_name [ string trim [ join [ lrange [ split [ string trim [ lindex $param 0 ] ] " \t" ] 1 end ] ] ]
			
			if { [ string match "*$param_name:*" $params_str ] } {
				append dest_module "\n" [ lindex $param 0 ] ":" [ lindex $param 1 ] ":= "
				append dest_module [ alt_up_convert_verilog_parameter_value_to_vhdl [ string map $params_list $param_name ] ] ";" [ lindex $param 4 ]  
			} else {
				append dest_module "\n" $line
			}

		} else {
			append dest_module "\n" $line
		}
	}

	return $dest_module
}

proc alt_up_convert_verilog_parameter_value_to_vhdl {param_value} {
	if { [ string match "*'b*" $param_value ] } {
		set param [ split $param_value "'b" ]
		set size [ string trim [ lindex $param 0 ] ]
		set value [ string trim [ lindex $param 2 ] ]
		
		return "B\"$value\""
	} elseif { [ string match "*'d*" $param_value ] } {
		set param [ split $param_value "'d" ]
		set size [ string trim [ lindex $param 0 ] ]
		set value [ string trim [ lindex $param 2 ] ]

		set result ""
		for {set i 0} {$i < $size} {incr i} {
			set result [ format "%1d$result" [ expr $value % 2 ] ]
			set value [ expr $value / 2 ]
		}
		
		return "B\"$result\""
	} elseif { [ string match "*'h*" $param_value ] } {
		set param [ split $param_value "'h" ]
		set size [ string trim [ lindex $param 0 ] ]
		set result [ string trim [ lindex $param 2 ] ]

		regsub -all "0" $result "0000" result
		regsub -all "1" $result "0001" result
		regsub -all "2" $result "0010" result
		regsub -all "3" $result "0011" result
		regsub -all "4" $result "0100" result
		regsub -all "5" $result "0101" result
		regsub -all "6" $result "0110" result
		regsub -all "7" $result "0111" result
		regsub -all "8" $result "1000" result
		regsub -all "9" $result "1001" result
		regsub -all "A" $result "1010" result
		regsub -all "a" $result "1010" result
		regsub -all "B" $result "1011" result
		regsub -all "b" $result "1011" result
		regsub -all "C" $result "1100" result
		regsub -all "c" $result "1100" result
		regsub -all "D" $result "1101" result
		regsub -all "d" $result "1101" result
		regsub -all "E" $result "1110" result
		regsub -all "e" $result "1110" result
		regsub -all "F" $result "1111" result
		regsub -all "f" $result "1111" result

		while { [string length $result] < $size } {
			set result "0$result"
		}

		set result [ string range $result [expr [ string length $result ] - $size] end ]

		return "B\"$result\""
	} else {
		return $param_value
	}
}

proc alt_up_modify_sections {module_str sections_str} {
	set sections_list [ split $sections_str ":;" ]
	set module_list [ split $module_str "\n" ]

#	send_message info "Starting UP Modify Sections with $sections_list"

	set recursive_ifs "0:1:1"
	set in_ifdef 0
	set allow_lines 1
	set valid_lines 1

	set line_num 0

	foreach line $module_list {
		set line_num [ expr $line_num + 1 ]
		if { [ string match "`if*" $line ] } {
			lappend recursive_ifs "$in_ifdef:$allow_lines:$valid_lines"
#			send_message info "In ifdef |$recursive_ifs| at $line_num"
			set in_ifdef 1
			set allow_lines [ expr ($valid_lines && $allow_lines) ]
			if { [ string match "`ifdef*" $line ] } {
				set section_name [ string trim [ join [ split $line "`ifdef" ] "" ] ]
				set valid_lines [ expr ([ string match "*$section_name:*" $sections_str ] && [ string map $sections_list $section_name ]) ]
			} else {
				set section_name [ string trim [ join [ split $line "`ifndef" ] "" ] ]
				set valid_lines [ expr ([ string match "*$section_name:*" $sections_str ] && (!([ string map $sections_list $section_name ]))) ]
			}
			set valid_lines [ expr ($valid_lines && $allow_lines) ]

#			if { $valid_lines } {
#				send_message info "$section_name is a valid section $in_ifdef:$allow_lines:$valid_lines"
#			} else {
#				send_message info "$section_name is NOT a valid section $in_ifdef:$allow_lines:$valid_lines"
#			}
			
		} elseif { [ string match "`elsif*" $line ] } {
			if { !($in_ifdef) } {
				send_message error "Unexpected `elsif statement at line $line_num"
			}
			set allow_lines [ expr (!($valid_lines) & $allow_lines) ] 
			set section_name [ string trim [ join [ split $line "`elsif" ] "" ] ]
			set valid_lines [ expr ([ string match "*$section_name:*" $sections_str ] && [ string map $sections_list $section_name ]) ]
			set valid_lines [ expr ($valid_lines && $allow_lines) ]

		} elseif { [ string match "`else*" $line ] } {
			if { !($in_ifdef) } {
				send_message error "Unexpected `else statement at line $line_num"
			}
			set allow_lines [ expr (!($valid_lines) & $allow_lines) ] 
			set valid_lines $allow_lines

		} elseif { [ string match "`endif*" $line ] } {
			if { !($in_ifdef) } {
				send_message error "Unexpected `endif statement at line $line_num"
			}
			set old_if [ split [ lindex $recursive_ifs end ] ":" ]
#			send_message info "New data |$recursive_ifs| |$old_if| at $line_num"
			set in_ifdef [ lindex $old_if 0 ]
			set allow_lines [ lindex $old_if 1 ]
			set valid_lines [ lindex $old_if 2 ]
			set recursive_ifs [ lrange $recursive_ifs 0 end-1 ] 
#			send_message info "New data $in_ifdef $valid_lines $allow_lines at $line_num"

		} elseif { $valid_lines } {
			append dest_module "\n" $line
		}
	}

	return $dest_module
}

proc alt_up_create_instantiation_template {dest_path module_name conduit_name} {
	set port_list ""
	set port_declaration ""
	set module_connections ""

	foreach port [ get_interface_ports $conduit_name ] {
		set direction [ get_port_property $port DIRECTION ]
		set width [ get_port_property $port WIDTH ]

		set port_list "$port_list\n\t$port,"

		set width_string "\t"
		if {$width > 9} {
			set width_string [ format "\[%u: 0\]" [ expr $width - 1 ] ]
		} elseif {$width > 1} {
			set width_string [ format "\[ %u: 0\]" [ expr $width - 1 ] ]
		}

		if {$direction == "Input"} {
			set port_declaration "$port_declaration\ninput"
		} elseif {$direction == "Bidir"} {
			set port_declaration "$port_declaration\ninout"
		} else {
			set port_declaration "$port_declaration\noutput"
		}
		set port_declaration "$port_declaration\t\t$width_string\t$port;"

		set module_connections "$module_connections\n\t.$port"
		if {$direction == "Input"} {
			set module_connections [ format "$module_connections%s" "_to_" ] 
		} elseif {$direction == "Bidir"} {
			set module_connections [ format "$module_connections%s" "_to_and_from_" ] 
		} else {
			set module_connections [ format "$module_connections%s" "_from_" ] 
		}
		set module_connections [ format "$module_connections%s$module_name\t\($port\)," "the_" ] 
	}

	set dest_file [ open "$dest_path" "w" ]

	puts $dest_file $port_list
	puts $dest_file $port_declaration
	puts $dest_file $module_connections

	close $dest_file

}

