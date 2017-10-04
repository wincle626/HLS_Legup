//===-- Utils.h -----------------------------------------------*- C++ -*-===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// These utilities are used by many LegUp classes
//
//===----------------------------------------------------------------------===//

#ifndef LEGUP_UTILS_H
#define LEGUP_UTILS_H

#include "llvm/ADT/StringExtras.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Function.h"
//#include "llvm/Assembly/Writer.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FormattedStream.h"
#include <string>
#include <set>

using namespace llvm;

namespace legup {

class State;
class FiniteStateMachine;

BasicBlock* isEmptyFirstBB(BasicBlock *BB);

bool isNumeric(std::string str);
  std::vector<std::string> stringsFromStringDelimitedByString(std::string string, const std::string del);

void printLabel(raw_ostream &Out, const BasicBlock *v);
std::string getLabel(const Value *v);
std::string getLabelStripped(const Value *v);

// cached lookup of LLVM value string representation
std::string getValueStr(const Value *V);

void setMetadataInt(Instruction *I, std::string kind, int value);
int getMetadataInt(const Instruction *I, std::string kind);
void setMetadataStr(Instruction *I, std::string kind, std::string value);
std::string getMetadataStr(const Instruction *I, std::string kind);

// convert an integer byte into a hex character
char itohex(char c);

// turn an lpsolve constraint: GE, LE, EQ into a string
std::string lpConstraintStr(int constr_type);

// convert the bytes at ptr into hex and store in buffer
// buffer should have a size of bytes*2+1
void hex_string(char *buffer, char *ptr, int bytes);

std::string getFileHeader();

void removeBasicBlocksFromFunction(Function &F);
void removeFunctionsFromVectorIfNotCalled(std::vector<Function *> &functions);

// see also utostr() for unsigned integers
std::string IntToString (int i);
std::string ftostr (float i);
bool strIsInt(std::string str);
int strToInt(std::string str);
bool stringEndsWith(std::string const & fullString, std::string const & ending);
bool stringStartsWith(std::string const & fullString, std::string const & start);
  
bool isString(Constant *C);

Function *getCalledFunction(CallInst *CI);
std::vector<Function *> getFunctionsCalledByFunction(Function *function);

// replace all occurrences of needle in haystack with replace
bool replaceAll(std::string &haystack, const std::string &needle,
                const std::string &replace);

void stripInvalidCharacters(std::string &str);

// Returns the number of bits required to hold max. Examples:
// max=0: bits=1
// max=1: bits=1
// max=10: bits=4
// max=7: bits=3
unsigned requiredBits(unsigned max);

int getGlobalMemLatency();
bool isaPrintCall(Instruction *I);
bool isaPrintCall(Function *F);

bool isaPrintfString(Value *V);

// isaDummyCall - return if this instruction does not actually call a function
bool isaDummyCall(const Instruction *instr);

// isaDummyDebugCall - return if this instruction is a dummy debug call
// (llvm.dbg.declare, llvm.dbg.value)
//bool isaDummyDebugCall(const Instruction *instr);

bool isLogicalShift(Instruction *instr);

bool op1Signed(Instruction *instr);
bool op2Signed(Instruction *instr);

unsigned getBitWidth(const Type* T);

// gets the index of the most significant bit of a signal
// ie. bitwidth minus 1
std::string getMSBIndex(const Type* T);


bool isRem(Instruction *instr);

bool isDiv(Instruction *instr);

bool isUnsignedDivOrRem(Instruction *instr);

bool isSignedDivOrRem(Instruction *instr);

bool isAdd(Instruction *instr);

bool isSub(Instruction *instr);

bool isMul(Instruction *instr);

bool isFPArith(Instruction *instr);

bool isFPCmp(Instruction *instr);

bool isFPCast(Instruction *instr);

bool functionRequiresMemory(Function *F);
bool isMem(Value *V);
bool isMem(Instruction *instr);

bool isShift(Instruction *instr);

bool isBitwiseOperation(Instruction *instr);

bool checkforPthreads(Function* Fp);

std::vector<std::string> split(const std::string &s, char delim);
std::vector<std::string> &split(const std::string &s, char delim,
        std::vector<std::string> &elems);

std::string charToString(unsigned char C, bool &LastWasHex);
std::string arrayToString(ConstantArray *CPA);

std::string getTransitionOp(State *s);

// is the value from a loop pipelined basic block
bool isPipelined(const Value *v);

// Given a state that loops to itself until a condition:
//      curState = (condition) ? loopState : nextState
// this function returns the nextState
State* getStateAfterLoop(State *loopState);

int getPipelineII(const BasicBlock *BB);

void printFSMDotFile(FiniteStateMachine *fsm, std::string fileName);

// replace string with ... after limit
void limitString(std::string &s, unsigned limit);

// For multi-cycle path debugging
raw_ostream &mc_debug();

bool fileExists(const std::string &name);

// this has to be declared in the header due to templates
template <typename T> class dotGraph {
public:
    // The callback must be defined for printing out node labels, for
    // example:
    //    void printNodeLabel(raw_ostream &out, InstructionNode *I) {
    //       out << *I->getInst();
    //    }
    dotGraph(formatted_raw_ostream &o,
            void (*callback_node_label)(raw_ostream &out, T *I),
            void (*callback_node_label_extras)(raw_ostream &out, T *I)=NULL
            ) :
        limit(40), out(o), printNodeLabel(callback_node_label),
        printNodeLabelExtras(callback_node_label_extras) {
        out << "digraph {\n";
    }

    ~dotGraph() {
        out << "}\n";
    }

    void printLabel(formatted_raw_ostream &out, T *I) {
        out << "[label=\"";

        std::string stripped;
        raw_string_ostream stream(stripped);

        printNodeLabel(stream, I);
        // need to flush the stream to write to string!
        stream.flush();

        // newlines aren't allowed in dot labels
        replaceAll(stripped, "\n", " ");

        // limit the size of the instruction string
        limitString(stripped, limit);

        out << stripped;

        out << "\"";

        if (printNodeLabelExtras) {
            printNodeLabelExtras(out, I);
        }

        out << "]";
    }

    void printNode(formatted_raw_ostream &out, T *I) {
        out << "Node" << static_cast<const void*>(I);
    }

    void connectDot(formatted_raw_ostream &out, T *driver,
            T *signal, std::string label) {

        if (seen.find(signal) == seen.end()) {
            printNode(out, signal);
            printLabel(out, signal);
            out << ";\n";
            seen.insert(signal);
        }

        if (seen.find(driver) == seen.end()) {
            printNode(out, driver);
            printLabel(out, driver);
            out << ";\n";
            seen.insert(driver);
        }

        printNode(out, driver);
        out << " -> ";
        printNode(out, signal);
        if (!label.empty()) out << "[" << label << "]";
        out << ";\n";
    }

    void setLabelLimit(int l) { limit = l; }

private:
    std::set<T*> seen;
    unsigned limit;
    formatted_raw_ostream &out;
    void (*printNodeLabel)(raw_ostream &out, T *I);
    void (*printNodeLabelExtras)(raw_ostream &out, T *I);
};

} // End legup namespace


#endif
