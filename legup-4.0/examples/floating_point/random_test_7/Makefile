NAME=random_test_7
LEVEL = ../..
# As this benchmark does not have any C source files but has LLVM source 
# file(s), the SRCS variable must be set
SRCS=$(NAME).llvm

# override all target
OVERRIDE=yes
all:
	$(LLVM_HOME)llvm-as $(NAME).llvm
	$(LLVM_HOME)llc $(LLC_FLAGS) -march=v $(NAME).llvm.bc -o $(VFILE)

include $(LEVEL)/Makefile.common
