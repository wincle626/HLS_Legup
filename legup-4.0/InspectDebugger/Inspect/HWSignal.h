/* 
 * File:   HWSignal.h
 * Author: nazanin
 *
 * Created on July 4, 2013, 2:21 PM
 */

#ifndef HWSIGNAL_H
#define	HWSIGNAL_H

#include <string>

#include "Function.h"
#include "State.h"
#include "Variable.h"

class HWSignal {
public:
    HWSignal(int id, std::string name, int width, Function* func, bool isConst, Variable* variable);
    virtual ~HWSignal();
    
    int id;
    std::string name;
    int width;
    bool isConst;
    Variable* variable;
    
    Function* func;
    std::vector<State*> states;
    
    //this method returns the module name in which the signal is defined in verilog code
    std::string getModuleName();
    std::string getFPGAPartialNodeName();
    
    std::string getValue() { return this->value; }
    std::string getPreviousValue() { return this->previousValue; }
    void setValue(std::string val, int stateId) {
        if (stateId == valueStateId)
            return;
        else {
            previousValueStateId = valueStateId;
            valueStateId = stateId;
            
            this->previousValue = this->value;
            this->value = val;
        }
    }    
    
private:    
    std::string value;
    std::string previousValue;
    int valueStateId;
    int previousValueStateId;
};

#endif	/* HWSIGNAL_H */
