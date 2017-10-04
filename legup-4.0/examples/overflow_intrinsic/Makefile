NAME=overflow_intrinsic
LEVEL = ..
# As this benchmark does not have any C source files but has LLVM source 
# file(s), the SRCS variable must be set
SRCS=$(NAME).llvm

# override all target
OVERRIDE=yes
all:
	$(LLVM_HOME)llvm-as $(NAME).llvm
	$(LLVM_HOME)opt $(OPT_FLAGS) -legup-prelto < $(NAME).llvm.bc > $(NAME).final.bc
	$(LLVM_HOME)llc $(LLC_FLAGS) -march=v $(NAME).final.bc -o $(VFILE)

include $(LEVEL)/Makefile.common
