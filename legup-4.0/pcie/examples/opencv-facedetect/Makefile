
NAME=facedectFPGA
LEVEL = ../../../examples
include $(LEVEL)/Makefile.common

all: pcieLibrary

libfacedectFPGA.so: facedectFPGA.c
	$(CC) --std=c99 -g -Wall -fPIC -c facedectFPGA.c -o facedectFPGA.o
	$(LD) -shared -soname $@ -o $@ facedectFPGA.o
