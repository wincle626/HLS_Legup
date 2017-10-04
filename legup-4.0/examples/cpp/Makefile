NAME=point
LEVEL=..
# As this benchmark does not have any C source files but has C++ source 
# file(s), the SRCS variable must be set
SRCS=$(NAME).cpp

OVERRIDE=yes
ifeq ($(NO_INLINE),)
	NO_INLINE=1
endif

all:
	# produces pre-link time optimization binary bitcode: $(NAME).prelto.bc
	$(FRONT_END)++ $(NAME).cpp -emit-llvm -c $(CFLAG) -o $(NAME).prelto.1.bc
	# linking may produce llvm mem-family intrinsics
	$(LLVM_HOME)llvm-link $(LDFLAG) $(NAME).prelto.1.bc -o $(NAME).prelto.linked.1.bc
	$(LLVM_HOME)opt -std-link-opts $(NAME).prelto.linked.1.bc -o $(NAME).prelto.linked.bc
	# performs intrinsic lowering so that the linker may be optimized
	$(LLVM_HOME)opt $(OPT_FLAGS) -legup-prelto < $(NAME).prelto.linked.bc > $(NAME).prelto.bc
	# produces $(NAME).bc binary bitcode and a.out shell script: lli $(NAME).bc
	$(LLVM_HOME)llvm-link $(LDFLAG) $(NAME).prelto.bc $(LEVEL)/lib/llvm/liblegup.bc -o $(NAME).1.bc
	$(LLVM_HOME)opt -std-link-opts $(NAME).1.bc -o $(NAME).bc
	# produces textual bitcodes: $(NAME).prelto.1.ll $(NAME).prelto.ll $(NAME).ll
	$(LLVM_HOME)llvm-dis $(NAME).prelto.1.bc
	$(LLVM_HOME)llvm-dis $(NAME).prelto.bc
	$(LLVM_HOME)llvm-dis $(NAME).bc
	# produces verilog: $(NAME).v
	$(LLVM_HOME)llc $(LLC_FLAGS) -march=v $(NAME).bc -o $(VFILE)
include $(LEVEL)/Makefile.common
