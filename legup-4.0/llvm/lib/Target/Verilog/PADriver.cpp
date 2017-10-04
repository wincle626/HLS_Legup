#define DEBUG_TYPE "PADriver"

#include <sstream>
#include <unistd.h>
#include <ios>
#include <fstream>
#include <string>
#include <iostream>

#include "llvm/IR/Value.h"
#include "llvm/IR/Use.h"
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/CallSite.h"
#include "llvm/Support/Debug.h"
#include "llvm/ADT/Statistic.h"

#include "utils.h"
#include "PADriver.h"
#include "PointerAnalysis.h"

using namespace llvm;
using namespace legup;

STATISTIC(PABaseCt, "Counts number of base constraints");
STATISTIC(PAAddrCt, "Counts number of address constraints");
STATISTIC(PALoadCt,  "Counts number of load constraints");
STATISTIC(PAStoreCt, "Counts number of store constraints");
STATISTIC(PANumVert, "Counts number of vertices");
STATISTIC(PAMerges,  "Counts number of merged vertices");
STATISTIC(PARemoves, "Counts number of calls to remove cycle");
STATISTIC(PAMemUsage, "kB of memory");

// ============================= //

PADriver::PADriver() {
    pointerAnalysis = new PointerAnalysis();
    currInd = 0;
    nextMemoryBlock = 1;

    PAAddrCt = 0;
    PABaseCt = 0;
    PALoadCt = 0;
    PAStoreCt = 0;
    PANumVert = 0;
    PARemoves = 0;
    PAMerges = 0;
    PAMemUsage = 0;

    numInst = 0;
}

bool PADriver::runOnModule(Module &M) {

    // struct timeval startTime, endTime;
    // struct rusage ru;

    // Get Time before analysis
	//getrusage(RUSAGE_SELF, &ru);
	//startTime = ru.ru_utime;
	if (pointerAnalysis == 0) pointerAnalysis = new PointerAnalysis();

	// Collect information from global variables
	for (Module::global_iterator git = M.global_begin(), gitE = M.global_end(); 
			git != gitE; ++git) {
		//errs() << "Global: " << *git << "\n";
		//errs() << "    -> " << git->getType()->isStructTy() << "\n";
		handleGlobalVariable(git);
	}

	// Collect information from functions
	for (Module::iterator F = M.begin(), E = M.end(); F != E; ++F) {
		if (!F->isDeclaration()) {
			addConstraints(*F);
			matchFormalWithActualParameters(*F);
			matchReturnValueWithReturnVariable(*F);
		}
    }

    // Run the analysis
    // assert(0);
    pointerAnalysis->solve();

    // map memory regions back to values (assuming no structs)
    for (std::map<Value *, std::vector<int>>::iterator i = memoryBlock.begin(),
                                                       e = memoryBlock.end();
         i != e; ++i) {
        Value *v = i->first;
        std::vector<int> list = i->second;
        assert(list.size() == 1); // i.e. no structs
        int2mem[list[0]] = v;
    }

    double vmUsage, residentSet;
    process_mem_usage(vmUsage, residentSet);
    PAMemUsage = vmUsage;

    // Get some statistics
	PAMerges = pointerAnalysis->getNumOfMertgedVertices();
	PARemoves = pointerAnalysis->getNumCallsRemove();
	PANumVert = pointerAnalysis->getNumVertices();

	// Get Time after analysis
    // getrusage(RUSAGE_SELF, &ru);
    // endTime = ru.ru_utime;

    // Print time spent with analysis
    // double tS = startTime.tv_sec + (double)startTime.tv_usec / 1000000;
    // double tE = endTime.tv_sec + (double)endTime.tv_usec / 1000000;
    // double deltaTime = tE - tS;
    // std::stringstream ss(std::stringstream::in | std::stringstream::out);
    // ss << std::fixed << std::setprecision(4) << deltaTime;
    // std::string deltaTimeStr;
    // ss >> deltaTimeStr;
    // errs() << deltaTimeStr << " Time to perform the pointer analysis\n";

    // errs() << "Num. Instructions: " << numInst << "\n";

    return false;
}

// ============================= //

// Get the int ID of a new memory space
int PADriver::getNewMem(std::string name) {

	// Get a new ID for the memory
	int n = ++currInd;
	nameMap[n] = name;
	//PAMemUsage += sizeof(std::pair<int,std::string>);

	//errs() << "MemVal (" << v << " - " << n << "): " << nameMap[n] << "\n";
	//errs() << "Name of mem: " << nameMap[n] << "\n";

	return n;
}

