/*
 *  Copyright (C) 2007-2009  Anders Gavare.  All rights reserved.
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
 *  M88K load/store instructions; the following args are used:
 *  
 *  arg[0] = pointer to the register to load to or store from (d)
 *  arg[1] = pointer to the base register (s1)
 *  arg[2] = pointer to the offset register (s2), or an uint32_t offset
 *
 *  The GENERIC function always checks for alignment, and supports both big
 *  and little endian byte order.
 *
 *  The quick function is included twice (big/little endian) for each
 *  GENERIC function.
 *
 *
 *  Defines:
 *	LS_LOAD or LS_STORE (only one)
 *	LS_INCLUDE_GENERIC (to generate the generic function)
 *	LS_GENERIC_N is defined as the name of the generic function
 *	LS_N is defined as the name of the fast function
 *	LS_1, LS_2, LS_4, or LS_8 (only one)
 *	LS_SIZE is defined to 1, 2, 4, or 8
 *	LS_SIGNED is defined for signed loads
 *	LS_LE or LS_BE (only one)
 *	LS_SCALED for scaled accesses
 *	LS_USR for usr accesses
 *	LS_REGOFS is defined when arg[2] is a register pointer
 */


#ifdef LS_INCLUDE_GENERIC
void LS_GENERIC_N(struct cpu *cpu, struct m88k_instr_call *ic)
{
#ifdef LS_USR
	const int memory_rw_flags = CACHE_DATA | MEMORY_USER_ACCESS;
#else
	const int memory_rw_flags = CACHE_DATA;
#endif
	uint32_t addr = reg(ic->arg[1]) +
#ifdef LS_REGOFS
#ifdef LS_SCALED
	    LS_SIZE *
#endif
	    reg(ic->arg[2]);
#else
	    ic->arg[2];
#endif
	uint8_t data[LS_SIZE];
	uint64_t x;

	/*  Synchronize the PC:  */
	int low_pc = ((size_t)ic - (size_t)cpu->cd.m88k.cur_ic_page)
	    / sizeof(struct m88k_instr_call);
	cpu->pc &= ~((M88K_IC_ENTRIES_PER_PAGE-1)<<M88K_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << M88K_INSTR_ALIGNMENT_SHIFT);

	/*
	 *  Update the memory transaction registers:
	 */
	cpu->cd.m88k.dmt[1] = 0;

	cpu->cd.m88k.dmt[0] = DMT_VALID;
#ifdef LS_STORE
	cpu->cd.m88k.dmt[0] |= DMT_WRITE;
#else
	{
		int dreg = (((uint32_t *)ic->arg[0]) - &cpu->cd.m88k.r[0]);
		if (dreg < 1 || dreg > 31) {
			fatal("HUH? dreg = %i in cpu_m88k_instr_loadstore.c."
			    " Internal error.\n", dreg);
			exit(1);
		}
		cpu->cd.m88k.dmt[0] |= dreg << DMT_DREGSHIFT;
	}
#ifdef LS_SIGNED
	cpu->cd.m88k.dmt[0] |= DMT_SIGNED;
#endif
#endif
	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		cpu->cd.m88k.dmt[0] |= DMT_BO;

#ifndef LS_USR
	if (cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_MODE)
		cpu->cd.m88k.dmt[0] |= DMT_DAS;		/*  supervisor  */
#endif

	/*  EN bits:  */
#ifdef LS_1
	/*  TODO: Is the EN offset only valid for Big-Endian?  */
	cpu->cd.m88k.dmt[0] |= 1 << DMT_ENSHIFT << (3 - (addr & 3));
#else
#ifdef LS_2
	cpu->cd.m88k.dmt[0] |= 3 << DMT_ENSHIFT << (2 - (addr & 2));
#else
	cpu->cd.m88k.dmt[0] |= 0xf << DMT_ENSHIFT;
#endif
#endif

	cpu->cd.m88k.dma[0] = addr & ~0x3;
	cpu->cd.m88k.dmd[0] = 0;

#ifdef LS_8
	cpu->cd.m88k.dmt[1] = cpu->cd.m88k.dmt[0];
	cpu->cd.m88k.dmt[0] |= DMT_DOUB1;
	cpu->cd.m88k.dma[1] = cpu->cd.m88k.dma[0] + sizeof(uint32_t);
	cpu->cd.m88k.dmd[1] = 0;
#ifdef LS_LOAD
	{
		int dreg = (((uint32_t *)ic->arg[0]) - &cpu->cd.m88k.r[0]);
		dreg ++;
		if (dreg < 1 || dreg > 31) {
			fatal("HUH? dreg = %i in cpu_m88k_instr_loadstore.c."
			    " Internal error.\n", dreg);
			exit(1);
		}
		cpu->cd.m88k.dmt[1] &= ~((0x1f) << DMT_DREGSHIFT);
		cpu->cd.m88k.dmt[1] |= dreg << DMT_DREGSHIFT;
	}
#endif
#endif


#ifdef LS_USR
	if (!(cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_MODE)) {
		/*  Cause a privilege violation exception:  */
		m88k_exception(cpu, M88K_EXCEPTION_PRIVILEGE_VIOLATION, 0);
		return;
	}
#endif

#ifndef LS_1
	/*  Check alignment:  */
	if (addr & (LS_SIZE - 1)) {
#if 0
		/*  Cause an address alignment exception:  */
		m88k_exception(cpu, M88K_EXCEPTION_MISALIGNED_ACCESS, 0);
#else
		fatal("{ m88k dyntrans alignment exception, size = %i,"
		    " addr = %08"PRIx32", pc = %08"PRIx32" }\n", LS_SIZE,
		    (uint32_t) addr, (uint32_t) cpu->pc);

		/*  TODO: Generalize this into a abort_call, or similar:  */
		cpu->running = 0;
		debugger_n_steps_left_before_interaction = 0;
		cpu->cd.m88k.next_ic = &nothing_call;

		if (cpu->delay_slot)
			cpu->delay_slot |= EXCEPTION_IN_DELAY_SLOT;
#endif
		return;
	}
#endif

#ifdef LS_LOAD
	if (!cpu->memory_rw(cpu, cpu->mem, addr, data, sizeof(data),
	    MEM_READ, memory_rw_flags)) {
		/*  Exception.  */
		return;
	}
	x = memory_readmax64(cpu, data, LS_SIZE);
#ifdef LS_8
	if (cpu->byte_order == EMUL_BIG_ENDIAN) {
		reg(ic->arg[0]) = x >> 32;
		reg(ic->arg[0] + 4) = x;
	} else {
		reg(ic->arg[0]) = x;
		reg(ic->arg[0] + 4) = x >> 32;
	}
#else
#ifdef LS_SIGNED
#ifdef LS_1
	x = (int8_t)x;
#endif
#ifdef LS_2
	x = (int16_t)x;
#endif
#ifdef LS_4
	x = (int32_t)x;
#endif
#endif
	reg(ic->arg[0]) = x;
#endif

#else	/*  LS_STORE:  */

#ifdef LS_8
	if (cpu->byte_order == EMUL_BIG_ENDIAN)
		x = ((uint64_t)reg(ic->arg[0]) << 32) + reg(ic->arg[0] + 4);
	else
		x = ((uint64_t)reg(ic->arg[0] + 4) << 32) + reg(ic->arg[0]);
	cpu->cd.m88k.dmd[0] = reg(ic->arg[0]);
	cpu->cd.m88k.dmd[1] = reg(ic->arg[0] + 4);
#else
	x = reg(ic->arg[0]);
	cpu->cd.m88k.dmd[0] = x;
#endif
	memory_writemax64(cpu, data, LS_SIZE, x);
	if (!cpu->memory_rw(cpu, cpu->mem, addr, data, sizeof(data),
	    MEM_WRITE, memory_rw_flags)) {
		/*  Exception.  */
		return;
	}
#endif
}
#endif	/*  LS_INCLUDE_GENERIC  */


