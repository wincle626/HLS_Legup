NAME=simplestruct
LEVEL = ..
# As this benchmark does not have any C source files but has LLVM source 
# file(s), the SRCS variable must be set
SRCS=$(NAME).llvm

# override all target
OVERRIDE=yes
all:
	$(LLVM_HOME)llvm-as $(NAME).llvm
	# produce a.out executable
	$(LLVM_HOME)llvm-link $(NAME).llvm.bc -o $(NAME).1.bc
	$(LLVM_HOME)opt -std-link-opts $(NAME).1.bc -o $(NAME).bc
	$(LLVM_HOME)llc $(LLC_FLAGS) -march=v $(NAME).llvm.bc -o $(VFILE)

include $(LEVEL)/Makefile.common
