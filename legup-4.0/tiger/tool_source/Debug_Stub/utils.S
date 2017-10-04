.text
.align 2
.set noreorder
.set nobopt
.set nomacro

#include "regdef.h"

.globl flushICache
	
	#Hardware interrupts must be disabled for this to work correctly
	#(if they are enabled it's possible for the flush to not happen if
	#an interrupt occurs at the wrong moment).
flushICache:
	#flush instruction cache by writing 1 to bit 0 of CP0 register $3
	li t0, 3
	mtc0 t0, $3
	#The effect of the next 5 instructions in sequence after a flush
	#is undefined, so we put 5 nops
	nop
	nop
	nop
	nop
	nop
	
	jr ra
	nop