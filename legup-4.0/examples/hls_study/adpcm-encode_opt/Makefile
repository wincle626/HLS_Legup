NAME=adpcm

CFLAG += -mllvm -inline-threshold=400
UNROLL = -unroll-allow-partial -unroll-threshold=200

LOCAL_CONFIG = -legup-config=config.tcl
LEVEL = ../..
include $(LEVEL)/Makefile.common
