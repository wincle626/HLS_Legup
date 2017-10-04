//===- MinimizeBitwidth.h -------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This pass shrinks arbitrary precision integer bitwidths from standard
// C sizes: i8, i16, i32, i64 to arbritrary precisions i31, i9 etc.
// Having a better idea of integer widths aids functional unit binding
//
//===----------------------------------------------------------------------===//

#ifndef MINIMIZE_BITWIDTH_H
#define MINIMIZE_BITWIDTH_H

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "utils.h"
#include "uint128_t.h"
#include "LegupConfig.h"
#include <stdio.h>
#include <map>

using namespace llvm;

namespace legup {

enum RangeType {Unknown, Regular, Empty};

class Range {
private:
	APInt l;	// The lower bound of the range.
	APInt u;	// The upper bound of the range.
	RangeType type;

public:
	Range();
	Range(APInt lb, APInt ub, RangeType type = Regular);
	~Range();
	APInt getLower() const {return l;}
	APInt getUpper() const {return u;}
	void setLower(const APInt& newl) {this->l = newl;}
	void setUpper(const APInt& newu) {this->u = newu;}
	bool isUnknown() const {return type == Unknown;}
	void setUnknown() {type = Unknown;}
	bool isRegular() const {return type == Regular;}
	void setRegular() {type = Regular;}
	bool isEmpty() const {return type == Empty;}
	void setEmpty() {type = Empty;}
	bool isMaxRange() const;
	void print(raw_ostream& OS) const;
	Range add(const Range& other);
	Range sub(const Range& other);
	Range mul(const Range& other);
	Range udiv(const Range& other);
	Range sdiv(const Range& other);
	Range urem(const Range& other);
	Range srem(const Range& other);
	Range shl(const Range& other);
	Range lshr(const Range& other);
	Range ashr(const Range& other);
	Range And(const Range& other);
	Range Or(const Range& other);
	Range Xor(const Range& other);
	Range truncate(unsigned bitwidth) const;
//	Range signExtend(unsigned bitwidth) const;
//	Range zeroExtend(unsigned bitwidth) const;
	Range sextOrTrunc(unsigned bitwidth) const;
	Range zextOrTrunc(unsigned bitwidth) const;
	Range intersectWith(const Range& other) const;
	Range unionWith(const Range& other) const;
	bool operator==(const Range& other) const;
	bool operator!=(const Range& other) const;
};


/*
 * triStateMask represents a 64 bit bit mask for an up to 64 bit wide signal, where each bit in
 * the bitmask corresponds to one of four things.
 * 1. A static 0 value independent of the circuit inputs
 * 2. A static 1 value independent of the circuit inputs
 * 3. An unknown value (could be 0 or 1 based on circuit inputs)
 * 4. A sign extended value
 * To represent such a mask, three uint128_t values are maintained.  
 * bitIsDynamic - each bit is 0 if the corresponding bit in the signal is known, and 1 otherwise
 * bitValue - If bitIsDynamic[i] is 0, bitValue[i] is equal signal[i]
 *            - else, bitValue[i] = 0
 *  bitIsSext - If bitIsDynamic[i] is 1, bitIsSext[i] is 1 if signal[i] is sign extended from signal[MSB]
 *            - else bitIsSext[i] = 0
 * MSB is the index of the left-most non-zero bit
 * LSB is the index of the right-most non-zero bit
 */
class triStateMask {
    friend bool operator==(const triStateMask& a, const triStateMask& b);
    public:
        uint128_t bitIsDynamic;
        uint128_t bitValue;
        uint8_t bitwidth,MSB,LSB,extendFrom;
        //64 1s
        triStateMask (Value *inst,int maxBitwidth,bool maxIsPositive, bool isPositive, bool USE_RANGE_FILE);
        triStateMask ();
        void print ();
        uint128_t maxPossibleVal();
        uint128_t minPossibleVal();
        bool isPositive();
        bool isNegative();
        void max(triStateMask a,triStateMask b);
        void min(triStateMask a,triStateMask b);
        void updateLSB();
        void updateMSB();
        void updateExtendFrom();
        void setLimits(uint8_t new_LSB,uint8_t new_MSB);
        static uint128_t pow2(uint8_t exp)
        {
            if(exp>=128) printf("exp:%u\n",exp);
            assert(exp<128);
            return (((uint128_t)1)<<exp);
        }

