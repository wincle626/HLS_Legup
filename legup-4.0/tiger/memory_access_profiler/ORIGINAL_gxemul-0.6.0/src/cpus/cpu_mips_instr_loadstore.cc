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
 *  MIPS load/store instructions; the following args are used:
 *  
 *  arg[0] = pointer to the register to load to or store from
 *  arg[1] = pointer to the base register
 *  arg[2] = offset (as an int32_t)
 *
 *  The GENERIC function always checks for alignment, and supports both big
 *  and little endian byte order.
 *
 *  The quick function is included twice (big/little endian) for each
 *  GENERIC function.
 */


#ifdef LS_INCLUDE_GENERIC
void LS_GENERIC_N(struct cpu *cpu, struct mips_instr_call *ic)
{
	MODE_int_t addr = reg(ic->arg[1]) + (int32_t)ic->arg[2];
	uint8_t data[LS_SIZE];
#ifdef LS_LOAD
	uint64_t x;
#endif

	/*  Synchronize the PC:  */
	int low_pc = ((size_t)ic - (size_t)cpu->cd.mips.cur_ic_page)
	    / sizeof(struct mips_instr_call);
	cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)<<MIPS_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << MIPS_INSTR_ALIGNMENT_SHIFT);

#ifndef LS_1
	/*  Check alignment:  */
	if (addr & (LS_SIZE - 1)) {
#if 1
		/*  Cause an address alignment exception:  */
		mips_cpu_exception(cpu,
#ifdef LS_LOAD
		    EXCEPTION_ADEL,
#else
		    EXCEPTION_ADES,
#endif
		    0, addr, 0, 0, 0, 0);
#else
		fatal("{ mips dyntrans alignment exception, size = %i,"
		    " addr = %016"PRIx64", pc = %016"PRIx64" }\n", LS_SIZE,
		    (uint64_t) addr, cpu->pc);

		/*  TODO: Generalize this into a abort_call, or similar:  */
		cpu->running = 0;
		debugger_n_steps_left_before_interaction = 0;
		cpu->cd.mips.next_ic = &nothing_call;
#endif
		return;
	}
#endif

#ifdef LS_LOAD
	if (!cpu->memory_rw(cpu, cpu->mem, addr, data, sizeof(data),
	    MEM_READ, CACHE_DATA)) {
		/*  Exception.  */
		return;
	}
	x = memory_readmax64(cpu, data, LS_SIZE);
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
#else	/*  LS_STORE:  */
	memory_writemax64(cpu, data, LS_SIZE, reg(ic->arg[0]));
	if (!cpu->memory_rw(cpu, cpu->mem, addr, data, sizeof(data),
	    MEM_WRITE, CACHE_DATA)) {
		/*  Exception.  */
		return;
	}
#endif
}
#endif	/*  LS_INCLUDE_GENERIC  */


void LS_N(struct cpu *cpu, struct mips_instr_call *ic)
{
	MODE_uint_t addr = reg(ic->arg[1]) + (int32_t)ic->arg[2];
	unsigned char *p;
#ifdef MODE32
#ifdef LS_LOAD
	p = cpu->cd.mips.host_load[addr >> 12];
#else
	p = cpu->cd.mips.host_store[addr >> 12];
#endif
#else	/*  !MODE32  */
	const uint32_t mask1 = (1 << DYNTRANS_L1N) - 1;
	const uint32_t mask2 = (1 << DYNTRANS_L2N) - 1;
	const uint32_t mask3 = (1 << DYNTRANS_L3N) - 1;
	uint32_t x1, x2, x3;
	struct DYNTRANS_L2_64_TABLE *l2;
	struct DYNTRANS_L3_64_TABLE *l3;

	x1 = (addr >> (64-DYNTRANS_L1N)) & mask1;
	x2 = (addr >> (64-DYNTRANS_L1N-DYNTRANS_L2N)) & mask2;
	x3 = (addr >> (64-DYNTRANS_L1N-DYNTRANS_L2N-DYNTRANS_L3N)) & mask3;
	/*  fatal("X3: addr=%016"PRIx64" x1=%x x2=%x x3=%x\n",
	    (uint64_t) addr, (int) x1, (int) x2, (int) x3);  */
	l2 = cpu->cd.DYNTRANS_ARCH.l1_64[x1];
	/*  fatal("  l2 = %p\n", l2);  */
	l3 = l2->l3[x2];
	/*  fatal("  l3 = %p\n", l3);  */
#ifdef LS_LOAD
	p = l3->host_load[x3];
#else
	p = l3->host_store[x3];
#endif
	/*  fatal("  p = %p\n", p);  */
#endif

	if (p == NULL
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
	*((uint64_t *)ic->arg[0]) =
#ifdef LS_BE
#ifdef HOST_BIG_ENDIAN
	    ( *(uint64_t *)(p + addr) );
#else
	    ((uint64_t)p[addr] << 56) + ((uint64_t)p[addr+1] << 48) +
	    ((uint64_t)p[addr+2] << 40) + ((uint64_t)p[addr+3] << 32) +
	    ((uint64_t)p[addr+4] << 24) +
	    (p[addr+5] << 16) + (p[addr+6] << 8) + p[addr+7];
#endif
#else
#ifdef HOST_LITTLE_ENDIAN
	    ( *(uint64_t *)(p + addr) );
#else
	    p[addr+0] + (p[addr+1] << 8) + (p[addr+2] << 16) +
	    ((uint64_t)p[addr+3] << 24) + ((uint64_t)p[addr+4] << 32) +
	    ((uint64_t)p[addr+5] << 40) + ((uint64_t)p[addr+6] << 48) +
	    ((uint64_t)p[addr+7] << 56);
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
	{ uint64_t x = *(uint64_t *)(ic->arg[0]);
#ifdef LS_BE
#ifdef HOST_BIG_ENDIAN
	*((uint64_t *)(p+addr)) = x; }
#else
	p[addr]   = x >> 56; p[addr+1] = x >> 48; p[addr+2] = x >> 40;
	p[addr+3] = x >> 32; p[addr+4] = x >> 24; p[addr+5] = x >> 16;
	p[addr+6] = x >> 8;  p[addr+7] = x; }
#endif
#else
#ifdef HOST_LITTLE_ENDIAN
	*((uint64_t *)(p+addr)) = x; }
#else
	p[addr]   = x;       p[addr+1] = x >>  8; p[addr+2] = x >> 16;
	p[addr+3] = x >> 24; p[addr+4] = x >> 32; p[addr+5] = x >> 40;
	p[addr+6] = x >> 48; p[addr+7] = x >> 56; }
#endif
#endif
#endif	/*  LS_8  */

#endif	/*  store  */
}

