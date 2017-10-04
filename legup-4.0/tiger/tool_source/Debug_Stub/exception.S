.text
.align 2
.set noreorder
.set nomacro
.set nobopt
.globl __exception
.extern debug_stub

#include "regdef.h"

__exception:
#determine cause of exception
	mfc0 k0, cause #retrieve cause register
	andi k0, k0, 0x1E #mask off and shift ExcCode
	srl k0, k0, 1
	li k1, 8
	beq k0, k1, syscall_exception #ExcCode = 8 syscall exception
	li k1, 9
	beq k0, k1, break_exception #ExcCode = 9 break exception
	nop
	beq k0, zero, hardware_exception #ExcCode = 0 hardware interrupt
	mfc0 k0, epc #otherwise just jump back without doing anything
	j end
	nop
syscall_exception:
	#jump to debug break giving PC we want to restart at in k0
	#and the reason we're calling the debug stub in k1
	#here we want to restart at the fixed address 0x900
	#and the reason for calling the debug stub is this is
	#the first time
	li k0, 0x9A0
	li k1, 0
	j debug_break
	nop
break_exception:
	#jump to debug break giving PC we want to restart at in k0
	#and the reason we're calling the debug stub in k1
	#here we want to restart at the epc (where the break instruction
	#was located) and the reason for calling the debug stub
	#is we've hit a breakpoint
	mfc0 k0, epc
	li k1, 1
	j debug_break
	nop
hardware_exception:
	mfc0 k0, cause
	andi k0, k0, 0xFC00 #mask off and shift interrupt number
	srl k0, k0, 10
	beq k0, zero, debug_interrupt #irqNumber = 0, debug interrupt
	nop
	mfc0 k0, epc #otherwise just jump back to where interrupt occured
	j end
	nop
debug_interrupt:
	mfc0 k0, epc
	li k1, 2
	j debug_break
	nop
end:
	li k1, 0
	j k0
	mtc0 k1, status
	
debug_break:
	#store all registers on the stack
	addi sp, sp, -136
	sw $1, 0(sp)
	sw $2, 4(sp)
	sw $3, 8(sp)
	sw $4, 12(sp)
	sw $5, 16(sp)
	sw $6, 20(sp)
	sw $7, 24(sp)
	sw $8, 28(sp)
	sw $9, 32(sp)
	sw $10, 36(sp)
	sw $11, 40(sp)
	sw $12, 44(sp)
	sw $13, 48(sp)
	sw $14, 52(sp)
	sw $15, 56(sp)
	sw $16, 60(sp)
	sw $17, 64(sp)
	sw $18, 68(sp)
	sw $19, 72(sp)
	sw $20, 76(sp)
	sw $21, 80(sp)
	sw $22, 84(sp)
	sw $23, 88(sp)
	sw $24, 92(sp)
	sw $25, 96(sp)
	sw $26, 100(sp)
	sw $27, 104(sp)
	sw $28, 108(sp)
	#don't save $29 here as it's the stack pointer
	#so we need to save sp + 136 in order to get the correct
	#sp saved
	sw $30, 116(sp)
	sw $31, 120(sp)
	
	#get lo and hi and store them on the stack
	mflo t0
	mfhi t1
	#need to store sp, however we changed
	#it so we get the original by adding 136
	addi t2, sp, 136
	
	sw t0, 124(sp)
	sw t1, 128(sp)
	sw t2, 112(sp)
	sw k0, 132(sp)
	
	#call debug stub, passing arguments
	#of sp which is a pointer to an array of the register
	#values, k1 which is the reason debug_stub is being
	#called and whether or not we're in a branch delay slot
	move a0, sp
	move a1, k1
	mfc0 a2, cause #retrieve cause register
	srl a2, a2, 31 #get bd (branch delay) bit
	
	jal debug_stub
		
	lw t0, 124(sp)
	lw t1, 128(sp)
	
	mtlo t0
	mthi t1
	
	lw $1, 0(sp)
	lw $2, 4(sp)
	lw $3, 8(sp)
	lw $4, 12(sp)
	lw $5, 16(sp)
	lw $6, 20(sp)
	lw $7, 24(sp)
	lw $8, 28(sp)
	lw $9, 32(sp)
	lw $10, 36(sp)
	lw $11, 40(sp)
	lw $12, 44(sp)
	lw $13, 48(sp)
	lw $14, 52(sp)
	lw $15, 56(sp)
	lw $16, 60(sp)
	lw $17, 64(sp)
	lw $18, 68(sp)
	lw $19, 72(sp)
	lw $20, 76(sp)
	lw $21, 80(sp)
	lw $22, 84(sp)
	lw $23, 88(sp)
	lw $24, 92(sp)
	lw $25, 96(sp)
	lw $26, 100(sp)
	lw $27, 104(sp)
	lw $28, 108(sp)

	lw $30, 116(sp)
	lw $31, 120(sp)
	
	lw k0, 132(sp)
	
	addi sp, sp, 136
	
	j end
	nop