// ============================= //

void PADriver::print(raw_ostream &O, const Module *M) const {
    std::stringstream dotFileSS;
    pointerAnalysis->print(O);

    // print out value2Int mapping
    O << "Variable: Int -> Value* mapping\n";
    for (std::map<Value*, int>::const_iterator i = value2int.begin(), e =
            value2int.end(); i != e; ++i) {
        const Value *v = i->first;
        const int n = i->second;
        O << n << ": " << *v << "\n";
    }

    O << "Memory: Int -> Value* mapping\n";
    for (std::map<Value*, std::vector<int> >::const_iterator i = memoryBlock.begin(), e =
            memoryBlock.end(); i != e; ++i) {
        const Value *v = i->first;
        const std::vector<int> list = i->second;
        assert(list.size() == 1); // i.e. no structs
        O << list[0] << ": " << *v << "\n";
    }


	pointerAnalysis->printDot(dotFileSS, M->getModuleIdentifier(), nameMap);
	O << dotFileSS.str();
}

// ============================= //

std::string PADriver::intToStr(int v) {
	std::stringstream ss;
	ss << v;
	return ss.str();
}

// ============================= //

void PADriver::process_mem_usage(double& vm_usage, double& resident_set) {
	using std::ios_base;
	using std::ifstream;
	using std::string;

	vm_usage     = 0.0;
	resident_set = 0.0;

	// 'file' stat seems to give the most reliable results
	//
	ifstream stat_stream("/proc/self/stat",ios_base::in);

	// dummy vars for leading entries in stat that we don't care about
	//
	string pid, comm, state, ppid, pgrp, session, tty_nr;
	string tpgid, flags, minflt, cminflt, majflt, cmajflt;
	string utime, stime, cutime, cstime, priority, nice;
	string O, itrealvalue, starttime;

	// the two fields we want
	//
	unsigned long vsize;
	long rss;

	stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
		>> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
		>> utime >> stime >> cutime >> cstime >> priority >> nice
		>> O >> itrealvalue >> starttime >> vsize >> rss; // don't care about the rest

	stat_stream.close();

	long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
	vm_usage     = vsize / 1024.0;
	resident_set = rss * page_size_kb;
}

// ============================= //

void PADriver::findAllPointerOperands(User *U, std::set<Value *> &ptrs) {
    for (unsigned i = 0; i < U->getNumOperands(); i++) {
        Value *srcV = U->getOperand(i);
        User *src = dyn_cast<User>(srcV);
        if (!src)
            continue;
        const Type *srcTy = src->getType();
        if (srcTy->isPointerTy() ||
            // this is the case where we have an operand
            // that we know has a pointer assigned to it, like the trunc:
            //      %4 = add i64 %3, zext (i32 ptrtoint ([24 x i32]* @tqmf to
            //      i32) to i64)
            //      %5 = trunc i64 %4 to i32
            value2int.count(src)) {
            // errs() << "Found pointer: " << *src << "\n";
            ptrs.insert(src);
        }

        ConstantExpr *C = dyn_cast<ConstantExpr>(src);
        if (C) {
            // errs() << "Found constant: " << *C << "\n";
            findAllPointerOperands(src, ptrs);
        }
    }
}

