/* 
 * File:   OnChipSignal.cpp
 * Author: nazanin
 * 
 * Created on October 25, 2013, 4:35 PM
 */

#include "OnChipSignal.h"

OnChipSignal::OnChipSignal(int id, std::string name) {
    this->id = id;
    this->name = name;
}

OnChipSignal::OnChipSignal(int id, std::string name, std::vector<int> values) {
    this->id = id;
    this->name = name;
    this->values = values;
}

OnChipSignal::OnChipSignal(const OnChipSignal& orig) {
}

OnChipSignal::~OnChipSignal() {
}

