NAME=adpcm

CFLAG += -mllvm -inline-threshold=500
UNROLL = -unroll-allow-partial -unroll-threshold=1000

LOCAL_CONFIG = -legup-config=config.tcl
LEVEL = ../..
include $(LEVEL)/Makefile.common