void PADriver::addConstraints(Function &F) {
    for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB) {
        for (BasicBlock::iterator I = BB->begin(), E = BB->end(); I != E; ++I) {
            if (isa<PHINode>(I)) {
				PHINode *Phi = dyn_cast<PHINode>(I);
				const Type *Ty = Phi->getType();

				if (Ty->isPointerTy()) {
					unsigned n = Phi->getNumIncomingValues();
					std::vector<Value*> values;

					for (unsigned i = 0; i < n; i++) {
						Value *v = Phi->getIncomingValue(i);

						values.push_back(v);
					}

					phiValues[I] = values;
				}
			}
		}
	}

	for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB) {
		for (BasicBlock::iterator I = BB->begin(), E = BB->end(); I != E; ++I) {

			numInst++;
			//errs() << "Instruction: " << *I << "\n";

			if (isa<CallInst>(I)) {
				CallInst *CI = dyn_cast<CallInst>(I);

				if (CI) {
					Function *FF = CI->getCalledFunction();

					if (FF && (FF->getName() == "malloc" || FF->getName() == "realloc" ||
								FF->getName() == "calloc")) {
						std::vector<int> mems;

						if (!memoryBlock.count(I)) {
							mems.push_back(getNewMemoryBlock());
							memoryBlock[I] = mems;
						} else {
							mems = memoryBlock[I];
						}

						int a = Value2Int(I);
						pointerAnalysis->addAddr(a, mems[0]);
						PAAddrCt++;
					}
				}
			}

            // Handle special operations
            switch (I->getOpcode()) {
            case Instruction::Select: {

                Type *Ty = I->getType();
                if (Ty->isPointerTy()) {
                    // %.data.i.i = select i1 %24, i8* %21, i8* %1
                    int a = Value2Int(I);

                    int b = Value2Int(I->getOperand(1));
                    pointerAnalysis->addBase(a, b);

                    int c = Value2Int(I->getOperand(2));
                    pointerAnalysis->addBase(a, c);
                }
                break;
            }

            case Instruction::Alloca: {
                handleAlloca(I);

                        break;
					}
				case Instruction::GetElementPtr:
					{
                    handleGetElementPtr(I);
                    break;
                }

                case Instruction::Add:
                case Instruction::Trunc: {
                    std::set<Value *> ptrs;
                    findAllPointerOperands(I, ptrs);

                    // to handle:
                    //  %4 = add i64 %3, zext (i32 ptrtoint ([24 x i32]* @tqmf
                    //  to i32) to i64)
                    //  %5 = trunc i64 %4 to i32
                    //  %s.i.0 = inttoptr i32 %5 to i32*
                    for (std::set<Value *>::iterator i = ptrs.begin(),
                                                     e = ptrs.end();
                         i != e; ++i) {
                        Value *src = *i;
                        int a = Value2Int(I);
                        int b = Value2Int(src);
                        pointerAnalysis->addBase(a, b);
                        // errs() << "I: " << *I << "\n\tfound source pointer: "
                        // << *src << "\n";
                    }
                }

                case Instruction::IntToPtr:
                case Instruction::PtrToInt: {
                    // to handle:
                    //  %s.i.0 = inttoptr i32 %5 to i32*
                    //  %.c = ptrtoint i8* %19 to i32
                    Value *src = I->getOperand(0);
                    Value *dst = I;

                    int a = Value2Int(dst);
                    int b = Value2Int(src);
                    pointerAnalysis->addBase(a, b);
                }
                case Instruction::BitCast: {
                    // %s.0 = bitcast i8* %scevgep to i32*

                    Value *src = I->getOperand(0);
                    Value *dst = I;

                        const Type *srcTy = src->getType();
						const Type *dstTy = dst->getType();

						if (srcTy->isPointerTy()) {
							if (dstTy->isPointerTy()) {
								const PointerType *PoTy = cast<PointerType>(dstTy);
								const Type *Ty = PoTy->getElementType();

								if (Ty->isStructTy()) {
									if (memoryBlock.count(src)) {
										std::vector<int> mems = memoryBlock[src];
										int parent = mems[0];

										handleNestedStructs(Ty, parent);
										memoryBlock[I] = mems;
									}
								}
							}

							int a = Value2Int(I);
							int b = Value2Int(src);
							pointerAnalysis->addBase(a, b);
							PABaseCt++;
						}

						break;
					}
				case Instruction::Store:
					{
						// *ptr = v
						StoreInst *SI = dyn_cast<StoreInst>(I);
                        Value *v = SI->getValueOperand();
                        Value *ptr = SI->getPointerOperand();

                        // handle when ptr is a GEP constant:
                        //      store i32 43, i32* getelementptr inbounds ([32
                        //      x i32]* @key, i32 0, i32 0), align 4, !tbaa !1
                        handleGetElementPtrConst(ptr);

                        if (v->getType()->isPointerTy()) {
                            int a = Value2Int(ptr);
                            int b = Value2Int(v);

                            // Handle the case when the value operand is a
                            // GetElementPtr Constant Expression
                            // For example:
                            // I: store i8* getelementptr inbounds ([2048 x
                            //      i8]* @ld_Rdbfr, i32 1, i32 0), i8**
                            //      @ld_Rdptr, align 4, !tbaa !1
                            // CE: i8* getelementptr inbounds ([2048 x i8]*
                            //      @ld_Rdbfr, i32 1, i32 0) @ld_Rdbfr =
                            //      internal global [2048 x i8]
                            //      zeroinitializer, align 1
                            handleGetElementPtrConst(v);

                            // Andrew: The code below breaks, I get an error:
                            //      "Use still stuck around after Def"
                            // Because new GEP instructions are created
                            /*
                            ConstantExpr *CE = dyn_cast<ConstantExpr>(v);
                            if (CE &&
                                CE->getOpcode() == Instruction::GetElementPtr) {

                                // Here we create an instruction from
                                // the value operand
                                Vector<Value*,4> ValueOperands;
                                for (User::op_iterator opi = ce->op_begin(), E =
                                        ce->op_end(); opi != E; ++opi)
                                    ValueOperands.push_back(cast<Value>(opi));
                                ArrayRef<Value*> Ops(ValueOperands);

                                GetElementPtrInst* gepi = 0;
                                if (cast<GEPOperator>(ce)->isInBounds())
                                    gepi = GetElementPtrInst::CreateInBounds(
                                            Ops[0], Ops.slice(1), "GEPI");
                                else
                                    gepi= GetElementPtrInst::Create(Ops[0],
                                            Ops.slice(1), "GEPI");

                                //errs() << *gepi << "\n";
                                handleGetElementPtr(gepi);
                                b = Value2Int(gepi);

                            }
                            */

                            // errs() << "  >> Pointer: " << *ptr << "\n";
                            // errs() << "  >> Value:   " << *v << "\n";

                            pointerAnalysis->addStore(a, b);
                            PAStoreCt++;
                        }

                        break;
                }
                case Instruction::Load:
					{
						// I = *ptr
                    LoadInst *LI = dyn_cast<LoadInst>(I);
                    Value *ptr = LI->getPointerOperand();

                    int a = Value2Int(I);
                    int b = Value2Int(ptr);
                    pointerAnalysis->addLoad(a, b);
                    PALoadCt++;

                    // int a = Value2Int(I);
                    // int b;

                    // handle a case like:
                    //   %4 = load i32* getelementptr inbounds ([18 x i32]*
                    //                  @key_P, i32 0, i32 0)
                    handleGetElementPtrConst(ptr);

                    break;
                }
                case Instruction::AtomicRMW:
					{
						// I = *ptr
						AtomicRMWInst *LI = dyn_cast<AtomicRMWInst>(I);
						Value *ptr = LI->getPointerOperand();

						int a = Value2Int(I);
						int b = Value2Int(ptr);
						pointerAnalysis->addLoad(a, b);
						PALoadCt++;

						break;
					}
				case Instruction::AtomicCmpXchg:
					{
						// I = *ptr
						AtomicCmpXchgInst *LI = dyn_cast<AtomicCmpXchgInst>(I);
						Value *ptr = LI->getPointerOperand();

						int a = Value2Int(I);
						int b = Value2Int(ptr);
						pointerAnalysis->addLoad(a, b);
						PALoadCt++;

						break;
					}
				case Instruction::PHI:
					{
						PHINode *Phi = dyn_cast<PHINode>(I);
						const Type *Ty = Phi->getType();

						if (Ty->isPointerTy()) {
							unsigned n = Phi->getNumIncomingValues();
							std::vector<Value*> values;

                            for (unsigned i = 0; i < n; i++) {
                                Value *v = Phi->getIncomingValue(i);

                                handleGetElementPtrConst(v);

                                int a = Value2Int(I);
                                int b = Value2Int(v);
                                pointerAnalysis->addBase(a, b);
                                PABaseCt++;

								values.push_back(v);

								if (phiValues.count(v)) {
									if (memoryBlocks.count(v)) {
										memoryBlocks[I] = std::vector<std::vector<int> >();
										memoryBlocks[I].insert(memoryBlocks[I].end(), memoryBlocks[v].begin(), memoryBlocks[v].end());
									}
								} else {
									if (memoryBlock.count(v)) {
										memoryBlocks[I] = std::vector<std::vector<int> >();

										if (isa<BitCastInst>(v)) {
											BitCastInst *BC = dyn_cast<BitCastInst>(v);

											Value *v2 = BC->getOperand(0);

											if (memoryBlock.count(v2)) {
												std::vector<int> mems = memoryBlock[v2];
												int parent = mems[0];
												std::vector<int> mems2 = memoryBlock2[parent];

												memoryBlocks[I].push_back(mems2);
											}
										} else
											memoryBlocks[I].push_back(memoryBlock[v]);
									}
								}
							}
						}

						break;
					}
			}

			//errs() << "\n";
		}
	}
}

