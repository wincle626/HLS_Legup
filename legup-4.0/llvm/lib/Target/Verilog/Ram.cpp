//===-- Ram.cpp -------------------------------------------------*- C++ -*-===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the Ram class
//
//===----------------------------------------------------------------------===//

#include "Ram.h"
#include "Allocation.h"
#include "utils.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "DebugType.h"
#include <cstdio>

#define DEBUG_TYPE "LegUp:Ram"

using namespace llvm;
using namespace legup;

namespace legup {

void RAM::setName(std::string _name) {
    name = _name;
}

std::string RAM::getName() const { 
    if (!name.empty()) {
        // user specified name
        return name;
    }
    if (phyRAM) {
        return phyRAM->getName();
    } else {
        return alloc->verilogName(value); 
    }
}

std::string RAM::getTagAddr() const {
    std::string ret = "{`";
    ret += getTag();

    // if there are multiple instances
    // tag always points to the first instance
    // offset is added in the address signal 
    // to steer to the right instance
    if (numInstances != 1)
        ret += "_inst0";

    ret += ", ";
    ret += utostr( (int)alloc->getDataLayout()->getPointerSizeInBits() -
            alloc->getTagSize() );

    int offset = 0;
    if (phyRAM) {
        offset = phyRAM->getOffset(this);
    }

    ret += "'d" + utostr(offset) + "}";

    return ret;
}


void RAM::visitConstantStruct(
        const ConstantStruct *cs,
        std::stack<const Constant*> &lifo,
        std::stack<unsigned> &elemLifo,
        std::stack<unsigned> &structOffset,
        unsigned &elemCount,
        unsigned &offset
        ) {
    const StructLayout *SL = alloc->getDataLayout()->getStructLayout(cs->getType());
    if (elemCount == cs->getNumOperands()) {
        lifo.pop();
        elemLifo.pop();
        structOffset.pop();
    } else {
        if (elemCount == 0) {
            structOffset.push(offset);
        } else {
            offset = structOffset.top() + SL->getElementOffset(elemCount);
        }
        lifo.push(cs->getOperand(elemCount));
        elemCount++;
        elemLifo.pop();
        elemLifo.push(elemCount);
        elemLifo.push(0);
    }
}

void RAM::visitConstantArray(
        const ConstantArray *ca,
        std::stack<const Constant*> &lifo,
        std::stack<unsigned> &elemLifo,
        std::stack<unsigned> &structOffset,
        unsigned &elemCount,
        unsigned &offset
        ) {
    if (elemCount == 0) {
        if (isa<ConstantStruct>(ca->getOperand(0)))
            structOffset.push(offset);
    } else {
        if (const ConstantInt *ci = dyn_cast<ConstantInt>(ca->getOperand(0))) {
            offset += ci->getBitWidth() / 8;
		} else if (const ConstantStruct *cs = dyn_cast<ConstantStruct>(ca->getOperand(0))) {
            const StructLayout *SL = alloc->getDataLayout()->getStructLayout(cs->getType());
            offset = structOffset.top() + SL->getSizeInBytes();
        }
    }
    if (elemCount == ca->getNumOperands()) {
        lifo.pop();
        elemLifo.pop();
        if (isa<ConstantStruct>(ca->getOperand(0)))
            structOffset.pop();
    } else {
        lifo.push(ca->getOperand(elemCount));
        elemCount++;
        elemLifo.pop();
        elemLifo.push(elemCount);
        elemLifo.push(0);
    }

}

void RAM::visitConstantDataArray(
        const ConstantDataArray *cda,
        std::stack<const Constant*> &lifo,
        std::stack<unsigned> &elemLifo,
        std::stack<unsigned> &structOffset,
        unsigned &elemCount,
        unsigned &offset
        ) {
	//errs() << "CDA: " << *cda << " cnt: " << elemCount << " offset: " << offset << "\n";
    if (elemCount == 0) {
		//errs() << "elemCount == 0\n";
        if (isa<ConstantStruct>(cda->getElementAsConstant(0)))
		{
			//errs() << "constant struct\n";
            structOffset.push(offset);
		}
    } else {
        if (const ConstantInt *ci = dyn_cast<ConstantInt>(cda->getElementAsConstant(0)))
		{
			//errs() << "constant int: " << *ci << "; bitwidth: " << ci->getBitWidth() / 8 << "\n";
            offset += ci->getBitWidth() / 8;
		} else if (const ConstantStruct *cs = dyn_cast<ConstantStruct>(cda->getElementAsConstant(0))) {
			//errs() << "constant struct\n";
            const StructLayout *SL = alloc->getDataLayout()->getStructLayout(cs->getType());
            offset = structOffset.top() + SL->getSizeInBytes();
        }
    }
    if (elemCount == cda->getNumElements()) {
        lifo.pop();
        elemLifo.pop();
        if (isa<ConstantStruct>(cda->getElementAsConstant(0)))
		{
			//errs() << "constant struct\n";
            structOffset.pop();
		}
    } else {
        lifo.push(cda->getElementAsConstant(elemCount));
        elemCount++;
        elemLifo.pop();
        elemLifo.push(elemCount);
        elemLifo.push(0);
    }

}

//NC changes...
std::string RAM::convertSmallVectorToString(SmallVectorImpl<char>& v) {
    std::string str;
    for (unsigned int i = 0; i < v.size(); i++)
        str += v[i];
    return str;
}

// assign appropriate bits of 64-bit value
// for example ci is a 2-byte short:
//      ci = 0xAABB
//      ci->getZExtValue = 0x000000000000AABB (64-bits)
//      wordAlignedByteOffset = 2
//      offset       7  6  5  4  3  2  1  0
//      val =       00 0F FF FF 00 00 00 00  (previous value)
//      bitwise or val with:
//      val |=      00 00 00 00 AA BB 00 00  (ci << (2 * 8))
//      val =       00 0F FF FF AA BB 00 00
uint64_t RAM::getConstantVal(const Constant *c, unsigned offset, uint64_t val) {
    const ConstantInt *ci = dyn_cast<ConstantInt>(c);
    
    //NC changes
    if (LEGUP_CONFIG->getParameterInt("INSPECT_DEBUG")) {
        insertDebugInitialValue(ci->getValue());
    }
    unsigned wordByteSize = 8;
    unsigned wordAlignedByteOffset = offset % wordByteSize;

    unsigned intSize;
    if (ci) {
        intSize = ci->getBitWidth() / 8;
    } else {
        intSize = alloc->getDataLayout()->getPointerSize();
    }

    // assert alignment
    assert(wordAlignedByteOffset % intSize == 0);
    // assert that we are not overwriting any values
    assert(val >> (wordAlignedByteOffset * 8) == 0);

    if (ci) {
        val |= ci->getZExtValue() << (wordAlignedByteOffset * 8);
    } else {
        val = alloc->getRamTagNum(dyn_cast<Value>(c));
        val <<= (32 - alloc->getTagSize());
    }
    return val;
}

void RAM::visitConstant(
        const Constant *c,
        uint64_t *val,
        std::stack<const Constant*> &lifo,
        std::stack<unsigned> &elemLifo,
        std::stack<unsigned> &structOffset,
        unsigned &elemCount,
        unsigned &offset
        ) {
    if (offset > 0 && offset % 8 == 0) {
        initial.push_back(APInt(getDataWidth(), *val));
        *val = 0;
    }

    if (isa<ConstantInt>(c) || (isa<PointerType>(c->getType()) &&
                !isa<ConstantPointerNull>(c))) {

        *val = getConstantVal(c, offset, *val);

    } else {
        assert(isa<ConstantAggregateZero>(c) ||
                isa<ConstantPointerNull>(c) ||
                isa<UndefValue>(c));
    }

    lifo.pop();
    elemLifo.pop();
}

// we have a struct somewhere, may be nested in array or another struct
void RAM::initializeStruct() {

    assert (isStruct());

    unsigned offset = 0, elemCount = 0;
    uint64_t val = 0;

    // parent elements when iterating
    std::stack<const Constant*> lifo;
    // element number, used for iterating through arrays and structs
    std::stack<unsigned> elemLifo;
    // base offset when iterating through structs
    std::stack<unsigned> structOffset;

    // push parent element with element count of 0
    lifo.push(getInitializer());
    elemLifo.push(0);

    while (!lifo.empty()) {
        const Constant *c = lifo.top();
        elemCount = elemLifo.top();
        if (const ConstantStruct *cs = dyn_cast<ConstantStruct>(c)) {
            visitConstantStruct(cs, lifo, elemLifo, structOffset, elemCount, offset);

        } else if (const ConstantArray *ca = dyn_cast<ConstantArray>(c)) {

            visitConstantArray(ca, lifo, elemLifo, structOffset, elemCount, offset);
        } else if (const ConstantDataArray *cda = dyn_cast<ConstantDataArray>(c)) {
            visitConstantDataArray(cda, lifo, elemLifo, structOffset, elemCount, offset);
        } else {

            visitConstant(c, &val, lifo, elemLifo, structOffset, elemCount, offset);

        }
    }
    initial.push_back(APInt(getDataWidth(), val));

    // the following shouldn't be necessary
    /*assert(initial.size() <= elements);
      while (initial.size() < elements)
      initial.push_back(APInt(getDataWidth(), 0));*/

    // make sure the RAM is not uninitialized
    // otherwise, struct padding will never be initialized
    APInt Zero = APInt(getDataWidth(), 0);
    for (unsigned i = 0; i < elements; i++) {
        initial.push_back(Zero);
    }
}

void RAM::initializeToZero() {
    APInt Zero = APInt(getDataWidth(), 0);

    for (unsigned i = 0; i < elements; i++) {
        initial.push_back(Zero);
        //NC changes
        if (LEGUP_CONFIG->getParameterInt("INSPECT_DEBUG")) {
            insertDebugInitialValue("0");
        }        
    }
}

// store in row-major order:
// array[2][2][2] = {{{0, 1}, {2, 3}}, {{4, 5}, {7, 8}}}
// Dimensions: array[A][B][C] A=2, B=2, C=2
// When accessing array[a][b][c]:
//      offset = c + C*b + C*B*a = c + C*(b + B*a)
//
// change to a stack for column-major order
// TODO: LLVM 3.4 update: may be ConstantArray or ConstantDataArray.. common ancestor is Constant
//void RAM::initializeArray(const ConstantArray *C) {
void RAM::initializeArray(const Constant *C) {
    std::queue<const Constant*> fifo;
    fifo.push(C);
    while (!fifo.empty()) {
        const Constant *c = fifo.front();
        fifo.pop();

		if (const ConstantAggregateZero *CAZ = dyn_cast<ConstantAggregateZero>(c) ) {
				ArrayType *ATy = dyn_cast<ArrayType>(CAZ->getType());
				assert(ATy);
				int elements = ATy->getNumElements();
                //errs() << "elements: " << elements << "\n";
                //errs() << "c: " << *c << "\n";
                APInt Zero = APInt(getBitWidth(ATy->getElementType()), 0);
				for (int i = 0; i < elements; i++) {
					initial.push_back(Zero);
                                        //NC changes
                                        if (LEGUP_CONFIG->getParameterInt("INSPECT_DEBUG")) {
                                            insertDebugInitialValue("0");                                            
                                        }
				}
		}


		unsigned e;
		if(const ConstantDataArray *CDA = dyn_cast<ConstantDataArray>(c)) {
			e = CDA->getNumElements();
		} else {
			e = c->getNumOperands();
		}

        for (unsigned i = 0; i != e; ++i) {
            Value *op;
			if(const ConstantDataArray *CDA = dyn_cast<ConstantDataArray>(c)) {
				op = CDA->getElementAsConstant(i);
			}else{
				op = c->getOperand(i);
			}
            if (const ConstantInt *I = dyn_cast<ConstantInt>(op)) {
                //errs() << I->getValue() << "\n";
                initial.push_back(I->getValue());
                //NC changes
                if (LEGUP_CONFIG->getParameterInt("INSPECT_DEBUG")) {  
                    insertDebugInitialValue(I->getValue());                    
                }
                
            } else if (const ConstantArray *CA = dyn_cast<ConstantArray>(op)) {
                fifo.push(CA);
            } else if (const ConstantExpr *ce = dyn_cast<ConstantExpr>(op)) {
                // ie. [2 x i32*] [i32* getelementptr inbounds ([28 x i32]* @head1_0, i32 0, i32 0), i32* null]
                //fifo.push(ce->getOperand(0));
                APInt p = getPointerFromGEP(ce);
                initial.push_back(p);
                //NC changes
                if (LEGUP_CONFIG->getParameterInt("INSPECT_DEBUG")) {
                    insertDebugInitialValue(p);                                        
                }
                
            } else if (isa<ConstantPointerNull>(op)) {
                APInt Zero = APInt(getDataWidth(), 0);
                //NC changes
                if (LEGUP_CONFIG->getParameterInt("INSPECT_DEBUG")) {
                    insertDebugInitialValue("0");                    
                }
                initial.push_back(Zero);
            } else if (const ConstantFP *F = dyn_cast<ConstantFP>(op)) {
                APFloat f = F->getValueAPF();                
                FPinitial.push_back(f); //Floating point
                //NC changes
                if (LEGUP_CONFIG->getParameterInt("INSPECT_DEBUG")) {                                        
                    insertDebugInitialValue(f);                    
                }
                
			} else if (const ConstantAggregateZero *CAZ =
					dyn_cast<ConstantAggregateZero>(op) ) {
				// this is like a ConstantArray but full of zeros
				// [2 x i8] zeroinitializer
				fifo.push(CAZ);
                //APInt Zero = APInt(getDataWidth(), 0);
                //initial.push_back(Zero);
				/*
				ArrayType *ATy = dyn_cast<ArrayType>(CAZ->getType());
				assert(ATy);
				elements = ATy->getNumElements();
                errs() << "elements: " << elements << "\n";
                errs() << "op: " << *op << "\n";
                APInt Zero = APInt(getBitWidth(ATy->getElementType()), 0);
				Constant* zeroc = ConstantInt::get(ATy->getElementType(), Zero);
				std::vector<Constant*> Initializer;
				for (int i = 0; i < elements; i++) {
					//initial.push_back(Zero);
					Initializer.push_back(zeroc);
				}
				Constant *C = llvm::ConstantArray::get(ATy, Initializer);
				ConstantAggregateZero *Z = dyn_cast<ConstantAggregateZero>(C);
				assert(Z);
				fifo.push(Z);
				*/
				//ConstantArray *CA = dyn_cast<ConstantArray>(C); assert(CA); errs() << "CA: " << *CA << "\n";
				// TODO LLVM 3.4 update: add these possibilities
			} else if (const ConstantDataArray *CDA = dyn_cast<ConstantDataArray>(op))
			{
				fifo.push(CDA);
				//errs() << "ConstantDataArray fifo size: " << fifo.size() << "\n";
			} else if (isa<ConstantDataVector>(op))
			{
				errs() << "Error: Not implemented: ConstantDataVector\n";
                llvm_unreachable(0);
			} else if (isa<ConstantVector>(op))
			{
				errs() << "Error: Not implmented: ConstantVector\n";
                llvm_unreachable(0);
            } else {
                errs() << "Array: " << *C << "\n";
                errs() << "Operand: " << *op << "\n";
                llvm_unreachable(0);
            }
        }
    }
}

void RAM::initializeConstantExpr(const ConstantExpr *expr) {
    APInt p = getPointerFromGEP(expr);
    initial.push_back(p);
    
    //NC changes
    if (LEGUP_CONFIG->getParameterInt("INSPECT_DEBUG")) {    
        insertDebugInitialValue(p);        
    }
}

APInt RAM::getPointerFromGEP(const ConstantExpr *expr) {
    if (!expr->isGEPWithNoNotionalOverIndexing()) {
        llvm_unreachable(0);
    }
    uint64_t val = alloc->getRamTagNum(dyn_cast<Value>(expr->getOperand(0)));
    val <<= (32 - alloc->getTagSize());
    // Compute the offset that this GEP adds to the pointer.
    SmallVector<Value*, 8> Indices(expr->op_begin()+1, expr->op_end());
    uint64_t GEPOffset = alloc->getDataLayout()->getIndexedOffset(
            expr->getOperand(0)->getType(), Indices);
    return APInt(getDataWidth(), val + GEPOffset);
}

//NC changes...
//this is usually used to insert zero constant strings to the debug initial value list.
void RAM::insertDebugInitialValue(std::string str) {
    debugInitialValues.push_back(str);
}

void RAM::insertDebugInitialValue(const APInt& intValue) {    
	//TODO: LLVM 3.4 update: SmallVectorImpl constructor no longer public -> just use SmallVector?
    //SmallVectorImpl<char> v(50);
    SmallVector<char, 50> v;
    intValue.toString(v, 16, true);
    debugInitialValues.push_back(convertSmallVectorToString(v));
    //v.~SmallVectorImpl();
    v.~SmallVector<char, 50>();
}

void RAM::insertDebugInitialValue(const APFloat& floatValue) {
    //for some unknown reasons, when the smallVector is passed
    //to the toString method from an APFloat object containing
    //a double value (64 bit), it gives exceptions...
    //only in this situation, the other function overload is being used
    //that may thrash the heap by creating std::string objects
    //inside the called method.
    std::string str = floatValue.bitcastToAPInt().toString(16, true);        
    debugInitialValues.push_back(str);    
}

void RAM::initialize() {
    if (const ConstantArray *C = dyn_cast<ConstantArray>(initializer)) {
        initializeArray(C);
	} else if (const ConstantDataArray *C = dyn_cast<ConstantDataArray>(initializer)) {
		// TODO: LLVM 3.4 update
        initializeArray(C);
    } else if (const ConstantInt *c = dyn_cast<ConstantInt>(initializer)) {
        initial.push_back(c->getValue());
        //NC changes        
        if (LEGUP_CONFIG->getParameterInt("INSPECT_DEBUG")) {
            insertDebugInitialValue(c->getValue());            
        }
        
    } else if (const ConstantFP *fp = dyn_cast<ConstantFP>(initializer)) {
        FPinitial.push_back(fp->getValueAPF());
        //NC changes        
        if (LEGUP_CONFIG->getParameterInt("INSPECT_DEBUG")) {
            insertDebugInitialValue(fp->getValueAPF());
        }        
    } else if (const ConstantExpr *expr = dyn_cast<ConstantExpr>(initializer)) {
        initializeConstantExpr(expr);
    } else {
        llvm_unreachable(0);
    }
}


void RAM::buildInitializer() {
    if (!initializer) {
        return;
    }
    if (isa<ConstantAggregateZero>(initializer) ||
            isa<ConstantPointerNull>(initializer)) {
        initializeToZero();
    } else if (isStruct()) {
        initializeStruct();
    } else {

        initialize();
        //check if array is floating point or integer
        if (FPinitial.size()>initial.size())
            assert(FPinitial.size() == elements);
        else
            assert(initial.size() == elements);
    }
}

// write out a .mif file for ram initialization
void RAM::generateMIFHeader(raw_fd_ostream &File, int depth, int width) {

    File << "Depth = " << depth << ";\n";
    File << "Width = " << width << ";\n";
    File << "Address_radix = dec;\n";
    File << "Data_radix = hex;\n";
    File << "Content\n";
    File << "Begin\n";

}

void RAM::generateMIFContent(raw_fd_ostream &File, std::string name) {
    if (!initializer) {
        return;
    }
    buildInitializer();

    int offset = 0;
    if (phyRAM) {
        int bytes = getDataWidth() / 8;
        // make sure bytes isn't 0
        bytes = (bytes == 0) ? 1 : bytes;
        offset = phyRAM->getOffset(this) / bytes;
    }

//Check if the array is floating point or integer
    if (FPinitial.size()>initial.size()){
        for (unsigned i = 0; i < elements; i++) {
            File << i+offset << ": ";
            //check if floating point array is float or double.
            char buffer[17] = {0};
            // Warning: do not try to use a single 'temp' variable. temp
            // must be either 'float' or 'double' for hex_string to work
            if (getDataWidth() == 32) {
                float temp = FPinitial[i].convertToFloat(); //APFloat -> Float
                hex_string(buffer, (char*)&temp, 4);

                File << buffer << ";\t-- " << name << "[" << i
                    << "] = " << temp << "\n";
            } else {
                double temp = FPinitial[i].convertToDouble(); //APFloat -> Double
                hex_string(buffer, (char*)&temp, 8);

                File << buffer << ";\t-- " << name << "[" << i
                    << "] = " << temp << "\n";
            }
        }
    }
    else
        for (unsigned i = 0; i < elements; i++) {
            File << i+offset << ": ";
            SmallString<40> E;
            // todo: signed values!
            initial[i].toStringUnsigned(E, 16);
            // todo: add this for comments
            //File.PadToColumn(20);
            unsigned leadingZeros = initial[i].countLeadingZeros() / 4;
            if (initial[i] == 0 && leadingZeros > 1) leadingZeros--;
            std::string zeroPad = std::string(leadingZeros, '0');
            File << zeroPad << E.str() << ";";
            // put decimal value in comment
            File << "\t-- " << name << "[" << i << "] = " << initial[i] << "\n";
        }
}

void RAM::generateMIFFooter(raw_fd_ostream &File) {
    File << "End;\n";
}


void RAM::getRAMTypeForArray(ArrayType *ATy) {
    elements = ATy->getNumElements();

    // arrays can hold other arrays
    while (ArrayType *ATy2 = dyn_cast<ArrayType>(ATy->getElementType())) {
        elements = elements * ATy2->getNumElements();
        ATy = ATy2;
    }

    // calculate data width
    Type *dataTy = ATy->getElementType();
    DEBUG(errs() << "elements: " << elements << ", type: " << *dataTy << "\n");
    if (const IntegerType *ITy = dyn_cast<IntegerType>(dataTy)) {
        datawidth = ITy->getBitWidth();
    } else if (dataTy->isFloatTy()){
        datawidth = 4 * 8; //float type is 4 byte
    } else if (dataTy->isDoubleTy()){
        datawidth = 8 * 8; //double type is 8 byte
    } else if (StructType *STy = dyn_cast<StructType>(dataTy)) {
        IsStruct = true;
        datawidth = 64;

        int structSize = getStructElements(STy);
        elements *= structSize;
    } else if (isa<PointerType>(dataTy)) {
        datawidth = alloc->getDataLayout()->getPointerSizeInBits();
    } else {
        llvm_unreachable("Unsupported Array Type");
    }

    // remember that we can ignore the 0th element
    // ie. 16 elements will require 4 bits (not 5 bits required to store the
    // _number_ 16)
    addresswidth = requiredBits(elements-1);
}

void RAM::getRAMTypeForStruct(StructType *STy) {
    IsStruct = true;
    datawidth = 64;

    elements = getStructElements(STy);
    addresswidth = requiredBits(elements-1);
}

int RAM::getStructElements(StructType *STy) {
    const StructLayout *SL = alloc->getDataLayout()->getStructLayout(STy);
    int elements = SL->getSizeInBytes();
    // round up to nearest 8, then divide by 8
    if (elements % 8 == 0) {
        elements /= 8;
    } else {
        elements = elements / 8 + 1;
    }
    return elements;
}

//NC changes...
//this function added by me for debugging purpose
void RAM::findStructure(Type* T) {
    DebugType* dt = typesToDebugTypes[T];
    if (ArrayType *ATy = dyn_cast<ArrayType>(T)) {
        dt->numElements = ATy->getNumElements();
        dt->type = 4;
        DebugType* elementDebugType = new DebugType();
        dt->elementTypes.push_back(elementDebugType);
        typesToDebugTypes[ATy->getElementType()] = elementDebugType;
        findStructure(ATy->getElementType());
    } else if (StructType *STy = dyn_cast<StructType>(T)) {
        dt->numElements = STy->getNumContainedTypes();
        dt->type = 5;
        for (StructType::element_iterator it = STy->element_begin(); it != STy->element_end(); ++it) {
            DebugType* elementDebugType = new DebugType();
            dt->elementTypes.push_back(elementDebugType);
            typesToDebugTypes[(*it)] = elementDebugType;
            findStructure((*it));
        }
    } else if (IntegerType *ITy = dyn_cast<IntegerType>(T)) {
        dt->numElements = 1;
        dt->byteSize = ITy->getBitWidth() / 8;
        dt->type = 0;
    } else if (T->isFloatTy()) {
        dt->numElements = 1;
        dt->byteSize = 4;
        dt->type = 1;        
    } else if (T->isDoubleTy()) {
        dt->numElements = 1;
        dt->byteSize = 8;
        dt->type = 2;
    } else if (isa<PointerType>(T)) {
        dt->numElements = 1;
        dt->type = 3;
        dt->byteSize = alloc->getDataLayout()->getPointerSize();
    }
}
//
//NC changes...
//this function added by me for debugging purpose
void RAM::getRAMStructure() {
    Type *T = value->getType();
    PointerType *PTy = dyn_cast<PointerType>(T);
    assert(PTy);
    
    Type *ETy = PTy->getElementType();
    debugType = new DebugType();
    //debugType->byteSize = this->elements
    typesToDebugTypes[ETy] = debugType;
    findStructure(ETy);
}

void RAM::getRAMType() {
    // RAM value should always be a pointer
    Type *T = value->getType();
    PointerType *PTy = dyn_cast<PointerType>(T);
    assert(PTy);

    // RAM value points to either an array or an integer
    Type *ETy = PTy->getElementType();

    IsStruct = false;

    if (ArrayType *ATy = dyn_cast<ArrayType>(ETy)) {
        getRAMTypeForArray(ATy);
    } else if (const IntegerType *ITy = dyn_cast<IntegerType>(ETy)) {
        DEBUG(errs() << *ETy << "\n");
        DEBUG(errs() << *PTy << "\n");
        elements = 1;
        addresswidth = 1;
        datawidth = ITy->getBitWidth();
    } else if (isa<PointerType>(ETy)) {
        elements = 1;
        addresswidth = 1;
        datawidth = alloc->getDataLayout()->getPointerSizeInBits();
    } else if (StructType *STy = dyn_cast<StructType>(ETy)) {
        getRAMTypeForStruct(STy);
    } else if (ETy->isFloatTy()){
        elements = 1;
        addresswidth = 1;
        datawidth = 4 * 8; //float type is 4 byte
    } else if (ETy->isDoubleTy()){
        elements = 1;
        addresswidth = 1;
        datawidth = 8 * 8; //double type is 8 byte
    } else {
        errs() << "LegUp does not support the type: " << *ETy << "\n";
        report_fatal_error("Unsupported type");
    }
}

// return the latency in cycles of the RAM
// this is set in the tcl file by:
//      set_operation_latency mem_dual_port 2
//      set_operation_latency local_mem_dual_port 1
// TODO: be able to set the latency of a specific local ram
int RAM::getLatency(Allocation *alloc) {
    RAM *R = this;
    int latency = 1;
    if (LEGUP_CONFIG->getParameterInt("LOCAL_RAMS")) {
        std::string FuName = "mem_dual_port";
        if (alloc->isGlobalRams.find(R) == alloc->isGlobalRams.end()) {
            FuName = "local_mem_dual_port";
        }
        int constraint;
        //errs() << "FuName: " << FuName << "\n";
        assert(LEGUP_CONFIG->getOperationLatency(FuName, &constraint));
        if (alloc->isGlobalRams.find(R) == alloc->isGlobalRams.end()) {
            // local ram
            latency = constraint;

            // If these local RAMs can be sources of multi-cycle paths, each load
            // will need a unique output register. This can be done by reducing the
            // latency of the RAMs by 1.
            if (LEGUP_CONFIG->duplicate_load_reg()) {
                if (latency < 2) {
                    errs() << "\nRAM latency must be >1 when pulling out load registers";
                    assert(0);
                }
                --latency;
            }
        } else {
            // global RAM take 1 off latency
            // (1 cycle is taken by the memory controller)
            latency = constraint-1;
        }
    }
    return latency;
}

RAM::RAM(const Value *R, Allocation *alloc) :
    alloc(alloc), initializer(NULL), ROM(false), scope(GLOBAL), phyRAM(NULL), numInstances(1)
{
    value = R;

    getRAMType();
    
    //NC changes...
    if (LEGUP_CONFIG->getParameterInt("INSPECT_DEBUG")) {
    	getRAMStructure();
    	this->debugType->calculateByteSizes();
    }

    // deal with initialization
    if (const GlobalVariable *G = dyn_cast<GlobalVariable>(R)) {
        initializer = G->getInitializer();
    }

    DEBUG(errs() << "RAM: " << *R << "\n");
    DEBUG(errs() << "Allocated RAM. Elements " << elements
                 << ", data width: " << getDataWidth()
                 << ", Address width: " << addresswidth << "\n");
}

struct offsetSort {
    std::map<const RAM *, unsigned> *ramOffset;
    bool operator()(RAM *r1, RAM *r2) {
        if ((*ramOffset)[r1] < (*ramOffset)[r2]) {
            return true;
        }
        return false;
    }
} offsetSortObj;

bool memAllocSort(RAM *r1, RAM *r2) {
    if (r1->getAddressAlignment() > r2->getAddressAlignment()) {
        return true;
    } else if (r1->getAddressAlignment() == r2->getAddressAlignment()) {
        if (r1->getElements() > r2->getElements()) {
            return true;
        }
    }
    return false;
}

// allocate the individual arrays/RAMs assigned to this physical RAM
// at specific address offsets in the physical RAM
void PhysicalRAM::staticMemoryAllocation(formatted_raw_ostream &out) {
    out << "Running static memory allocation for grouped physical ram: "
        << getName() << "\n";

    unsigned datawidth = getDataWidth();
    unsigned bytes = (datawidth > 8) ? datawidth / 8 : 1;

    // sort memory arrays in descending order by address alignment, then
    // descending by array size
    std::sort(ramList.begin(), ramList.end(), memAllocSort);

    // list of the RAMs in actual order placed in the RAM
    std::list<RAM *> allocation;

    struct HOLE {
        unsigned start;
        unsigned end;
        HOLE(unsigned s, unsigned e) : start(s), end(e) {}
    };

    // list of the available "holes" or unused space in actual order of
    // the RAM
    std::list<HOLE> holes;
    // available space: 0 -> 2^23-1
    HOLE init = HOLE(0, 8388607);
    holes.push_back(init);

    for (PhysicalRAM::ram_iterator r = ram_begin(), re = ram_end(); r != re;
         ++r) {
        RAM *R = *r;

        unsigned size = R->getElements() * bytes;
        unsigned alignment = R->getAddressAlignment();
        // if (!R->isROM() && datawidth == 32) alignment = alignment*2;

        unsigned arrayStart = 0;
        bool found = false;
        std::list<HOLE>::iterator i = holes.begin();

        // greedily place at first available hole (unused space) in the memory
        // that meets address alignment constraint
        // i.e. for an alignment of 4096 we can only place the RAM
        // at offsets of 0, 4096, 8192, 12288, ...
        while (!found) {
            HOLE &hole = *i;
            while (hole.start > arrayStart) {
                arrayStart += alignment;
            }
            unsigned arrayEnd = arrayStart + size - 1;
            if (arrayEnd <= hole.end) {
                // we can place this RAM in this available hole
                ramOffset[R] = arrayStart;
                out << "Placing RAM: " << R->getName() << " at: " << arrayStart
                    << "\n";
                // split hole
                int start1 = hole.start;
                int end1 = arrayStart - 1;
                int start2 = end1 + 1 + size;
                int end2 = hole.end;
                // remove the current hole and replace with new ones...
                i = holes.erase(i);
                if (start1 <= end1) {
                    holes.insert(i, HOLE(start1, end1));
                }
                if (start2 <= end2) {
                    holes.insert(i, HOLE(start2, end2));
                }
                found = true;
            }
            ++i;
        }

        out << "Holes after allocating ram: " << R->getName()
            << " size: " << size << " alignment: " << alignment
            << " offset: " << ramOffset[R] << "\n";
        for (std::list<HOLE>::iterator i = holes.begin(), ie = holes.end();
             i != ie; ++i) {
            HOLE &hole = *i;
            assert(hole.end >= hole.start);
            out << "Hole: " << hole.start << " - " << hole.end
                << " size = " << (hole.end - hole.start + 1) << "\n";
        }
    }

    // re-sort by offset
    offsetSortObj.ramOffset = &ramOffset;
    std::sort(ramList.begin(), ramList.end(), offsetSortObj);

    out << "Final memory allocation for physical ram: " << getName() << "\n";
    char buf[200];
    unsigned prevOffset = 0;
    unsigned prevSize = 0;
    unsigned totalUnused = 0;
    unsigned totalMemory = 0;
    for (PhysicalRAM::ram_iterator r = ram_begin(), re = ram_end(); r != re;
         ++r) {
        RAM *R = *r;
        unsigned datawidth = R->getDataWidth();
        unsigned bytes = (datawidth > 8) ? datawidth / 8 : 1;
        unsigned size = R->getElements() * bytes;

        unsigned alignment = R->getAddressAlignment();
        unsigned offset = ramOffset[R];
        // offset should be word-aligned
        assert(offset % bytes == 0);
        unsigned unused = offset - prevOffset - prevSize;
        totalMemory = offset + size;
        totalUnused += unused;
        prevOffset = offset;
        prevSize = size;
        snprintf(buf, 200, "ram: %-40s\t"
                           "size (bytes): %-5d (hex:0x%-5x)\t"
                           "alignment (bytes): %-5d (hex:0x%-5x) "
                           "offset (bytes): %-5d (hex:0x%-5x) "
                           "unused (bytes): %-5d\n",
                 R->getName().c_str(), size, size, alignment, alignment, offset,
                 offset, unused);
        out << buf;

        // sanity check...all possible addresses should not overlap with offset
        for (unsigned i = 0; i < size - 1; i++) {
            assert((offset + i) == (offset | i));
            /*
            unsigned ADD = (offset + i);
            unsigned OR = (offset | i);
            if (ADD != OR) {
                out << "Error: i = " << i << " " << ADD << " != " << OR << "\n";
            }
            */
        }
    }

    out << "Total Unused (B): " << totalUnused << "\n";
    out << "Total Memory (B): " << totalMemory << "\n";
    double frag = (double)totalUnused / totalMemory;
    int fragP = frag * 100;
    out << "Fragmentation Ratio (Total Unused / Total Memory): " << frag << " ("
        << fragP << "%)\n";

    numwords = totalMemory / bytes;
}

void PhysicalRAM::addRAM(RAM *r, formatted_raw_ostream &out) {
    unsigned datawidth = r->getDataWidth();
    unsigned bytes = (datawidth > 8) ? datawidth / 8 : 1;

    if (!LEGUP_CONFIG->getParameterInt("GROUP_RAMS_SIMPLE_OFFSET")) {
        ramOffset[r] = numwords * bytes;
        numwords += r->getElements();
        out << "Adding RAM: " << r->getName()
            << " to physical ram: " << getName() << "\n";
    } else {

        unsigned memBytes = r->getElements() * bytes;

        unsigned alignment, log;

        // out << "memBytes: " << memBytes << "\n";
        log = log2((double)memBytes) + 0.5;
        // out << "log2(memBytes): " << log << "\n";
        alignment = pow(2, log);
        if (alignment < memBytes) {
            // get the nearest higher power of 2
            alignment *= 2;
        }
        assert(alignment >= memBytes);
        r->setAddressAlignment(alignment);
    }
    assert(getDataWidth() == r->getDataWidth());
    IsStruct = r->isStruct();
    ramList.push_back(r);
}

} // End legup namespace
