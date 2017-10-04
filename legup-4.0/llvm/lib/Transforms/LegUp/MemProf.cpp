//===- MemProf.cpp - LegUp uses this pass to add memory profiling output --===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// This pass instruments the IR, adding printf calls for each:
//  * load
//  * store
//  * call
//  * return
//  * alloca
//
// This information can be used to determine memory access patterns.
//
//===----------------------------------------------------------------------===//

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Use.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/ADT/StringExtras.h"
#include "utils.h"
#include <vector>
#include <queue>

// TODO: if already a call to pthread_self => then use that value ???
// TODO: when adding to list openmpFunction or pthreadFunction -> do not add
// functions
// such as omp_get_thread_num, omp_get_num_threads, pthread_self etc.

//#define MY_DEBUG(args) errs() << args;
#define MY_DEBUG(args)                                                         \
    if (0)                                                                     \
        errs() << args;

using namespace llvm;

static cl::opt<bool>
    noLoadStore("no-ldst", cl::init(false),
                cl::desc("Disable printing of load and store operations"));

static cl::opt<bool>
    noCallReturn("no-callret", cl::init(false),
                 cl::desc("Disable printing of call and return instructions"));

static cl::opt<bool>
    noAlloca("no-alloca", cl::init(false),
             cl::desc("Disable printing of alloca instructions"));

namespace legup {
struct MemProf : public FunctionPass {
    static char ID; // Pass identification, replacement for typeid
    MemProf() : FunctionPass(ID){};

    // adds parallel functions to parallelFunction vector
    // function is parallel if:
    // pthread -> called by pthread_create
    // openmp -> called by GOMP_parallel_start
    // or called by a parallel function
    bool isParallelFunctionCall(CallInst *CI) {
        if (CI->getCalledFunction() == NULL) {
            errs() << "Could not find function being called\n";
            return false;
        }

        int sizeBefore = pthreadFunction.size() + openmpFunction.size();
        std::string calledFuncName = CI->getCalledFunction()->getName();
        Function *pF = NULL;
        if (calledFuncName == "pthread_create") { // pthread check argument #3
            // add name of the 3rd argument as parallel function
            if ((pF = dyn_cast<Function>(CI->getArgOperand(2))) &&
                !functionInParallelList(pF)) {
                errs() << "pthread function: " << pF->getName() << "\n";
                pthreadFunction.push_back(pF);
            }
        } else if (calledFuncName == "GOMP_parallel_start") { // omp
            // add name of the 1st argument as parallel function
            if ((pF = dyn_cast<Function>(CI->getArgOperand(0))) &&
                !functionInParallelList(pF)) {
                errs() << "omp function: " << pF->getName() << "\n";
                openmpFunction.push_back(pF);
            }
        } else if (isPthreadFunction(CI->getParent()->getParent()) &&
                   !isAPrintCall(CI) &&
                   !functionInParallelList(CI->getCalledFunction())) {
            errs() << "pthread function: " << CI->getCalledFunction()->getName()
                   << "\n";
            pthreadFunction.push_back(CI->getCalledFunction());
        } else if (isOpenMPFunction(CI->getParent()->getParent()) &&
                   !isAPrintCall(CI) &&
                   !functionInParallelList(CI->getCalledFunction())) {
            errs() << "omp function: " << CI->getCalledFunction()->getName()
                   << "\n";
            openmpFunction.push_back(CI->getCalledFunction());
        }
        int sizeAfter = pthreadFunction.size() + openmpFunction.size();
        if (sizeBefore < sizeAfter) {
            return true;
        } else {
            return false;
        }
    }

    // Add parallel functions to appropriate vectors:
    // pthreadFunction/openmpFunction
    // Also remove calls to hybrid-only functions
    void findParallelFunctions(Module &M) {
        bool foundParallelFunc = true;
        while (foundParallelFunc) {
            foundParallelFunc = false;
            for (auto F = M.begin(), FE = M.end(); F != FE; ++F) {
                for (auto BB = F->begin(), BE = F->end(); BB != BE; ++BB) {
                    for (auto I = BB->begin(), IE = BB->end(); I != IE;) {
                        Instruction *inst = I++;
                        if (CallInst *CI = dyn_cast<CallInst>(inst)) {
                            if (removeHybridOnlyFunctionCalls(CI))
                                continue;
                            foundParallelFunc |= isParallelFunctionCall(CI);
                        }
                    }
                }
            }
        }
    }

