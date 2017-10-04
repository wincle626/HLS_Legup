//===- MinimizeBitwidth.cpp - shrink arbitrary precision integer bitwidths ===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// This pass shrinks arbitrary precision integer bitwidths from standard
// C sizes: i8, i16, i32, i64 to arbritrary precisions i31, i9 etc.
// Having a better idea of integer widths aids functional unit binding
//
// LLVM binary operations are limited to bit widths of 8, 16, 32, and 64.
// However, these widths don't reflect the real hardware because Quartus
// will optimize an operation's bit width. For example, given a 32 bit
// integer x, x *and* 60 only requires the lower 6 bits of x (the upper
// 26 bits are zero). If x is the result of a 32 bit addition with only
// one successor (the *and* operation) then the adder only requires 6
// bits. To calculate the required bit width of each operation we
// performed an iterative forward pass and a single backward pass. During
// the forward pass we look for add, and, right shift, and sign/zero
// extend operations. We set the bit width of each add to the max of its
// predecessor's bit widths plus one. For an *and* with a constant, the
// bit width is set to the smallest width that will accommodate the
// constant. For a right shift by a constant the bit width is equal to
// the input width minus the constant. Sign/zero extend operations have
// their bit width set to the width of their original operand before
// extension. The forward pass iterates until no more changes are made.
// Next there is a single backwards pass through the graph, where each
// operation's bit width is set to the maximum width of any of its
// successors.
//
//===----------------------------------------------------------------------===//

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/CodeGen/IntrinsicLowering.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/Debug.h"
#include "utils.h"
#include "MinimizeBitwidth.h"
#include "llvm/Support/ErrorHandling.h"
#include "LegupConfig.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include <sstream>
#define INFLATION_FACTOR 1.0


using namespace llvm;

