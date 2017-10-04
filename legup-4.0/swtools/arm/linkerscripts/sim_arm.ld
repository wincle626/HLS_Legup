ENTRY(__arm_sim_reset)
SECTIONS
{
 . = 0x10000;
 .arm_sim_startup . : { arm_sim_startup.o(.text) } 
 .text : { *(.text) }
 .data : { *(.data) }
 .bss : { *(.bss COMMON) }
 . = ALIGN(8);
 . = . + 0x1000; /* 4kB of stack memory */
 stack_top = .;
}
