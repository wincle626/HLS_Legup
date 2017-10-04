NAME=aes
SRCS=aes.c
LOCAL_CONFIG = -legup-config=config.tcl
LEVEL = ../..

#UNROLL = -unroll-allow-partial -unroll-threshold=500
CFLAG += -mllvm -inline-threshold=10000000

include $(LEVEL)/Makefile.common
