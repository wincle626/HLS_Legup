    set sopc "/home/choijon5/altera/11.1/quartus//sopc_builder"
    set sopc_perl "/home/choijon5/altera/11.1/quartus//linux/perl"
    echo "Sopc_Builder Directory: $sopc";

# ModelSimPE and OEM have different requirements
# regarding how they simulate their test bench.
# We account for that here.
if { [ string match "*ModelSim ALTERA*" [ vsim -version ] ] } {
 alias _init_setup {puts -nonewline ""} } else {
 alias _init_setup {
                       vmap lpm_ver       work
                       vmap altera        work
                       vmap altera_mf_ver work
                       vmap sgate_ver     work
                       } } 


# ModelSimPE and OEM have different requirements
# regarding how they simulate their test bench.
# We account for that here.
if { [ string match "*ModelSim ALTERA*" [ vsim -version ] ] } {
 alias _vsim {vsim -t ps +nowarnTFMPC  -L lpm_ver -L sgate_ver -L altera_mf_ver -L altgxb_ver -L stratixiigx_hssi_ver -L stratixgx_ver -L stratixgx_gxb_ver -L stratixiigx -L altera_ver -L stratixiii_ver -L stratixii_ver -L cycloneii_ver -L cycloneiii_ver -L stratixiv_hssi_ver -L arriaii_ver -L arriaii_pcie_hip_ver -L arriaii_hssi_ver -L stratixiv_ver -L stratixiv_pcie_hip_ver -L cycloneiv_pcie_hip_ver -L cycloneiv_hssi_ver -L hardcopyiv_pcie_hip_ver -L hardcopyiv_hssi_ver -L stratixv_ver -L stratixv_hssi_ver -L stratixv_pcie_hip_ver -L altera_lnsim_ver -pli /home/choijon5/altera/11.1/ip/altera/sopc_builder_ip/altera_avalon_jtag_phy/libbytestream_pli.so test_bench }  } else {
 alias _vsim {vsim -t ps +nowarnTFMPC -pli /home/choijon5/altera/11.1/ip/altera/sopc_builder_ip/altera_avalon_jtag_phy/libbytestream_pli.so test_bench }  } 

alias test_contents_files {if {[ file exists "contents_file_warning.txt" ]} { set ch [open "contents_file_warning.txt" r];  while { 1 } { if ([eof $ch]) {break}; gets $ch line; puts $line; }; close $ch; } }
alias s "vlib work;
_init_setup
 
 
vlog +incdir+.. ../tiger.v;
_vsim;
do virtuals.do
; test_contents_files"
alias r "exec $sopc_perl/bin/perl -I $sopc/bin/perl_lib -I $sopc/bin $sopc/bin/run_command_in_unix_like_shell.pl $sopc { cd ../;  ./tiger_generation_script  } "
alias c "echo {Regenerating memory contents.
 (This may take a moment)...}; restart -f; exec $sopc_perl/bin/perl -I $sopc/bin/perl_lib -I $sopc/bin $sopc/bin/run_command_in_unix_like_shell.pl $sopc { cd ../;  ./tiger_generation_script  }  --software_only=1"
alias w "do wave_presets.do"
alias l "do list_presets.do"
#
# s_cycloneiv: compile simulation models for cyclone 4.
proc s_cycloneiv {} {
  if { [ expr ! [ string match "*ModelSim ALTERA*" [ vsim -version ] ] ] } {
    vlib work
    _init_setup
    set qsimlib /home/choijon5/altera/11.1/quartus/eda/sim_lib
    set v_files [ list \
      $qsimlib/cycloneiv_atoms.v \
      $qsimlib/cycloneiv_hssi_atoms.v \
      $qsimlib/cycloneiv_pcie_hip_atoms.v \
      $qsimlib/cycloneive_atoms.v \
    ]
    foreach file $v_files  {if {[ file exists $file ]} {vlog $file}}
  }
}
#
# s_stratixiv: compile simulation models for stratix 4.
proc s_stratixiv {} {
  if { [ expr ! [ string match "*ModelSim ALTERA*" [ vsim -version ] ] ] } {
    vlib work
    _init_setup
    set qsimlib /home/choijon5/altera/11.1/quartus/eda/sim_lib
    set v_files [ list \
      $qsimlib/stratixiv_atoms.v \
      $qsimlib/stratixiv_pcie_hip_atoms.v \
      $qsimlib/stratixiv_hssi_atoms.v \
    ]
    foreach file $v_files  {if {[ file exists $file ]} {vlog $file}}
  }
}
#
# s_stratixv: compile simulation models for stratix 5.
proc s_stratixv {} {
  if { [ expr ! [ string match "*ModelSim ALTERA*" [ vsim -version ] ] ] } {
    vlib work
    _init_setup
    set qsimlib /home/choijon5/altera/11.1/quartus/eda/sim_lib
    set vendor_sv_files [ list \
      $qsimlib/mentor/stratixv_pcie_hip_atoms_ncrypt.v \
    ]
    set vendor_v_files [ list \
      $qsimlib/mentor/stratixv_atoms_ncrypt.v \
      $qsimlib/mentor/stratixv_hssi_atoms_ncrypt.v \
      $qsimlib/mentor/stratixv_atoms_for_vhdl.v \
      $qsimlib/mentor/stratixv_hssi_atoms_for_vhdl.v \
    ]
    foreach file $vendor_sv_files {if {[ file exists $file ]} {vlog -sv $file}}
    foreach file $vendor_v_files  {if {[ file exists $file ]} {vlog $file}}
    set v_files [ list \
      $qsimlib/stratixv_atoms.v \
      $qsimlib/stratixv_hssi_atoms.v \
    ]
    set sv_files [ list \
      $qsimlib/stratixv_pcie_hip_atoms.v \
      $qsimlib/altera_lnsim.sv \
    ]
    foreach file $sv_files {if {[ file exists $file ]} {vlog -sv $file}}
    foreach file $v_files  {if {[ file exists $file ]} {vlog $file}}
  }
}
alias h "
echo @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
echo @@
echo @@        setup_sim.do
echo @@
echo @@   Defined aliases:
echo @@
echo @@   s  -- Load all design (HDL) files.
echo @@           re-vlog/re-vcom and re-vsim the design.
echo @@
echo @@   s_cycloneiv -- For Modelsim SE, compile Cyclone IV models. (Ignored
echo @@     in Modelsim AE.)
echo @@
echo @@   s_stratixiv -- For Modelsim SE, compile Stratix IV models. (Ignored
echo @@     in Modelsim AE.)
echo @@
echo @@   s_stratixv -- For Modelsim SE, compile Stratix V models. (Ignored
echo @@     in Modelsim AE.)
echo @@
echo @@   w  -- Sets-up waveforms for this design
echo @@          Each SOPC-Builder component may have
echo @@          signals 'marked' for display during
echo @@          simulation.  This command opens a wave-
echo @@          window containing all such signals.
echo @@
echo @@   l  -- Sets-up list waveforms for this design
echo @@          Each SOPC-Builder component may have
echo @@          signals 'marked' for listing during
echo @@          simulation.  This command opens a list-
echo @@          window containing all such signals.
echo @@
echo @@   h  -- print this message 
echo @@
echo @@"

h
