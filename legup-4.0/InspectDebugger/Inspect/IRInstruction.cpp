/* 
 * File:   IRInstruction.cpp
 * Author: nazanin
 * 
 * Created on June 19, 2013, 12:46 PM
 */

#include "IRInstruction.h"

IRInstruction::IRInstruction(int id, std::string dump) {
    this->id = id;
    this->dump = dump;
    this->lineNumber = -1;
    this->HLInstr_id = 0;
}

IRInstruction::~IRInstruction() {
}