    CallInst *insertPthreadSelfCall(Function &F) {
        // insert the call to the entry block after phi instruction
        Instruction *insertBefore = F.begin()->getFirstNonPHI();

        IRBuilder<> builder(insertBefore);
        CallInst *call = builder.CreateCall(PthreadSelfFunc);

        return call;
    }

    CallInst *insertOMPThreadNumCall(Function &F) {
        // insert the call to the entry block after phi instruction
        Instruction *insertBefore = F.begin()->getFirstNonPHI();

        IRBuilder<> builder(insertBefore);
        CallInst *call = builder.CreateCall(OMPThreadNumFunc);

        return call;
    }

    virtual bool doInitialization(Module &M) {
        Mod = &M;

        if (noLoadStore)
            errs() << "Disabled printing of load/store operations\n";
        if (noCallReturn)
            errs() << "Disabled printing of call/return instructions\n";
        if (noAlloca)
            errs() << "Disabled printing of alloca instructions\n";

        // get constants needed to create a printf call
        Type *charPointerType =
            PointerType::get(IntegerType::get(Mod->getContext(), 8), 0);
        FunctionType *printfTy =
            FunctionType::get(IntegerType::get(Mod->getContext(), 32),
                              std::vector<Type *>(1, charPointerType), true);

        // insert function prototype if needed
        PrintFunc = M.getOrInsertFunction("printf", printfTy);

        pthreadFunction.clear();
        openmpFunction.clear();

        findParallelFunctions(M);

        errs() << "Number of parallel functions found: "
               << pthreadFunction.size() + openmpFunction.size() << "\n";

        // insert function prototype if needed for pthread_self function
        // FunctionType *pthread_selfTy =
        //    FunctionType::get(Type::getVoidTy(M.getContext()),
        //    Type::getVoidTy(M.getContext()));
        if (pthreadFunction.size() > 0) {
            FunctionType *pthread_selfTy = FunctionType::get(
                IntegerType::get(Mod->getContext(), 32), false);
            PthreadSelfFunc =
                Mod->getOrInsertFunction("pthread_self", pthread_selfTy);
        }

        // insert function prototype if needed for omp_get_thread_num function
        if (openmpFunction.size() > 0) {
            FunctionType *openmp_get_thread_numTy = FunctionType::get(
                IntegerType::get(Mod->getContext(), 32), false);
            OMPThreadNumFunc = Mod->getOrInsertFunction(
                "omp_get_thread_num", openmp_get_thread_numTy);
        }

        return false;
    }

    // return true if function already in openmpFunction or pthreadFunction
    // lists
    bool functionInParallelList(Function *F) {
        return isPthreadFunction(F) || isOpenMPFunction(F);
    }

    // returns true if parallel function
    bool isParallelFunction(Function *F) {
        return isPthreadFunction(F) || isOpenMPFunction(F);
    }

    bool isPthreadFunction(Function *F) {
        if (std::find(pthreadFunction.begin(), pthreadFunction.end(), F) !=
            pthreadFunction.end()) {
            return true;
        }
        return false;
    }

    bool isOpenMPFunction(Function *F) {
        if (std::find(openmpFunction.begin(), openmpFunction.end(), F) !=
            openmpFunction.end()) {
            return true;
        }
        return false;
    }

    bool isaHybridOnlyFunction(const std::string funcName) {
        if (funcName == "legup_start_counter" ||
            funcName == "legup_stop_counter")
            return true;
        return false;
    }

    // Remove calls to functions that are only used in the hybrid flow while we
    // are not using the hybrid flow
    // i.e. legup_start_counter, legup_stop_counter
    bool removeHybridOnlyFunctionCalls(CallInst *CI) {
        if (isaHybridOnlyFunction(CI->getCalledFunction()->getName().str()) &&
            !LEGUP_CONFIG->isHybridFlow()) {

            MY_DEBUG("Deleted call to hybrid-only function: "
                     << CI->getCalledFunction()->getName() << "\n");
            deleteInstruction(CI);
            return true;
        }
        return false;
    }

