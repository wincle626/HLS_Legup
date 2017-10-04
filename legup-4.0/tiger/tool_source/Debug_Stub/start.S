.text
.align 2
.set noreorder
.set nobopt
.set nomacro

#include "regdef.h"

start:
	li sp, 0x01000000
	jal init
	nop
	syscall
start.end:
	j start.end
	nop