void LS_N(struct cpu *cpu, struct m88k_instr_call *ic)
{
	uint32_t addr = reg(ic->arg[1]) +
#ifdef LS_REGOFS
#ifdef LS_SCALED
	    LS_SIZE *
#endif
	    reg(ic->arg[2]);
#else
	    ic->arg[2];
#endif

#ifdef LS_USR
#ifdef LS_LOAD
	uint8_t *p = cpu->cd.m88k.host_load_usr[addr >> 12];
#else
	uint8_t *p = cpu->cd.m88k.host_store_usr[addr >> 12];
#endif
#else
#ifdef LS_LOAD
	uint8_t *p = cpu->cd.m88k.host_load[addr >> 12];
#else
	uint8_t *p = cpu->cd.m88k.host_store[addr >> 12];
#endif
#endif

	/*
	 *  Call the generic function, if things become too complicated:
	 *
	 *  1)  .usr used in non-supervisor mode
	 *  2)  the page pointer is NULL
	 *  3)  unaligned access
	 */
	if (
#ifdef LS_USR
	    !(cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_MODE) ||
#endif

	    p == NULL
#ifndef LS_1
	    || addr & (LS_SIZE - 1)
#endif
	    ) {
		LS_GENERIC_N(cpu, ic);
		return;
	}

	addr &= 0xfff;

#ifdef LS_LOAD
	/*  Load:  */

#ifdef LS_1
	reg(ic->arg[0]) =
#ifdef LS_SIGNED
	    (int8_t)
#endif
	    p[addr];
#endif	/*  LS_1  */

#ifdef LS_2
	reg(ic->arg[0]) =
#ifdef LS_SIGNED
	    (int16_t)
#endif
#ifdef LS_BE
#ifdef HOST_BIG_ENDIAN
	    ( *(uint16_t *)(p + addr) );
#else
	    ((p[addr]<<8) + p[addr+1]);
#endif
#else
#ifdef HOST_LITTLE_ENDIAN
	    ( *(uint16_t *)(p + addr) );
#else
	    (p[addr] + (p[addr+1]<<8));
#endif
#endif
#endif	/*  LS_2  */

#ifdef LS_4
	reg(ic->arg[0]) =
#ifdef LS_SIGNED
	    (int32_t)
#else
	    (uint32_t)
#endif
#ifdef LS_BE
#ifdef HOST_BIG_ENDIAN
	    ( *(uint32_t *)(p + addr) );
#else
	    ((p[addr]<<24) + (p[addr+1]<<16) + (p[addr+2]<<8) + p[addr+3]);
#endif
#else
#ifdef HOST_LITTLE_ENDIAN
	    ( *(uint32_t *)(p + addr) );
#else
	    (p[addr] + (p[addr+1]<<8) + (p[addr+2]<<16) + (p[addr+3]<<24));
#endif
#endif
#endif	/*  LS_4  */

#ifdef LS_8

	/*  Load first word in pair:  */
	reg(ic->arg[0]) =
#ifdef LS_BE
#ifdef HOST_BIG_ENDIAN
	    ( *(uint32_t *)(p + addr) );
#else
	    ((p[addr]<<24) + (p[addr+1]<<16) + (p[addr+2]<<8) + p[addr+3]);
#endif
#else
#ifdef HOST_LITTLE_ENDIAN
	    ( *(uint32_t *)(p + addr + 4) );
#else
	    (p[addr+4] + (p[addr+5]<<8) + (p[addr+6]<<16) + (p[addr+7]<<24));
#endif
#endif

	/*  Load second word in pair:  */
	reg(ic->arg[0] + 4) =
#ifdef LS_BE
#ifdef HOST_BIG_ENDIAN
	    ( *(uint32_t *)(p + addr + 4) );
#else
	    ((p[addr+4]<<24) + (p[addr+5]<<16) + (p[addr+6]<<8) + p[addr+7]);
#endif
#else
#ifdef HOST_LITTLE_ENDIAN
	    ( *(uint32_t *)(p + addr) );
#else
	    (p[addr] + (p[addr+1]<<8) + (p[addr+2]<<16) + (p[addr+3]<<24));
#endif
#endif

#endif	/*  LS_8  */

#else
	/*  Store: */

#ifdef LS_1
	p[addr] = reg(ic->arg[0]);
#endif
#ifdef LS_2
	{ uint32_t x = reg(ic->arg[0]);
#ifdef LS_BE
#ifdef HOST_BIG_ENDIAN
	*((uint16_t *)(p+addr)) = x; }
#else
	p[addr] = x >> 8; p[addr+1] = x; }
