@ECHO OFF

del *.o
del ROM.*

path=../../windows_tools/tiger_tools
gcc -c start.S -o start.o -EL
gcc -c exception.S -o exception.o -EL
gcc -c utils.S -o utils.o -EL
gcc -c uart_io.c -o uart_io.o -EL -O2 -nostartfiles -nostdlib -nodefaultlibs
gcc -c init.c -o init.o -EL -O2 -nostartfiles -nostdlib -nodefaultlibs
gcc -c debug_stub.c -o debug_stub.o -EL -G 0 -O2 -nostartfiles -nostdlib -nodefaultlibs

ld init.o utils.o uart_io.o debug_stub.o -Tlink.ld -EL -o ROM.bin --oformat=binary
ld init.o utils.o uart_io.o debug_stub.o -Tlink.ld -EL -o ROM.elf

objdump -xsd ROM.elf > rom.lst

MifMaker ROM.bin ROM.mif