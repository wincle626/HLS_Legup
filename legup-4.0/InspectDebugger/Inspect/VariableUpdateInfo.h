/* 
 * File:   VariableUpdateInfo.h
 * Author: nazanin
 *
 * Created on August 2, 2013, 1:53 PM
 */

#ifndef VARIABLEUPDATEINFO_H
#define	VARIABLEUPDATEINFO_H

#include <vector>

#include "HWSignal.h"
#include "State.h"

class VariableUpdateInfo {
public:    
    VariableUpdateInfo(int infoID, HWSignal* signal, std::string memoryPort, int offset, int lineNumber);
    virtual ~VariableUpdateInfo();
    int infoID;
    HWSignal* signal;
    std::string port;
    int offset;
    int lineNumber;
    
private:

};

#endif	/* VARIABLEUPDATEINFO_H */