// ============================= //

void PADriver::matchFormalWithActualParameters(Function &F) {
    if (F.arg_empty() || F.use_empty())
        return;

    /*
    */
    // errs() << "F: " << F.getName() << "\n";
    for (Value::user_iterator UI = F.user_begin(), E = F.user_end(); UI != E;
         ++UI) {
        User *U = *UI;

        // errs() << "U: " << *U << "\n";

        if (isa<ConstantExpr>(U)) {
            // %1 = call i8* bitcast (void (i8*, i8, i32)* @legup_memset_4 to
            // i8* (i8*, i8, i64)*)(i8* bitcast ([24 x i32]* @tqmf to i8*), i8
            // 0, i64 96) #2
            U = dyn_cast<User>(U->getOperand(0));
            // assert(0);
        }

        if (isa<BlockAddress>(U))
            continue;
        if (!isa<CallInst>(U) && !isa<InvokeInst>(U)) {
            // assert(0);
            continue;
        }

        CallSite CS(cast<Instruction>(U));
        if (CS.getCalledFunction() != &F)
            continue;

        // errs() << "U-After: " << *U << "\n";

        CallSite::arg_iterator actualArgIter = CS.arg_begin();
        Function::arg_iterator formalArgIter = F.arg_begin();
        int size = F.arg_size();

        for (int i = 0; i < size; ++i, ++actualArgIter, ++formalArgIter) {
            Value *actualArg = *actualArgIter;

            // handle a case like:
            //  tail call fastcc void @upzero(i32 %127, i32* getelementptr
            //  inbounds ([6 x i32]* @delay_dltx, i32 0, i32 0), i32*
            //  getelementptr inbounds ([6 x i32]* @delay_bpl, i32 0, i32 0))
            handleGetElementPtrConst(actualArg);
            // ConstantExpr *CE = dyn_cast<ConstantExpr>(actualArg);
            // if (CE && CE->getOpcode() == Instruction::GetElementPtr) {
            // actualArg = CE->getOperand(0);
            //}

            Value *formalArg = formalArgIter;

            int a = Value2Int(formalArg);
            int b = Value2Int(actualArg);
			pointerAnalysis->addBase(a, b);
            PABaseCt++;
        }
    }
}

