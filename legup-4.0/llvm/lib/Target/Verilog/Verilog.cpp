//===-- Verilog.cpp - Library for converting LLVM code to Verilog ---------===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// This library converts LLVM code to Verilog code LLVM calls this pass through
// runOnFunction(Function &F)
//
//===----------------------------------------------------------------------===//

#include "LegupPass.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/PassManager.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetRegistry.h"
#include "VerilogTargetMachine.h"
#include "llvm/Analysis/Passes.h"
#include <fstream>

using namespace llvm;
using namespace legup;

// Register the Verilog target with LLVM
extern "C" void LLVMInitializeVerilogTarget() { 
  RegisterTargetMachine<VerilogTargetMachine> X(TheVerilogTarget);
}

char LegupPass::ID = 0;


//===----------------------------------------------------------------------===//
//                       External Interface declaration
//===----------------------------------------------------------------------===//

bool VerilogTargetMachine::addPassesToEmitFile(PassManagerBase &PM,
                                         formatted_raw_ostream &o,
                                         CodeGenFileType FileType,
                                         bool DisableVerify,
                                         AnalysisID StartAfter,
                                         AnalysisID StopAfter) {
  if (FileType != TargetMachine::CGFT_AssemblyFile) return true;
  PassRegistry &Registry = *PassRegistry::getPassRegistry();
  initializeBasicAliasAnalysisPass(Registry);
  initializeLoopInfoPass(Registry);
  initializeScalarEvolutionPass(Registry);

  if (LEGUP_CONFIG->getParameterInt("LLVM_PROFILE")) {
    // Also check if the llvm profiling ran and the file 
    // llvmprof.out was generated. If not, disable profile analysis
    ifstream prof_file("llvmprof.out");
    if (prof_file) {
		// TODO LLVM 3.4 update: had to comment these lines out.  these functions no longer exist
        //initializePathProfileLoaderPassPass(Registry);
        //PM.add(createProfileLoaderPass());
    } 
    else {
        LEGUP_CONFIG->setParameter("LLVM_PROFILE", "0");
    }
  }

  PM.add(createBasicAliasAnalysisPass());
  PM.add(new LegupPass(o));
  return false;
}

