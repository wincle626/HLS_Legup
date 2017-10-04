##===- lib/Transforms/LegUp/Makefile -----------------------*- Makefile -*-===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

LEVEL = ../../..
LIBRARYNAME = LLVMLegUp
LOADABLE_MODULE = 1
#USEDLIBS = LLVMCodeGen.a LLVMVerilog.a

include $(LEVEL)/Makefile.common

# if you modify a file in Target/Verilog then you'll need to run 'make' from
# the legup base directory
CPP.Flags += -I$(LLVM_SRC_ROOT)/lib/Target/Verilog
#-I$(LLVM_SRC_ROOT)/../cloog/install/include \
#-I$(LLVM_SRC_ROOT)/tools/polly/include \