// ============================= //

void PADriver::matchReturnValueWithReturnVariable(Function &F) {
	if (F.getReturnType()->isVoidTy() || F.mayBeOverridden()) return;

	std::set<Value*> retVals;

	for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB) {
		if (ReturnInst *RI = dyn_cast<ReturnInst>(BB->getTerminator())) {
			Value *v = RI->getOperand(0);

			retVals.insert(v);
        }
    }

    for (Value::user_iterator UI = F.user_begin(), E = F.user_end(); UI != E;
         ++UI) {
        // User *U = *UI;
        CallSite CS(*UI);
        Instruction *Call = CS.getInstruction();

        if (!Call || CS.getCalledFunction() != &F)
            continue;

        if (Call->use_empty())
            continue;

        for (std::set<Value*>::iterator it = retVals.begin(), E = retVals.end(); it != E; ++it) {

			int a = Value2Int(CS.getCalledFunction());
			int b = Value2Int(*it);
			pointerAnalysis->addBase(a, b);
			PABaseCt++;
		}
	}
}

// ============================= //

void PADriver::handleAlloca(Instruction *I) {
	AllocaInst *AI = dyn_cast<AllocaInst>(I);
	const Type *Ty = AI->getAllocatedType();

    std::vector<int> mems;
    unsigned numElems = 1;
    // bool isStruct = false;

    if (Ty->isStructTy()) { // Handle structs
        // const StructType *StTy = dyn_cast<StructType>(Ty);
        // numElems = StTy->getNumElements();
        // isStruct = true;
    }

    if (!memoryBlock.count(I)) {
        for (unsigned i = 0; i < numElems; i++) {
            mems.push_back(getNewMemoryBlock());

            /*
            if (isStruct) {
                const StructType *StTy = dyn_cast<StructType>(Ty);

                if (StTy->getElementType(i)->isStructTy())
                    handleNestedStructs(StTy->getElementType(i), mems[i]);
            }
            */
        }

        memoryBlock[I] = mems;
    } else {
		mems = memoryBlock[I];
	}

	for (unsigned i = 0; i < mems.size(); i++) {
		int a = Value2Int(I);
		pointerAnalysis->addAddr(a, mems[i]);
		PAAddrCt++;
	}
}