        static uint128_t biggestVal(uint8_t bitwidth)
        {
            uint128_t temp;
            if(bitwidth==0) temp=uint128_t(0,0);
            else if(bitwidth<=64) temp=uint128_t(0,(uint64_t)0xFFFFFFFFFFFFFFFFLL>>((uint8_t)(64-bitwidth)));
            else if(bitwidth<=128)
            {
                temp=uint128_t((uint64_t)0xFFFFFFFFFFFFFFFFLL>>((uint8_t)(128-bitwidth)),(uint64_t)0xFFFFFFFFFFFFFFFFLL);
            }
            else
            {
                printf("error bitwidth>128 = %u\n",bitwidth);
                assert(bitwidth<=128);
            }
            return temp;
        }
        static uint128_t getOnesMask(uint8_t right,uint8_t left)
        {
            uint128_t onesLeft = biggestVal(left);
            uint128_t onesRight = biggestVal(right);
            return (onesLeft^onesRight);
        }
        void printValue(Value *inst);
    private:
};




class MinimizeBitwidth : public FunctionPass {
    public:
        static char ID; // Pass identification, replacement for typeid
        MinimizeBitwidth() : FunctionPass(ID) {}

        virtual bool doInitialization(Module &M);

        virtual bool runOnFunction(Function &F);

        virtual bool doFinalization(Module &M);
        uint8_t getMSB(const Value *I){ return triStateMap[I].MSB;}
        uint8_t getLSB(const Value *I){ return triStateMap[I].LSB;}
        uint8_t getExtendFrom(const Value *I){ return triStateMap[I].extendFrom;}
        uint8_t getBitwidth(const Value *I){ return triStateMap[I].bitwidth;}
        bool bitwidthIsKnown(const Value *I) { if(triStateMap.find(I)==triStateMap.end()) return false;
            return (triStateMap[I].extendFrom != 127);
        }
        uint8_t getMinBitwidth(const Value *I) {
            assert(triStateMap.find(I) != triStateMap.end() &&
                   "Value not found in triStateMap\n");
            if (!LEGUP_CONFIG->getParameterInt("MB_MINIMIZE_HW"))
                return (triStateMap[I].bitwidth);
            else
                return (triStateMap[I].extendFrom + 1);
        }
        bool isSigned(const Value *I) {
            assert(triStateMap.find(I) != triStateMap.end() && "Value not found in triStateMap\n");
            return (triStateMap[I].MSB > triStateMap[I].extendFrom);
        }
        void populateRangeMap(std::string filename);
        void verifyRangeMap();

        virtual void getAnalysisUsage(AnalysisUsage &AU) const {
            AU.setPreservesAll();
            AU.addRequired<LoopInfo>();
            AU.addRequired<ScalarEvolution>();
        }

        void minInductionVars();
        virtual bool forwardPropagate(Instruction *inst);
        virtual bool backwardPropagate(Instruction *inst);
        int64_t myllabs(int64_t n);
        bool getRange(Value *inst,int64_t *max, int64_t *min,bool *isPositive,bool *maxIsPositive);

        virtual void getTriStateMasks(Instruction *inst,triStateMask **op0,triStateMask **op1,
                                      triStateMask **op2, triStateMask **out);
        virtual bool countThisInst(Instruction *inst);
        void printValue(Value *inst);
        bool canHaveRange(Instruction *inst);
        int getMaxBitwidth(int64_t maxVal);
        void printStats(Function &F);

    private:


        triStateMask forwardPropagateZExt(triStateMask *out, triStateMask *op0);
        triStateMask forwardPropagateSExt(triStateMask *out, triStateMask *op0); 
        triStateMask forwardPropagateTrunc(triStateMask *out, triStateMask *op0);
        triStateMask forwardPropagateAnd(triStateMask *out, triStateMask *op0, triStateMask *op1);
        triStateMask forwardPropagateOr(triStateMask *out, triStateMask *op0, triStateMask *op1);
        triStateMask forwardPropagateXor(triStateMask *out, triStateMask *op0, triStateMask *op1);
        triStateMask forwardPropagateAdd(triStateMask *out, triStateMask *op0, 
                                         triStateMask *op1, bool cin);
        triStateMask forwardPropagateSub(triStateMask *out, triStateMask *op0, triStateMask *op1);
        triStateMask forwardPropagateMul(triStateMask *out, triStateMask *op0, triStateMask *op1);
        triStateMask forwardPropagateAShr(triStateMask *out, triStateMask *op0, triStateMask *op1);
        triStateMask forwardPropagateLShr(triStateMask *out, triStateMask *op0, triStateMask *op1);
        triStateMask forwardPropagateShl(triStateMask *out, triStateMask *op0, triStateMask *op1);
        triStateMask forwardPropagateUDiv(triStateMask *out, triStateMask *op0, triStateMask *op1);
        triStateMask forwardPropagateSDiv(triStateMask *out, triStateMask *op0, triStateMask *op1);
        triStateMask forwardPropagateURem(triStateMask *out, triStateMask *op0, triStateMask *op1);
        triStateMask forwardPropagateSRem(triStateMask *out, triStateMask *op0, triStateMask *op1);
        triStateMask forwardPropagateSelect(triStateMask *out, triStateMask *op0, 
                                            triStateMask *op1,triStateMask *op2);
        triStateMask forwardPropagatePHI(triStateMask *out, Instruction *inst);
        triStateMask backwardPropagateExt(triStateMask *out, triStateMask *in);
        triStateMask backwardPropagateXor(triStateMask *out, triStateMask *in, triStateMask *op0,
                                          triStateMask *op1, Instruction *use);
        triStateMask backwardPropagateMul(triStateMask *out, triStateMask *in, 
                                          triStateMask *op0, triStateMask *op1);
        triStateMask backwardPropagateShr(triStateMask *prev_in_temp, triStateMask *out, 
                                          triStateMask *in, 
                                          triStateMask *op0, triStateMask *op1);
        triStateMask backwardPropagateShl(triStateMask *prev_in_temp, triStateMask *out,
                                          triStateMask *in, 
                                          triStateMask *op0, triStateMask *op1);
        triStateMask backwardPropagatePHI(triStateMask *in, Instruction *use);
        triStateMask* getOp(Instruction *inst, int op_index, int64_t maxVal, int64_t minVal,
                            bool isPositive, bool maxIsPositive);
        std::map<const Value *, triStateMask> triStateMap;
        std::map<std::string,int64_t> minInstrVal;
        std::map<std::string,int64_t> maxInstrVal;
        std::map<std::string,bool> rangeIsPositive;
        std::map<std::string,bool> maxRangeIsPositive;
        std::map<std::string,bool> rangeIsValid;
        //std::map<Instruction *, unsigned> minBitwidthMap;
        Module *Mod;
        std::map<unsigned,unsigned long> highBitsEliminated;
        std::map<unsigned,unsigned long> lowBitsEliminated;
        std::map<unsigned,unsigned long> bitsEliminated;
        std::map<unsigned,unsigned long> totalBits;
        std::map<Value *,uint8_t> oldInstrExtendFrom;
        std::map<Value *,uint8_t> oldInstrLSB;
        std::map<Value *,uint8_t> oldBitsEliminated;
        bool USE_RANGE_FILE;

};


} // End of LegUp namespace

//using namespace legup;

#endif

