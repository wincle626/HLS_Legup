//===-- Utils.cpp -----------------------------------------------*- C++ -*-===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the utility functions
//
//===----------------------------------------------------------------------===//
#include "utils.h"
#include "LegupPass.h"
#include "LegupConfig.h"
#include "RTL.h"
#include "State.h"
#include "FiniteStateMachine.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Intrinsics.h"
#include <lp_lib.h>
#include <map>
#include <string>
#include <sstream>
#include <sys/stat.h>
#include "llvm/Support/FileSystem.h"

using namespace llvm;
using namespace legup;

namespace legup {

// An example of an empty basic block at the start of main:
// define i32 @main() nounwind {
//   br label %1
// Returns NULL if the BB is not empty
BasicBlock *isEmptyFirstBB(BasicBlock *BB) {
    // basic block should only include a branch instruction
    if (BB->size() != 1)
        return NULL;

    BasicBlock *firstBB = BB->getParent()->begin();
    if (firstBB != BB)
        return NULL;

    // look for an unconditional branch to expected successor
    TerminatorInst *TI = BB->getTerminator();
    assert(TI);
    if (isa<ReturnInst>(TI))
        return NULL;
    BranchInst *B = dyn_cast<BranchInst>(TI);
    if (!B)
        return NULL; // a switch statement
    if (TI->getNumSuccessors() != 1) {
        return NULL;
    } else {
        BasicBlock *succ = dyn_cast<BasicBlock>(TI->getSuccessor(0));
        assert(succ);
        return succ;
    }
}

bool isNumeric(std::string str) {
   if (str.empty()) return false;
   for (unsigned i = 0; i < str.length(); i++) {
       if (!std::isdigit(str[i])) return false;
   }
   return true;
}