// ============================= //

void PADriver::handleGlobalVariable(GlobalVariable *G) {
	const Type *Ty = G->getType();

	std::vector<int> mems;
	unsigned numElems = 1;
	bool isStruct = false;

	if (Ty->isStructTy()) { // Handle structs
		const StructType *StTy = dyn_cast<StructType>(Ty);
		numElems = StTy->getNumElements();
		isStruct = true;
	}

	if (!memoryBlock.count(G)) {
		for (unsigned i = 0; i < numElems; i++) {
			mems.push_back(getNewMemoryBlock());

			if (isStruct) {
				const StructType *StTy = dyn_cast<StructType>(Ty);

				if (StTy->getElementType(i)->isStructTy())
					handleNestedStructs(StTy->getElementType(i), mems[i]);
			}
		}

		memoryBlock[G] = mems;
	} else {
		mems = memoryBlock[G];
	}

	for (unsigned i = 0; i < mems.size(); i++) {
		int a = Value2Int(G);
		pointerAnalysis->addAddr(a, mems[i]);
		PAAddrCt++;
    }
}

// handle cases where there is a constant getelementptr value as an operand:
//      store i32 43, i32* getelementptr inbounds ([32 x i32]* @key, i32 0, i32
//      0), align 4, !tbaa !1
void PADriver::handleGetElementPtrConst(Value *op) {
    ConstantExpr *CE = dyn_cast<ConstantExpr>(op);
    if (CE && CE->getOpcode() == Instruction::GetElementPtr) {
        // errs() << "op: " << *op << "\n";
        Value *mem = CE->getOperand(0);
        // errs() << "mem: " << *mem << "\n";

        int src = Value2Int(mem);
        int dst = Value2Int(op);
        pointerAnalysis->addBase(dst, src);
        PABaseCt++;
    }
}

// ============================= //

