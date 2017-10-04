
# Include Makefile.config to get OPT_FLAGS, CFLAGS, LDFLAGS, LLVM_HOME, ARM_PREFIX, etc.
LEVEL = ../../examples
include $(LEVEL)/Makefile.config



################################################################################
# Common Setup
################################################################################

# Common flags
LD_EXTRA = $(LEVEL)/lib/llvm/liblegup.bc $(LEVEL)/lib/llvm/libm.bc
AR_FLAG = rs

# Targets to build
# Every supported architecture should have these
COMMON_TARGET_NAMES =	\
	libuart.a		\
	libsimuart.a	\
	libmem.a

OUTPUT_DIRS		=
TARGETS			=
CLEAN_TARGETS	=


################################################################################
# ARM-Specific Setup
################################################################################

# Where to put compiled object files
ARM_LIB_DIR = ../arm/lib

# ARM-specific flags for assembler and llvm compiler
ARM_AS_FLAG = -mfloat-abi=soft -march=armv7-a -mcpu=cortex-a9 -EL --gstabs
ARM_LLC_FLAG = -march=arm -relocation-model=static -mcpu=cortex-a9

ARM_TARGET_NAMES =	\
	libprof.a		\
	arm_startup.o	\
	arm_sim_startup.o

# Targets with the appropriate directory prepended
ARM_TARGETS		 = $(addprefix $(ARM_LIB_DIR)/,  $(COMMON_TARGET_NAMES))
ARM_TARGETS		+= $(addprefix $(ARM_LIB_DIR)/,  $(ARM_TARGET_NAMES))

OUTPUT_DIRS		+= $(ARM_LIB_DIR)
TARGETS			+= $(ARM_TARGETS)
CLEAN_TARGETS	+= clean_arm



################################################################################
# MIPS-Specific Setup
################################################################################

# Where to put compiled object files
MIPS_LIB_DIR= ../mips/lib

# MIPS-specific flags for assembler and llvm compiler
MIPS_AS_FLAG = -mips1 -mabi=32 -EL
MIPS_LLC_FLAG = -march=mipsel -relocation-model=static -mcpu=mips1 -mips-ssection-threshold=0

MIPS_TARGET_NAMES = \
	mips_startup.o

# Targets with the appropriate directory prepended
MIPS_TARGETS	 = $(addprefix $(MIPS_LIB_DIR)/, $(COMMON_TARGET_NAMES))
MIPS_TARGETS	+= $(addprefix $(MIPS_LIB_DIR)/, $(MIPS_TARGET_NAMES))

OUTPUT_DIRS		+= $(MIPS_LIB_DIR)
TARGETS			+= $(MIPS_TARGETS)
CLEAN_TARGETS	+= clean_mips









################################################################################
# Common Targets
################################################################################

# Make the targets
all: make_lib_dirs $(TARGETS)

make_lib_dirs:
	mkdir -p $(OUTPUT_DIRS)

# Clean targets
clean: $(CLEAN_TARGETS)
	rm -f *.bc *.ll *.out *.o

cleanall: clean
	rm -f $(TARGETS)

.PHONY: clean cleanall all make_lib_dirs $(CLEAN_TARGETS)



################################################################################
# ARM-Specific Targets
################################################################################

# Turn object files into archives
$(ARM_LIB_DIR)/lib%.a: %.arm.o
	#
	# Creating archive $@:
	$(ARM_PREFIX)ar $(AR_FLAG) $@ $<
	@echo

# Target for compiling .c files into .o files
%.arm.o: %.c
	#
	# Compiling object file $@:
	$(FRONT_END) $< -emit-llvm -c -DARM -O3 $(CFLAG) $(CLANG_FLAG) -o $*.arm.1.bc
	$(LLVM_HOME)opt $(OPT_FLAGS) -legup-prelto $*.arm.1.bc -o $*.arm.bc
	$(LLVM_HOME)llvm-dis $*.arm.bc
	$(LLVM_HOME)llc $*.arm.bc $(ARM_LLC_FLAG) -o $*.arm.s
	$(ARM_PREFIX)as $*.arm.s $(ARM_AS_FLAG) -o $@

# Target for copying .o files to LIB directory
$(ARM_LIB_DIR)/%.o : %.arm.o
	#
	# Copying object file $@ to $^:
	cp $^ $@

# Target for compiling .s files into .o files
%.arm.o : %.s
	@#echo && echo $@:
	#
	# Assembling object file $@:
	$(ARM_PREFIX)as $< $(ARM_AS_FLAG) -o $@
	@echo

clean_arm:
	rm -f *.arm.s


################################################################################
# MIPS-Specific Targets
################################################################################

# Turn object files into archives
$(MIPS_LIB_DIR)/lib%.a: %.mips.o
	#
	# Creating archive $@:
	$(MIPS_PREFIX)ar $(AR_FLAG) $@ $<
	@echo

# Target for compiling .c files into .o files
%.mips.o: %.c
	#
	# Compiling object file $@:
	$(FRONT_END) $< -emit-llvm -c -DMIPS -O3 $(CFLAG) $(CLANG_FLAG) -o $*.mips.1.bc
	$(LLVM_HOME)opt $(OPT_FLAGS) -legup-prelto $*.mips.1.bc -o $*.mips.bc
	$(LLVM_HOME)llvm-dis $*.mips.bc
	$(LEVEL)/remove_attributes.sh $*.mips.ll
	$(MIPS_LLC_DIR)llc $*.mips.ll $(MIPS_LLC_FLAG) -o $*.mips.s
	$(MIPS_PREFIX)as $*.mips.s $(MIPS_AS_FLAG) -o $@

# Target for copying .o files to LIB directory
$(MIPS_LIB_DIR)/%.o : %.mips.o
	#
	# Copying object file $@ to $^:
	cp $^ $@

# Target for compiling .s files into .o files
%.mips.o : %.s
	@#echo && echo $@:
	#
	# Assembling object file $@:
	$(MIPS_PREFIX)as $< $(MIPS_AS_FLAG) -o $@
	@echo

clean_mips:
	rm -f *.mips.s


################################################################################
# EOF
################################################################################
