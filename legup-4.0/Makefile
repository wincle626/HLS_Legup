# run 'make' after a git pull or git clone

VERSION = 4.0

# if your libraries are installed locally (scinet)
ifdef USE_SCINET
	CLOOG_LIBS = --with-gmp-prefix=/home/j/janders/jchoi/libgmp
	LLVM_LIBS = --with-lpsolve-include=/home/j/janders/jchoi/legup_binaries --with-lpsolve-lib=/home/j/janders/jchoi/legup_binaries --with-gmp=/home/j/janders/jchoi/libgmp	
endif

export CC = gcc
export CXX = g++

# Dragonegg should work with gcc-4.5 to gcc-4.8
#DRAGONEGG_GCC_VERSION = gcc-4.7
DRAGONEGG_GCC_VERSION = gcc-4.8
# NOTE: if you change the GCC version here, you will also need to change the
# DRAGONEGG variable in examples/Makefile.config. The gcc version used there
# needs to match the version passed to dragonegg here.
# Also make sure to install gcc-4.X-plugin-dev to match this gcc version

ifndef LLVM_BUILD
	LLVM_BUILD = Release+Asserts
endif

# Uncomment to enable gdb debugging
#DEBUG_MODE = --disable-optimized
# Also change your LLVM_BUILD environment variable:
#export LLVM_BUILD=Debug+Asserts

all: llvm/tools/polly/Makefile.config llvm/Makefile.config llvm-2.9_mips/Makefile.config
	$(MAKE) all -C swtools/binutils
	# $(MAKE) -C mips-binutils
	$(MAKE) -C llvm
	$(MAKE) -C llvm-2.9_mips tools-only ONLY_TOOLS=llc
	$(MAKE) -C dragonegg GCC=$(DRAGONEGG_GCC_VERSION) LLVM_CONFIG=../llvm/$(LLVM_BUILD)/bin/llvm-config
	$(MAKE) -C examples/lib/llvm
	$(MAKE) all -C swtools/lib-source
	$(MAKE) -C tiger/processor
	$(MAKE) -C tiger/linux_tools
	$(MAKE) -C tiger/tool_source/profiling_tools
	$(MAKE) -C pcie
	$(MAKE) -C gui
	$(MAKE) -C ip
	if [ -d .git ]; then \
		cd ./.git/hooks/ ; ln -sf ../../hooks/* ./ ; \
	fi

llvm/Makefile.config: cloog/install/include/isl
	cd llvm && ./configure --with-cloog=$(PWD)/cloog/install --with-isl=$(PWD)/cloog/install $(LLVM_LIBS) $(DEBUG_MODE)

llvm-2.9_mips/Makefile.config:
	cd llvm-2.9_mips && ./configure $(DEBUG_MODE) --enable-assertions

llvm/tools/polly/Makefile.config: llvm/Makefile.config

cloog/install/include/isl: cloog/isl/Makefile
	$(MAKE) -C cloog
	$(MAKE) install -C cloog

cloog/isl/Makefile:
	mkdir -p cloog/install
	cd cloog && ./autogen.sh
	cd cloog && ./configure --prefix=$(PWD)/cloog/install $(CLOOG_LIBS)

clean:
	-$(MAKE) clean -C cloog
	-$(MAKE) clean -C swtools/binutils
	# -$(MAKE) clean -C mips-binutils
	-$(MAKE) clean -C llvm
	-$(MAKE) clean -C llvm-2.9_mips
	-$(MAKE) clean -C tiger/hybrid/processor
	-$(MAKE) clean -C tiger/processor
	-$(MAKE) clean -C tiger/linux_tools
	-$(MAKE) clean -C examples/lib/llvm
	-$(MAKE) clean -C tiger/tool_source/profiling_tools
	-$(MAKE) clean -C pcie
	-$(MAKE) clean -C gui
	-$(MAKE) clean -C ip
	rm -rf cloog/isl/Makefile
	rm -rf llvm-2.9_mips/Makefile.config
	rm -rf llvm/Makefile.config
	rm -rf llvm/tools/polly/Makefile.config

release:
	git archive --format=tar --prefix=legup-$(VERSION)/ HEAD | gzip > legup-$(VERSION).tar.gz

.PHONY = all

