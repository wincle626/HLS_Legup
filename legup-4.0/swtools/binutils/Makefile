PREFIX = ${shell echo $$PWD}
BINUTILS_PKG = binutils-2.24.tar.bz2
BINUTILS = binutils-2.24

# all: arm-altera-eabi mips-elf
all: bin/mipsel-elf-as bin/arm-altera-eabi-as
	rm -rf $(BINUTILS)

bin/arm-altera-eabi-as: binutils-2.24.tar.bz2
	make arm-altera-eabi

arm-altera-eabi: untar
	# Make binutils for ARM
	mkdir -p arm-build-binutils
	cd arm-build-binutils && ../$(BINUTILS)/configure --target=arm-altera-eabi --prefix=$(PREFIX) CFLAGS="-g -O2 -Wno-error"
	cd arm-build-binutils && $(MAKE) all && $(MAKE) install
	rm -rf arm-build-binutils

bin/mipsel-elf-as: binutils-2.24.tar.bz2
	make mips-elf

mips-elf: untar
	# Make binutils for MIPS
	mkdir -p mips-build-binutils
	cd mips-build-binutils && ../$(BINUTILS)/configure --target=mipsel-elf --prefix=$(PREFIX) CFLAGS="-g -O2 -Wno-error"
	cd mips-build-binutils && $(MAKE) all && $(MAKE) install
	rm -rf mips-build-binutils

untar:
	tar xjfv $(BINUTILS_PKG)

clean:
	rm -rf arm-build-binutils mips-build-binutils $(BINUTILS) bin arm-altera-eabi mipsel-elf share

.NOTPARALLEL = all untar clean bin/arm-altera-eabi-as arm-altera-eabi bin/mipsel-elf-as mips-elf
