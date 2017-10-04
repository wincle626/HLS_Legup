NAME=gsm
LOCAL_CONFIG = -legup-config=config.tcl
LEVEL = ../..

#DEBUG_G_FLAG = 1
#UNROLL = -unroll-allow-partial -unroll-threshold=5000
# All functions are inlined already with out flag, but putting this helps cycle count for some reason
# maybe because clang inline everything, which is then optimized by llvm?
CFLAG += -mllvm -inline-threshold=10000000

include $(LEVEL)/Makefile.common