#endif
#else
#ifdef HOST_LITTLE_ENDIAN
	*((uint16_t *)(p+addr)) = x; }
#else
	p[addr] = x; p[addr+1] = x >> 8; }
#endif
#endif
#endif  /*  LS_2  */
#ifdef LS_4
	{ uint32_t x = reg(ic->arg[0]);
#ifdef LS_BE
#ifdef HOST_BIG_ENDIAN
	*((uint32_t *)(p+addr)) = x; }
#else
	p[addr] = x >> 24; p[addr+1] = x >> 16; 
	p[addr+2] = x >> 8; p[addr+3] = x; }
#endif
#else
#ifdef HOST_LITTLE_ENDIAN
	*((uint32_t *)(p+addr)) = x; }
#else
	p[addr] = x; p[addr+1] = x >> 8; 
	p[addr+2] = x >> 16; p[addr+3] = x >> 24; }
#endif
#endif
#endif  /*  LS_4  */
#ifdef LS_8

	/*  First word in pair:  */
	{ uint32_t x = reg(ic->arg[0]);
#ifdef LS_BE
#ifdef HOST_BIG_ENDIAN
	*((uint32_t *)(p+addr)) = x; }
#else
	p[addr]   = x >> 24; p[addr+1] = x >> 16;
	p[addr+2] = x >> 8;  p[addr+3] = x; }
#endif
#else
#ifdef HOST_LITTLE_ENDIAN
	*((uint32_t *)(p+addr+4)) = x; }
#else
	p[addr  +4] = x;        p[addr+1+4] = x >> 8;
	p[addr+2+4] = x >> 16;  p[addr+3+4] = x >> 24; }
#endif
#endif

	/*  Second word in pair:  */
	{ uint32_t x = reg(ic->arg[0] + 4);
#ifdef LS_BE
#ifdef HOST_BIG_ENDIAN
	*((uint32_t *)(p+addr+4)) = x; }
#else
	p[addr  +4] = x >> 24; p[addr+1+4] = x >> 16;
	p[addr+2+4] = x >> 8;  p[addr+3+4] = x; }
#endif
#else
#ifdef HOST_LITTLE_ENDIAN
	*((uint32_t *)(p+addr)) = x; }
#else
	p[addr  ] = x;        p[addr+1] = x >> 8;
	p[addr+2] = x >> 16;  p[addr+3] = x >> 24; }
#endif
#endif

#endif	/*  LS_8  */

#endif	/*  store  */
}