void PADriver::handleGetElementPtr(Instruction *I) {

    //errs() << "INSIDE GetElementPtrInst \n";

	GetElementPtrInst *GEPI = dyn_cast<GetElementPtrInst>(I);
	Value *v = GEPI->getPointerOperand();
	const PointerType *PoTy = cast<PointerType>(GEPI->getPointerOperandType());
	const Type *Ty = PoTy->getElementType();

	if (Ty->isStructTy()) {
		if (phiValues.count(v)) {
			std::vector<Value*> values = phiValues[v];

			for (unsigned i = 0; i < values.size(); i++) {
				Value* vv = values[i];

				if (memoryBlocks.count(vv)) {
					for (unsigned j = 0; j < memoryBlocks[vv].size(); j++) {
						int i = 0;
						unsigned pos = 0;
						bool hasConstantOp = true;

						for (User::op_iterator it = GEPI->idx_begin(), e = GEPI->idx_end(); it != e; ++it) {
							if (i == 1) {
								if (isa<ConstantInt>(*it))
									pos = cast<ConstantInt>(*it)->getZExtValue();
								else
									hasConstantOp = false;
							}

							i++;
						}
						if (hasConstantOp) {
							std::vector<int> mems = memoryBlocks[vv][j];
							int a = Value2Int(I);
							if (pos < mems.size()) {
								pointerAnalysis->addAddr(a, mems[pos]);
								PAAddrCt++;
							}
						}
					}
				} else {
					if (memoryBlock.count(vv)) {
						if (isa<BitCastInst>(vv)) {
							BitCastInst *BC = dyn_cast<BitCastInst>(vv);

							Value *v2 = BC->getOperand(0);

							if (memoryBlock.count(v2)) {
								int i = 0;
								unsigned pos = 0;
								bool hasConstantOp = true;

								for (User::op_iterator it = GEPI->idx_begin(), e = GEPI->idx_end(); it != e; ++it) {
									if (i == 1) {
										if (isa<ConstantInt>(*it))
											pos = cast<ConstantInt>(*it)->getZExtValue();
										else
											hasConstantOp = false;
									}

									i++;
								}

								if (hasConstantOp) {
									std::vector<int> mems = memoryBlock[v2];
									int parent = mems[0];
									if (memoryBlock2.count(parent)) {
										std::vector<int> mems2 = memoryBlock2[parent];

										int a = Value2Int(I);
										if (pos < mems2.size()) {
											pointerAnalysis->addAddr(a, mems2[pos]);
											PAAddrCt++;
										}
									}
								}
							}
						} else {
							int i = 0;
							unsigned pos = 0;
							bool hasConstantOp = true;

							for (User::op_iterator it = GEPI->idx_begin(), e = GEPI->idx_end(); it != e; ++it) {
								if (i == 1) {
									if (isa<ConstantInt>(*it))
										pos = cast<ConstantInt>(*it)->getZExtValue();
									else
										hasConstantOp = false;
								}

								i++;
							}

							if (hasConstantOp) {
								std::vector<int> mems = memoryBlock[vv];
								int a = Value2Int(I);
								//pointerAnalysis->addBase(a, mems[pos]);
								if (pos < mems.size()) {
									pointerAnalysis->addAddr(a, mems[pos]);
									PAAddrCt++;
								}
							}
						}
					} else {
						GetElementPtrInst *GEPI2 = dyn_cast<GetElementPtrInst>(vv);

						if (!GEPI2) return;

						Value *v2 = GEPI2->getPointerOperand();

						if (memoryBlock.count(v2)) {
							int i = 0;
							unsigned pos = 0;
							bool hasConstantOp = true;

							for (User::op_iterator it = GEPI2->idx_begin(), e = GEPI2->idx_end(); it != e; ++it) {
								if (i == 1) {
									if (isa<ConstantInt>(*it))
										pos = cast<ConstantInt>(*it)->getZExtValue();
									else
										hasConstantOp = false;
								}

								i++;
							}

							if (hasConstantOp) {
								std::vector<int> mems = memoryBlock[v2];
								if (pos < mems.size()) {
									int parent = mems[pos];

									i = 0;
									unsigned pos2 = 0;

									for (User::op_iterator it = GEPI->idx_begin(), e = GEPI->idx_end(); it != e; ++it) {
										if (i == 1)
											pos2 = cast<ConstantInt>(*it)->getZExtValue();

										i++;
									}

									if (memoryBlock2.count(parent)) {
										std::vector<int> mems2 = memoryBlock2[parent];
										int a = Value2Int(I);

										if (pos2 < mems2.size()) {
											pointerAnalysis->addAddr(a, mems2[pos2]);
											PAAddrCt++;
											memoryBlock[v] = mems2;
										}
									}

								}
							}
						}
					}
				}
			}
		} else {
			if (memoryBlock.count(v)) {
				if (isa<BitCastInst>(v)) {
					BitCastInst *BC = dyn_cast<BitCastInst>(v);

					Value *v2 = BC->getOperand(0);

					if (memoryBlock.count(v2)) {
						int i = 0;
						unsigned pos = 0;
						bool hasConstantOp = true;

						for (User::op_iterator it = GEPI->idx_begin(), e = GEPI->idx_end(); it != e; ++it) {
							if (i == 1) {
								if (isa<ConstantInt>(*it))
									pos = cast<ConstantInt>(*it)->getZExtValue();
								else
									hasConstantOp = false;
							}

							i++;
						}

						if (hasConstantOp) {
							std::vector<int> mems = memoryBlock[v2];
							int parent = mems[0];
							if (memoryBlock2.count(parent)) {
								std::vector<int> mems2 = memoryBlock2[parent];

								int a = Value2Int(I);
								if (pos < mems2.size()) {
									pointerAnalysis->addAddr(a, mems2[pos]);
									PAAddrCt++;
								}
							}
						}
					}
				} else {
					int i = 0;
					unsigned pos = 0;
					bool hasConstantOp = true;

					for (User::op_iterator it = GEPI->idx_begin(), e = GEPI->idx_end(); it != e; ++it) {
						if (i == 1) {
							if (isa<ConstantInt>(*it))
								pos = cast<ConstantInt>(*it)->getZExtValue();
							else
								hasConstantOp = false;
						}

						i++;
					}

					if (hasConstantOp) {
						std::vector<int> mems = memoryBlock[v];
						int a = Value2Int(I);
						//pointerAnalysis->addBase(a, mems[pos]);
						if (pos < mems.size()) {
							pointerAnalysis->addAddr(a, mems[pos]);
							PAAddrCt++;
						}
					}
				}
			} else {
				GetElementPtrInst *GEPI2 = dyn_cast<GetElementPtrInst>(v);

				if (!GEPI2) return;

				Value *v2 = GEPI2->getPointerOperand();

				if (memoryBlock.count(v2)) {
					int i = 0;
					unsigned pos = 0;
					bool hasConstantOp = true;

					for (User::op_iterator it = GEPI2->idx_begin(), e = GEPI2->idx_end(); it != e; ++it) {
						if (i == 1) {
							if (isa<ConstantInt>(*it))
								pos = cast<ConstantInt>(*it)->getZExtValue();
							else
								hasConstantOp = false;
						}

						i++;
					}

					if (hasConstantOp) {
						std::vector<int> mems = memoryBlock[v2];
						if (pos < mems.size()) {
							int parent = mems[pos];

							i = 0;
							unsigned pos2 = 0;

							for (User::op_iterator it = GEPI->idx_begin(), e = GEPI->idx_end(); it != e; ++it) {
								if (i == 1)
									pos2 = cast<ConstantInt>(*it)->getZExtValue();

								i++;
							}

							if (memoryBlock2.count(parent)) {
								std::vector<int> mems2 = memoryBlock2[parent];
								int a = Value2Int(I);

								if (pos2 < mems2.size()) {
									pointerAnalysis->addAddr(a, mems2[pos2]);
									PAAddrCt++;
									memoryBlock[v] = mems2;
								}
							}
						}
					}
				}
			}
		}
	} else {
		//errs() << "  >> I: " << *I << "\n";
		//errs() << "  >> v: " << *v << "\n";
		int a = Value2Int(I);
		int b = Value2Int(v);
		pointerAnalysis->addBase(a, b);
		PABaseCt++;
	}
}

