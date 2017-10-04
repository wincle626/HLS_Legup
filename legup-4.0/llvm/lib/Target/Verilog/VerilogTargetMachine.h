//===-- VerilogTargetMachine.h - Verilog backend ----------------*- C++ -*-===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the TargetMachine that is used by the Verilog backend.
//
//===----------------------------------------------------------------------===//

#ifndef VERILOGTARGETMACHINE_H
#define VERILOGTARGETMACHINE_H

#include "llvm/Target/TargetMachine.h"
#include "llvm/IR/DataLayout.h"

namespace llvm {

struct VerilogTargetMachine : public TargetMachine {
  VerilogTargetMachine(const Target &T, StringRef TT,
                   StringRef CPU, StringRef FS,
				   const TargetOptions &Options,
                   Reloc::Model RM, CodeModel::Model CM,
				   CodeGenOpt::Level OL)
    : TargetMachine(T, TT, CPU, FS, Options) {}
  

  virtual bool addPassesToEmitFile(PassManagerBase &PM,
                                   formatted_raw_ostream &o,
                                   CodeGenFileType FileType,
                                   bool DisableVerify,
                                   AnalysisID StartAfter,
                                   AnalysisID StopAfter);
  
  virtual const DataLayout *getDataLayout() const { return 0; }
};

extern Target TheVerilogTarget;

} // End llvm namespace


#endif
