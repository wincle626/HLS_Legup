//===- LVA.cpp ------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements Live Variable Analysis as in:
// http://cursuri.cs.pub.ro/~cpl/wiki/images/1/15/Hello.txt
//
//===----------------------------------------------------------------------===//

#ifndef LVA_H
#define LVA_H

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/IR/ValueMap.h"
#include <map>

using namespace llvm;

namespace legup {
  class LiveVariableAnalysis : public FunctionPass {
        public:
          static char ID; // Pass identification, replacement for typeid
          LiveVariableAnalysis() : FunctionPass(ID) {}
          virtual bool doInitialization(Module &M);
          virtual bool runOnFunction(Function &F);
          virtual bool doFinalization(Module &M);

            struct BasicBlockLivenessInfo {
                BitVector *use;
                BitVector *def;
                BitVector *in;
                BitVector *out;
                BasicBlock *block;

                /**
                 * Constructor that builds block information out of LLVM basic block
                 * and definition count.
                 * 
                 * @param block LLVM basic block.
                 * @param defcount Definition count.
                 * 
                 */
                BasicBlockLivenessInfo(BasicBlock *block, unsigned defcount) {
                    this->block = block;
                    use = new BitVector(defcount);
                    def = new BitVector(defcount);
                    in = new BitVector(defcount);
                    out = new BitVector(defcount);
                }

                ~BasicBlockLivenessInfo() {
                    delete use;
                    delete def;
                    delete in;
                    delete out;
                }
            };

            typedef ValueMap<Value *, unsigned> LatticeEncoding;
            LatticeEncoding instToLatticeBit;

            unsigned getLatticeBit(Value *i) {
                assert(instToLatticeBit.count(i));
                return instToLatticeBit[i];
            }
            typedef ValueMap<BasicBlock *, BasicBlockLivenessInfo *> BlockInfoMapping;
            BlockInfoMapping blockToInfo;

            typedef std::map<std::pair<BasicBlock *, BasicBlock *>, BitVector *> FlowMask;

    private:
            void initInstrToLatticeBit(Function &F);
            void initBlocksInfo(unsigned defcount, Function &F);
            void initBlockDef(BasicBlockLivenessInfo *blockInfo);
            void initEntryArgDef(BasicBlockLivenessInfo *blockInfo, unsigned argCount);
            void markUses(Value::user_iterator user_begin, Value::user_iterator user_end,
              Instruction *defInst, unsigned defBit);
            void initBlocksUse(Function &F);
            void initMask(FlowMask &mask, Function &F);
            void printBitVector(BitVector *bv);
            void printBitVectorDetailed(BitVector *bv);
            BitVector getMaskPhiValues(BasicBlock *phiBlock);

      Module *Mod;
  };
} // End of LegUp namespace

//using namespace legup;

#endif