namespace legup {

triStateMask::triStateMask(Value *inst, int maxBitwidth,bool maxIsPositive, bool isPositive, 
                           bool USE_RANGE_FILE)
{
    uint8_t actualBitwidth;
    bool isValidPositive = maxBitwidth && isPositive;
    ConstantInt *constVal = dyn_cast<ConstantInt>(inst);
    if(constVal)
    {
        APInt value = constVal->getValue();
        bitValue = value.getSExtValue();
        actualBitwidth=bitwidth=value.getActiveBits();
        bitIsDynamic=LSB=0;
        //If value is positive and is used by a signed division, we need to pad the bitwidth by 1
        //bit to get a sign bit of 0.
        /*if(value.isNonNegative()) {
            bool usedAsSigned = false;
            for (Value::use_iterator i = inst->use_begin(), end =
                inst->use_end(); i != end; ++i) {
                Instruction *use = dyn_cast<Instruction> (*i);
                if(use->getOpcode()==Instruction::SDiv ||
                   use->getOpcode()==Instruction::SRem) {
                    printValue(use);                
                    usedAsSigned=true;
                    break;
                }
            }
            if(usedAsSigned) {
                actualBitwidth++;
                errs() << "actualBitwidth = "<<utostr(actualBitwidth)<<"\n";
            }
        }*/
         
    }
    else
    {
        IntegerType *ty = dyn_cast<IntegerType>(inst->getType());
        bitwidth = ty?ty->getBitWidth():128;
        assert(bitwidth<=128);
//        if(USE_RANGE_FILE) {
            switch(maxBitwidth) {
                case 0: actualBitwidth=bitwidth; break;
                case -1: actualBitwidth=0; break;
                default:
                    if(maxIsPositive && !isPositive) maxBitwidth++;
                    actualBitwidth=std::min(bitwidth,(uint8_t)(maxBitwidth*INFLATION_FACTOR));
//                    errs()<<"actualBitwidth: "<<utostr(actualBitwidth)<<"\n";
                    break;
            }

            if(isValidPositive) bitIsDynamic = biggestVal(actualBitwidth);
            else bitIsDynamic = biggestVal(bitwidth);
//        }
//        else {
//            actualBitwidth=bitwidth;
//            bitIsDynamic = biggestVal(bitwidth);
//        }
        bitValue=LSB=0;
        /*
        if(actualBitwidth<=64)
        {                        
            
            uint64_t temp = 0xFFFFFFFFFFFFFFFFLL>>((uint8_t)std::max(65-bitwidth,(64-actualBitwidth)));
            APInt minValue,maxValue = APInt(bitwidth,temp);            
            if(isValidPositive && actualBitwidth<bitwidth)
                minValue = APInt(bitwidth,0,false);
            else
                minValue = APInt(bitwidth,0xFFFFFFFFFFFFFFFFLL^temp,true);
        }
        else
        {
            uint64_t temp[2] = {(uint64_t)1<<(actualBitwidth-65),0xFFFFFFFFFFFFFFFFLL};
            APInt maxValue = APInt(bitwidth,2,temp);
            APInt minValue = APInt(bitwidth,0,false);
        }*/
    }    
    extendFrom= actualBitwidth>0 ? actualBitwidth-1 : 0;
    if(isValidPositive)
        MSB=extendFrom;
    else MSB = bitwidth>0 ? bitwidth-1:0;
    //this->updateMSB();
    //this->updateLSB();
    //this->updateExtendFrom();
}
    //create an empty wide triStateMask by default
triStateMask::triStateMask ()
{
    bitwidth=128;
    MSB=0;
    LSB=127;
    bitIsDynamic = 0;
    bitValue=0;
    extendFrom=0;
//    assert(0);
}


void triStateMask::print()
{
    uint8_t max_MSB=std::min((uint8_t)127,
                    (uint8_t)(std::min(MSB,(uint8_t)(extendFrom+1))));
    uint128_t mask = triStateMask::pow2(max_MSB);
    for(int8_t i=max_MSB;i>=0;i--)
    {
        if(!(bitIsDynamic&mask)) printf("%u",(bitValue&mask)>0);
        else
        {
            if(i>extendFrom) printf("S");
            else printf("X");
        }
        mask >>= 1;
    }
//    printf("(%u,%u,%u,%u,%llu %llu,%llu %llu (%lld->%lld))\n",
//            bitwidth,MSB,extendFrom,LSB,bitIsDynamic.upper(),bitIsDynamic.lower(),
//            bitValue.upper(),bitValue.lower(),r.getLower().getSExtValue(),
//            r.getUpper().getSExtValue());
}

//see if LSB should be moved to the left
void triStateMask::updateLSB()
{
    uint8_t i;
    uint128_t mask;
    bool found_LSB=false;
    uint128_t notStaticZero = bitValue|bitIsDynamic;
    for(mask=(uint128_t)1<<std::min(MSB,LSB),i=std::min(MSB,LSB);i<=bitwidth;i++,mask<<=1)
    {
        if(notStaticZero&mask)
        {
            LSB=i;
            found_LSB=true;
            break;
        }
    }
    if(!found_LSB) LSB=0;
}

//set new LSB and MSB for this mask, updating all data accordingly.
void triStateMask::setLimits(uint8_t new_LSB,uint8_t new_MSB)
{
    if(new_MSB<MSB || new_LSB>LSB || new_MSB<LSB)
    {
        if(new_MSB<LSB) MSB=LSB=0;
        else
        {
            MSB=std::min(new_MSB,MSB);
            LSB=std::min(MSB,std::max(new_LSB,LSB));
        }
        uint128_t mask = getOnesMask(LSB,MSB+1);
        bitIsDynamic&=mask;
        bitValue&=mask;
        extendFrom=std::min(extendFrom,MSB);
    }
}


//see if MSB should be moved to the right
void triStateMask::updateMSB()
{
    int8_t i;
    uint128_t mask;
    //update MSB of out
    bool found_MSB=false;
    uint128_t notStaticZero = bitValue|bitIsDynamic;
    for(mask=(uint128_t)1<<MSB,i=MSB;i>=0;i--,mask>>=1)
    {            
        if(notStaticZero&mask)
        {
            MSB=i;
            found_MSB=true;
            break;
        }
    }
    if(!found_MSB) MSB=0;
}

//see if extendFrom should be moved to the left or right
void triStateMask::updateExtendFrom()
{
    if(!bitIsDynamic) return;
    //if MSB is <= extendFrom, then no sign extended bits exist and exntedFrom should be equal to
    //MSB
    if(MSB<=extendFrom) extendFrom=MSB;
    //else if MSB is larger than extendFrom, see if extendFrom should be moved to the left.  For
    //example, if the extendFrom bit has been optimized way, the sign bits should be extended from a
    //bit further to the left
    else
    {
        uint8_t i;
        uint128_t mask;
        bool foundDynamic=false;
        for(mask=(uint128_t)1<<extendFrom,i=extendFrom;i<=MSB;i++,mask<<=1)
        {            
            if(bitIsDynamic&mask)
            {
                extendFrom=i;
                foundDynamic=true;
                break;
            }
        }
        if(!foundDynamic) extendFrom=MSB;
    }
}

bool operator==(triStateMask& a, triStateMask& b)
{
    if(a.MSB!=b.MSB) return false;
    if(a.LSB!=b.LSB) return false;
    if(a.extendFrom!=b.extendFrom) return false;
    if(a.bitIsDynamic != b.bitIsDynamic) return false;
    if(a.bitValue!=b.bitValue) return false;
    if(a.bitwidth!=b.bitwidth) return false;
    return true;
}

void triStateMask::max(triStateMask a,triStateMask b)
{
    MSB = std::max(a.MSB,b.MSB);
    bitwidth = std::max(a.bitwidth,b.bitwidth);
    int extendFromATemp=a.extendFrom;
    if(a.extendFrom==a.MSB && a.bitwidth < b.bitwidth)
        extendFromATemp++;
    int extendFromBTemp=b.extendFrom;
    if(b.extendFrom==b.MSB && b.bitwidth < a.bitwidth)
        extendFromBTemp++;
    extendFrom = std::max(extendFromATemp,extendFromBTemp);
    LSB = std::min(a.LSB,b.LSB);
    //if a bit is dynamic in either a or b or if it's value is different in a or b,
    //the max is dynamic
    bitIsDynamic = (a.bitIsDynamic | b.bitIsDynamic) | (a.bitValue ^ b.bitValue);
    bitValue =  a.bitValue & b.bitValue;
}

//return min possible unsigned value
uint128_t triStateMask::minPossibleVal()
{
    //look for any static 1s in the bitmask, and set all others to static 0s to get the minimum
    return bitValue;
}

//return max possible unsigned value
uint128_t triStateMask::maxPossibleVal()
{
    //set all unknowns to 1, and all sign extended bits to 0
    return (bitValue | (bitIsDynamic & biggestVal(extendFrom+1)));
}

bool triStateMask::isPositive()
{
    //the triStateMask is positive if the MSB is less than the bitwidth-1
    return (MSB < bitwidth-1);
}

bool triStateMask::isNegative()
{
    //the triStateMask is negative if the MSB is equal to bitwidth-1 and is 1
    return (MSB == bitwidth-1 && bitValue&pow2(MSB));
}

void triStateMask::min(triStateMask a,triStateMask b)
{
    //bitwidth = std::min(a.bitwidth,b.bitwidth);
/*    MSB = std::min(a.MSB,b.MSB);
    LSB = std::max(a.LSB,b.LSB);
    extendFrom = std::min(MSB,std::min(a.extendFrom,b.extendFrom));
    bitIsDynamic = (a.bitIsDynamic & b.bitIsDynamic)&pow2(MSB);
    bitValue = (a.bitValue | b.bitValue) &pow2(MSB);
    return;*/
    
    MSB = std::min((uint8_t)(bitwidth-1),std::min(a.MSB,b.MSB));
    LSB = std::min(MSB,std::max(a.LSB,b.LSB));
    extendFrom = std::max(LSB,std::min(MSB,std::min(a.extendFrom,b.extendFrom)));
    bitIsDynamic = (a.bitIsDynamic & b.bitIsDynamic)&biggestVal(MSB+1);
    uint128_t staticZero = ~(a.bitValue|a.bitIsDynamic) | ~(b.bitValue|b.bitIsDynamic);
    bitValue = ((a.bitValue&~(a.bitIsDynamic)) | (b.bitValue&~(b.bitIsDynamic))) &(~staticZero);
    bitValue &= biggestVal(MSB+1);
    if(MSB < (bitwidth-1) && extendFrom<MSB)
    {
        bitIsDynamic |=getOnesMask(MSB+1,bitwidth); 
        MSB=bitwidth-1;
    }
}

triStateMask* MinimizeBitwidth::getOp(Instruction *inst, int op_index, int64_t maxVal, 
                                      int64_t minVal, bool isPositive, bool maxIsPositive) {
    assert(inst->getOperand(op_index));
    int maxBitwidth;
    if(triStateMap.find(inst->getOperand(op_index)) == triStateMap.end())
    {
        if(getRange(inst->getOperand(op_index),&maxVal,&minVal,&isPositive,&maxIsPositive)) 
            maxBitwidth=getMaxBitwidth(maxVal);
        else maxBitwidth=0;
        triStateMap[inst->getOperand(op_index)]=triStateMask(inst->getOperand(op_index),
                                                maxBitwidth,maxIsPositive,isPositive,USE_RANGE_FILE);
    }
    return &(triStateMap[inst->getOperand(op_index)]);
}

void MinimizeBitwidth::getTriStateMasks(Instruction *inst,triStateMask **op0,triStateMask **op1,
                      triStateMask **op2, triStateMask **out)
{
    bool isPositive,maxIsPositive;
    int maxBitwidth;
    int64_t maxVal,minVal;
    assert(inst);
    if(out)
    {
        *out=NULL;
        if(triStateMap.find(inst) == triStateMap.end())
        {
            if(getRange(inst,&maxVal,&minVal,&isPositive,&maxIsPositive)) 
                maxBitwidth=getMaxBitwidth(maxVal);
            else maxBitwidth=0;
            triStateMap[inst]=triStateMask(inst,maxBitwidth,maxIsPositive,isPositive,USE_RANGE_FILE);
        }
        *out = &(triStateMap[inst]);
        if(triStateMap[inst].MSB < (triStateMap[inst].bitwidth-1) && 
           triStateMap[inst].extendFrom < triStateMap[inst].MSB)
        {
            triStateMap[inst].print();
            printf("\n");
            assert(0);
        }
        assert((*out)->bitwidth<=128);
    }
    switch(inst->getNumOperands())
    {
        case 3:
            if(op2) *op2 = getOp(inst,2,maxVal,minVal,isPositive,maxIsPositive);
        case 2:
            if(op1) *op1 = getOp(inst,1,maxVal,minVal,isPositive,maxIsPositive);
        case 1:
            if(op0) *op0 = getOp(inst,0,maxVal,minVal,isPositive,maxIsPositive);
        default: break;
    }
}

void MinimizeBitwidth::printValue(Value *inst)
{

        std::string myStr(""); // declare empty string
        llvm::raw_string_ostream myStream(myStr); // create a string stream 
        inst->print(myStream); // print instruction name to the string [note that this includes the %N -- the destination register!]
        errs() << myStream.str();
}

void triStateMask::printValue(Value *inst)
{

        std::string myStr(""); // declare empty string
        llvm::raw_string_ostream myStream(myStr); // create a string stream 
        inst->print(myStream); // print instruction name to the string [note that this includes the %N -- the destination register!]
        errs() << myStream.str();
}


//Rule: in = trunc(out) except for the MSB of in, which is dynamic if any of
//the bits left of in->bitwidth are dynamic in "out", since these bits have been
//extended from the MSB of "in"
triStateMask MinimizeBitwidth::backwardPropagateExt(triStateMask *out, triStateMask *in) {
    triStateMask in_temp = *out;
    in_temp.bitwidth=in->bitwidth;
    if(out->extendFrom > in->extendFrom) {
        in_temp.MSB = in->MSB;
        in_temp.extendFrom = in->MSB;                
        if(out->LSB > in->MSB) in_temp.LSB=in->MSB;
        in_temp.bitIsDynamic = out->bitIsDynamic | triStateMask::pow2(in->MSB);
        in_temp.bitValue = 0;
    }
    in_temp.setLimits(out->LSB,in->MSB);
    return in_temp;
}

//xor is tricky because static 0s in the output might correspond to two 1s in the inputs so
//just because the MSB of the output is set to n, it doesn't mean that the MSB of the inputs
//is also n. 
triStateMask MinimizeBitwidth::backwardPropagateXor(triStateMask *out, triStateMask *in, 
                                            triStateMask *op0,triStateMask *op1, Instruction *use) {
    triStateMask in_temp = *in;
    Value *otherIn = op0==in ? use->getOperand(1): use->getOperand(0);
    uint8_t LSBlim,MSBlim;
    if(otherIn->hasOneUse() && (op0==in?op1->bitIsDynamic:op0->bitIsDynamic)) {
        LSBlim=out->LSB;
        MSBlim=out->MSB;
    }
    else {
        LSBlim = std::min(out->LSB,op0==in?op1->LSB:op0->LSB);
        MSBlim = std::max(out->MSB,op0==in?op1->MSB:op0->MSB);
    }
    in_temp.setLimits(LSBlim,MSBlim);
    return in_temp;
}

//If the other input has an LSB greater than 0, meaning that it has a bunch of zeros as
//it's left most bits, then we don't need the any significant bits left of MSB of out
//minus LSB of other input.
//For example, with in * 2 = out, where both in and out are 32 bits wide, the 31st bit
//of in can not impact the final result in out.  This is an obvious example because it's
//a power of 2, but in * 6 works the same way since both 2 = 0b10 and 6=0b110 have the
//same LSB
triStateMask MinimizeBitwidth::backwardPropagateMul(triStateMask *out, triStateMask *in, 
                                                    triStateMask *op0, triStateMask *op1) {
    //if there's any dynamic bit, it's not constant
    triStateMask in_temp = *in;
    triStateMask *other_in = in==op0?op1:op0;
    in_temp.setLimits(in->LSB,out->MSB-other_in->LSB);
    return in_temp;
}


triStateMask MinimizeBitwidth::backwardPropagateShr(triStateMask *prev_in_temp, triStateMask *out,
                                                    triStateMask *in, 
                                                    triStateMask *op0, triStateMask *op1) {
    uint8_t minShift,maxShift;
    triStateMask in_temp = *prev_in_temp;
    in_temp.bitwidth=in->bitwidth;
    if(op1->bitIsDynamic) {
        minShift=op1->minPossibleVal().lower();
        uint128_t max_shift = op1->maxPossibleVal();
        if(max_shift<128) maxShift = max_shift.lower();
        else maxShift = 128;
    }
    else maxShift=minShift=op1->bitValue.lower();            
    minShift=std::min(op0->MSB,minShift);
    maxShift=std::min(op0->MSB,maxShift);
    if(op1==in) in_temp = *in;
    else {
        in_temp.MSB=std::min(in->MSB,(uint8_t)(out->MSB+maxShift));
        if(op1->bitIsDynamic) {
            in_temp.LSB=out->LSB;
            in_temp.bitValue=0;
            in_temp.bitIsDynamic = triStateMask::getOnesMask(in_temp.LSB,in_temp.MSB+1);
        }
        else {
            in_temp.LSB= std::min(in_temp.MSB,(uint8_t)(out->LSB + op1->bitValue.lower()));
            in_temp.bitIsDynamic = (out->bitIsDynamic << op1->bitValue.lower()) &
                                     triStateMask::biggestVal(in_temp.bitwidth);
            in_temp.bitValue=(out->bitValue << op1->bitValue.lower()) &
                                    triStateMask::biggestVal(in_temp.bitwidth);
            if(out->extendFrom + minShift > in_temp.MSB) {
                in_temp.bitIsDynamic |= triStateMask::pow2(in_temp.MSB);
                in_temp.bitValue &= triStateMask::biggestVal(in_temp.MSB);
            }
        }
        in_temp.extendFrom=std::min(in_temp.MSB,(uint8_t)(out->extendFrom + maxShift));
    }                

   in_temp.updateMSB();
   in_temp.updateLSB();
   in_temp.updateExtendFrom();
   return in_temp;
}


triStateMask MinimizeBitwidth::backwardPropagateShl(triStateMask *prev_in_temp, triStateMask *out,
                                                    triStateMask *in, 
                                                    triStateMask *op0, triStateMask *op1) {
    uint8_t minShift,maxShift;
    triStateMask in_temp = *prev_in_temp;
    in_temp.bitwidth=in->bitwidth;
    if(op1->bitIsDynamic) {
        minShift=op1->minPossibleVal().lower();
        uint128_t max_shift = op1->maxPossibleVal();
        if(max_shift<128) maxShift = max_shift.lower();
        else maxShift = 128;
    }
    else maxShift=minShift=op1->bitValue.lower();            
    minShift=std::min(out->MSB,minShift);
    maxShift=std::min(out->MSB,maxShift);
    if(op1==in) in_temp = *in;
    else {
        in_temp.LSB=(out->LSB > maxShift) ? out->LSB-maxShift : 0;
        in_temp.MSB = out->MSB > minShift ? out->MSB - minShift:0;
        in_temp.extendFrom = out->extendFrom > minShift ? out->extendFrom - minShift:0;
        //minShift will be 0 for an unknown shift amount
        if(!(op1->bitIsDynamic)) {
            in_temp.bitValue = out->bitValue >> minShift;
            in_temp.bitIsDynamic = out->bitIsDynamic >> minShift;
        }
        else {
            in_temp.bitValue=0;
            in_temp.bitIsDynamic = triStateMask::getOnesMask(std::min(out->bitwidth,(uint8_t)(out->MSB+1)),out->LSB > maxShift? out->LSB - maxShift:0);
        }
    }

   in_temp.updateMSB();
   in_temp.updateLSB();
   in_temp.updateExtendFrom();
    return in_temp;
}


triStateMask MinimizeBitwidth::backwardPropagatePHI(triStateMask *in, Instruction *use) {
    triStateMask in_temp = *in;
    PHINode *phi = dyn_cast<PHINode>(use);
    for(unsigned i=0;i<phi->getNumIncomingValues();i++) {
        Value *phi_in = phi->getIncomingValue(i);
        if(triStateMap.find(phi_in) != triStateMap.end()) {
            //doesn't seem to work... I forget why
            if(triStateMap[phi_in]==*in) {
                //in_temp = *out;
                //out->print();
                //printf("\n");
                break;
            }
        }
    }
    return in_temp;
}



/*
 * backwardPropagate modifies the bitmask associated with operands of the instruction "inst" by
 * applying a backward transit function to the instruction output.  Each type of instruction
 * has a different transit function.
 */

bool MinimizeBitwidth::backwardPropagate(Instruction *inst){
    std::string error = "";
    bool isPositive,maxIsPositive;
    int maxBitwidth;
    int64_t maxVal,minVal;

    triStateMask *in,*out,*op0,*op1,*op2;
    triStateMask in_max;
    assert(inst);
    if(triStateMap.find(inst) == triStateMap.end()) {
        if(getRange(inst,&maxVal,&minVal,&isPositive,&maxIsPositive)) 
            maxBitwidth=getMaxBitwidth(maxVal);
        else maxBitwidth=0;
        triStateMap[inst]=triStateMask(inst,maxBitwidth,maxIsPositive,isPositive,USE_RANGE_FILE);
    }
    in = &(triStateMap[inst]);
    triStateMask in_temp = *in;
    triStateMask in_bk = *in;

    assert(in->bitwidth <= 128);
    bool found = false;
    for (Value::user_iterator i = inst->user_begin(), end = inst->user_end();
         i != end; ++i) {
        Instruction *use = dyn_cast<Instruction>(*i);
        if (!use)
            continue;
        getTriStateMasks(use, &op0, &op1, &op2, &out);
        unsigned Opcode = use->getOpcode();
        switch (Opcode) {
        case Instruction::ZExt:
            case Instruction::SExt: 
                in_temp = backwardPropagateExt(out,in);
                break;
            case Instruction::Trunc:
            case Instruction::And:
            case Instruction::Or:
                in_temp = *out;
                break;
            case Instruction::Xor:
                in_temp = backwardPropagateXor(out,in,op0,op1,use);
                break;
            case Instruction::Sub:
            case Instruction::Add:
                in_temp = *in;
                in_temp.setLimits(in->LSB,out->MSB);
                break;
            case Instruction::Mul:
                in_temp = backwardPropagateMul(out,in,op0,op1); 
                break;
            case Instruction::AShr:
            case Instruction::LShr:
                in_temp = backwardPropagateShr(&in_temp,out,in,op0,op1);
                break;
            case Instruction::Shl:
                in_temp = backwardPropagateShl(&in_temp,out,in,op0,op1);
                break;
            case Instruction::UDiv:
            case Instruction::SDiv:
            case Instruction::URem:
            case Instruction::SRem:
                in_temp = *in;
                break;
            case Instruction::Select:
                if(in==op0) in_temp=*in; //op0 is the select condition
                else {
                    in_temp=*out;
                    in_temp.setLimits(std::max(out->LSB,in->LSB),std::min(out->MSB,in->MSB));
                }
                break;
            case Instruction::PHI:
                in_temp = backwardPropagatePHI(in,use);
                break;
            default:
                in_temp = *in;
                break;
        }
        if(!found) {
            in_max = in_temp;
            found=true;
        }
        else {
            in_max.max(in_max, in_temp);
        }
    }
    if (found) {
        in->min(*in, in_max);
        in->updateMSB();
        in->updateLSB();
        in->updateExtendFrom();

        if (*in == in_bk)
            return false;
        // sanity check
        /*        if((in->extendFrom < in->MSB) && in->MSB < (in->bitwidth-1))
                    error = "BackwardPropagate: Signed number with one or more
           leading zero\n";
                if(in->LSB>0 &&
           triStateMask::pow2(in->LSB-1)&(in->bitIsDynamic|in->bitValue))
                    error = "backwardPropagate: LSB-1 is dynamic or static 1\n";
                if(in->MSB>0 &&
           !(triStateMask::pow2(in->LSB)&(in->bitIsDynamic|in->bitValue)))
                    error = "backwardPropagate: LSB is static 0\n";
                //uint128_t temp0 = !(in->bitIsDynamic|in->bitValue);
                if(in->MSB!=0 &&
           (!((in->bitIsDynamic|in->bitValue)&triStateMask::pow2(in->MSB))))
           error = "backwardPropagate: MSB is 0\n";
                //if(in->MSB!=0 && (temp0&triStateMask::pow2(in->MSB))) error =
           "backwardPropagate: MSB is 0\n";
                if(in->extendFrom<in->LSB) error="backwardPropagate: extendFrom
           is < LSB\n";
                if(in->LSB > in->MSB) error="backwardPropagate: LSB > MSB\n";
                if(!in->bitIsDynamic && in->MSB!=in->extendFrom)
                    error="backwardPropagate: extendFrom < MSB when not
           dynamic\n";
                if(error!="")
                {
                    printValue(inst);
                    printf("\n%s",error.c_str());
                    in->print();
                    printf("\n");
                    printf("min(old:");
                    in_temp.print();
                    printf(",new:");
                    in_max.print();
                    printf(")\n");
                    if(error!="printAll\n") assert(0);
                }*/

        return true;
    }
    return false;
}


// For ZExt, because bitValue is initialized to 0 left of the type bitwidth, all we need to do
// is copy all the values from op0 to the result.
triStateMask MinimizeBitwidth::forwardPropagateZExt(triStateMask *out, triStateMask *op0) {
    triStateMask out_temp = *out;
    out_temp.bitValue = op0->bitValue;
    out_temp.MSB = op0->MSB;
    out_temp.LSB = op0->LSB;
    out_temp.bitIsDynamic = op0->bitIsDynamic; 
    out_temp.extendFrom = op0->extendFrom;
    return out_temp;
}

/*
 * For SExt, all bits left of op0 bitwidth need to be assigned to the same value as the leftmost
 * bit in op0 bitwidth. If we know the value of this bit, we can just set the bits left of op0
 * bitwidth to the same constant, otherwise, we set bitIsDynamic to be 1 for those
 * bits, to indicate that they should take their value from the MSB
 */
triStateMask MinimizeBitwidth::forwardPropagateSExt(triStateMask *out, triStateMask *op0) {
    triStateMask out_temp = *out;
    out_temp.bitValue = op0->bitValue;
    out_temp.MSB = op0->MSB;
    out_temp.LSB = op0->LSB;
    out_temp.bitIsDynamic = op0->bitIsDynamic; 
    out_temp.extendFrom = op0->extendFrom;
    if(op0->MSB==op0->bitwidth-1) {            
        //printf("SExt getOnesMask(%u,%u)\n",op0->bitwidth,out_temp.bitwidth);
        uint128_t onesMask = triStateMask::getOnesMask(op0->bitwidth,out_temp.bitwidth);
        //if leftmost bit is dynamic
        if(out_temp.bitIsDynamic & triStateMask::pow2(op0->MSB)) {
            out_temp.extendFrom=op0->bitwidth-1;
            out_temp.bitIsDynamic |= onesMask;
            if(out_temp.bitValue & triStateMask::pow2(op0->MSB)) out_temp.bitValue |= onesMask;
        }        
        else {
            out_temp.extendFrom=out->bitwidth-1;
            out_temp.bitValue |= onesMask; //left most bit in op0 is static 1
        }
        out_temp.MSB=out_temp.bitwidth-1;
    }
    return out_temp;
}

triStateMask MinimizeBitwidth::forwardPropagateTrunc(triStateMask *out, triStateMask *op0) {
    triStateMask out_temp = *out;
    out_temp.MSB=std::min((uint8_t)(out_temp.bitwidth-1),op0->MSB);
    out_temp.LSB=std::min(out_temp.MSB,op0->LSB);
    out_temp.extendFrom=std::min(out_temp.MSB,op0->extendFrom);
    out_temp.bitValue = op0->bitValue & triStateMask::biggestVal(out_temp.bitwidth);
    out_temp.bitIsDynamic = op0->bitIsDynamic & triStateMask::biggestVal(out_temp.bitwidth);
    return out_temp;
}

triStateMask MinimizeBitwidth::forwardPropagateAnd(triStateMask *out, triStateMask *op0, triStateMask *op1) {
    triStateMask out_temp = *out;
    //mask1 is 0 if the bit is definitely 0 and 1 if it's 1 or X
    uint128_t mask = (op0->bitValue | op0->bitIsDynamic) & 
                     (op1->bitValue | op1->bitIsDynamic); 
    out_temp.bitIsDynamic = mask & (op0->bitIsDynamic | op1->bitIsDynamic);                 
    //take the non-constant 1s away from mask1 and assign to bitValue because we assume that       
    //bitIsDynamic ^ bitIsDynamic is the inverse of bitIsDynamic so 1 means it's constant
    out_temp.bitValue = mask & (~out_temp.bitIsDynamic);
    //one of the two is 1 and the other has a static value of 1
    out_temp.MSB = std::min(op0->MSB,op1->MSB);
    out_temp.LSB = std::max(op0->LSB,op1->LSB);
    out_temp.extendFrom = std::max(op0->extendFrom,op1->extendFrom);
    out_temp.updateMSB();
    out_temp.updateLSB();
    out_temp.updateExtendFrom();
    return out_temp;
}

triStateMask MinimizeBitwidth::forwardPropagateOr(triStateMask *out, triStateMask *op0, triStateMask *op1) {
    triStateMask out_temp = *out;
    //definitely 1
    out_temp.bitValue  = (op0->bitValue | op1->bitValue); 
    uint128_t maybeZero = ~out_temp.bitValue;
    out_temp.bitIsDynamic = maybeZero & (op0->bitIsDynamic | op1->bitIsDynamic);                 
    out_temp.MSB = std::max(op0->MSB,op1->MSB);
    out_temp.LSB = std::min(op0->LSB,op1->LSB);
    out_temp.extendFrom = std::max(op0->extendFrom,op1->extendFrom);
    out_temp.updateMSB();
    out_temp.updateLSB();
    out_temp.updateExtendFrom();
    return out_temp;
}


/*
 * For Xor, we don't know the result unless both Op0 and Op1 are static.
 * ExtendFrom is the max of op0 and op1
 */ 
triStateMask MinimizeBitwidth::forwardPropagateXor(triStateMask *out, triStateMask *op0, triStateMask *op1) {
    triStateMask out_temp = *out;
    out_temp.extendFrom = std::max(op0->extendFrom,op1->extendFrom);
    out_temp.bitIsDynamic = op0->bitIsDynamic | op1->bitIsDynamic;
    uint128_t bitIsNotDynamic = ~out_temp.bitIsDynamic;
    out_temp.bitValue = bitIsNotDynamic & (op0->bitValue ^ op1->bitValue);
    out_temp.MSB = std::max(op0->MSB,op1->MSB);
    out_temp.LSB = std::min(op0->LSB,op1->LSB);
    return out_temp;
}



triStateMask MinimizeBitwidth::forwardPropagateAdd(triStateMask *out, triStateMask *op0, triStateMask *op1, bool cin) {
    triStateMask out_temp = *out;
    uint128_t bitValue=op1->bitValue.lower();
    uint8_t MSB=op1->MSB;
    out_temp.bitValue=cin?1:0;
    out_temp.bitIsDynamic=0;
    out_temp.LSB=std::min(op0->LSB,op1->LSB);
    out_temp.MSB=std::min((uint8_t)(std::max(op0->MSB,MSB)+1),(uint8_t)(out_temp.bitwidth-1));
    uint128_t combinedStatic = ~(op0->bitIsDynamic | op1->bitIsDynamic);
    uint128_t op0IsZero = ~op0->bitValue;
    uint128_t op1IsZero = ~bitValue;
    uint128_t noCout = combinedStatic & op0IsZero & op1IsZero;
    uint128_t yesCout = combinedStatic & op0->bitValue & bitValue;
    uint128_t maybeCout = ~(noCout | yesCout);
    bool cinIsStatic=true;
    uint128_t mask=1,ones=1;
    for(uint8_t i=0;i<=out_temp.MSB;i++) {
        //if this bit position is static in both operands, we can know the result at this bit
        //position
        if((mask&combinedStatic) && cinIsStatic) {
            out_temp.bitValue+=(op0->bitValue&mask)+(bitValue&mask);                
        }
        else {
            out_temp.bitValue&=(ones^mask); //clear the cin
            out_temp.bitIsDynamic|=mask; //add a dynamic bit
            //what to do about the cout                                
            if(maybeCout&mask) cinIsStatic=false;
            else {
                cinIsStatic=true;
                if(yesCout) out_temp.bitValue+=(mask<<1);
            }
        }
        mask<<=1;
        ones|=mask;
    }
    out_temp.extendFrom=std::min(out_temp.MSB,
        (uint8_t)(std::max(op0->extendFrom,op1->extendFrom)+1));
    out_temp.updateMSB();
    out_temp.updateLSB();
    out_temp.updateExtendFrom();
    return out_temp;
}

/*triStateMask MinimizeBitwidth::forwardPropagateSub(triStateMask *out, triStateMask *op0, triStateMask *op1) {
    triStateMask out_temp = *out;
    //Sub is done by first finding the twos complement of op1 then adding that to op0
    //let's first create some temporary variables for the twos complemented version
    uint128_t bitValue=op1->bitValue.lower();
    uint8_t MSB=op1->MSB;
    //do twos complement
    bitValue = ~(bitValue | op1->bitIsDynamic);
    out_temp.bitValue=1; //this is the cin to the addition since 2s comp is inverse + 1
    //MSB should be moved to left most dynamic value or static 0
    int8_t i = op1->bitwidth-1;
    MSB=0;
    for(uint128_t mask=triStateMask::pow2(i);i>=0;i--,mask>>=1) {
        if((~op1->bitValue)&mask) {
            MSB=i;
            break;
        }
    }
    return forwardPropagateAdd(&out_temp,op0,op1,true);
}*/

triStateMask MinimizeBitwidth::forwardPropagateSub(triStateMask *out, triStateMask *op0, triStateMask *op1) {

    triStateMask op1_temp = *op1;
    //Sub is done by first finding the twos complement of op1 then adding that to op0
    //let's first create some temporary variables for the twos complemented version
    //do twos complement
    op1_temp.bitValue = ~(op1->bitValue | op1->bitIsDynamic);
    //MSB should be moved to left most dynamic value or static 0
    int8_t i = out->bitwidth-1;
    op1_temp.MSB=0;
    for(uint128_t mask=triStateMask::pow2(i);i>=0;i--,mask>>=1) {
        if((~op1->bitValue)&mask) {
            op1_temp.MSB=i;
            break;
        }
    }
    triStateMask out_temp = forwardPropagateAdd(out,op0,&op1_temp,true);
    return out_temp;
}

triStateMask MinimizeBitwidth::forwardPropagateMul(triStateMask *out, triStateMask *op0, triStateMask *op1) {
    triStateMask out_temp = *out;
    /*
     * We can't statically determine very much about the bits in a multiplication other than their
     * bounds.  The only bits we can statically determine are the bits left of min(right most
     * dynamic bit of op0, right most dynamic bit of op1)
     * The number of the bits in the result is the width of op0 + width of op1
     */

    //First, find the most consecutive zeros in the rightmost bits of either op0 or op1
    //update LSB while we're at it
    op0->updateMSB();
    op0->updateLSB();
    op0->updateExtendFrom();
    op1->updateMSB();
    op1->updateLSB();
    op1->updateExtendFrom();
    uint8_t rightMostDyn=std::max(op0->LSB,op1->LSB),i=std::min(op0->LSB,op1->LSB);
    //find right most non-static bit in either op0 or op1
    i=std::min(op0->LSB,op1->LSB);
    for(uint128_t mask=(uint128_t)1<<i;i<=out_temp.bitwidth;i++,mask<<=1) {
        if((op0->bitIsDynamic|op1->bitIsDynamic)&mask) {
            rightMostDyn=std::max(rightMostDyn,i);
            break;
        }
    }
    //multiply rightmost static values in op0 and op1, where the result can't be larger than
    //out's bitwidth
    out_temp.bitValue = (((uint128_t)((op0->bitValue&triStateMask::biggestVal(rightMostDyn)).lower() * 
                     ((op1->bitValue&triStateMask::biggestVal(rightMostDyn))).lower())
                    & (uint128_t)triStateMask::biggestVal(out_temp.bitwidth)));
    out_temp.MSB=std::min((uint8_t)(out_temp.bitwidth-1),(uint8_t)(op0->MSB+op1->MSB+1));        
    out_temp.extendFrom = std::min(out_temp.bitwidth-1,op0->extendFrom+op1->extendFrom+1);
    out_temp.bitIsDynamic = triStateMask::getOnesMask(rightMostDyn,std::min((uint8_t)(out_temp.MSB+1),out_temp.bitwidth));
    out_temp.updateMSB();
    out_temp.updateLSB();
    out_temp.updateExtendFrom();
    return out_temp;
}

triStateMask MinimizeBitwidth::forwardPropagateAShr(triStateMask *out, triStateMask *op0, triStateMask *op1) {
    triStateMask out_temp = *out;
    uint8_t minShift,maxShift;
    if(op1->bitIsDynamic) {
        minShift=op1->minPossibleVal().lower();
        uint128_t max_shift = op1->maxPossibleVal();
        if(max_shift<128) maxShift = max_shift.lower();
        else maxShift = 128;
    }
    else maxShift=minShift=op1->bitValue.lower();
    minShift=std::min(op0->MSB,minShift);
    maxShift=std::min(op0->MSB,maxShift);
    //three cases for AShr:
    //1. the leftmost bit is dynamic, so we're shifting in an unknown
    bool leftBitDyn = op0->bitIsDynamic&triStateMask::pow2(op0->bitwidth-1);
    //2. the leftmost bit is static 1, so we know that we're shifting in 1s
    bool leftBitOne = op0->bitValue&triStateMask::pow2(op0->bitwidth-1);
    //3. the leftmost bit is static 0, so this is the same as LShr
    out_temp.LSB=(op0->LSB > maxShift) ? op0->LSB-maxShift : 0;
    out_temp.MSB=out_temp.bitwidth-1;
    out_temp.extendFrom = op0->extendFrom > minShift ? op0->extendFrom - minShift:0;
    //minShift will be 0 for an unknown shift amount
    if(minShift) {
        out_temp.bitValue = op0->bitValue >> minShift;
        uint8_t fillLSB = out_temp.MSB>minShift?out_temp.MSB-minShift:0;
        if(leftBitOne) out_temp.bitValue|=triStateMask::getOnesMask(out_temp.bitwidth,fillLSB);
        out_temp.bitIsDynamic = op0->bitIsDynamic >> minShift;
        if(leftBitDyn)
            out_temp.bitIsDynamic|=triStateMask::getOnesMask(out_temp.bitwidth,fillLSB);
    }
    else {
        out_temp.bitValue=0;
        out_temp.bitIsDynamic = triStateMask::getOnesMask(std::min(op0->bitwidth,
                    (uint8_t)(op0->MSB+1)),op0->LSB > maxShift? op0->LSB - maxShift:0);
    }
    return out_temp;
}


triStateMask MinimizeBitwidth::forwardPropagateLShr(triStateMask *out, triStateMask *op0, triStateMask *op1) {
    triStateMask out_temp = *out;
    uint8_t minShift,maxShift;
    if(op1->bitIsDynamic) {
        minShift=op1->minPossibleVal().lower();
        uint128_t max_shift = op1->maxPossibleVal();
        if(max_shift<128) maxShift = max_shift.lower();
        else maxShift = 128;
    }
    else maxShift=minShift=op1->bitValue.lower();
    minShift=std::min(op0->MSB,minShift);
    maxShift=std::min(op0->MSB,maxShift);
    bool leftBitDyn=false;
    bool leftBitOne=false;
    out_temp.LSB=(op0->LSB > maxShift) ? op0->LSB-maxShift : 0;
    out_temp.extendFrom=out_temp.MSB = op0->MSB > minShift ? op0->MSB - minShift:0;
    //minShift will be 0 for an unknown shift amount
    if(minShift) {
        out_temp.bitValue = op0->bitValue >> minShift;
        uint8_t fillLSB = out_temp.MSB>minShift?out_temp.MSB-minShift:0;
        if(leftBitOne) out_temp.bitValue|=triStateMask::getOnesMask(out_temp.bitwidth,fillLSB);
        out_temp.bitIsDynamic = op0->bitIsDynamic >> minShift;
        if(leftBitDyn)
            out_temp.bitIsDynamic|=triStateMask::getOnesMask(out_temp.bitwidth,fillLSB);
    }
    else {
        out_temp.bitValue=0;
        out_temp.bitIsDynamic = triStateMask::getOnesMask(std::min(op0->bitwidth,
                    (uint8_t)(op0->MSB+1)),op0->LSB > maxShift? op0->LSB - maxShift:0);
    }
    return out_temp;
}

triStateMask MinimizeBitwidth::forwardPropagateShl(triStateMask *out, triStateMask *op0, triStateMask *op1) {
    triStateMask out_temp = *out;
    uint8_t minShift,maxShift;
    if(op1->bitIsDynamic) {
        minShift=op1->minPossibleVal().lower();
        uint128_t max_shift = op1->maxPossibleVal();
        if(max_shift<128) maxShift = max_shift.lower();
        else maxShift = 128;
    }
    else maxShift=minShift=op1->bitValue.lower();
    minShift=std::min(out->MSB,minShift);
    maxShift=std::min(out->MSB,maxShift);
    op0->updateMSB();
    op0->updateLSB();
    op0->updateExtendFrom();
    out_temp.MSB = std::min((uint8_t)(out_temp.bitwidth-1),(uint8_t)(op0->MSB + maxShift));
    if(op1->bitIsDynamic) {
        out_temp.LSB=op0->LSB;
        out_temp.bitValue = 0;
        //printf("AShr getOnesMask(%u,%u)\n",out_temp.LSB,out_temp.MSB+1);
        out_temp.bitIsDynamic = triStateMask::getOnesMask(out_temp.LSB,out_temp.MSB+1);            
    }
    else {
        out_temp.LSB=op0->LSB + op1->bitValue.lower();
        out_temp.bitValue = op0->bitValue << op1->bitValue.lower();
        out_temp.bitIsDynamic = op0->bitIsDynamic << op1->bitValue.lower();
    }
    out_temp.extendFrom = std::min(out_temp.MSB,(uint8_t)(op0->extendFrom + maxShift));
    return out_temp;
}

triStateMask MinimizeBitwidth::forwardPropagateUDiv(triStateMask *out, triStateMask *op0, triStateMask *op1) {
    triStateMask out_temp = *out;
    uint8_t minShift = (int)(log2(op1->minPossibleVal().lower()));
    if(op0->MSB>=minShift) out_temp.MSB = op0->MSB-minShift;
    else out_temp.MSB=0;
    out_temp.LSB=0;
    out_temp.bitIsDynamic = triStateMask::biggestVal(out_temp.MSB+1);
    out_temp.bitValue = 0; //don't bother with the bitValue
    out_temp.extendFrom = op0->extendFrom-minShift;
    return out_temp;
}

triStateMask MinimizeBitwidth::forwardPropagateSDiv(triStateMask *out, triStateMask *op0, triStateMask *op1) {
    triStateMask out_temp = *out;
    uint8_t minShift = (int)(log2(op1->minPossibleVal().lower()));    
    if(op1->isNegative()) minShift=0; 
    //errs()<<"minShift: "<<utostr(minShift)<<"minVal:"<<utostr(op1->minPossibleVal().lower())<<" op0->extendFrom:"<<utostr(op0->extendFrom)<<"\n";
    //op1->print();
    //printf("\n");
//    assert(minShift<=op0->extendFrom && "minShift must be less than bitwidth of out\n");
    if(op0->extendFrom>=minShift) out_temp.extendFrom = op0->extendFrom-minShift;
    else out_temp.extendFrom=0;
    //if either operand is signed
    if(op1->MSB==op1->bitwidth-1 || op0->MSB==op0->bitwidth-1) {
        out_temp.MSB=out_temp.bitwidth-1;
        if(op0->MSB!=op0->bitwidth-1) 
            out_temp.extendFrom++;
    }
    else {
        out_temp.MSB=out_temp.extendFrom;
    }
    out_temp.LSB=0;
    out_temp.bitIsDynamic = triStateMask::biggestVal(out_temp.MSB+1);
    out_temp.bitValue = 0; //don't bother with the bitValue
/*    if(out_temp.extendFrom > op0->extendFrom) {
        errs() << "op0 bitwidth: "<<utostr(op0->bitwidth)<<" MSB: "<<utostr(op0->MSB)<<" op0 extendFrom: "<<utostr(op0->extendFrom)<<"\n";
        errs() << "op1 MSB: "<<utostr(op1->MSB)<<" op1 extendFrom:"<<utostr(op1->extendFrom)
               <<"minShift:"<<utostr(minShift)<<"\n";
        errs() << "MSB: "<<utostr(out_temp.MSB)<<" extendFrom: "<<utostr(out_temp.extendFrom)<<"\n";
    }*/
    return out_temp;
}

triStateMask MinimizeBitwidth::forwardPropagateURem(triStateMask *out, triStateMask *op0, 
                                                    triStateMask *op1) {
    triStateMask out_temp = *out;
    //if this is unsigned, MSB can't be larger than op1
    out_temp.MSB=op1->MSB;
    out_temp.LSB=0;
    out_temp.bitIsDynamic = triStateMask::biggestVal(out_temp.MSB+1);
    out_temp.bitValue = 0; //don't bother with the bitValue
    out_temp.extendFrom = op1->extendFrom;
    return out_temp;
}

triStateMask MinimizeBitwidth::forwardPropagateSRem(triStateMask *out, triStateMask *op0, 
                                                    triStateMask *op1) {
    triStateMask out_temp = *out;
    //if this is unsigned, MSB can't be larger than op1
    if(op0->MSB==op0->bitwidth-1) out_temp.MSB=out_temp.bitwidth-1;
    out_temp.LSB=0;
    out_temp.bitIsDynamic = triStateMask::biggestVal(out_temp.MSB+1);
    out_temp.bitValue = 0; //don't bother with the bitValue
    //if op1 is unsigned, we need to treat it as a signed number since it's in a SRem.
    //This means using the first 0-bit in op1 as the extendFrom of out_temp, which is actually
    //op1->extendFrom+1
    if(op1->extendFrom==op1->MSB)
        out_temp.extendFrom = std::min(op1->extendFrom+1,(int)op0->extendFrom);
    else
        out_temp.extendFrom = op1->extendFrom;
    
    return out_temp;
}

triStateMask MinimizeBitwidth::forwardPropagateSelect(triStateMask *out, triStateMask *op0, 
                                                        triStateMask *op1,triStateMask *op2) {
    triStateMask out_temp = *out;
    out_temp.MSB = std::max(op1->MSB,op2->MSB);
    out_temp.LSB = std::min(op1->LSB,op2->LSB);
    out_temp.extendFrom = std::max(op1->extendFrom,op2->extendFrom);
    out_temp.bitIsDynamic = (op1->bitIsDynamic | op2->bitIsDynamic) | 
                            (op1->bitValue^op2->bitValue);
    out_temp.bitValue = op1->bitValue & ~out_temp.bitIsDynamic;
    out_temp.updateLSB();
    return out_temp;
}


triStateMask MinimizeBitwidth::forwardPropagatePHI(triStateMask *out, Instruction *inst) {
    triStateMask out_temp = *out;
    PHINode *phi = dyn_cast<PHINode>(inst);
    for(unsigned i=0;i<phi->getNumIncomingValues();i++) {
        Value *phi_in = phi->getIncomingValue(i);
        if(triStateMap.find(phi_in) == triStateMap.end()) {
            int64_t maxVal,minVal;
            bool isPositive,maxIsPositive;
            int maxBitwidth;
            if(getRange(phi_in,&maxVal,&minVal,&isPositive,&maxIsPositive)) 
                maxBitwidth=getMaxBitwidth(maxVal);
            else maxBitwidth=0;
            triStateMap[phi_in]=triStateMask(phi_in,maxBitwidth,maxIsPositive,isPositive,
                                             USE_RANGE_FILE);
        }
        if(i==0) out_temp = triStateMap[phi_in];
        else {
         out_temp.max(triStateMap[phi_in],out_temp);
        }
    }
    out_temp.updateMSB();
    out_temp.updateLSB();
    out_temp.updateExtendFrom();
    return out_temp;
}


/*
 * forwardPropagate modifies the bitmask associated with instruction "inst" by
 * applying a forward transit function to the operand bitmasks.  Each type of instruction
 * has a different transit function.
 */
bool MinimizeBitwidth::forwardPropagate(Instruction *inst){

    triStateMask *op0,*op1,*op2,*out;
    uint128_t mask,ones,maybeZero,onesMask,bitIsNotDynamic,bitValue,combinedStatic,op0IsZero,op1IsZero;
    uint128_t noCout,yesCout,maybeCout,max_shift;
     
    getTriStateMasks(inst,&op0,&op1,&op2,&out); //populate op0,op1,op2,out based on inst
    triStateMask out_temp = *out; //temporary value of out that we'll modify in this function
    unsigned Opcode = inst->getOpcode();
    switch(Opcode) {
        case Instruction::ZExt:
            out_temp = forwardPropagateZExt(out,op0);
            break;
        case Instruction::SExt:
            out_temp = forwardPropagateSExt(out,op0);
            break;
        case Instruction::Trunc :
            out_temp = forwardPropagateTrunc(out,op0);
            break;
        case Instruction::And :
            out_temp = forwardPropagateAnd(out,op0,op1);
            break;
        case Instruction::Or :
            out_temp = forwardPropagateOr(out,op0,op1);
            break;
        case Instruction::Xor :
            out_temp = forwardPropagateXor(out,op0,op1);
            break;
        case Instruction::Sub :    
            out_temp = forwardPropagateSub(out,op0,op1);
            break;
        case Instruction::Add :
            out_temp = forwardPropagateAdd(out,op0,op1,false);
            break; 
        case Instruction::Mul :
            out_temp = forwardPropagateMul(out,op0,op1);
            break;
        case Instruction::AShr :
            out_temp = forwardPropagateAShr(out,op0,op1);
            break;
        case Instruction::LShr :
            out_temp = forwardPropagateLShr(out,op0,op1);
            break;
        case Instruction::Shl :
            out_temp = forwardPropagateShl(out,op0,op1);
            break;
        case Instruction::UDiv :
            out_temp = forwardPropagateUDiv(out,op0,op1);
            break;
        case Instruction::SDiv :
            out_temp = forwardPropagateSDiv(out,op0,op1);
            break;
        case Instruction::URem :
            out_temp = forwardPropagateURem(out,op0,op1);
            break;
        case Instruction::SRem :
            out_temp = forwardPropagateSRem(out,op0,op1);
            break;
        case Instruction::Select:
            out_temp = forwardPropagateSelect(out,op0,op1,op2);
            break;
        case Instruction::PHI:
            out_temp = forwardPropagatePHI(out,inst);
            break;
        default: break;
    }

    triStateMask outBak = *out;

    out->min(*out,out_temp);    
    out->updateMSB();
    out->updateLSB();
    out->updateExtendFrom();


    // Sanity check
    /*
    std::string error = "";
    if(out->MSB > out->extendFrom && out->MSB<(out->bitwidth-1)) 
        error = "forwardPropagate: Signed number with one or more leading zero\n";
    if(out->MSB==0 && out->bitValue==0 && out->bitIsDynamic>1)
        error = "forwardPropagate: loose bitIsDynamic bits\n";
    if(out->LSB>0 && triStateMask::pow2(out->LSB-1)&(out->bitIsDynamic|out->bitValue))
        error = "fowardPropagate: LSB-1 is dynamic or static 1\n";
    if(out->MSB>0 && !(triStateMask::pow2(out->LSB)&(out->bitIsDynamic|out->bitValue)))
        error = "forwardPropagate: LSB is static 0\n";
    uint128_t temp0 = out->bitIsDynamic|out->bitValue;
    uint128_t temp1 = triStateMask::pow2(out->MSB);
    if(out->MSB!=0 && (!(temp0 & temp1))) error = "forwardPropagate: MSB of in is 0\n";
    if(out->extendFrom<out->LSB) error="forwardPropagate: extendFrom is < LSB\n";
    if(out->LSB > out->MSB) error="forwardPropagate: LSB > MSB\n";
    if(!out->bitIsDynamic && out->MSB!=out->extendFrom) 
        error="forwardPropagate: extendFrom < MSB when not dynamic\n";
    if(error!="")
    {
        printf("%s\n",error.c_str());
        printValue(inst);        
        printf("\n%s: ",inst->getOpcodeName());
        if(op0) op0->print();
        printf(" OP ");
        if(op1) op1->print();
        if(op2)
        {
            printf(" OP ");
            op2->print();
        }
        printf(" = ");
        if(out) out->print();
        printf("\n");
        printf("min(new:");
        out_temp.print();
        printf(",old:");
        outBak.print();
        printf(")\n");
        if(error!="printALL") assert(0);
    }*/
    //if there's no change in the output bitmask, return false
    if(outBak==*out) return false;
    return true;
}



bool MinimizeBitwidth::doInitialization(Module &M) {
   Mod = &M;

   return false;
}

bool MinimizeBitwidth::doFinalization(Module &M) {
    return false;
}

bool MinimizeBitwidth::canHaveRange(Instruction *inst) {

#ifdef USE_RANGES
    if(!inst) return false;
    switch(inst->getOpcode()) {
        case Instruction::Add:
        case Instruction::Sub:
        case Instruction::Mul:
        case Instruction::SRem:
        case Instruction::URem:
        case Instruction::SDiv:
        case Instruction::UDiv:
        case Instruction::And:
        case Instruction::Or:
        case Instruction::Xor:
        case Instruction::Shl:
        case Instruction::AShr:
        case Instruction::LShr:
        case Instruction::PHI:
        case Instruction::Select: return true;
        default: return false;
    }
#else
    return false;
#endif
}

bool MinimizeBitwidth::getRange(Value *inst,int64_t *max,int64_t *min,bool *isPositive, bool *maxIsPositive)
{
    std::string myStr(""); // declare empty string
    llvm::raw_string_ostream myStream(myStr); // create a string stream 
    inst->print(myStream); // print instruction name to the string [note that this includes the %N -- the destination register!]
    const char* ValueName = myStream.str().data();
    unsigned i=0;
    while(isspace(ValueName[i])) i++;
    if(ValueName[i]=='%')
    {
        std::string instrNum = ValueName+i;
        if(maxInstrVal.find(instrNum) != maxInstrVal.end())
        {
            *min = minInstrVal[instrNum];
            *max = maxInstrVal[instrNum];
            *isPositive = rangeIsPositive[instrNum];
            *maxIsPositive = maxRangeIsPositive[instrNum]; 
//           if(dyn_cast<Instruction>(inst)->getOpcode()==Instruction::AShr)
//               printf("ValueName:%s has value range (%lld->%lld)\n",instrNum.c_str(),*min,*max);
        return true;
        }
        else if(rangeIsValid.find(instrNum)!=rangeIsValid.end() && 
                !rangeIsValid[instrNum]) 
        {
            printf("range is not valid for ");
            printValue(inst);
            printf("\n");
            return false;
        }
        else if(canHaveRange(dyn_cast<Instruction>(inst)))
        {
            printf("Assuming the following can be optimized out:\n");
            printValue(inst);
            printf("\n");
            *max = *min = 0;
            *isPositive = *maxIsPositive = true;                
            return true;
        }
    }
    return false;
}

int64_t MinimizeBitwidth::myllabs(int64_t n)
{
    if(n<=-9223372036854775807LL || n==9223372036854775807LL) return 9223372036854775806LL;
    else return llabs(n);
}

int MinimizeBitwidth::getMaxBitwidth(int64_t maxVal)
{
    if(maxVal==9223372036854775807LL || maxVal<=-9223372036854775807LL) return 63;
    if(maxVal==0) return -1;
    else return (uint8_t)(1+(log2(myllabs(maxVal))));
}

void MinimizeBitwidth::verifyRangeMap()
{
    errs() << "Verifying that static ranges do not fall outside of dynamic ranges\n";
    char buffer[10000];
    std::string instrNum;
    int64_t instrVal;
    FILE *fp = fopen("range.profile.dynamic","r");
    while(fscanf(fp,"%s",buffer)!=EOF)
    {
        if(strlen(buffer)>=100) {
            while(getc(fp)!='\n') {};
            continue;
        }
        instrNum=buffer;    
        bool first=true;
        while(strcmp(buffer,"RESULT:"))
        {
            if(first) first=false;
            else instrNum=instrNum+" "+buffer;
            fscanf(fp,"%s",buffer);
        }
        fscanf(fp,"%s",buffer);
        if(!strncmp(buffer,"INVALID",7))
        {
            rangeIsValid[instrNum]=false;
            errs() << "range is invalid for "+instrNum+"\n";
            continue;
        }            
        instrVal = strtoll(buffer,NULL,10);
        if(instrVal==-1) instrVal=1;
        if(minInstrVal.find(instrNum) != minInstrVal.end()) {
            if(instrVal<0 && rangeIsPositive[instrNum]) {
                errs() << "Error: instrVal = "<<instrVal<<" but static range is positive\n"<<instrNum<<"\n";
                llvm_unreachable(0);
            }
            else if(myllabs(instrVal)>myllabs(maxInstrVal[instrNum])) {
                errs() << "Error: instrVal = "<<instrVal<<" which is more than maximum value in static range: "<<maxInstrVal[instrNum]<<"\n"<<instrNum<<"\n";
                llvm_unreachable(0);
            }
        }
    }
    fclose(fp);
}

void MinimizeBitwidth::populateRangeMap(std::string filename)
{
    assert(filename!="" && "Can't read range map from nameless file");
    char buffer[10000];
    std::string instrNum;
//    int instrNum;
    int64_t instrVal;
    FILE *fp = fopen(filename.c_str(),"r");
    while(fscanf(fp,"%s",buffer)!=EOF) {
        if(strlen(buffer)<100) {
            instrNum=buffer;    
            bool first=true;
            while(strcmp(buffer,"RESULT:")) {
                if(first) first=false;
                else instrNum=instrNum+" "+buffer;
                assert(fscanf(fp,"%s",buffer));
            }
            assert(fscanf(fp,"%s",buffer));
            if(!strncmp(buffer,"INVALID",7)) {
                rangeIsValid[instrNum]=false;
                errs()<< "range is invalid for "<< instrNum <<"\n";

                continue;
            }            
            instrVal = strtoll(buffer,NULL,10);
            //if(instrVal==-1) instrVal=1;
            if(minInstrVal.find(instrNum) == minInstrVal.end()) {
                rangeIsValid[instrNum]=true;
                minInstrVal[instrNum] = instrVal;
                maxInstrVal[instrNum] = instrVal;
                rangeIsPositive[instrNum] = instrVal>=0?true:false;
                maxRangeIsPositive[instrNum] = true;
            }
            else {

                if(instrVal<0) rangeIsPositive[instrNum] = false;
                if(myllabs(instrVal)<myllabs(minInstrVal[instrNum])) {
                    minInstrVal[instrNum] = instrVal;
                }
                if(myllabs(instrVal)>myllabs(maxInstrVal[instrNum])) {
                    maxInstrVal[instrNum] = instrVal;
                }

            }
        }
        else {
            //get rid of rest of line
            while(getc(fp)!='\n') {};
        }
    }
    fclose(fp);
   /* printf("Min and Max values of instructions:\n");
    std::map<std::string, long>::const_iterator itr;
    std::map<std::string, long>::const_iterator itr2;
    for(itr = minInstrVal.begin(),itr2 = maxInstrVal.begin(); itr != minInstrVal.end();++itr,++itr2)
    {
            printf("%s (%ld->%ld)\n",itr->first.c_str(),itr->second,itr2->second);
    }*/
}

bool MinimizeBitwidth::countThisInst(Instruction *inst)
{
    if(!inst) return false;
    switch (inst->getOpcode()) {
        case Instruction::Add:  ;
        case Instruction::Sub:  ;
        case Instruction::Mul:  ;
        case Instruction::SRem: ;
        case Instruction::URem: ;
        case Instruction::SDiv: ;
        case Instruction::UDiv: ;
        case Instruction::And:  ;
        case Instruction::Or:   ;
        case Instruction::Xor:  ;
        case Instruction::Shl:  ;
        case Instruction::AShr: ;
        case Instruction::LShr: ;
        case Instruction::ICmp: ;
        case Instruction::Select: ;
        case Instruction::PHI: return true;
        default: return false;
    }
}

void MinimizeBitwidth::printStats(Function &F) {
    triStateMask *out;
    for (Function::iterator BB = F.begin(), EE = F.end(); BB != EE; ++BB) {
        for (BasicBlock::iterator I = BB->begin(), E = BB->end(); I !=
                E; ++I) {
            Instruction *inst = dyn_cast<Instruction>(I);
            if (!countThisInst(inst)) continue;
            getTriStateMasks(inst,NULL,NULL,NULL,&out);
            if(oldInstrLSB.find(inst)!=oldInstrLSB.end())
            {
                totalBits[inst->getOpcode()]-=out->bitwidth;    
                lowBitsEliminated[inst->getOpcode()]-=oldInstrLSB[inst];    
                highBitsEliminated[inst->getOpcode()]-=(out->bitwidth-(oldInstrExtendFrom[inst]+1));    
                bitsEliminated[inst->getOpcode()]-=oldBitsEliminated[inst];
            }

            uint8_t dynamicCount = 0;
            if(totalBits.find(inst->getOpcode())==totalBits.end())
            {
                totalBits[inst->getOpcode()]=out->bitwidth;
                lowBitsEliminated[inst->getOpcode()]=out->LSB;
                highBitsEliminated[inst->getOpcode()]=out->bitwidth - (out->extendFrom+1);
                uint128_t mask=1;
                for(uint8_t i = 0;i<out->bitwidth;i++,mask=mask<<1) {
                    if((out->bitIsDynamic&mask) && (i<=out->extendFrom)) dynamicCount++;
                }   
                bitsEliminated[inst->getOpcode()]=out->bitwidth - dynamicCount;
            }
            else
            {
                totalBits[inst->getOpcode()]+=out->bitwidth;
                lowBitsEliminated[inst->getOpcode()]+=out->LSB;
                highBitsEliminated[inst->getOpcode()]+=(out->bitwidth - (out->extendFrom+1));
                uint128_t mask=1;
                for(uint8_t i = 0;i<out->bitwidth;i++,mask=mask<<1) {
                    if((out->bitIsDynamic&mask) && i<=out->extendFrom) dynamicCount++;
                }   
                bitsEliminated[inst->getOpcode()]+=out->bitwidth - dynamicCount;

            }
            oldInstrLSB[inst]=out->LSB;
            oldInstrExtendFrom[inst]=out->extendFrom;
            oldBitsEliminated[inst]=out->bitwidth-dynamicCount;
        }
    }
    printf("Percent Reduction for each instruction type:\n");
    std::map<unsigned, unsigned long>::const_iterator itr1;
    std::map<unsigned, unsigned long>::const_iterator itr2;
    std::map<unsigned, unsigned long>::const_iterator itr3;
    std::map<unsigned, unsigned long>::const_iterator itr4;
    printf("%-10s %-10s %-10s %-10s %-10s %-10s %-10s %-10s \n", "op",
           "total bits", "bitsElim", "%", "highElim", "high %", "lowElim",
           "low%");
    unsigned total_bits = 0;
    unsigned total_high_bits_eliminated = 0;
    unsigned total_low_bits_eliminated = 0;
    unsigned total_bits_eliminated = 0;
    for (itr1 = totalBits.begin(), itr3 = lowBitsEliminated.begin(),
        itr2 = highBitsEliminated.begin(), itr4 = bitsEliminated.begin();
         itr1 != totalBits.end(); ++itr1, ++itr2, ++itr3, ++itr4) {
        std::string name;
        switch (itr1->first) {
            case Instruction::Add:  name = "Add"; break;
            case Instruction::Sub:  name = "Sub"; break;
            case Instruction::Mul:  name = "Mul"; break;
            case Instruction::SRem: name = "Srem"; break;
            case Instruction::URem: name = "Urem"; break;
            case Instruction::SDiv: name = "Sdiv"; break;
            case Instruction::UDiv: name = "Udiv"; break;
            case Instruction::And:  name = "And"; break;
            case Instruction::Or:   name = "Or"; break;
            case Instruction::Xor:  name = "Xor"; break;
            case Instruction::Shl:  name = "Shl"; break;
            case Instruction::AShr: name = "Ashr"; break;
            case Instruction::LShr: name = "Lshr"; break;
            case Instruction::ICmp: name = "ICmp"; break;
            case Instruction::Select: name = "Select"; break;
            case Instruction::PHI: name = "PHI"; break;
            default: name = "INVALID"; break;
        }
        if (name != "INVALID") {
            total_bits += itr1->second;
            total_high_bits_eliminated += itr2->second;
            total_low_bits_eliminated += itr3->second;
            total_bits_eliminated += itr4->second;
            printf("%-10s %-10lu %-10lu %-10f %-10lu %-10f %-10lu %-10f\n",
                   name.c_str(), itr1->second, itr4->second,
                   (double)itr4->second / itr1->second, itr2->second,
                   (double)itr2->second / itr1->second, itr3->second,
                   (double)itr3->second / itr1->second);
        } else {
            printf("Invalid instruction type in map\n");
        }
    }
    printf("Total percentage bitwidth reduction:\n");
    printf("%-10s %-10s %-10s %-10s %-10s %-10s %-10s \n", "total bits",
           "bitsElim", "%", "highElim", "high %", "lowElim", "low%");
    printf("%-10d %-10d %-10f %-10d %-10f %-10d %-10f \n", total_bits,
           total_bits_eliminated, (double)total_bits_eliminated / total_bits,
           total_high_bits_eliminated,
           (double)total_high_bits_eliminated / total_bits,
           total_low_bits_eliminated,
           (double)total_low_bits_eliminated / total_bits);
}

void MinimizeBitwidth::minInductionVars() {
    LoopInfo &LI = getAnalysis<LoopInfo>();
    ScalarEvolution *SE = &getAnalysis<ScalarEvolution>();

    for (LoopInfo::iterator i = LI.begin(), e = LI.end(); i != e; ++i) {
        //This is for subLoops ... and you can iteratively do it as the same.
        //std::vector<Loop*> subLoops = i->getSubLoops();
        Loop *thisLoop = *i;
        PHINode *indv = thisLoop->getCanonicalInductionVariable();
        Value *val = indv;
        if (indv) {
            const SCEV *thisScev = SE->getMaxBackedgeTakenCount(thisLoop);
            if (isa<SCEVCouldNotCompute>(thisScev))
                continue;
            std::string myStr("");                    // declare empty string
            llvm::raw_string_ostream myStream(myStr); // create a string stream
            thisScev->print(myStream);
            int64_t tripCount;
            //            std::stringstream str(myStream.str());
            //            str >> testBitWidth;
            std::stringstream s_str(myStream.str());
            s_str >> tripCount;
            if (tripCount <= 0)
                continue;
            tripCount++;
            // errs()<<"tripCount: "<<utostr(tripCount)<<"maxBitwidth:
            // "<<utostr(getMaxBitwidth(tripCount))<<"\n";
            triStateMap[val] = triStateMask(val, getMaxBitwidth(tripCount),
                                            true, true, USE_RANGE_FILE);
            //            errs() << "Set max val of ";
            //            printValue(val);
            //            errs() << " to "<<utostr(tripCount)<<" with
            //            MSB="<<utostr(triStateMap[val].MSB)<<"\n";
        }
    }

}

/*
 * This is the top level of the MinimizeBitwidth pass
 * This function repeatedly propagates bitmasks associated with each instruction in the forward
 * direction and in the backward direction until no more bitwidth reductions are possible.
 */
bool MinimizeBitwidth::runOnFunction(Function &F) {


    std::string range_filename = LEGUP_CONFIG->getParameter("MB_RANGE_FILE");
    USE_RANGE_FILE = range_filename!="";
    //populates RangeMap from filename specific in legup.tcl
    if(USE_RANGE_FILE) populateRangeMap(range_filename); 
    //uncomment verifyRangeMap() if you want to ensure that static ranges fall within dynamic ranges
    //verifyRangeMap(); 
    triStateMap.clear(); //clear map of instructions to bitmasks
    minInductionVars();
    // propagate the bitwidth changes FORWARD in the data flow graph
    //as long as there are changes in the back propagation, we want to keep checking if we can do
    // any more on the forward propagation
    int numPasses = 0;
    int maxNumPasses = LEGUP_CONFIG->getParameterInt("MB_MAX_BACK_PASSES");
    bool minBitwidth = LEGUP_CONFIG->getParameterInt("MB_MINIMIZE_HW");
    bool changed_in_back_propagation = true;
    while (changed_in_back_propagation) {
        changed_in_back_propagation = false;
        //propagate bitwidths forward
        //errs()<<"forward\n";
        for (Function::iterator BB = F.begin(), EE = F.end(); BB != EE; ++BB) {
            for (BasicBlock::iterator I = BB->begin(), E = BB->end(); I != E; ++I) {
                Instruction *inst = dyn_cast<Instruction>(I);
                if (!inst) continue;
                forwardPropagate(inst);
            }
        }
        // errs()<<"backward\n";

        // propagate the bitwidth changes BACKWARDS in the data flow graph
        if (minBitwidth && (maxNumPasses == -1 || numPasses < maxNumPasses)) {
            for (Function::iterator BB = F.begin(), EE = F.end(); BB != EE;
                 ++BB) {
                for (BasicBlock::iterator I = BB->begin(), E = BB->end();
                     I != E; ++I) {
                    Instruction *inst = dyn_cast<Instruction>(I);
                    if (!inst)
                        continue;
                    if (backwardPropagate(inst))
                        changed_in_back_propagation = true;
                }
            }
        }
        numPasses++;
    }

    //#ifdef PRINT_MINIMIZE_STATS
    if (LEGUP_CONFIG->getParameterInt("MB_PRINT_STATS")) {
        printStats(F);
    }
//#endif
    return false;
}

void rewriteIR(Instruction *inst, Module *Mod, unsigned minWidth, Value *reg,
               APInt value) {
    // errs() << *inst << "\n";
    // errs() << minWidth << "\n";
    IntegerType *newWidthType = IntegerType::get(Mod->getContext(), minWidth);
    Instruction *trunc =
        CastInst::CreateTruncOrBitCast(reg, newWidthType, "min_width", inst);

    APInt newOpValue = value.trunc(minWidth);
    Constant *newOp = ConstantInt::get(trunc->getType(), newOpValue);

    Instruction *newAnd = BinaryOperator::Create(Instruction::And, trunc, newOp,
            "", inst);
    //BasicBlock::iterator ii(inst);
    //ReplaceInstWithInst(inst->getParent()->getInstList(), inst, newAnd);

    Instruction *zext =
        CastInst::CreateZExtOrBitCast(newAnd,
                inst->getType(), "min_width", inst);
    inst->replaceAllUsesWith(zext);
    //inst->replaceAllUsesWith(newAnd);
}

} // end of legup namespace

using namespace legup;


