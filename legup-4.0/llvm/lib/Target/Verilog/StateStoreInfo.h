/* 
 * File:   StateStoreInfo.h
 * Author: nazanin
 *
 * Created on February 23, 2014, 2:17 PM
 */

#ifndef STATESTOREINFO_H
#define	STATESTOREINFO_H

//#include "Allocation.h"
#include "RTL.h"
#include "State.h"

namespace legup {

class StateStoreInfo {
public:
    StateStoreInfo(State* state, std::string port, RTLSignal* addressSignal, int adrOffsetValue, Instruction* IR);
    virtual ~StateStoreInfo();
    
    State* state;
    std::string port;
    RTLSignal* addressSignal;
    int adrOffsetValue;
    Instruction* IR;    
private:

};

}

#endif	/* STATESTOREINFO_H */