// ============================= //2

void PADriver::handleNestedStructs(const Type *Ty, int parent) {
	const StructType *StTy = dyn_cast<StructType>(Ty);
	unsigned numElems = StTy->getNumElements();
	std::vector<int> mems;

	for (unsigned i = 0; i < numElems; i++) {
		mems.push_back(getNewMemoryBlock());

		if (StTy->getElementType(i)->isStructTy())
			handleNestedStructs(StTy->getElementType(i), mems[i]);
	}

	memoryBlock2[parent] = mems;

	for (unsigned i = 0; i < mems.size(); i++) {
		pointerAnalysis->addAddr(parent, mems[i]);
		PAAddrCt++;
	}
}

// ============================= //

int PADriver::getNewMemoryBlock() {
	return nextMemoryBlock++;
}

// ============================= //

int PADriver::getNewInt() {
	return getNewMemoryBlock();
}

// ============================= //

int PADriver::Value2Int(Value *v) {

	int n;

	if (value2int.count(v))
		return value2int[v];

    n = getNewInt();
    value2int[v] = n;
    int2value[n] = v;

    // Also get a name for it
    if (v->hasName()) {
        nameMap[n] = v->getName();
	}
	else if (isa<Constant>(v)) {
		nameMap[n] = "constant";
		//errs() << "Constant: " << *v << "\n";
        // nameMap[n] += intToStr(n);
    } else {
        // run with -instnamer to avoid this:
        nameMap[n] = "unknown";
        // errs() << "Unnamed: " << *(v->getType()) << "\n";
    }

    return n;
}

// ============================= //

// Get a (possibly new) int ID associated with
// the given Value
//int PADriver::Value2Int(Value* v) {
	//// If we already have this value,
	//// just return it!
	//if (valMap.count(v))
		//return valMap[v];

	//// If it's a new value, get new ID
	//// and save it
	//int n = ++currInd;
	//valMap[v] = n;

	//// Count the memory usage
	////PAMemUsage += sizeof(std::pair<Value*,int>);

	//// Also get a name for it
	//if (v->hasName()) {
		//nameMap[n] = v->getName();
	//}
	//else if (isa<Constant>(v)) {
		//nameMap[n] = "constant";
		////nameMap[n] += intToStr(n);
	//}
	//else {
		//nameMap[n] = "unknown";
	//}

	//return n;
//}

// ============================= //
/*
Value* AddrLeaks::Int2Value(int x)
{
	return int2value[x];
}
*/
// ========================================= //

// Register the pass to the LLVM framework
// char PADriver::ID = 0;
// static RegisterPass<PADriver> X("pa", "Pointer Analysis Driver Pass");
