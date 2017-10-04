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

#include "LVA.h"
#include "utils.h"

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Analysis/ConstantFolding.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/ADT/SmallVector.h"
#include <cstdio>

using namespace llvm;

namespace legup {

/**
 * Assign lattice bits to definitions.
 * 
 * @param instToLatticeBit Mapping from definitions to lattice bits.
 * @param F LLVM function object.
 * 
 */
void LiveVariableAnalysis::initInstrToLatticeBit(Function &F) {
    unsigned instIdx = 0;

    // Arguments count as definitions as well - first F.arg_size() bits.
    for (Function::arg_iterator argIt = F.arg_begin(); argIt != F.arg_end(); argIt++) {
        instToLatticeBit.insert(std::make_pair(&*argIt, instIdx));
        instIdx++;
    }
    // Don't bother filtering out non-definition instructions. Their
    // corresponding use / in / out bits will be always zero and the
    // resulting in / out sets will look right.
    for (inst_iterator instIt = inst_begin(F); instIt != inst_end(F); instIt++) {
        instToLatticeBit.insert(std::make_pair(&*instIt, instIdx));
        instIdx++;
    }
}


/**
 * Assign information to basic blocks.
 * 
 * @param blockToInfo Mapping from LLVM basic blocks to liveness information.
 * @param F LLVM function object.
 * 
 */
void LiveVariableAnalysis::initBlocksInfo(unsigned defcount, Function &F) {
    for (Function::iterator blockIt = F.begin(); blockIt != F.end(); blockIt++) {
        BasicBlockLivenessInfo *info = new BasicBlockLivenessInfo(&*blockIt, defcount);
        blockToInfo.insert(std::make_pair(blockIt, info));
    }
}

/**
 * Set defs for a given block information object.
 * 
 * @param blockInfo Block information object to be filled with def bits.
 * @param instToLatticeBit Lattice bit encoding for the current function.
 * 
 */
void LiveVariableAnalysis::initBlockDef(BasicBlockLivenessInfo *blockInfo) {
    BasicBlock *block = blockInfo->block;
    for (BasicBlock::iterator instIt = block->begin(); instIt != block->end(); instIt++) {
        blockInfo->def->set(getLatticeBit(instIt));
    }
}

/**
 * Set additional defs for the entry block information object.
 * 
 * @param blockInfo Entry block information object to be filled with
 *                  argument def bits.
 * @param argCount Current function argument count.
 * 
 */
void LiveVariableAnalysis::initEntryArgDef(BasicBlockLivenessInfo *blockInfo, unsigned argCount) {
    for (unsigned argBit = 0; argBit < argCount; argBit++)
        blockInfo->def->set(argBit);
}

/**
 * Set corresponding use bit for all blocks using a given definition.
 * 
 * @param use_begin First use of the given definition.
 * @param use_end One-past-last use of the given definition.
 * @param blockToInfo Mapping from LLVM basic blocks to liveness information.
 * @param defInst Definition for which use bits are set
 *                (NULL if argument, not instruction).
 * @param defBit Corresponding bit for the definition in the lattice.
 * 
 */
void LiveVariableAnalysis::markUses(Value::user_iterator user_begin, Value::user_iterator user_end,
              Instruction *defInst, unsigned defBit) {
    for (Value::user_iterator useIt = user_begin; useIt != user_end; useIt++) {
        Instruction *useInst = dyn_cast<Instruction>(*useIt);
        if (!useInst) continue;
        BasicBlock *useBlock = useInst->getParent();

        // Avoid setting use bit for uses in the same block as the
        // definition. This is equivalent with the way use sets are
        // defined for the liveness analysis when working with SSA
        // form (thus having a textual-unique, dominant definition
        // for every use of a value).

        if (defInst != NULL) {
            // STEFAN: The assumption that in SSA you never have a use followed
            // by a def neglects the case where an instruction enters a block
            // through a phi but is then also defined in that block (see
            // adpcm).
            if (defInst->getParent() == useBlock && !isa<PHINode>(useInst)) {
                continue;
            }
        }

        BasicBlockLivenessInfo *useBlockInfo = blockToInfo[useBlock];
        useBlockInfo->use->set(defBit);
    }
}

/**
 * Set use sets for all the blocks.
 * 
 * @param blockToInfo Mapping from LLVM basic blocks to liveness information.
 * @param instToLatticeBit Mapping from definitions to lattice bits.
 * @param F LLVM function object.
 */
void LiveVariableAnalysis::initBlocksUse(Function &F) {
    for (inst_iterator defIt = inst_begin(F); defIt != inst_end(F); defIt++) {
        Instruction *defInst = &*defIt;
        markUses(defIt->user_begin(), defIt->user_end(), defInst, getLatticeBit(defInst));
    }
    for (Function::arg_iterator defIt = F.arg_begin(); defIt != F.arg_end(); defIt++) {
        markUses(defIt->user_begin(), defIt->user_end(), NULL, getLatticeBit(&*defIt));
    }
}

// create a mask that is 1 for every instruction except instructions that are
// only used in a phi node
// ie.  %i = phi i32 [ %b, %bb1 ], [ %c, %bb2 ]
//      %d = add i32 %c, 0
// Here we would set the mask to 1 for %i, %c (because of the use by %d), and
// %d and set the mask to 0 for %b
BitVector LiveVariableAnalysis::getMaskPhiValues(BasicBlock *phiBlock) {
    BitVector mask = BitVector(instToLatticeBit.size(), true);
    for (BasicBlock::iterator instIt = phiBlock->begin(); instIt != phiBlock->end(); instIt++) {
        if (!isa<PHINode>(&*instIt))
            continue;
        PHINode *phiNode = dyn_cast<PHINode>(&*instIt);
        for (unsigned crtIncomingIdx = 0; crtIncomingIdx < phiNode->getNumIncomingValues(); crtIncomingIdx++) {
            Value *crtValue = phiNode->getIncomingValue(crtIncomingIdx);
            Instruction *crtInst = dyn_cast<Instruction>(crtValue);
            if (!crtInst) continue;
            BasicBlock *defBlock = crtInst->getParent();

            // check if the incoming value is only used in a phi instr
            bool usedInNonPhi = false;
            for (Value::user_iterator useIt = crtInst->user_begin(); useIt !=
                    crtInst->user_end(); useIt++) {
                Instruction *useInst = dyn_cast<Instruction>(*useIt);
                if (!useInst) continue;
                BasicBlock *useBlock = useInst->getParent();
                // use/def in the same basic block hides upwards use
                if (useBlock == defBlock) continue;
                // ignore phi uses
                if (isa<PHINode>(useInst)) continue;
                // should be in the same basic block as phi
                if (useBlock != phiBlock) continue;

                usedInNonPhi = true;
            }

            if (!usedInNonPhi) {
                mask.reset( getLatticeBit(crtValue) );
            }
        }
    }
    return mask;
}


/**
 * Set flow mask for the current function.
 * 
 * Liveness analysis is tricky using the SSA form because of the phi nodes.
 * A phi node appears to use more than one value, but this is actually
 * flow-sensitive. Basically, a phi node looks - data-flow wise - as a
 * different instruction (using a single value) from each incoming block.
 * Therefore, when computing out set for each incoming block, the in sets
 * for the successors containing phi nodes will be adjusted accordingly by
 * using this mask. 
 * 
 * @param mask The flow mask between blocks containing phi-nodes and the
 *             incoming blocks.
 * @param blockToInfo Mapping from LLVM basic blocks to liveness information.
 * @param instToLatticeBit Mapping from definitions to lattice bits.
 * @param F LLVM function object. 
 */
void LiveVariableAnalysis::initMask(FlowMask &mask, Function &F) {

    for (Function::iterator blockIt = F.begin(); blockIt != F.end(); blockIt++) {
        BasicBlock *phiBlock = blockIt;

        BitVector initialMask = getMaskPhiValues(phiBlock);

        // now that we have masked out all the phi dependent uses, create a mask
        // for each crtBlock -> phiBlock control flow
        // ie.  %i = phi i32 [ %b, %bb1 ], [ %c, %bb2 ]
        // we should change the mask for %b to 1 for %bb1 -> phiBlock
        // and leave the mask for %b at 0 for %bb2 -> phiBlock
        for (BasicBlock::iterator instIt = phiBlock->begin(); instIt != phiBlock->end(); instIt++) {
            if (!isa<PHINode>(&*instIt))
                continue;
            PHINode *phiNode = dyn_cast<PHINode>(&*instIt);
            for (unsigned crtIncomingIdx = 0; crtIncomingIdx < phiNode->getNumIncomingValues(); crtIncomingIdx++) {
                BasicBlock *crtBlock = phiNode->getIncomingBlock(crtIncomingIdx);
                Value *crtValue = phiNode->getIncomingValue(crtIncomingIdx);
                if (!isa<Instruction>(crtValue)) continue;
                std::pair<BasicBlock *, BasicBlock *> maskKey = std::make_pair(crtBlock, phiBlock);
                std::map<std::pair<BasicBlock *, BasicBlock *>, BitVector *>::iterator maskIt =
                    mask.find(maskKey);
                if (maskIt == mask.end()) {
                    mask[maskKey] = new BitVector(initialMask);
                }
                mask[maskKey]->set(getLatticeBit(crtValue));
            }
        }
    }
}

void LiveVariableAnalysis::printBitVector(BitVector *bv) {
    for (unsigned bit = 0; bit < bv->size(); bit++)
        dbgs() << (bv->test(bit) ? '1' : '0');
}

void LiveVariableAnalysis::printBitVectorDetailed(BitVector *bv) {
    for (unsigned bit = 0; bit < bv->size(); bit++) {
        Value *instr = 0;
        for (LatticeEncoding::iterator it = instToLatticeBit.begin(); it !=
                instToLatticeBit.end(); it++) {
            if (it->second == bit) {
                instr = it->first;
            }
        }
        assert(instr);
        dbgs() << (bv->test(bit) ? '1' : '0') << " - " << *instr << "\n";
    }
}

static RegisterPass<LiveVariableAnalysis> X("legup-LiveVariableAnalysis", "LVA");
char LiveVariableAnalysis::ID = 0;

bool LiveVariableAnalysis::doInitialization(Module &M) {
    Mod = &M;
    return false;
}

bool LiveVariableAnalysis::runOnFunction(Function &F) {  // Executes for each LLVM function    

    // initialize encoding
    instToLatticeBit.clear();
    initInstrToLatticeBit(F);
    unsigned defcount = instToLatticeBit.size();

    // initialize block to info mapping
    for (BlockInfoMapping::iterator it = blockToInfo.begin(); it != blockToInfo.end(); it++) {
        delete it->second;
    }
    blockToInfo.clear();
    initBlocksInfo(defcount, F);

    // initialize def and use sets
    initEntryArgDef(blockToInfo[F.begin()], F.arg_size());
    for (BlockInfoMapping::iterator it = blockToInfo.begin(); it != blockToInfo.end(); it++) {
        initBlockDef(it->second);
    }
    initBlocksUse(F);

    // initialize flow mask
    FlowMask mask;
    initMask(mask, F);

    // compute fixed-point liveness information - no sorting of
    // blocks in quasi-topological order, works anyway
    bool inChanged = true;
    while (inChanged) {
        inChanged = false;
        // out[B] = U(in[S] & mask[B][S]) where B < S
        for (Function::iterator B = F.begin(); B != F.end(); B++) {
            (blockToInfo[B]->out)->reset();
            for (succ_iterator succIt = succ_begin(B); succIt != succ_end(B); succIt++) {
                BasicBlock *S = *succIt;
                std::pair<BasicBlock *, BasicBlock *> key =
                    std::make_pair(B, S);
                if (mask.find(key) != mask.end()) {
					// TODO LLVM 3.4 update
                    //*(blockToInfo[B]->out) |= ((*(blockToInfo[S]->in)) & (*(mask[key])));
					BitVector a = (*(blockToInfo[S]->in));
					a &= (*(mask[key]));
					*(blockToInfo[B]->out) |= a;
                } else {
                    *(blockToInfo[B]->out) |= *(blockToInfo[S]->in);
                }
            }
            // in[B] = use[B] U (out[B] - def[B])
            BitVector oldIn = *(blockToInfo[B]->in);

			// TODO LLVM 3.4 update
            //*(blockToInfo[B]->in) = (*(blockToInfo[B]->use) | (*(blockToInfo[B]->out) & ~(*(blockToInfo[B]->def))));

			BitVector temp = (*(blockToInfo[B]->def));
			temp.flip();
			temp &= *(blockToInfo[B]->out);
			temp |= *(blockToInfo[B]->use);
			*(blockToInfo[B]->in) = temp;

            if (*(blockToInfo[B]->in) != oldIn) {
                inChanged = true;
            }
        }
    }

/* // dump information to stderr
//    dbgs() << "Function name: " << F.getName() << "\n";
//    dbgs() << "-------------------\n";
//    dbgs() << "#blocks = " << blockToInfo.size() << '\n';
//    dbgs() << "#insts = " << defcount << "\n\n";
    for (Function::iterator blockIt = F.begin(); blockIt != F.end(); blockIt++) {
        dbgs() << "[ " << blockIt->getName() << " ]\n";
        for (BasicBlock::iterator instIt = blockIt->begin(); instIt != blockIt->end(); instIt++) {
            dbgs() << *instIt << "\n";
        }
        BasicBlockLivenessInfo *info = blockToInfo[blockIt];
        BitVector *def = info->def;
        BitVector *use = info->use;
        BitVector *in = info->in;
        BitVector *out = info->out;
        dbgs() << "Def:\t";
        printBitVector(def);
        dbgs() << '\n';
        dbgs() << "Use:\t";
        printBitVector(use);
        dbgs() << '\n';
        dbgs() << "In:\t";
        printBitVector(in);
        dbgs() << '\n';
        dbgs() << "Out:\t";
        printBitVector(out);
        dbgs() << '\n';
    }
    dbgs() << "\nMask size: " << mask.size() << '\n';
    for (std::map<std::pair<BasicBlock *, BasicBlock *>, BitVector *>::iterator maskIt = mask.begin(); maskIt != mask.end(); maskIt++) {
        printBBLabel(dbgs(), (maskIt->first).first);
        dbgs() << " -> ";
        printBBLabel(dbgs(), (maskIt->first).second);
        dbgs() << ":\t";
        //dbgs() << (maskIt->first).first->getName() << " -> " << (maskIt->first).second->getName() << ":\t";
        printBitVector(maskIt->second);
        dbgs() << '\n';
    }
    dbgs() << "===================\n";
*/
    for (FlowMask::iterator i = mask.begin(), e = mask.end(); i != e; ++i) {
        delete i->second;
    }

    return true;
}

bool LiveVariableAnalysis::doFinalization(Module &M) {
    for (BlockInfoMapping::iterator it = blockToInfo.begin(); it != blockToInfo.end(); it++) {
        delete it->second;
    }
    return false;
}
}

