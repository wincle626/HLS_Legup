NAME=dfadd_dwarv
SRCS=dfadd_dwarv.c
# can't do this while running "make parallel"
#UNROLL = -unroll-allow-partial -unroll-threshold=500
#CFLAG += -mllvm -inline-threshold=10000
LOCAL_CONFIG = -legup-config=config.tcl
LEVEL = ../..
include $(LEVEL)/Makefile.common
