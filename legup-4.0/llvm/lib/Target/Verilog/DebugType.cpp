/* 
 * File:   DebugType.cpp
 * Author: nazanin
 * 
 * Created on March 1, 2014, 4:24 PM
 */

#include "DebugType.h"

using namespace legup;
using namespace llvm;

DebugType::DebugType() {
    this->byteSize = 0;
    this->numElements = 0;
}

DebugType::~DebugType() {
}

void DebugType::dump() {
    switch(this->type) {
        case 0://primitive int
            std::cout << "int ";
            break;
        case 1:
            std::cout << "float ";
            break;            
        case 2:
            std::cout << "double ";
            break;
        case 3:
            std::cout << "ptr ";
            break;
        case 4:
            std::cout << this->numElements << " * [";
            this->elementTypes[0]->dump();
            std::cout << "]";
            break;
        case 5:
            std::cout << "struct [";
            for (unsigned i = 0 ; i < this->elementTypes.size(); i++) {
                this->elementTypes[i]->dump();
                std::cout << " , ";
            }
            std::cout << "]";
            break;
            
    }
}

void DebugType::calculateByteSizes() {
    switch(this->type) {       
        case 0://primitive int
            //this->byteSize = 4;
            return;
            break;
        case 1:
            //this->byteSize = 4;
            return;
            break;
        case 2:
            //this->byteSize = 8;
            return;
            break;
        case 3:
            //this->byteSize = 4;
            return;
            break;
        case 4://array
            this->elementTypes[0]->calculateByteSizes();
            this->byteSize = this->numElements * this->elementTypes[0]->byteSize;
            return;
            break;
        case 5://struct
            std::vector<int> elemTypesSizes();
            for (unsigned i = 0 ; i < this->elementTypes.size(); i++) {
                this->elementTypes[i]->calculateByteSizes();
                this->byteSize += this->elementTypes[i]->byteSize;
            }
            return;
            break;
    }
}