    // get the number of GEP dimensions, and add an arg for each dimension
    int getGEPDims(GEPOperator *g, std::vector<Value *> &Args) {
        int num_dims = 0;
        unsigned int index_offset = 0;
        PointerType *ty = dyn_cast<PointerType>(g->getPointerOperandType());

        // if it is an array or vector, the first index, 0, can be skipped
        // see online LLVM doc: The Often Misunderstood GEP Instruction
        // http://llvm.org/docs/GetElementPtr.html#why-is-the-extra-0-index-required
        if (dyn_cast<ArrayType>(ty->getElementType()) ||
            dyn_cast<VectorType>(ty->getElementType())) {
            // MY_DEBUG(" array or vector type ");
            index_offset = 1;
        }

        // skip index 0
        for (unsigned int i = 1 + index_offset; i <= g->getNumIndices(); i++) {
            Value *operand = g->getOperand(i);
            if (ConstantInt *ndx = dyn_cast<ConstantInt>(operand)) {
                Args.push_back(ConstantInt::get(
                    Type::getInt32Ty(getGlobalContext()), ndx->getSExtValue()));
                MY_DEBUG("[" << ndx->getSExtValue() << "]");
            } else {
                Args.push_back(operand);
                MY_DEBUG("[" << operand->getName() << "]");
            }
            num_dims++;
        }
        MY_DEBUG("\n");
        return num_dims;
    }

    // determine if phi nodes have identical GEPs feeding each input
    // this doesn't happend often, but does in some benchmarks
    bool phi_inputs_are_identical_geps(PHINode *phi) {
        GEPOperator *g0 = dyn_cast<GEPOperator>(phi->getIncomingValue(0));
        if (!g0)
            return false;
        for (unsigned int i = 1; i < phi->getNumIncomingValues(); i++) {
            GEPOperator *gi = dyn_cast<GEPOperator>(phi->getIncomingValue(i));
            if (!gi)
                return false;
            // make sure number of indices are the same
            if (g0->getNumIndices() != gi->getNumIndices())
                return false;
            for (unsigned int j = 0; j <= g0->getNumIndices(); j++) {
                if (g0->getOperand(j) != gi->getOperand(j))
                    return false;
            } // for j
        }     // for i
        return true;
    }

    // get a reference to a global string, creating it if necessary
    Value *getOrAddString(std::string s, IRBuilder<> builder) {
        // if string doesn't exist, add it
        if (strings_map.find(s) == strings_map.end())
            strings_map[s] = builder.CreateGlobalStringPtr(s.c_str());

        return strings_map[s];
    }

