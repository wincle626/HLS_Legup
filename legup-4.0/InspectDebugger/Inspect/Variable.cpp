/* 
 * File:   Variable.cpp
 * Author: nazanin
 * 
 * Created on July 11, 2013, 10:57 PM
 */

#include "Variable.h"
#include "Globals.h"

Variable::Variable(int id, int functionId, std::string name, std::string tag, int tagNum, std::string tagAddressName, int addressWidth, std::string mifFileName, int dataWidth, int numElements, bool isStruct, IRInstruction* IR, VariableType* vt) {
    this->id = id;
    this->functionId = functionId;
    this->name = name;    
    this->tag = tag;
    this->tagNum = tagNum;
    this->tagAddressName = tagAddressName;
    this->addressWidth = addressWidth;
    this->mifFileName = mifFileName;
    this->dataWidth = dataWidth;
    this->numElements = numElements;
    this->isStruct = isStruct;
    this->IR = IR;
    this->type = vt;    
    
    this->isGlobal = this->functionId == 1;    
}

Variable::~Variable() {
}

void Variable::InitializeContainers() {    
    this->HWValues.resize(numElements);//TODO: in case of struct, the numelement is not correct... for now, I set the size constant 1000    
    
    this->HWPlainValues.resize(numElements);
    
    this->HWPlainValueCycles.resize(numElements);            
        
    this->lastActiveValues.resize(numElements);
    for (int i = 0; i < this->lastActiveValues.size(); i++)
        this->lastActiveValues[i].setIsInitialized(false);
    
    this->lastSetValues.resize(numElements);
    for (int i = 0; i < this->lastSetValues.size(); i++) {
        this->lastSetValues[i].first = -1;
        this->lastSetValues[i].second.setIsInitialized(false);
    }
}
    
bool Variable::isPrimitive(Type type) {
    switch(type) {
        case PRIMITIVE_INT:
        case PRIMITIVE_FLOAT:
        case PRIMITIVE_DOUBLE:
        case POINTER:
            return true;
        default:
            return false;
    }
}

int Variable::calculateNumElements(VariableType* vt) {
    switch (vt->type) {
        case PRIMITIVE_INT:
        case PRIMITIVE_FLOAT:
        case PRIMITIVE_DOUBLE:
        case POINTER:
            return 1;
        case ARRAY: {                                
                int ret = calculateNumElements(vt->elementTypes[0]) * vt->numElements;
            return ret;
            break;
        }            
        case STRUCT: {
            int ret = 0;
            for (int i = 0; i < vt->elementTypes.size(); i++)
                ret += calculateNumElements(vt->elementTypes[i]);
            return ret;
            break;
        }
    }
    return 0;
}

bool Variable::findTypeByIndex(VariableType* type, int &currentIndex, int targetIndex, VariableType **varType) {
    if (currentIndex == targetIndex) {
        if (isPrimitive(type->type))
            *varType = type;
        else {
            VariableType* vt = type;
            do {                    
                vt = vt->elementTypes[0];
                *varType = vt;
            } while (!isPrimitive(vt->type));
        }
        return true;
    }

    switch(type->type) {
        case PRIMITIVE_INT:
        case PRIMITIVE_FLOAT:
        case POINTER: {
            currentIndex += 1;
            *varType = type;
            if (currentIndex == targetIndex)
                return true;
            break;
        }
        case PRIMITIVE_DOUBLE: {
            currentIndex += 1;
            *varType = type;
            if (currentIndex == targetIndex)
                return true;
            break;
        }
        case ARRAY: {
            VariableType* elementType = type->elementTypes[0];
            if (isPrimitive(elementType->type)) {
                for (int i = 0 ; i < type->numElements; i++) {
                    if (elementType->type == PRIMITIVE_DOUBLE)
                        currentIndex += 1;
                    else
                        currentIndex += 1;
                    *varType = elementType;
                    if (currentIndex == targetIndex)
                        return true;
                }
            } else {
                for (int i = 0; i < type->numElements; i++) {
                    if (findTypeByIndex(type->elementTypes[0], currentIndex, targetIndex, varType))
                        return true;
                }
            }
            break;
        }
        case STRUCT: {
            for (int i = 0; i < type->numElements; i++) {
                if (findTypeByIndex(type->elementTypes[i], currentIndex, targetIndex, varType))
                    return true;
            }
            break;
        }
    }
    return false;
}

