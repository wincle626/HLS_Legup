/* 
 * File:   State.cpp
 * Author: nazanin
 * 
 * Created on July 10, 2013, 8:38 AM
 */

#include "State.h"
#include "VariableUpdateInfo.h"
#include "Globals.h"

State::State(int id, int number, std::string name, Function* belongingFunction, Function* calledFunction) {
    this->id = id;
    this->number = number;
    this->name = name;
    this->belongingFunction = belongingFunction;
    this->calledFunction = calledFunction;
}

State::~State() {
}
//
bool compareByLineNumber(VariableUpdateInfo* v1, VariableUpdateInfo* v2) {
    return (v1->lineNumber < v2->lineNumber);
}

void State::sortUpdatingVariablesByLineNumber() {
    std::sort(this->updatingVariables.begin(), this->updatingVariables.end(), compareByLineNumber);
}