    // create a printf call for a load or store instruction that shows:
    //   * whether it is a load or store
    //   * the address accessed
    //   * the data read/written
    //   * the name of the variable, with any relavent array indices
    void printLoadStore(BasicBlock::iterator pos, Instruction *instr) {
        MY_DEBUG("printLoadStore\n");
        // want to add call _after_ the instruction
        bool isParallel = isParallelFunction(instr->getParent()->getParent());
        IRBuilder<> builder((Instruction *)(++pos));
        std::vector<Value *> Args;
        std::string operation;
        Value *operand, *data;
        int num_dims = 0;

        if (isa<LoadInst>(instr)) {
            operand = instr->getOperand(0);
            data = instr;
            operation = "ld";
        } else if (isa<StoreInst>(instr)) {
            operand = instr->getOperand(1);
            data = instr->getOperand(0);
            operation = "st";
        }

        // save space for format string
        Args.push_back(NULL);

        // depending on the type of the operand that specifies the address,
        // we may need to do different things
        // This can be:
        //  * a GEP operator
        //  * a PHI node
        //  * a pointer
        //  * an instruction
        //  * something else??
        if (GEPOperator *g = dyn_cast<GEPOperator>(operand)) {
            MY_DEBUG("GEP: " << g->getOperand(0)->getName());
            // address
            Args.push_back((Value *)g);
            // data
            Args.push_back(data);
            // name
            // if a second level of GEP
            // TODO: may be more than two levels of GEP?
            if (GEPOperator *g2 = dyn_cast<GEPOperator>(g->getOperand(0))) {
                Args.push_back(getOrAddString(
                    g2->getOperand(0)->getName().str(), builder));
            } else {
                Args.push_back(
                    getOrAddString(g->getOperand(0)->getName().str(), builder));
            }

            // TODO: array indices might be wrong if multiple levels of GEP?
            num_dims = getGEPDims(g, Args);

        } // GEP
        else if (PHINode *phi = dyn_cast<PHINode>(operand)) {
            // see if we can resolve the phi node, this doesn't happen often
            if (phi_inputs_are_identical_geps(phi)) {
                GEPOperator *g =
                    dyn_cast<GEPOperator>(phi->getIncomingValue(0));
                MY_DEBUG("PHI: " << g->getOperand(0)->getName());
                // address
                Args.push_back((Value *)phi);
                // data
                Args.push_back(data);
                // name
                Args.push_back(
                    getOrAddString(g->getOperand(0)->getName().str(), builder));
                // dims
                num_dims = getGEPDims(g, Args);
            } else {
                MY_DEBUG("PHI: cannot resolve: " << phi->getName() << "\n");
                // address
                Args.push_back((Value *)phi);
                // data
                Args.push_back(data);
                // name
                Args.push_back(
                    getOrAddString(operand->getName().str(), builder));
            }
        } // PHI
        else if (dyn_cast<PointerType>(operand->getType())) {
            MY_DEBUG("PTR: " << operand->getName() << "[0]\n");
            // address
            Args.push_back(operand);
            // data
            Args.push_back(data);
            // name
            Args.push_back(getOrAddString(operand->getName().str(), builder));
            // index 0
            Args.push_back(
                ConstantInt::get(Type::getInt32Ty(getGlobalContext()), 0));

            // if it is just a pointer, index is [0]
            num_dims = 1;
        } // PTR
        else if (Instruction *ins = dyn_cast<Instruction>(operand)) {
            // address
            Args.push_back(ins);
            // data
            Args.push_back(data);
            // name
            Args.push_back(getOrAddString(ins->getName().str(), builder));
            MY_DEBUG("INST: " << ins->getName() << "\n");
        }      // INST
        else { // Unknown
            MY_DEBUG("UNKNOWN: " << operand->getName() << "\n");
            // address
            Args.push_back(
                ConstantInt::get(Type::getInt32Ty(getGlobalContext()), 0));
            // data
            Args.push_back(data);
            // name
            Args.push_back(getOrAddString(operand->getName().str(), builder));
        } // UNKNOWN

        if (isParallel) {
            if (isPthreadFunction(instr->getParent()->getParent())) {
                Args.push_back(dyn_cast<Value>(PthreadSelfCall));
            } else if (isOpenMPFunction(instr->getParent()->getParent())) {
                Args.push_back(dyn_cast<Value>(OMPThreadNumCall));
            }
        }

        // create store string with appropriate # of dimensions
        std::string s = "#" + operation;
        s += " 0x%x %d (%s";
        for (int i = 0; i < num_dims; i++)
            s += "[%d]";
        s += ")";
        if (isParallel) {
            s += " {tid: %u}\n";
        } else {
            s += "\n";
        }

        // format string
        Args[0] = getOrAddString(s, builder);

        // Create the printf call
        builder.CreateCall(PrintFunc, Args);
    }

    // Create a printf() call for each alloca instruction that shows:
    //   * the name of the variable
    //   * the base address
    //   * the amount of memory allocated in bytes
    // TODO: print size split by dimensions, eg. '[2][64]' rather than '128'
    void printAlloca(BasicBlock::iterator pos, Instruction *i) {
        MY_DEBUG("printAlloca\n");
        bool isParallel = isParallelFunction(i->getParent()->getParent());
        unsigned int size = 0;
        // want to print call _after_ the instruction
        IRBuilder<> builder((Instruction *)(++pos));
        std::vector<Value *> Args;
        if (AllocaInst *a = dyn_cast<AllocaInst>(i)) {
            // format string
            if (isParallel) {
                Args.push_back(
                    getOrAddString("#alloc %s 0x%x %d {tid: %u}\n", builder));
            } else {
                Args.push_back(getOrAddString("#alloc %s 0x%x %d\n", builder));
            }

            // variable name
            Args.push_back(getOrAddString(a->getName().str(), builder));

            // address
            Args.push_back(i);

            Type *t = a->getAllocatedType();
            MY_DEBUG("alloca " << a->getName() << " ");

            // TODO: should probably deal with types other than array
            // if it is an array, get the size in bytes
            if (t->isArrayTy()) {
                // get num elements in first dimension
                size = t->getArrayNumElements();
                while (t->getArrayElementType()->isArrayTy()) {
                    // go to next dim of array
                    t = t->getArrayElementType();
                    // multiply size by the next dim of array
                    size *= t->getArrayNumElements();
                }
                // get element size
                size *= t->getArrayElementType()->getScalarSizeInBits();
                // want bytes, not bits
                size /= 8;
                // print size
                MY_DEBUG("size: " << size << "B");
            }

            // size
            Args.push_back(
                ConstantInt::get(Type::getInt32Ty(getGlobalContext()), size));

            // thread ID
            if (isParallel) {
                if (isPthreadFunction(i->getParent()->getParent())) {
                    Args.push_back(dyn_cast<Value>(PthreadSelfCall));
                } else if (isOpenMPFunction(i->getParent()->getParent())) {
                    Args.push_back(dyn_cast<Value>(OMPThreadNumCall));
                }
            }

            // create the printf call
            builder.CreateCall(PrintFunc, Args);

            MY_DEBUG("\n");
        }
    }

