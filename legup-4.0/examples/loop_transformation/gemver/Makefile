NAME=gemver.opt

ifeq ($(NAME), gemver.orig)
	LOCAL_CONFIG = -legup-config=config.orig.tcl
else ifeq ($(NAME), gemver.opt)
	LOCAL_CONFIG = -legup-config=config.opt.tcl
endif

# don't unroll the loop
CFLAG += -mllvm -unroll-threshold=0

LEVEL = ../..
include $(LEVEL)/Makefile.common
