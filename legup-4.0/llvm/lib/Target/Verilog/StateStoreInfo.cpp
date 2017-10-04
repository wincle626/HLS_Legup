/* 
 * File:   StateStoreInfo.cpp
 * Author: nazanin
 * 
 * Created on February 23, 2014, 2:17 PM
 */

#include "StateStoreInfo.h"

using namespace legup;
using namespace llvm;

StateStoreInfo::StateStoreInfo(State* state, std::string port, RTLSignal* addressSignal, int adrOffsetValue, Instruction* IR) {
    this->state = state;
    this->port = port;
    this->addressSignal = addressSignal;
    this->adrOffsetValue = adrOffsetValue;
    this->IR = IR;
    
}
StateStoreInfo::~StateStoreInfo() {
}