bool Variable::findType(VariableType* type, int &currentByteOffset, int targetByteOffset, Type &varType) {
    if (currentByteOffset == targetByteOffset) {
        if (isPrimitive(type->type))
            varType = type->type;
        else {
            VariableType* vt = type;
            do {                    
                vt = vt->elementTypes[0];
                varType = vt->type;
            } while (!isPrimitive(vt->type));
        }
        return true;
    }

    switch(type->type) {
        case PRIMITIVE_INT:
        case PRIMITIVE_FLOAT:
        case POINTER: {
            //currentByteOffset += 4;
            currentByteOffset += type->byteSize;
            varType = type->type;
            if (currentByteOffset == targetByteOffset)
                return true;
            break;
        }
        case PRIMITIVE_DOUBLE: {
            //currentByteOffset += 8;
            currentByteOffset += type->byteSize;
            varType = PRIMITIVE_DOUBLE;
            if (currentByteOffset == targetByteOffset)
                return true;
            break;
        }
        case ARRAY: {
            Type elementType = type->elementTypes[0]->type;
            if (isPrimitive(elementType)) {
                for (int i = 0 ; i < type->numElements; i++) {
                    if (elementType == PRIMITIVE_DOUBLE)
                        //currentByteOffset += 8;
                        currentByteOffset += type->byteSize;
                    else
                        //currentByteOffset += 4;
                        currentByteOffset += type->byteSize;
                    varType = elementType;
                    if (currentByteOffset == targetByteOffset)
                        return true;
                }
            } else {
                for (int i = 0; i < type->numElements; i++) {
                    if (findType(type->elementTypes[0], currentByteOffset, targetByteOffset, varType))
                        return true;
                }
            }
            break;
        }
        case STRUCT: {
            for (int i = 0; i < type->numElements; i++) {
                if (findType(type->elementTypes[i], currentByteOffset, targetByteOffset, varType))
                    return true;
            }
            break;
        }
    }
    return false;
}

bool Variable::findIndex(VariableType* type, int &currentByteOffset, int targetByteOffset, int &varIndex) {
    if (currentByteOffset == targetByteOffset)
        return true;

    switch(type->type) {
        case PRIMITIVE_INT:
        case PRIMITIVE_FLOAT:
        case POINTER: {
            //currentByteOffset += 4;
            currentByteOffset += type->byteSize;
            varIndex++;
            if (currentByteOffset == targetByteOffset)
                return true;
            break;
        }
        case PRIMITIVE_DOUBLE: {
            //currentByteOffset += 8;
            currentByteOffset += type->byteSize;
            varIndex++;
            if (currentByteOffset == targetByteOffset)
                return true;
            break;
        }
        case ARRAY: {
            Type elementType = type->elementTypes[0]->type;
            if (isPrimitive(elementType)) {
                for (int i = 0 ; i < type->numElements; i++) {
                    if (elementType == PRIMITIVE_DOUBLE)
                        //currentByteOffset += 8;
                        currentByteOffset += type->elementTypes[0]->byteSize;
                    else
                        //currentByteOffset += 4;
                        currentByteOffset += type->elementTypes[0]->byteSize;
                    varIndex++;
                    if (currentByteOffset == targetByteOffset)
                        return true;
                }
            } else {
                for (int i = 0; i < type->numElements; i++) {
                    if (findIndex(type->elementTypes[0], currentByteOffset, targetByteOffset, varIndex))
                        return true;
                }
            }
            break;
        }
        case STRUCT: {
            for (int i = 0; i < type->numElements; i++) {
                if (findIndex(type->elementTypes[i], currentByteOffset, targetByteOffset, varIndex))
                    return true;
            }
            break;
        }
    }
    return false;
}

int Variable::getVarIndexByByteIndex(int byteOffset) {        
    int varIndex = 0;
    int currentByteOffset = 0;
    findIndex(this->type, currentByteOffset, byteOffset, varIndex);
    return varIndex;        
}

Type Variable::getVarTypeByByteIndex(int byteOffset) {
    Type varType;
    int currentByteOffset = 0;
    findType(this->type, currentByteOffset, byteOffset, varType);
    return varType;
}

VariableType* Variable::getVarTypeByIndex(int index) {
    VariableType* varType;
    int currentIndex = 0;
    findTypeByIndex(this->type, currentIndex, index, &varType);
    return varType;
}        

