NAME=aes
SRCS=aes.c
UNROLL = -unroll-allow-partial -unroll-threshold=350
CFLAG += -mllvm -inline-threshold=10000
LOCAL_CONFIG = -legup-config=config.tcl
LEVEL = ../..
include $(LEVEL)/Makefile.common
