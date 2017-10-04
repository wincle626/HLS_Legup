/* 
 * File:   GDBWrapper.h
 * Author: nazanin
 *
 * Created on January 23, 2014, 10:32 PM
 */

#ifndef GDBWRAPPER_H
#define	GDBWRAPPER_H

#include <mi_gdb.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include "Function.h"
#include "VariableType.h"
#include <map>

class GDBWrapper;

/*
extern GDBWrapper *gdbWrapper;
extern std::string SWBinaryFilename;
extern std::string codeFilename;
extern std::string workDir;
extern std::vector<Function*> functions;
*/

class GDBWrapper {
public:
    GDBWrapper();    
    virtual ~GDBWrapper();
    
    void test_gdb();
    void initialize();
    void finalize();
    void doStepping();
    void checkFunctionReturn();
    void runForFirstTime();
    std::string normalizeVariableNameForGDB(std::string varName);
    std::string examineGlobalVariable(std::string varName, Type varType, int elemCount);    
    std::string examineVariable(std::string varName, Type varType, int elemCount);
    std::string examinePointerToArray(std::string varName, int elemCount);
    
    //std::string examineVariable(std::string varName);
    
    bool changeFrame(std::string fName);
    
    int currentGDBLine;
    int previousGDBLine;
    mi_frames *frames;
    bool programExited;
    bool programStarted;
    
    int callerLineNumber;
    bool isReturning;
    
private:    
    int currStackFrameDepth;
    int prevStackFrameDepth;
    
    int currCallerLineNumber;
    int prevCallerLineNumber;   
    
    mi_aux_term *child_vt;
    mi_h *h;    
    
    std::ofstream stackFrameFile;
    
    std::map<std::string, int> functionNamesToFrameNumbers;

};

#endif	/* GDBWRAPPER_H */
