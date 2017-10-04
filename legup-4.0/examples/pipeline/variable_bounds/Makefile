NAME=variable_bounds
LOCAL_CONFIG = -legup-config=config.tcl

# don't unroll the loop
CFLAG += -mllvm -unroll-threshold=0

ifeq ($(NO_INLINE),)
	NO_INLINE=1
endif

LEVEL = ../..
include $(LEVEL)/Makefile.common
