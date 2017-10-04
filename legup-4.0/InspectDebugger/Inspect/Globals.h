/* 
 * File:   Globals.h
 * Author: nazanin
 *
 * Created on July 6, 2014, 11:38 PM
 */

#ifndef GLOBALS_H
#define	GLOBALS_H

#include <string>
#include <vector>
#include <map>

//**** GLOBAL VARIABLES ****
//variables defined here has to be defined as extern in other project files in order to be used. This file is the last #include line in the main.cpp file

#ifdef PYTHON_WRAPPER
/* 
 * extern C - C linkage for shared object library compilation
 * definition for variables defined in main.cpp which is not defined elsewhere
 */

#ifdef TCPCLIENT_H
extern "C" { TCPClient tcpc("127.0.0.1", "2000");}
#endif
extern "C" { std::string modelsimListenerFilename = "ModelsimListener.tcl";}
extern "C" { std::string initilizeDesignTclFileName = "init.tcl";}
extern "C" { std::string vsimRunCommand = "vsim -do ";}

//these variables will be set from the Inspect.config file
extern "C" { std::string vsimDir, workDir, legUpDir, fileName, alteraMFLibPath;}
extern "C" { std::string dbHost, dbUser, dbPass, dbName;}
extern "C" { std::vector<std::string> alteraFPPaths;}

//these variable values willl be set in initialization (based on the fileName variable)
extern "C" { std::string designFilename, rawSourceFilename, codeFilename, SWBinaryFilename, stpFilename, csvFileName;}

extern "C" { std::string referenceSimulationDataFilename = "refsim.dat";}
extern "C" { std::string referenceSimulationCyclesToPathsFileName = "refsim_cycles_to_paths.dat";}
extern "C" { std::string onchipBugLogFilename = "onchip_bugs.log";}

extern "C" { bool runOnChip;}
extern "C" { bool writeOnChipDebugInfoOnFile;}
extern "C" { std::string onChipDebugInfoFileAddress = "onChipDebugInfo.dat";}
extern "C" { std::string statesToCyclesFileAddress;}
#ifdef DEBUGENGINE_H
extern "C" { OnChipDebugEngine *dbgEngine;}
#endif

extern "C" { std::string nodeNamesFilename = "nodenames.txt";}
extern "C" { std::string deviceInfoFileName = "deviceinfo.txt";}

//debug make script files
extern "C" { std::string dbgMakeFilePath = "dbgMake.sh";}
extern "C" { std::string increamentalDebugMakeFilePath = "incrementalDebug.sh";}


