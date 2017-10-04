/* 
 * File:   IRInstruction.h
 * Author: nazanin
 *
 * Created on June 19, 2013, 12:46 PM
 */

#ifndef IRINSTRUCTION_H
#define	IRINSTRUCTION_H

#include <string>
#include <vector>
#include "HWSignal.h"

class State;
class HWSignal;

class IRInstruction {
public:
    IRInstruction(int id, std::string dump);
    virtual ~IRInstruction();
        
    int id;
    std::string dump;
    std::vector<std::string> hardwareInfo;
    std::vector<HWSignal*> signalList;
    State* startState;
    State* endState;
    int function_id;
    int lineNumber;
    int HLInstr_id;
    
private:        

};

#endif	/* IRINSTRUCTION_H */