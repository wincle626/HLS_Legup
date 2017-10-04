##===- lib/Makefile ----------------------------------------*- Makefile -*-===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##
LEVEL = ..

include $(LEVEL)/Makefile.config

# Transforms/LegUp depends on IntrinsicLowering from CodeGen
DIRS := CodeGen Target Transforms

PARALLEL_DIRS := VMCore AsmParser Bitcode Archive Analysis \
                ExecutionEngine Linker MC CompilerDriver Object

include $(LEVEL)/Makefile.common

