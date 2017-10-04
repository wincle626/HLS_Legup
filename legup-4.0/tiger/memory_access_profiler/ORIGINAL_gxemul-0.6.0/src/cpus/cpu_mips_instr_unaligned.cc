/*
 *  Copyright (C) 2006-2009  Anders Gavare.  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 *  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 *  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *  SUCH DAMAGE.
 *
 *
 *  MIPS unaligned load/store instructions; the following args are used:
 *  
 *  arg[0] = pointer to the register to load to or store from
 *  arg[1] = pointer to the base register
 *  arg[2] = offset (as an int32_t)
 *
 *  NOTE/TODO: This is a very slow generic implementation, from the
 *  pre-Dyntrans emulation mode. It should be rewritten.
 */

#include "cop0.h"
#include "cpu.h"
#include "memory.h"
#include "misc.h"


void mips_unaligned_loadstore(struct cpu *cpu, struct mips_instr_call *ic,
	int is_left, int wlen, int store)
{
	/*  For L (Left):   address is the most significant byte  */
	/*  For R (Right):  address is the least significant byte  */
	uint64_t addr = *((uint64_t *)ic->arg[1]) + (int32_t)ic->arg[2];
	int i, dir, reg_dir, reg_ofs, ok;
	uint64_t result_value, tmpaddr;
	uint64_t aligned_addr = addr & ~(wlen-1);
	unsigned char aligned_word[8], databyte;

	int low_pc = ((size_t)ic - (size_t)cpu->cd.mips.cur_ic_page)
	    / sizeof(struct mips_instr_call);
	cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)
	    << MIPS_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << MIPS_INSTR_ALIGNMENT_SHIFT);

	dir = 1;		/*  big endian, Left  */
	reg_dir = -1;
	reg_ofs = wlen - 1;	/*  byte offset in the register  */
	if (!is_left) {
		dir = -dir;
		reg_ofs = 0;
		reg_dir = 1;
	}

	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		dir = -dir;

	result_value = *((uint64_t *)ic->arg[0]);

	if (cpu->is_32bit) {
		result_value = (int32_t)result_value;
		aligned_addr = (int32_t)aligned_addr;
		addr = (int32_t)addr;
	}

	if (store) {
		/*  Store:  */
		uint64_t oldpc = cpu->pc;

		/*
		 *  NOTE (this is ugly): The memory_rw()
		 *  call generates a TLBL exception, if there
		 *  is a tlb refill exception. However, since
		 *  this is a Store, the exception is converted
		 *  to a TLBS:
		 */
		ok = cpu->memory_rw(cpu, cpu->mem, aligned_addr,
		    &aligned_word[0], wlen, MEM_READ, CACHE_DATA);
		if (!ok) {
			if (cpu->pc != oldpc) {
				cpu->cd.mips.coproc[0]->reg[COP0_CAUSE] &=
				    ~CAUSE_EXCCODE_MASK;
				cpu->cd.mips.coproc[0]->reg[COP0_CAUSE] |=
				    (EXCEPTION_TLBS << CAUSE_EXCCODE_SHIFT);
			}
			return;
		}

		for (i=0; i<wlen; i++) {
			tmpaddr = addr + i*dir;
			/*  Have we moved into another word/dword? Then stop: */
			if ( (tmpaddr & ~(wlen-1)) != (addr & ~(wlen-1)) )
				break;

			/*  debug("unaligned byte at %016"PRIx64",
			    reg_ofs=%i reg=0x%016"PRIx64"\n",
			    tmpaddr, reg_ofs, (long long)result_value);  */

			/*  Store one byte:  */
			aligned_word[tmpaddr & (wlen-1)] =
			    (result_value >> (reg_ofs * 8)) & 255;

			reg_ofs += reg_dir;
		}

		ok = cpu->memory_rw(cpu, cpu->mem,
		    aligned_addr, &aligned_word[0], wlen,
		    MEM_WRITE, CACHE_DATA);

		return;
	}

	/*  Load:  */
	ok = cpu->memory_rw(cpu, cpu->mem, aligned_addr, &aligned_word[0], wlen,
	    MEM_READ, CACHE_DATA);
	if (!ok)
		return;

	for (i=0; i<wlen; i++) {
		tmpaddr = addr + i*dir;
		/*  Have we moved into another word/dword? Then stop: */
		if ( (tmpaddr & ~(wlen-1)) != (addr & ~(wlen-1)) )
			break;

		/*  debug("unaligned byte at %016"PRIx64", reg_ofs=%i reg="
		    "0x%016"PRIx64"\n", (uint64_t) tmpaddr,
		    reg_ofs, (uint64_t)result_value); */

		/*  Load one byte:  */
		databyte = aligned_word[tmpaddr & (wlen-1)];
		result_value &= ~((uint64_t)0xff << (reg_ofs * 8));
		result_value |= (uint64_t)databyte << (reg_ofs * 8);

		reg_ofs += reg_dir;
	}

	/*  Sign extend for 32-bit load lefts:  */
	if (!store && wlen == sizeof(uint32_t))
		result_value = (int32_t)result_value;

	(*(uint64_t *)ic->arg[0]) = result_value;
}

