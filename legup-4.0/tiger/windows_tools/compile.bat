@ECHO OFF

if (%1) == () echo "Usage: compile <filename.c>" && goto eof

@path=tiger_tools
gcc -I ../tool_source/lib/ -O2 -Wall -G0 -EL -c %1 -o %~n1.o
ld %~n1.o -T lib/prog_link.ld -e main -EL -o %~n1.elf -L lib -lgcc -lfloat -luart
objdump -D %~n1.elf > %~n1.src
