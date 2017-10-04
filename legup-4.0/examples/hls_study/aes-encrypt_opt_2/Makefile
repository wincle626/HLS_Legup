NAME=aes
SRCS=aes.c
LOCAL_CONFIG = -legup-config=config.tcl
LEVEL = ../..

#UNROLL = -unroll-allow-partial -unroll-threshold=1000
CFLAG += -mllvm -inline-threshold=10000

include $(LEVEL)/Makefile.common