std::string Variable::getHWValueStr(int index, int gdbPreviousLineNumber, bool sign) {
    if (this->HWValues[index].find(gdbPreviousLineNumber) != this->HWValues[index].end()) {                      

        IntFloatWrapper ret = this->HWValues[index][gdbPreviousLineNumber];

        //the value for this line number should be removed after that it is read one time.. this is because we may return to this line number but for another variable and not this one. so for this variable we should get the default value.
        //think of it as a loop that updates different indices of a variable in each iteration in exactly the same line. when you set arr[1] one time and you're coming in the same line to set arr[2] you don't want to get the value which is in here for arr[1].
        this->HWValues[index].erase(this->HWValues[index].find(gdbPreviousLineNumber));

        //lastActiveLines[index] = gdbPreviousLineNumber;
        lastActiveValues[index] = ret;//this is a by value copy... so it should be fine.
        if (sign)
            return ret.toString();
        else
            return ret.toStringUnsigned();
    } else {
        //IntFloatWrapper ret = this->HWValues[index][lastActiveLines[index]];//initial value
        IntFloatWrapper ret = this->lastActiveValues[index];
        if (sign)
            return ret.toString();
        else
            return ret.toStringUnsigned();
    }
}

IntFloatWrapper Variable::getHWValue(int index, int line) {
    return this->HWValues[index][line];
}

std::string Variable::getLastHWValue() {                
    return "NOT IMPLEMENTED";
    /*if (this->HWValues.size() == 0)
        return "N/A";
    return this->HWValues.back();*/
}    

void Variable::updateHWPlainValues(int currentUpdatingLine, int index) {

    for (int i = 0; i < this->numElements; i++) {
        std::string entry = "";
        bool isComplex = false;

        entry += lastSetValues[i].second.toString() + "|" + lastSetValues[i].second.toStringUnsigned();
        HWPlainValues[i].push_back(std::make_pair(currentUpdatingLine, entry));
        HWPlainValueCycles[i].push_back(std::make_pair(currentUpdatingLine, lastSetValues[i].second.getCycle()));
    }                        
}

void Variable::setHWValue(std::string val) {
    //this->HWValues.push_back(val);
}        

void Variable::setHWValueWithHex(int index, std::string hexValue, int line, bool updateLastActiveLine) {
    VariableType* varType = this->getVarTypeByIndex(index);                

    switch(varType->type) {
        case PRIMITIVE_INT: {
            if (varType->byteSize == 8) {
                long long value = hexToLongLong(hexValue);
                setHWValue(index, value, line, updateLastActiveLine);
            } else if (varType->byteSize == 4) {
                int value = hexToInt(hexValue);
                setHWValue(index, value, line, updateLastActiveLine);
            } else {
                short value = hexToInt(hexValue);
                setHWValue(index, value, line, updateLastActiveLine);
            }
            break;
        }
        case PRIMITIVE_FLOAT: {
            float value = hexToFloat(hexValue);
            setHWValue(index, value, line, updateLastActiveLine);
            break;
        }
        case PRIMITIVE_DOUBLE: {
            double value = hexToDouble(hexValue);
            setHWValue(index, value, line, updateLastActiveLine);
            break;
        }
        case POINTER: {
            if (varType->byteSize == 8) {
                long long value = hexToLongLong(hexValue);
                setHWValue(index, value, line, updateLastActiveLine);

            } else if (varType->byteSize == 4) {
                int value = hexToInt(hexValue);
                setHWValue(index, value, line, updateLastActiveLine);
            } else {
                short value = hexToInt(hexValue);
                setHWValue(index, value, line, updateLastActiveLine);
            }
            break;
        }
        default:
            //assert("Detected Type must be a primitive type" && false);
            break;                
    }                        
}

void Variable::setHWValueAsUnInitialized(int index) {
    this->HWValues[index][-1].setIsInitialized(false);
    this->HWValues[index][-1].setCycle(cycle_counter);
    this->lastActiveValues[index] = this->HWValues[index][-1];
    //std::cout << name << "[" << index << "] (line: -1) = N/A" << std::endl;
}

void Variable::setHWValue(int index, double value, int line, bool updateLastActiveValue) {

    this->HWValues[index][line].setVal(value);
    this->HWValues[index][line].setCycle(cycle_counter);
    //std::cout << name << "[" << index << "] (line: " << line << ") = " << value << std::endl;        

    if (updateLastActiveValue)            
        this->lastActiveValues[index] = this->HWValues[index][line];

    lastSetValues[index].first = line;
    lastSetValues[index].second = this->HWValues[index][line];
}

