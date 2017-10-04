
NAME=sha_driver

UNROLL = -unroll-allow-partial -unroll-threshold=500
CFLAG += -mllvm -inline-threshold=10000
LOCAL_CONFIG = -legup-config=config.tcl
LEVEL = ../..
include ../Makefile.common
include $(LEVEL)/Makefile.common
