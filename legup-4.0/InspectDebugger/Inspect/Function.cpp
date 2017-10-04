/* 
 * File:   Function.cpp
 * Author: nazanin
 * 
 * Created on August 15, 2013, 10:02 AM
 */

#include "Function.h"

Function::Function(int id, std::string name, int startLineNumber) {
    this->id = id;
    this->name = name;
    this->startLineNumber = startLineNumber;
}

Function::~Function() {
}

