NAME=bf
SRCS=bf.c

CFLAG += -mllvm -inline-threshold=5000
UNROLL = -unroll-allow-partial -unroll-threshold=500

LOCAL_CONFIG = -legup-config=config.tcl
LEVEL = ../..
include $(LEVEL)/Makefile.common
