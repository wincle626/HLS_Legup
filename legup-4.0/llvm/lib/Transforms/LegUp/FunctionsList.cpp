//===- FunctionsList.cpp - LegUp pre-LTO pass ------------------------------------===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// The FunctionsList pass generates a list of functions in the program (functions.list)
//
//===----------------------------------------------------------------------===//

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "utils.h"

using namespace llvm;

namespace legup {

class FuncList  : public ModulePass {
public:
    static char ID; // Pass identification, replacement for typeid
    FuncList() : ModulePass(ID) {}

    virtual bool runOnModule(Module &F);

    Module *Mod;
private:
};

char FuncList::ID = 0;
static RegisterPass<FuncList> F("legup-func-list",
        "Generates a list of functions");

/// generates a list of functions in the program (functions.list)
bool FuncList::runOnModule(Module &M) {
    Mod = &M;
	formatted_raw_ostream *function_list;
    std::string functionList = "functions.list";
    function_list = GetOutputStream(functionList);
    assert(function_list);
    raw_ostream &list = *function_list;

	//print out a list of functions
	for (Module::iterator F = M.begin(), FE = M.end(); F != FE; ++F) {
		std::string funcName = F->getName();
		//do not include main, printf, exit, and functions that start with llvm. in the list 
		if ((funcName.find("llvm.") == std::string::npos) && (funcName != "main") && (funcName != "printf") && (funcName != "exit")) {
			list << funcName << "\n";
		}
	}
    delete function_list;

    return false;

}

} // end of legup namespace