  std::vector<std::string> stringsFromStringDelimitedByString(std::string string, const std::string del)  {
    size_t index = 0, prev_index = 0;
    std::vector<std::string> substrings;
    while (index != std::string::npos) {
      index = string.find(del, index + 1);
      std::string substring = string.substr(prev_index, index - prev_index);
      substrings.push_back(substring);
      prev_index = index + 1;
    }
    return substrings;
  }
  
void printLabel(raw_ostream &Out, const Value *v) {
    //writeAsOperand(Out, v, false);
    v->printAsOperand(Out, false);
}

std::string getLabel(const Value *V) {
    assert(V);

    static std::map<const Value *, std::string> cache;
    if (cache.find(V) != cache.end()) {
        return cache[V];
    }

    std::string tmp;
    raw_string_ostream Out(tmp);
    //writeAsOperand(Out, V, false);
    V->printAsOperand(Out, false);

    cache[V] = Out.str();
    return Out.str();
}

std::string getLabelStripped(const Value *v) {
    std::string label = getLabel(v);
    stripInvalidCharacters(label);
    return label;
}

std::string lpConstraintStr(int constr_type) {
  switch (constr_type) {
    case GE:
      return ">=";
    case LE:
      return "<=";
    case EQ:
      return "==";
    default:
      assert(0);
  }
}


std::string getValueStr(const Value *V) {
    assert(V);
    static std::map<const Value *, std::string> cache;
    if (cache.find(V) != cache.end()) {
        return cache[V];
    }
    std::string tmp;
    raw_string_ostream Out(tmp);
    Out << *V;
    cache[V] = Out.str();
    return Out.str();
}

// split() taken from:
// http://stackoverflow.com/questions/236129/splitting-a-string-in-c
std::vector<std::string> &split(const std::string &s, char delim,
        std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

State* getStateAfterLoop(State *loopState) {
    assert(loopState->getNumTransitions() == 2);
    State *stateTrue = loopState->getTransitionState(0);
    State *stateFalse = loopState->getDefaultTransition();
    assert(stateTrue == loopState || stateFalse == loopState);
    if (stateTrue == loopState) {
        return stateFalse;
    }
    return stateTrue;
}

// print out the finite state machine
void printFSMDotFile(FiniteStateMachine *fsm, std::string fileName) {
      std::string FileError;
      raw_fd_ostream dotFile(fileName.c_str(), FileError, llvm::sys::fs::F_None);
      assert(FileError.empty() && "Error opening dot files");
      formatted_raw_ostream out(dotFile);
      fsm->printDot(out);
}


int getPipelineII(const BasicBlock *BB) {
    const TerminatorInst *TI = BB->getTerminator();
    int II = getMetadataInt(TI, "legup.II");
    return II;
}

// Converts an integer to a string
// see also utostr() for unsigned integers
std::string IntToString (int i) {
  std::string s;
  std::stringstream out;
    out << i;
    s = out.str();
  return s;
}

// converts a float to a string with 2 decimal places
std::string ftostr (float i) {
    char f[100];
    snprintf(f, 100, "%.2f", i);
    return f;
}

// returns true if the string can be converted to an int,
// returns false if the string cannot be converted to an int.
bool strIsInt(std::string str) {

    int out;
    std::stringstream convert(str);
    return convert >> out;
    
}

// converts a string to a float.
int strToInt(std::string str) {

  int out;
  std::stringstream convert(str);
  assert(convert >> out);
  return out;
  
}

bool isString(Constant *C) {
    ConstantArray *CPA = dyn_cast<ConstantArray>(C);
    if (!CPA) return false;

    // As a special case, print the array as a string if it is an array of
    // ubytes or an array of sbytes with positive values.
    const Type *ETy = CPA->getType()->getElementType();
    bool isString = ( ETy == Type::getInt8Ty(CPA->getContext()) );

    // Make sure the last character is a null char, as automatically added by C
    if (isString && (CPA->getNumOperands() == 0 ||
                !cast<Constant>(*(CPA->op_end()-1))->isNullValue()))
        isString = false;

    return isString;

}

std::string getFileHeader() {
    std::stringstream Out;
    time_t timer = time(NULL);
    Out << "//----------------------------------------------------------------------------//\n";
    Out << "// Generated by LegUp High-Level Synthesis Tool Version " << LEGUP_VERSION << " (http://legup.org)\n";
    Out << "// Compiled: " << __TIMESTAMP__ << "\n";
    Out << "// University of Toronto\n";
    Out << "// For research and academic purposes only. Commercial use is prohibited.\n";
    Out << "// Please send bugs to: legup@eecg.toronto.edu\n";
    Out << "// Date: " << ctime(&timer);
    Out << "//----------------------------------------------------------------------------//\n\n";

    return Out.str();
}

Function *getCalledFunction(CallInst *CI) {
    Function *called = CI->getCalledFunction();
    if (!called) {
        // indirect call, for example:
        // tail call void bitcast (i32 (...)* @exit to void (i32)*)(i32 1) noreturn nounwind
        Value *Callee = CI->getCalledValue();
        if (ConstantExpr *CE = dyn_cast<ConstantExpr>(Callee)) {
            if (Function *RF = dyn_cast<Function>(CE->getOperand(0))) {
                called = RF;
            }
        }
    }
    if (!called) {
        errs() << *CI << "\n";
        report_fatal_error("LegUp does not support function pointers");
    }

    return called;
}

  std::vector<Function *> getFunctionsCalledByFunction( Function *function ) {
    std::vector<Function *> functions;
    for (Function::iterator b = function->begin(), be = function->end(); b != be; ++b) {
        for (BasicBlock::iterator instr = b->begin(), ie = b->end(); instr !=
                ie; ++instr) {
            if (isaDummyCall(instr)) continue;

            if (CallInst *CI = dyn_cast<CallInst>(instr)) {	
                Function *called = getCalledFunction(CI);
		functions.push_back(called);
	    }
	}
    }
    return functions;
  }

  // replace all occurrences of needle in haystack with replace
  bool replaceAll(std::string &haystack, const std::string &needle,
                  const std::string &replace) {
      size_t i, from = 0;
      bool found = false;
      while ((i = haystack.find(needle, from)) != std::string::npos) {
          haystack.replace(i, needle.length(), replace);
          from = i + replace.length();
          found = true;
      }
      return found;
  }

  // LLVM uses '.', '-' in their labels, and these
  // characters are illegal in Verilog identifiers.  So
  // we must replace them.
  void stripInvalidCharacters(std::string &str) {
      replaceAll(str, ".", "");
      replaceAll(str, "-", "");
      replaceAll(str, " ", "");
      replaceAll(str, "%", "");
      replaceAll(str, "@", "");
      replaceAll(str, "\"", "");
      replaceAll(str, "\\", "");

      if ((str.front() >= '0') && (str.front() <= '9')) {
          str.insert(0, 1, '_');
      }
      // while (replaceAll(str, "__", "_"))
      //;
  }

  // Returns the number of bits required to hold max. Examples:
  // assert(requiredBits(0) == 1);
  // assert(requiredBits(1) == 1);
  // assert(requiredBits(10) == 4);
  // assert(requiredBits(7) == 3);
  // assert(requiredBits(4) == 3);
unsigned requiredBits(unsigned max) {
    int bits = 1;
    while ((unsigned)(1<<bits) <= max)
        bits++;
    return bits;
}

bool isaDummyCall(const Instruction *instr) {
    const CallInst *ci = dyn_cast<CallInst>(instr);
    if (!ci) {
        return false;
    }

    Function *calledFunc = ci->getCalledFunction();
    // ignore indirect function invocations
    if (!calledFunc) {
        return false;
    }

    //if it's an openmp function call
    //ignore all call instructions except for the first one with a threadID of 0
    int parallelInstances = getMetadataInt(instr, "NUMTHREADS");
    std::string functionType = getMetadataStr(instr, "TYPE");
	if (parallelInstances != 0 && functionType == "omp_function") {
        int threadID = getMetadataInt(instr, "THREADID");
        if (threadID != 0) {
            //errs () << "this is a dummy parallel function!\n";
            return true;
        }
    }

    std::string funcName = calledFunc->getName();
    return (isaPrintCall(calledFunc)
            || funcName == "exit"
            || funcName == "llvm.lifetime.start"
            || funcName == "llvm.lifetime.end"
            || funcName == "llvm.invariant.start"
            || funcName == "llvm.invariant.end"
            || funcName == "__legup_label"
            || funcName == "__legup_preserve_value"
            || funcName == "llvm.dbg.declare"//NC changes
            || funcName == "llvm.dbg.value"//NC changes
           );

}

//this function returns true if threads are used in this function Fp
bool checkforPthreads(Function* Fp) {

    for (Function::iterator b = Fp->begin(), be = Fp->end(); b != be; ++b) {
        for (BasicBlock::iterator instr = b->begin(), ie = b->end(); instr !=
                ie; ++instr) {
            if (isaDummyCall(instr)) continue;

            if (CallInst *CI = dyn_cast<CallInst>(instr)) {	
				if (getMetadataStr(CI, "TYPE") == "legup_wrapper_pthreadcall") {
                    return true;
				} 
            }
        }
    }
    return false;
}

bool isaPrintCall(Instruction *I) {
    const CallInst *ci = dyn_cast<CallInst>(I);
    if (!ci) {
        return false;
    }

    Function *calledFunc = ci->getCalledFunction();
    // ignore indirect function invocations
    if (!calledFunc) {
        return false;
    }
    return isaPrintCall(calledFunc);
}

bool isaPrintCall(Function *F) {
    return (F->getName() == "printf" || F->getName() == "puts" ||
            F->getName() == "putchar");
}

bool isaPrintf(User *instr) {
    //errs() << "use " << *instr << "\n";
    ConstantExpr *CE = dyn_cast<ConstantExpr>(instr);
    if (CE && CE->getOpcode() == Instruction::GetElementPtr) {
        if (isaPrintfString(CE)) {
            return true;
        }
    } else if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(instr)) {
        if (isaPrintfString(GEP)) {
            return true;
        }
    } else if (CallInst *CI = dyn_cast<CallInst>(instr)) {
        Function *called = getCalledFunction(CI);
        return isaPrintCall(called);
    }
    //errs() << "instr: " << *instr << "\n";
    return false;
}

bool isaPrintfString(Value *V) {
    //errs() << "isaPrintfString " << *V << "\n";
    if (GlobalVariable *G = dyn_cast<GlobalVariable>(V)) {
		if (ConstantDataArray *CDA = dyn_cast<ConstantDataArray>(G->getInitializer()))
            if (!CDA->isCString())
                return false;
    }

    // this variable must only be used by printf's or legup_label()'s
    for (Value::user_iterator user = V->user_begin(), e = V->user_end();
         user != e; ++user) {
        Instruction *I = dyn_cast<Instruction>(*user);
        bool dummy = I && isaDummyCall(I);
        if (!(isaPrintf(*user) || dummy)) {
            return false;
        }
    }
    return true;
}



unsigned getBitWidth(const Type* T) {
    unsigned size = T->getPrimitiveSizeInBits();
    assert(size > 0);
    return size;
}

void setMetadataStr(Instruction *I, std::string kind, std::string value) {
    I->setMetadata(I->getContext().getMDKindID(kind),
                   MDNode::get(I->getContext(),
                               MDString::get(I->getContext(),
                                             value)));
}

void setMetadataInt(Instruction *I, std::string kind, int value) {
    setMetadataStr(I, kind, utostr(value));
}

int getMetadataInt(const Instruction *I, std::string kind) {
    const MDNode *node = I->getMetadata( I->getContext().getMDKindID(kind));
    //assert(node);
    if (!node) return 0;
    const MDString *nodeStr = dyn_cast<const MDString>(node->getOperand(0));
    assert(nodeStr);
    return atoi(nodeStr->getString().str().c_str());
}

std::string getMetadataStr(const Instruction *I, std::string kind) {
    const MDNode *node = I->getMetadata( I->getContext().getMDKindID(kind));
    //assert(node);
    if (!node) return "";
    const MDString *nodeStr = dyn_cast<const MDString>(node->getOperand(0));
    assert(nodeStr);
    return (nodeStr->getString().str());
}

// gets the index of the most significant bit of a signal
// ie. bitwidth minus 1
std::string getMSBIndex(const Type* T) {
    std::string width;
    if (isa<PointerType>(T)) {
        width = "`MEMORY_CONTROLLER_ADDR_SIZE-1";
    } else {
        unsigned size = getBitWidth(T);
        if (size > 1) {
            width = utostr(size-1);
        }
    }
    return width;
}

bool isRem(Instruction *instr) {
    return (instr->getOpcode() == Instruction::URem ||
        instr->getOpcode() == Instruction::SRem);
}

bool isDiv(Instruction *instr) {
    return (instr->getOpcode() == Instruction::UDiv ||
        instr->getOpcode() == Instruction::SDiv);
}

bool isUnsignedDivOrRem(Instruction *instr) {
  return (instr->getOpcode() == Instruction::UDiv || instr->getOpcode() == Instruction::URem);
}

bool isSignedDivOrRem(Instruction *instr) {
  return (instr->getOpcode() == Instruction::SDiv || instr->getOpcode() == Instruction::SRem);
}

bool isAdd(Instruction *instr) {
    return (instr->getOpcode() == Instruction::Add );
}

bool isSub(Instruction *instr) {
    return (instr->getOpcode() == Instruction::Sub );
}

bool isMul(Instruction *instr) {
    return (instr->getOpcode() == Instruction::Mul );
}

bool isFPArith(Instruction *instr) {
    return (instr->getOpcode() == Instruction::FAdd ||
        instr->getOpcode() == Instruction::FSub ||
        instr->getOpcode() == Instruction::FMul ||
        instr->getOpcode() == Instruction::FDiv);
}

bool isFPCmp(Instruction *instr) {
    if (const FCmpInst *cmp = dyn_cast<FCmpInst>(instr)){
        switch (cmp->getPredicate()) {
            case (FCmpInst::FCMP_OEQ):
            case (FCmpInst::FCMP_UEQ):
            case (FCmpInst::FCMP_ONE):
            case (FCmpInst::FCMP_UNE):
            case (FCmpInst::FCMP_OLE):
            case (FCmpInst::FCMP_ULE):
            case (FCmpInst::FCMP_OLT):
            case (FCmpInst::FCMP_ULT):
            case (FCmpInst::FCMP_OGE):
            case (FCmpInst::FCMP_UGE):
            case (FCmpInst::FCMP_OGT):
            case (FCmpInst::FCMP_UGT):
                return true;
            default:
                return false;
        }
    }
    return false;
}

bool isFPCast(Instruction *instr) {
    return (isa<FPExtInst>(instr) ||
        isa<FPTruncInst>(instr) ||
        isa<SIToFPInst>(instr) ||
        isa<FPToSIInst>(instr));
}

bool functionRequiresMemory(Function *Fp) {
    for (Function::iterator BB = Fp->begin(), EE = Fp->end(); BB != EE; ++BB) {
	for (BasicBlock::iterator I = BB->begin(), EE = BB->end(); I != EE; ++I) {
	    if (isMem(I))
		return true;
	}
    }
    return true;
}

bool isMem(Value *V) {
    Instruction *I = dyn_cast<Instruction>(V);
    if (!I) return false;
    return isMem(I);
}

bool isMem(Instruction *instr) {
    return (isa<LoadInst>(*instr) || isa<StoreInst>(*instr));
}

bool isShift(Instruction *instr) {
    return (instr->getOpcode() == Instruction::AShr ||
        instr->getOpcode() == Instruction::Shl || 
        instr->getOpcode() == Instruction::LShr);
}

bool isBitwiseOperation(Instruction *instr) {
    return (instr->getOpcode() == Instruction::Or ||
        instr->getOpcode() == Instruction::Xor || 
        instr->getOpcode() == Instruction::And);
}

bool isLogicalShift(Instruction *instr) {
    return (instr->getOpcode() == Instruction::Shl ||
        instr->getOpcode() == Instruction::LShr);
}

bool op1Signed(Instruction *instr) {

    return (instr->getOpcode() == Instruction::AShr || op2Signed(instr));
}

bool op2Signed(Instruction *instr) {
    bool op2Signed = (instr->getOpcode() == Instruction::SRem ||
            instr->getOpcode() == Instruction::SDiv);

    if (const ICmpInst *cmp = dyn_cast<ICmpInst>(instr)) {
        switch (cmp->getPredicate()) {
            case ICmpInst::ICMP_SLT:
            case ICmpInst::ICMP_SLE:
            case ICmpInst::ICMP_SGT:
            case ICmpInst::ICMP_SGE: op2Signed = true;
            default: break;
        }
    }
    return op2Signed;
}

std::string charToString(unsigned char C, bool &LastWasHex) {

    std::stringstream Out;
    // Print it out literally if it is a printable character.  The only thing
    // to be careful about is when the last letter output was a hex escape
    // code, in which case we have to be careful not to print out hex digits
    // explicitly (the C compiler thinks it is a continuation of the previous
    // character, sheesh...)
    //
    if (isprint(C) && (!LastWasHex || !isxdigit(C))) {
        LastWasHex = false;
        if (C == '"' || C == '\\')
            Out << "\\" << (char)C;
        else
            Out << (char)C;
    } else {
        LastWasHex = false;
        switch (C) {
            case '\n': Out << "\\n"; break;
            case '\t': Out << "\\t"; break;
            case '\r': Out << "\\r"; break;
            case '\v': Out << "\\v"; break;
            case '\a': Out << "\\a"; break;
                       case '\"': Out << "\\\""; break;
            case '\'': Out << "\\\'"; break;
            default:
                Out << "\\x";
                Out << (char)(( C/16  < 10) ? ( C/16 +'0') : ( C/16 -10+'A'));
                Out << (char)(((C&15) < 10) ? ((C&15)+'0') : ((C&15)-10+'A'));
                LastWasHex = true;
                break;
        }
    }
    return Out.str();
}

std::string arrayToString(ConstantArray *CPA) {

    std::stringstream Out;

    assert(CPA);
    assert(isString(CPA));

    // Keep track of whether the last number was a hexadecimal escape
    bool LastWasHex = false;

    // Do not include the last character, which we know is null
    for (unsigned i = 0, e = CPA->getNumOperands()-1; i != e; ++i) {
        unsigned char C =
            cast<ConstantInt>(CPA->getOperand(i))->getZExtValue();

        Out << charToString(C, LastWasHex);

    }

    return Out.str();
}

char itohex(char c) {
    switch (c) {
        case 15: return 'F';
        case 14: return 'E';
        case 13: return 'D';
        case 12: return 'C';
        case 11: return 'B';
        case 10: return 'A';
        case 9: return '9';
        case 8: return '8';
        case 7: return '7';
        case 6: return '6';
        case 5: return '5';
        case 4: return '4';
        case 3: return '3';
        case 2: return '2';
        case 1: return '1';
        case 0: return '0';
        default: assert(0);
    }
}

int getGlobalMemLatency() {
    std::string FuName = "mem_dual_port";
    int latency;
    assert(LEGUP_CONFIG->getOperationLatency(FuName, &latency));
    return latency;
}

void hex_string(char *buffer, char *ptr, int bytes) {
    char *buffer_start = buffer;
    for (int i = 0; i < bytes; i++) {
        *buffer = itohex((*ptr & 0xF0)>>4);
        buffer++;
        *buffer = itohex(*ptr & 0x0F);
        buffer++;
        ptr++;
    }
    *buffer = '\0';

    int x = 1;
    bool littleEndian = (*(char*)&x == 1);
    if (littleEndian) {
        // convert 0000C03F -> 3FC00000
        buffer--; // last char is NULL
        buffer--; // swap two characters at once
        while (buffer_start < buffer) {
            char tmp = *buffer_start;
            char tmp2 = *(buffer_start+1);
            *buffer_start = *buffer;
            *(buffer_start+1) = *(buffer+1);
            *buffer = tmp;
            *(buffer+1) = tmp2;
            buffer_start++;
            buffer_start++;
            buffer--;
            buffer--;
        }
    }

}

std::string getTransitionOp(State *s) {
    RTLSignal *op = s->getTransitionSignal();
    if (op) {
        // ie. finish signal for functions
        return op->getName();
    } else {
        Value* transitionVariable = s->getTransitionVariable();
        if (transitionVariable) {
            return getLabel(transitionVariable);
        } else {
            // this can happen for the bindingFSM from GenerateRTL
            // the FSM is not quite complete yet
            return "<null>";
        }
    }
}

bool isPipelined(const Value *v) {
    if (!v) return false;
    bool PIPELINED = false;
    const BasicBlock *BB;
    if (const Instruction *I = dyn_cast<Instruction>(v)) {
        BB = I->getParent();
        if (getMetadataInt(BB->getTerminator(), "legup.pipelined")) {
            PIPELINED = true;
        }
    }
    return PIPELINED;
}

void limitString(std::string &s, unsigned limit) {
    if (s.length() > limit) {
        s.erase(limit);
        s = s + "...";
    }
}

// For multi-cycle path debugging
raw_ostream & mc_debug() {
    if (LEGUP_CONFIG->getParameterInt("MULTI_CYCLE_DEBUG")) {
        return errs();
    }
    return nulls();
}

bool stringEndsWith(std::string const & fullString,
		std::string const &ending) {
	int prefix_length = fullString.length() - ending.length();

	return (prefix_length >= 0) &&
		equal(ending.begin(), ending.end(), fullString.begin() + prefix_length);
}

bool stringStartsWith(std::string const & fullString,
		std::string const &starting) {
    return starting.length() <= fullString.length() &&
           equal(starting.begin(), starting.end(), fullString.begin());
}

bool fileExists(const std::string &name) {
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

} // End legup namespace