#ifdef DATAACCESS_H
extern "C" { DataAccess *DA;}
#endif
#ifdef IRINSTRUCTION_H
extern "C" { std::vector<IRInstruction*> IRInstructions;}
extern "C" { std::map<int, IRInstruction*> IRIdsToInstructions;}
#endif
#ifdef HLSTATEMENT_H
extern "C" { std::vector<HLStatement*> HLStatements;}
extern "C" { std::map<int, HLStatement*> HLIdsToStatements;}
extern "C" { std::map<int, std::vector<HLStatement*> > lineNumToStatement;//this is used to set end_col_nums
#endif
#ifdef HWSIGNAL_H
extern "C" { std::vector<HWSignal*> Signals;}
extern "C" { std::map<int, HWSignal*> IdsToSignals;}
#endif
#ifdef STATE_H
extern "C" { std::vector<State*> States;}
extern "C" { std::map<int, State*> IdsToStates;}
#endif
#ifdef VARIABLE_H
extern "C" { std::vector<Variable*> Variables;}
extern "C" { std::map<int, Variable*> IdsToVariables;}
#endif
#ifdef VARIABLETYPE_H
extern "C" { std::vector<VariableType*> VariableTypes;}
extern "C" { std::map<int, VariableType*> IdsToVariableTypes;}
#endif

#ifdef VARIABLE_H
#ifdef FUNCTION_H
extern "C" { std::map<Function*, std::vector<Variable*> > functionsToVariables;}
#endif
extern "C" { std::vector<Variable*> globalVariables;}
#endif

#ifdef VARIABLE_H
extern "C" { std::map<Variable*, VariableUpdateInfo*> variablesToUpdateInfo;}
#endif
#ifdef HLSTATEMENT_H
extern "C" { std::map<int, std::vector<HLStatement*> > statesToEffectiveStatements;}
#endif
#ifdef FUNCTION_H
extern "C" { std::vector<Function*> functions;}
extern "C" { std::map<int, Function*> IdsToFunctions;}
#endif
extern "C" { std::map<std::string, std::vector<int> > statesToCycles;}

extern "C" { int onChipDebugWindowSize;}

//recently added 
#ifdef ONCHIPSIGNAL_H
extern "C" { std::vector<OnChipSignal*> OnChipSignals;}
extern "C" { std::map<int, OnChipSignal*> IdsToOnChipSignals;}
#endif

#ifdef GDBWRAPPER_H
extern "C" { GDBWrapper *gdbWrapper;}
#endif
#ifdef STATE_H
extern "C" { std::vector<State*> observedStates;}
#endif
extern "C" { std::string simulationMainReturnVal;}
extern "C" { int unInitializedIntValue;}
extern "C" { float unInitializedFloatValue;}
extern "C" { double unInitializedDoubleValue;}
extern "C" { long long unInitializedLongLongValue;}

extern "C" { int cycle_counter;}
extern "C" { int dummy_cycle_counter;}

#else // PYTHON_WRAPPER
#ifdef TCPCLIENT_H
extern TCPClient tcpc;
#endif
extern std::string modelsimListenerFilename;
extern std::string initilizeDesignTclFileName;
extern std::string vsimRunCommand;

//these variables will be set from the Inspect.config file
extern std::string vsimDir, workDir, legUpDir, fileName, alteraMFLibPath;
extern std::string dbHost, dbUser, dbPass, dbName;
extern std::vector<std::string> alteraFPPaths;

//these variable values willl be set in initialization (based on the fileName variable)
extern std::string designFilename, rawSourceFilename, codeFilename, SWBinaryFilename, stpFilename, csvFileName;

extern std::string referenceSimulationDataFilename;
extern std::string referenceSimulationCyclesToPathsFileName;
extern std::string onchipBugLogFilename;

extern bool runOnChip;
extern bool writeOnChipDebugInfoOnFile;
extern std::string onChipDebugInfoFileAddress;
extern std::string statesToCyclesFileAddress;
#ifdef DEBUGENGINE_H
extern OnChipDebugEngine *dbgEngine;
#endif

extern std::string nodeNamesFilename;
extern std::string deviceInfoFileName;

//debug make script files
extern std::string dbgMakeFilePath;
extern std::string increamentalDebugMakeFilePath;


#ifdef DATAACCESS_H
extern DataAccess *DA;
#endif
#ifdef IRINSTRUCTION_H
extern std::vector<IRInstruction*> IRInstructions;
extern std::map<int, IRInstruction*> IRIdsToInstructions;
#endif
#ifdef HLSTATEMENT_H
extern std::vector<HLStatement*> HLStatements;
extern std::map<int, HLStatement*> HLIdsToStatements;
extern std::map<int, std::vector<HLStatement*> > lineNumToStatement;//this is used to set end_col_nums
#endif
#ifdef HWSIGNAL_H
extern std::vector<HWSignal*> Signals;
extern std::map<int, HWSignal*> IdsToSignals;
#endif
#ifdef STATE_H
extern std::vector<State*> States;
extern std::map<int, State*> IdsToStates;
#endif
#ifdef VARIABLE_H
extern std::vector<Variable*> Variables;
extern std::map<int, Variable*> IdsToVariables;
#endif
#ifdef VARIABLETYPE_H
extern std::vector<VariableType*> VariableTypes;
extern std::map<int, VariableType*> IdsToVariableTypes;
#endif

#ifdef VARIABLE_H
#ifdef FUNCTION_H
extern std::map<Function*, std::vector<Variable*> > functionsToVariables;
#endif
extern std::vector<Variable*> globalVariables;
#endif

#ifdef VARIABLE_H
extern std::map<Variable*, VariableUpdateInfo*> variablesToUpdateInfo;
#endif
#ifdef HLSTATEMENT_H
extern std::map<int, std::vector<HLStatement*> > statesToEffectiveStatements;
#endif
#ifdef FUNCTION_H
extern std::vector<Function*> functions;
extern std::map<int, Function*> IdsToFunctions;
#endif
extern std::map<std::string, std::vector<int> > statesToCycles;

extern int onChipDebugWindowSize;

//recently added 
#ifdef ONCHIPSIGNAL_H
extern std::vector<OnChipSignal*> OnChipSignals;
extern std::map<int, OnChipSignal*> IdsToOnChipSignals;
#endif

#ifdef GDBWRAPPER_H
extern GDBWrapper *gdbWrapper;
#endif
#ifdef STATE_H
extern std::vector<State*> observedStates;
#endif
extern std::string simulationMainReturnVal;
extern int unInitializedIntValue;
extern float unInitializedFloatValue;
extern double unInitializedDoubleValue;
extern long long unInitializedLongLongValue;

extern int cycle_counter;
extern int dummy_cycle_counter;
#endif // PYTHON_WRAPPER


#endif	/* GLOBALS_H */
