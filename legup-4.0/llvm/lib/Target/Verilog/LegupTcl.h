//===-- Tcl.h - Tcl Parser --------------------------------------*- C++ -*-===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the configuration file tcl parser
//
//===----------------------------------------------------------------------===//

#ifndef LEGUP_TCL_H
#define LEGUP_TCL_H

#include <tcl.h>

namespace legup {
class LegupConfig;

int set_custom_top_level_module(ClientData clientData, Tcl_Interp *interp, int
	argc, const char *argv[]);

int set_custom_test_bench_module(ClientData clientData, Tcl_Interp *interp, int
	argc, const char *argv[]);
  
int set_custom_verilog_file(ClientData clientData, Tcl_Interp *interp, int 
	argc, const char *argv[]);

int set_custom_verilog_function(ClientData clientData, Tcl_Interp *interp, int 
	argc, const char *argv[]);

int set_accelerator_function(ClientData clientData, Tcl_Interp *interp, int
        argc, const char *argv[]);

int set_parallel_accelerator_function(ClientData clientData, Tcl_Interp *interp, int
        argc, const char *argv[]);
        
int set_dcache_size(ClientData clientData, Tcl_Interp *interp, int
        argc, const char *argv[]);

int set_dcache_linesize(ClientData clientData, Tcl_Interp *interp, int
        argc, const char *argv[]);

int set_dcache_way(ClientData clientData, Tcl_Interp *interp, int
        argc, const char *argv[]);

int set_icache_size(ClientData clientData, Tcl_Interp *interp, int
        argc, const char *argv[]);

int set_icache_linesize(ClientData clientData, Tcl_Interp *interp, int
        argc, const char *argv[]);

int set_icache_way(ClientData clientData, Tcl_Interp *interp, int
        argc, const char *argv[]);

int set_dcache_ports(ClientData clientData, Tcl_Interp *interp, int
        argc, const char *argv[]);

int set_dcache_type(ClientData clientData, Tcl_Interp *interp, int
        argc, const char *argv[]);

int set_operation_attributes(ClientData clientData, Tcl_Interp *interp, int
	argc, const char *argv[]);
	
int set_device_specs(ClientData clientData, Tcl_Interp *interp, int
	argc, const char *argv[]);

int use_debugger(ClientData clientData, Tcl_Interp *interp, int
    argc, const char *argv[]);

int debugger_capture_mode(ClientData clientData, Tcl_Interp *interp, int
    argc, const char *argv[]);

bool parseTclFile(std::string &ConfigFile, LegupConfig *legupConfig);

bool parseTclString(std::string &commands, LegupConfig *legupConfig);

} // End legup namespace

#endif
