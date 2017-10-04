
    .section _mips_startup_section
    .global mips_startup
    .align 2
    .type main,@function
mips_startup:
    lui     $sp, %hi(__stack_top)
/*
NOTE:
Tiger simulation halts when the program counter is RESET_ADDRESS + 0x0C. Hence,
if we want to add more instructions here before 'jal main' we need to change the
simulation so it halts at RESET_ADDRESS + 0x10.  This address is currently set
in tiger_top.v.
NOTE:
The following instruction will be executed in the 'jal main' delay slot. i.e. it
will look out of order in the disassembly, but will still execute in this order.
*/
    addiu   $sp, $sp, %lo(__stack_top)
    jal     main
__end_loop:
/*
NOTE:
The LEAP profiler assumes that the startup code will be 32 bytes.  Pad with nops
to make this long enough. Ideally the QSys component parameter STARTING_PC
should be updated automatically to match the length of the startup code.  See
commit message for this change for more info.
*/
    nop
    nop
    nop
    nop
    b       __end_loop

