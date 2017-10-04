/* 
 * File:   VariableType.h
 * Author: nazanin
 *
 * Created on March 17, 2014, 4:02 PM
 */

#ifndef VARIABLETYPE_H
#define	VARIABLETYPE_H

#include <vector>

class VariableType;

enum Type {
    PRIMITIVE_INT,
    PRIMITIVE_FLOAT,
    PRIMITIVE_DOUBLE,
    POINTER,
    ARRAY,
    STRUCT
};

class VariableType {
public:
    
    VariableType() {
        
    }
    
    VariableType(int id, Type type, int numElements, int byteSize) {
        this->id = id;
        this->type = type;
        this->numElements = numElements;
        this->byteSize = byteSize;
    }
    int id;
    Type type;
    int numElements;
    int byteSize;
    std::vector<VariableType*> elementTypes;
};

#endif	/* VARIABLETYPE_H */