    // determine if this CallInst is a print call
    static bool isAPrintCall(CallInst *ci) {
        Function *calledFunc = ci->getCalledFunction();
        // ignore indirect function invocations
        if (!calledFunc) {
            return false;
        }
        std::string funcName = calledFunc->getName();
        return (funcName == "printf" || funcName == "putchar" ||
                funcName == "puts" || funcName == "mprintf");
    }

    // craete a printf call for each call or return instruction
    void printCallReturn(Instruction *i) {

        IRBuilder<> builder(i);
        std::vector<Value *> Args;
        std::string inst_type, fn_name;

        if (CallInst *c = dyn_cast<CallInst>(i)) {
            // ignore print calls
            if (isAPrintCall(c))
                return;

            if (c->getCalledFunction() == NULL)
                return;

            fn_name = c->getCalledFunction()->getName();
            inst_type = "call";
        } else if (ReturnInst *r = dyn_cast<ReturnInst>(i)) {
            // Instruction->BasicBlock->Function->getName()
            fn_name = r->getParent()->getParent()->getName();
            inst_type = "ret ";
        }

        // format string
        Args.push_back(getOrAddString("#" + inst_type + " %s\n", builder));

        // function name
        Args.push_back(getOrAddString(fn_name, builder));

        // create printf call
        builder.CreateCall(PrintFunc, Args);

        MY_DEBUG(inst_type << " " << fn_name << "\n");
    }

    virtual bool runOnFunction(Function &F) {
        bool fn_modified = false;
        MY_DEBUG("Function:\t" << F.getName() << "\n");

        // add call to pthread_self() if parallel function
        if (isPthreadFunction(&F)) {
            PthreadSelfCall = insertPthreadSelfCall(F);
            OMPThreadNumCall = NULL;
        } else if (isOpenMPFunction(&F)) {
            OMPThreadNumCall = insertOMPThreadNumCall(F);
            PthreadSelfCall = NULL;
        } else {
            PthreadSelfCall = NULL;
            OMPThreadNumCall = NULL;
        }

        for (Function::iterator BB = F.begin(), EE = F.end(); BB != EE; ++BB) {
            for (BasicBlock::iterator I = BB->begin(), E = BB->end(); I != E;
                 ++I) {
                Instruction *instr = I;

                if ((isa<LoadInst>(instr) || isa<StoreInst>(instr)) &&
                    !noLoadStore) {
                    printLoadStore(I, instr);
                    fn_modified |= true;
                } else if (isa<AllocaInst>(instr) && !noAlloca) {
                    printAlloca(I, instr);
                    fn_modified |= true;
                } else if ((isa<CallInst>(instr) || isa<ReturnInst>(instr)) &&
                           !noCallReturn) {
                    printCallReturn(instr);
                    fn_modified |= true;
                }
            }
        }
        return fn_modified;
    }

  private:
    Module *Mod;
    Constant *PrintFunc;
    Constant *PthreadSelfFunc;
    CallInst *PthreadSelfCall;
    Constant *OMPThreadNumFunc;
    CallInst *OMPThreadNumCall;
    std::map<std::string, Value *> strings_map;
    std::vector<Function *> pthreadFunction;
    std::vector<Function *> openmpFunction;
};
}

using namespace legup;

char MemProf::ID = 0;
static RegisterPass<MemProf> X(
    "legup-mem-prof",
    "Instrument code with print statements for each store, load, call, return");
