NAME=dfsin_dwarv
SRCS=dfsin_dwarv.c
# can't do this while running "make parallel"
#UNROLL = -unroll-allow-partial -unroll-threshold=500
#CFLAG += -mllvm -inline-threshold=1000
LOCAL_CONFIG = -legup-config=config.tcl
LEVEL = ../..
include $(LEVEL)/Makefile.common
