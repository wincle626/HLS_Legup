NAME=reg_detect.orig

ifeq ($(NAME), reg_detect.orig)
	LOCAL_CONFIG = -legup-config=config.orig.tcl
else ifeq ($(NAME), reg_detect.opt)
	LOCAL_CONFIG = -legup-config=config.opt.tcl
endif

# don't unroll the loop
CFLAG += -mllvm -unroll-threshold=0

LEVEL = ../..
include $(LEVEL)/Makefile.common
