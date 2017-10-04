/* 
 * File:   VariableUpdateInfo.cpp
 * Author: nazanin
 * 
 * Created on August 2, 2013, 1:53 PM
 */

#include "VariableUpdateInfo.h"
#include "Globals.h"

VariableUpdateInfo::VariableUpdateInfo(int infoID, HWSignal* signal, std::string memoryPort, int offset, int lineNumber) {
    this->infoID = infoID;
    this->signal = signal;
    this->port = memoryPort;
    this->offset = offset;
    this->lineNumber = lineNumber;
}

VariableUpdateInfo::~VariableUpdateInfo() {
}

