LLVM_HOME = ../../../llvm/Release/bin/
MIPS_PREFIX = ../../../swtools/binutils/mipsel-elf/bin/
MIPS_GCC = mipsel-unknown-elf-gcc
all:
	# for running on Tiger
	#llvm-gcc uart.c -emit-llvm -c -o uart.bc 
	#$(LLVM_HOME)llc uart.bc -march=mipsel -relocation-model=static -mips-ssection-threshold=0 -mcpu=mips1 -o uart.s
	#$(MIPS_PREFIX)as uart.s -mips1 -mabi=32 -o uart.o -EL
	$(MIPS_GCC) -O3 uart.c -c -o uart.o
	#$(MIPS_GCC) -O3 altera_avalon_performance_counter.c -c
	$(MIPS_GCC) -O3 performance_counter.c -c
	$(MIPS_PREFIX)ar rvs libuart.a uart.o
	cp libuart.a ../../windows_tools/lib/
	mv libuart.a ../../linux_tools/lib/
	$(MIPS_GCC) mem.c -c
	$(MIPS_PREFIX)ar rvs libmem.a mem.o
	cp libmem.a ../../windows_tools/lib/
	mv libmem.a ../../linux_tools/lib/

	$(MIPS_GCC) -O0 ../Debug_Stub/utils.S -c -o utils.o

	# for simulation
	#llvm-gcc -D SIM uart.c -emit-llvm -c -o uart.bc 
	#$(LLVM_HOME)llc uart.bc -march=mipsel -relocation-model=static -mips-ssection-threshold=0 -mcpu=mips1 -o uart.s
	#$(MIPS_PREFIX)as uart.s -mips1 -mabi=32 -o uart.o -EL
	$(MIPS_GCC) -O3 -D SIM uart.c -c
	#$(MIPS_GCC) -O3 -D SIM altera_avalon_performance_counter.c -c
	$(MIPS_GCC) -O3 -D SIM performance_counter.c -c
	$(MIPS_PREFIX)ar rvs libuart_el_sim.a uart.o
	cp libuart_el_sim.a ../../windows_tools/lib/
	mv libuart_el_sim.a ../../linux_tools/lib/
	$(MIPS_GCC) -D SIM mem.c -c
	$(MIPS_PREFIX)ar rvs libmem_el_sim.a mem.o
	cp libmem_el_sim.a ../../windows_tools/lib/
	mv libmem_el_sim.a ../../linux_tools/lib/
