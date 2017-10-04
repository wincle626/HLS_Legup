/* 
 * File:   State.h
 * Author: nazanin
 *
 * Created on July 10, 2013, 8:38 AM
 */

#ifndef STATE_H
#define	STATE_H

#include <string>
#include <vector>

#include "Function.h"

class Variable;
class VariableUpdateInfo;
class HWSignal;

class State {
public:
    State(int id, int number, std::string name, Function* belongingFunction, Function* calledFunction);
    virtual ~State();
    
    int id;
    int number;
    std::string name;    
    std::vector<VariableUpdateInfo*> updatingVariables;
    Function* belongingFunction;
    Function* calledFunction;        
    
    bool isCallState() { return calledFunction != NULL; }
    void sortUpdatingVariablesByLineNumber();
    
    
private:

};

#endif	/* STATE_H */

