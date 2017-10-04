/* 
 * File:   HWSignal.cpp
 * Author: nazanin
 * 
 * Created on July 4, 2013, 2:21 PM
 */

#include "HWSignal.h"

HWSignal::HWSignal(int id, std::string name, int width, Function* func, bool isConst, Variable* variable) {
    this->id = id;
    this->name = name;
    this->width = width;
    this->value = "N/A";
    this->func = func;
    this->isConst = isConst;
    this->variable = variable;    
    this->previousValue = this->value = "N/A";
    this->previousValueStateId = this->valueStateId = -1;
}

HWSignal::~HWSignal() {
}

std::string HWSignal::getModuleName() {
    std::string name = this->func->name;
    return name + "_inst";
}

std::string HWSignal::getFPGAPartialNodeName() {
    return this->getModuleName() + "|" + this->name;
}