void Variable::ResetLastSetValues() {
    for (int i = 0; i < lastSetValues.size(); i++) {
        lastSetValues[i].first = -1;
        lastSetValues[i].second.setIsInitialized(false);
    }
}

void Variable::setHWValue(int index, long long value, int line, bool updateLastActiveValue) {         

    this->HWValues[index][line].setVal(value);
    this->HWValues[index][line].setCycle(cycle_counter);
    //std::cout << name << "[" << index << "] (line: " << line << ") = " << value << std::endl;

    if (updateLastActiveValue)            
        this->lastActiveValues[index] = this->HWValues[index][line];

    lastSetValues[index].first = line;
    lastSetValues[index].second = this->HWValues[index][line];
}

void Variable::setHWValue(int index, float value, int line, bool updateLastActiveValue) {

    this->HWValues[index][line].setVal(value);
    this->HWValues[index][line].setCycle(cycle_counter);
    //std::cout << name << "[" << index << "] (line: " << line << ") = " << value << std::endl;        

    if (updateLastActiveValue)            
        this->lastActiveValues[index] = this->HWValues[index][line];

    lastSetValues[index].first = line;
    lastSetValues[index].second = this->HWValues[index][line];
}

void Variable::setHWValue(int index, short value, int line, bool updateLastActiveValue) {

    this->HWValues[index][line].setVal(value);
    this->HWValues[index][line].setCycle(cycle_counter);
    //std::cout << name << "[" << index << "] (line: " << line << ") = " << value << std::endl;        

    if (updateLastActiveValue)            
        this->lastActiveValues[index] = this->HWValues[index][line];

    lastSetValues[index].first = line;
    lastSetValues[index].second = this->HWValues[index][line];
}

void Variable::setHWValue(int index, int value, int line, bool updateLastActiveValue) {

    this->HWValues[index][line].setVal(value);
    this->HWValues[index][line].setCycle(cycle_counter);
    //std::cout << name << "[" << index << "] (line: " << line << ") = " << value << std::endl;        

    //TODO: line condition must be removed
    if (updateLastActiveValue /*|| line == -1*/)
        this->lastActiveValues[index] = this->HWValues[index][line];

    lastSetValues[index].first = line;
    lastSetValues[index].second = this->HWValues[index][line];
}

void Variable::setHWInitialValue(int index, std::string hexValue) {
    VariableType* varType = getVarTypeByIndex(index);

    //the value is uninitialized
    if (hexValue == "N/A") {
        setHWValueAsUnInitialized(index);
        return;
    }

    //the type must be a primitive type
    switch(varType->type) {
        case PRIMITIVE_INT: {
            if (varType->byteSize == 8) {
                long long value = hexToLongLong(hexValue);
                setHWValue(index, value, -1, true);                    
            } else if (varType->byteSize == 4) {
                int value = hexToInt(hexValue);
                setHWValue(index, value, -1, true);
            } else {
                short value = hexToInt(hexValue);
                setHWValue(index, value, -1, true);
            }
            break;
        }
        case PRIMITIVE_FLOAT: {
            float value = hexToFloat(hexValue);
            setHWValue(index, value, -1, true);
            break;
        }
        case PRIMITIVE_DOUBLE: {
            double value = hexToDouble(hexValue);
            setHWValue(index, value, -1, true);
            break;
        }
        case POINTER: {
            if (varType->byteSize == 8) {
                long long value = hexToLongLong(hexValue);
                setHWValue(index, value, -1, true);

            } else if (varType->byteSize == 4) {
                int value = hexToInt(hexValue);
                setHWValue(index, value, -1, true);
            } else {
                short value = hexToInt(hexValue);
                setHWValue(index, value, -1, true);
            }
            break;
        }
        default:
            //assert("Detected Type must be a primitive type" && false);
            break;

    }
}      

std::string Variable::getLastSWValue() {
    if (this->SWValues.size() == 0)
        return "N/A";
    return this->SWValues.back().second;
}    

void Variable::setSWValue(std::string val, int line) {
    if (val == "N/A") {
        //std::cout << "N/A value is skipping..." << std::endl;
        return;
    }
    this->SWValues.push_back(std::make_pair(line, val));
}  

bool Variable::isSWValuesEmpty() {
    if (this->SWValues.size() == 0)
        return true;

    for (int i = 0; i < SWValues.size(); i++)
        if (SWValues[i].second != "N/A")
            return false;

    return true;                
}
