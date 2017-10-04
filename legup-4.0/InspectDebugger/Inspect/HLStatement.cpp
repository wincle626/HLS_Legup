/* 
 * File:   HLStatement.cpp
 * Author: nazanin
 * 
 * Created on June 27, 2013, 2:21 PM
 */

#include "HLStatement.h"

HLStatement::HLStatement() {
}

HLStatement::HLStatement( int id, int line_number, int column_number, std::string fileName) {
    this->id = id;
    this->line_number = line_number;
    this->start_column_number = column_number;
    this->fileName = fileName;
}

HLStatement::~HLStatement() {
}

