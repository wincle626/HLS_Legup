/* 
 * File:   Variable.h
 * Author: nazanin
 *
 * Created on July 11, 2013, 10:57 PM
 */

#ifndef VARIABLE_H
#define	VARIABLE_H

#include "IRInstruction.h"
#include "Utility.h"
#include "VariableType.h"
#include "GDBWrapper.h"
#include "DataAccess.h"
class IRInstruction;

/*
extern int unInitializedIntValue;
extern float unInitializedFloatValue;
extern double unInitializedDoubleValue;
extern long long unInitializedLongLongValue;

extern int cycle_counter;
*/


class IntFloatWrapper {
private:
    int intVal;
    short shortVal;
    float floatVal;
    double doubleVal;
    long long longlongVal;
    
    bool isInt;
    bool isShort;
    bool isFloat;
    bool isDouble;
    bool isLongLong;
    bool isInitialized;
    
    int cycle;
    
public:
    
    void setVal(int val) {
        this->intVal = val;
        this->shortVal = 0;
        this->floatVal = 0;
        this->doubleVal = 0;
        this->longlongVal = 0;
        
        
        
        this->isInt = true;
        this->isShort = false;
        this->isFloat = false;
        this->isDouble = false;
        this->isLongLong = false;
        this->isInitialized = true;
    }
    
    void setVal(short val) {
        this->shortVal = val;
        this->intVal = 0;
        this->floatVal = 0;
        this->doubleVal = 0;
        this->longlongVal = 0;
        
        this->isShort = true;
        this->isInt = false;
        this->isFloat = false;
        this->isDouble = false;
        this->isLongLong = false;
        this->isInitialized = true;
    }
    
    void setVal(float val) {
        this->floatVal = val;        
        this->intVal = 0;
        this->shortVal = 0;
        this->doubleVal = 0;
        this->longlongVal = 0;
        
        this->isFloat = true;
        this->isInt = false;
        this->isShort = false;
        this->isDouble = false;
        this->isLongLong = false;
        this->isInitialized = true;
    }
    
    void setVal(double val) {
        this->doubleVal = val;
        this->intVal = 0;
        this->shortVal = 0;
        this->floatVal = 0;
        this->longlongVal = 0;
        
        this->isDouble = true;
        this->isInt = false;
        this->isShort = false;
        this->isFloat = false;
        this->isLongLong = false;
        this->isInitialized = true;
    }
    
    void setVal(long long val) {
        this->longlongVal = val;
        this->intVal = 0;
        this->shortVal = 0;
        this->floatVal = 0;
        this->doubleVal = 0;
        
        this->isLongLong = true;
        this->isInt = false;
        this->isShort = false;
        this->isFloat = false;
        this->isDouble = false;
        this->isInitialized = true;
    }
    
    void setCycle(int cycle) {
        this->cycle = cycle;
    }
    
    int getCycle() {
        return this->cycle;
    }
    
    int getIntVal() { 
        return this->intVal;
    }
    
    short getShortVal() {
        return this->shortVal;
    }
    
    int getFloatVal() { 
        return this->floatVal;
    }
    
    int getDoubleVal() { 
        return this->doubleVal;
    }
    
    int getLongLongVal() { 
        return this->longlongVal;
    }    
    
    bool getIsInitialized() {
        return this->isInitialized;
    }
    void setIsInitialized(bool val) {
        this->isInitialized = val;
    }
    
    IntFloatWrapper() {
        this->intVal = 0;
        this->shortVal = 0;
        this->floatVal = 0;
        this->doubleVal = 0;
        this->longlongVal = 0;
        
        this->isInt = true;
        this->isShort = false;
        this->isFloat = false;
        this->isDouble = false;
        this->isLongLong = false;
        
        this->isInitialized = false;
        this->cycle = 0;
    }
    
    IntFloatWrapper(int iVal) { setVal(iVal); setCycle(0); }
    IntFloatWrapper(short iVal) { setVal(iVal); setCycle(0); }
    IntFloatWrapper(float fVal) { setVal(fVal); setCycle(0); }
    IntFloatWrapper(double dVal) { setVal(dVal); setCycle(0);}
    IntFloatWrapper(long long lVal) { setVal(lVal); setCycle(0); }
    
