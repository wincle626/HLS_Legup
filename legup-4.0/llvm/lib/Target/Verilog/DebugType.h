/* 
 * File:   DebugType.h
 * Author: nazanin
 *
 * Created on March 1, 2014, 4:24 PM
 */

#ifndef DEBUGTYPE_H
#define	DEBUGTYPE_H

#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include <iostream>

using namespace llvm;

namespace legup {

class DebugType {
public:
    DebugType();
    virtual ~DebugType();
    
    int type;//0: primitive int, 1: primitive float, 2: primitive double, 3: pointer type, 4: array, 5: struct
    std::vector<DebugType*> elementTypes;
    int numElements;//only for array
    int byteSize;
    void dump();
    void calculateByteSizes();
private:

};

}

#endif	/* DEBUGTYPE_H */