    std::string toStringUnsigned() {
        if (!this->isInitialized)
            return "Not Initialized";
        else if (this->isInt)            
            return IntToString((unsigned int)this->intVal);
        else if (this->isShort)
            return IntToString((unsigned short)this->shortVal);
        else if (this->isFloat)
            return ftostr(this->floatVal);
        else if (this->isDouble)
            return doubleToStr(this->doubleVal);
        else if (this->isLongLong)
            return unsignedLongLongToStr((unsigned long long)this->longlongVal);
    }
    
    std::string toString() {
        if (!this->isInitialized)
            return "Not Initialized";
        else if (this->isInt)
            return IntToString(this->intVal);
        else if (this->isShort)
            return IntToString(this->shortVal);
        else if (this->isFloat)
            return ftostr(this->floatVal);
        else if (this->isDouble)
            return doubleToStr(this->doubleVal);
        else if (this->isLongLong)
            return longlongToStr(this->longlongVal);
    }
};

class Variable {
public:        
    Variable(int id, int functionId, std::string name, std::string tag, int tagNum, std::string tagAddressName, int addressWidth, std::string mifFileName, int dataWidth, int numElements, bool isStruct, IRInstruction* IR, VariableType * vt);
    virtual ~Variable();
    
private:
    std::vector<std::map<int, IntFloatWrapper> > HWValues;//container holding FPGA/verilog run values for the variable   
    std::vector<int> lastActiveLines;    
    std::vector<IntFloatWrapper> lastActiveValues;        
    std::vector<std::pair<int, IntFloatWrapper> > lastSetValues;//first: line number, second: value
    
public:    
    
    std::vector<std::pair<int, std::string> > SWValues;//container holding GDB run values for the variable first: line number, second: value    
    std::vector<std::vector<std::pair<int, std::string> > > HWPlainValues;
    std::vector<std::vector<std::pair<int, int> > > HWPlainValueCycles;
    
    int id;
    std::string name;
    int functionId;
    std::string tag;
    int tagNum;
    std::string tagAddressName;
    int addressWidth;
    std::string mifFileName;
    int dataWidth;
    int numElements;
    bool isStruct;
    IRInstruction* IR;
    
    bool isArrayType;
    bool isGlobal;
    
    VariableType* type;
    
    void InitializeContainers();
    
    bool isPrimitive(Type type);
    
    int calculateNumElements(VariableType* vt);
    
    bool findTypeByIndex(VariableType* type, int &currentIndex, int targetIndex, VariableType **varType);
    
    bool findType(VariableType* type, int &currentByteOffset, int targetByteOffset, Type &varType);
    
    bool findIndex(VariableType* type, int &currentByteOffset, int targetByteOffset, int &varIndex);
    
    int getVarIndexByByteIndex(int byteOffset);
    
    Type getVarTypeByByteIndex(int byteOffset);
    
    VariableType* getVarTypeByIndex(int index);
    
    std::string getHWValueStr(int index, int gdbPreviousLineNumber, bool sign = true);
   
    IntFloatWrapper getHWValue(int index, int line);
    
    std::string getLastHWValue();
    
    void updateHWPlainValues(int currentUpdatingLine, int index);
    
    void setHWValue(std::string val);
    
    void setHWValueWithHex(int index, std::string hexValue, int line, bool updateLastActiveLine);
    
    void setHWValueAsUnInitialized(int index);
    
    void setHWValue(int index, double value, int line, bool updateLastActiveValue);
    
    void setHWValue(int index, long long value, int line, bool updateLastActiveValue);
    
    void setHWValue(int index, float value, int line, bool updateLastActiveValue);
    
    void setHWValue(int index, short value, int line, bool updateLastActiveValue);
    
    void setHWValue(int index, int value, int line, bool updateLastActiveValue);
    
    void ResetLastSetValues();
    
    void setHWInitialValue(int index, std::string hexValue);
    
    std::string getLastSWValue();
    
    void setSWValue(std::string val, int line);
    
    bool isSWValuesEmpty();
        
private:

};

#endif	/* VARIABLE_H */
