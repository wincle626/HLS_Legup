/*
 *  Copyright (C) 2005-2009  Anders Gavare.  All rights reserved.
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
 *  TODO:  Many things...
 *
 *	o)  Big-endian ARM loads/stores.
 *
 *	o)  Alignment checks!
 *
 *	o)  Native load/store if the endianness is the same as the host's
 *	    (only implemented for little endian, so far, and it assumes that
 *	    alignment is correct!)
 *
 *	o)  "Base Updated Abort Model", which updates the base register
 *	    even if the memory access failed.
 *
 *	o)  Some ARM implementations use pc+8, some use pc+12 for stores?
 *
 *	o)  All load/store variants with the PC register are not really
 *	    valid. (E.g. a byte load into the PC register. What should that
 *	    accomplish?)
 *
 *	o)  Perhaps an optimization for the case when offset = 0, because
 *	    that's quite common, and also when the Reg expression is just
 *	    a simple, non-rotated register (0..14).
 */


#if defined(A__SIGNED) && !defined(A__H) && !defined(A__L)
#define A__LDRD
#endif
#if defined(A__SIGNED) && defined(A__H) && !defined(A__L)
#define A__STRD
#endif


/*
 *  General load/store, by using memory_rw(). If at all possible, memory_rw()
 *  then inserts the page into the translation array, so that the fast
 *  load/store routine below can be used for further accesses.
 */
void A__NAME__general(struct cpu *cpu, struct arm_instr_call *ic)
{
#if !defined(A__P) && defined(A__W)
	const int memory_rw_flags = CACHE_DATA | MEMORY_USER_ACCESS;
#else
	const int memory_rw_flags = CACHE_DATA;
#endif

#ifdef A__REG
	uint32_t (*reg_func)(struct cpu *, struct arm_instr_call *)
	    = (uint32_t (*)(struct cpu *, struct arm_instr_call *))
	      (void *)(size_t)ic->arg[1];
#endif

#if defined(A__STRD) || defined(A__LDRD)
	unsigned char data[8];
	const int datalen = 8;
#else
#ifdef A__B
	unsigned char data[1];
	const int datalen = 1;
#else
#ifdef A__H
	unsigned char data[2];
	const int datalen = 2;
#else
	const int datalen = 4;
#ifdef HOST_LITTLE_ENDIAN
	unsigned char *data = (unsigned char *) ic->arg[2];
#else
	unsigned char data[4];
#endif
#endif
#endif
#endif

	uint32_t addr, low_pc, offset =
#ifndef A__U
	    -
#endif
#ifdef A__REG
	    reg_func(cpu, ic);
#else
	    ic->arg[1];
#endif

	low_pc = ((size_t)ic - (size_t)cpu->cd.arm.
	    cur_ic_page) / sizeof(struct arm_instr_call);
	cpu->pc &= ~((ARM_IC_ENTRIES_PER_PAGE-1)
	    << ARM_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << ARM_INSTR_ALIGNMENT_SHIFT);

	addr = reg(ic->arg[0])
#ifdef A__P
	    + offset
#endif
	    ;


#if defined(A__L) || defined(A__LDRD)
	/*  Load:  */
	if (!cpu->memory_rw(cpu, cpu->mem, addr, data, datalen,
	    MEM_READ, memory_rw_flags)) {
		/*  load failed, an exception was generated  */
		return;
	}
#if defined(A__B) && !defined(A__LDRD)
	reg(ic->arg[2]) =
#ifdef A__SIGNED
	    (int32_t)(int8_t)
#endif
	    data[0];
#else
#if defined(A__H) && !defined(A__LDRD)
	reg(ic->arg[2]) =
#ifdef A__SIGNED
	    (int32_t)(int16_t)
#endif
	    (data[0] + (data[1] << 8));
#else
#ifndef A__LDRD
#ifdef HOST_LITTLE_ENDIAN
	/*  Nothing.  */
#else
	reg(ic->arg[2]) = data[0] + (data[1] << 8) +
	    (data[2] << 16) + (data[3] << 24);
#endif
#else
	reg(ic->arg[2]) = data[0] + (data[1] << 8) +
	    (data[2] << 16) + (data[3] << 24);
	reg(((uint32_t *)ic->arg[2]) + 1) = data[4] + (data[5] << 8) +
	    (data[6] << 16) + (data[7] << 24);
#endif
#endif
#endif
#else
	/*  Store:  */
#if !defined(A__B) && !defined(A__H) && defined(HOST_LITTLE_ENDIAN)
#ifdef A__STRD
	*(uint32_t *)data = reg(ic->arg[2]);
	*(uint32_t *)(data + 4) = reg(ic->arg[2] + 4);
#endif
#else
	data[0] = reg(ic->arg[2]);
#ifndef A__B
	data[1] = reg(ic->arg[2]) >> 8;
#if !defined(A__H) || defined(A__STRD)
	data[1] = reg(ic->arg[2]) >> 8;
	data[2] = reg(ic->arg[2]) >> 16;
	data[3] = reg(ic->arg[2]) >> 24;
#ifdef A__STRD
	data[4] = reg(ic->arg[2] + 4);
	data[5] = reg(ic->arg[2] + 4) >> 8;
	data[6] = reg(ic->arg[2] + 4) >> 16;
	data[7] = reg(ic->arg[2] + 4) >> 24;
#endif
#endif
#endif
#endif
	if (!cpu->memory_rw(cpu, cpu->mem, addr, data, datalen,
	    MEM_WRITE, memory_rw_flags)) {
		/*  store failed, an exception was generated  */
		return;
	}
#endif

#ifdef A__P
#ifdef A__W
	reg(ic->arg[0]) = addr;
#endif
#else	/*  post-index writeback  */
	reg(ic->arg[0]) = addr + offset;
#endif
}


/*
 *  Fast load/store, if the page is in the translation array.
 */
void A__NAME(struct cpu *cpu, struct arm_instr_call *ic)
{
#if defined(A__LDRD) || defined(A__STRD)
	/*  Chicken out, let's do this unoptimized for now:  */
	A__NAME__general(cpu, ic);
#else
#ifdef A__REG
	uint32_t (*reg_func)(struct cpu *, struct arm_instr_call *)
	    = (uint32_t (*)(struct cpu *, struct arm_instr_call *))
	      (void *)(size_t)ic->arg[1];
#endif
	uint32_t offset =
#ifndef A__U
	    -
#endif
#ifdef A__REG
	    reg_func(cpu, ic);
#else
	    ic->arg[1];
#endif
	uint32_t addr = reg(ic->arg[0])
#ifdef A__P
	    + offset
#endif
	    ;
	unsigned char *page = cpu->cd.arm.
#ifdef A__L
	    host_load
#else
	    host_store
#endif
	    [addr >> 12];


#if !defined(A__P) && defined(A__W)
	/*
	 *  T-bit: userland access: check the corresponding bit in the
	 *  is_userpage array. If it is set, then we're ok. Otherwise: use the
	 *  generic function.
	 */
	uint32_t x = cpu->cd.arm.is_userpage[addr >> 17];
	if (!(x & (1 << ((addr >> 12) & 31))))
		A__NAME__general(cpu, ic);
	else
#endif


	if (page == NULL) {
		A__NAME__general(cpu, ic);
	} else {
#ifdef A__L
#ifdef A__B
		reg(ic->arg[2]) =
#ifdef A__SIGNED
		    (int32_t)(int8_t)
#endif
		    page[addr & 0xfff];
#else
#ifdef A__H
		reg(ic->arg[2]) =
#ifdef A__SIGNED
		    (int32_t)(int16_t)
#endif
		    (page[addr & 0xfff] + (page[(addr & 0xfff) + 1] << 8));
#else
#ifdef HOST_LITTLE_ENDIAN
		reg(ic->arg[2]) = *(uint32_t *)(page + (addr & 0xffc));
#else
		reg(ic->arg[2]) = page[addr & 0xfff] +
		    (page[(addr & 0xfff) + 1] << 8) +
		    (page[(addr & 0xfff) + 2] << 16) +
		    (page[(addr & 0xfff) + 3] << 24);
#endif
#endif
#endif
#else
#ifdef A__B
		page[addr & 0xfff] = reg(ic->arg[2]);
#else
#ifdef A__H
		page[addr & 0xfff] = reg(ic->arg[2]);
		page[(addr & 0xfff)+1] = reg(ic->arg[2]) >> 8;
#else
#ifdef HOST_LITTLE_ENDIAN
		*(uint32_t *)(page + (addr & 0xffc)) = reg(ic->arg[2]);
#else
		page[addr & 0xfff] = reg(ic->arg[2]);
		page[(addr & 0xfff)+1] = reg(ic->arg[2]) >> 8;
		page[(addr & 0xfff)+2] = reg(ic->arg[2]) >> 16;
		page[(addr & 0xfff)+3] = reg(ic->arg[2]) >> 24;
#endif
#endif
#endif
#endif

		/*  Index Write-back:  */
#ifdef A__P
#ifdef A__W
		reg(ic->arg[0]) = addr;
#endif
#else
		/*  post-index writeback  */
		reg(ic->arg[0]) = addr + offset;
#endif
	}
#endif	/*  not STRD  */
}


/*
 *  Special case when loading or storing the ARM's PC register, or when the PC
 *  register is used as the base address register.
 *
 *  o)	Loads into the PC register cause a branch. If an exception occured
 *	during the load, then the PC register should already point to the
 *	exception handler, in which case we simply recalculate the pointers a
 *	second time (no harm is done by doing that).
 *
 *	TODO: A tiny performance optimization would be to separate the two
 *	cases: a load where arg[0] = PC, and the case where arg[2] = PC.
 *
 *  o)	Stores store "PC of the current instruction + 12". The solution I have
 *	choosen is to calculate this value and place it into a temporary
 *	variable (tmp_pc), which is then used for the store.
 */
void A__NAME_PC(struct cpu *cpu, struct arm_instr_call *ic)
{
#ifdef A__L
	/*  Load:  */
	if (ic->arg[0] == (size_t)(&cpu->cd.arm.tmp_pc)) {
		/*  tmp_pc = current PC + 8:  */
		uint32_t low_pc, tmp;
		low_pc = ((size_t)ic - (size_t) cpu->cd.arm.cur_ic_page) /
		    sizeof(struct arm_instr_call);
		tmp = cpu->pc & ~((ARM_IC_ENTRIES_PER_PAGE-1) <<
		    ARM_INSTR_ALIGNMENT_SHIFT);
		tmp += (low_pc << ARM_INSTR_ALIGNMENT_SHIFT);
		cpu->cd.arm.tmp_pc = tmp + 8;
	}
	A__NAME(cpu, ic);
	if (ic->arg[2] == (size_t)(&cpu->cd.arm.r[ARM_PC])) {
		cpu->pc = cpu->cd.arm.r[ARM_PC];
		quick_pc_to_pointers(cpu);
		if (cpu->machine->show_trace_tree)
			cpu_functioncall_trace(cpu, cpu->pc);
	}
#else
	/*  Store:  */
	uint32_t low_pc, tmp;
	/*  Calculate tmp from this instruction's PC + 12  */
	low_pc = ((size_t)ic - (size_t) cpu->cd.arm.cur_ic_page) /
	    sizeof(struct arm_instr_call);
	tmp = cpu->pc & ~((ARM_IC_ENTRIES_PER_PAGE-1) <<
	    ARM_INSTR_ALIGNMENT_SHIFT);
	tmp += (low_pc << ARM_INSTR_ALIGNMENT_SHIFT);
	cpu->cd.arm.tmp_pc = tmp + 12;
	A__NAME(cpu, ic);
#endif
}


#ifndef A__NOCONDITIONS
/*  Load/stores with all registers except the PC register:  */
void A__NAME__eq(struct cpu *cpu, struct arm_instr_call *ic)
{ if (cpu->cd.arm.flags & ARM_F_Z) A__NAME(cpu, ic); }
void A__NAME__ne(struct cpu *cpu, struct arm_instr_call *ic)
{ if (!(cpu->cd.arm.flags & ARM_F_Z)) A__NAME(cpu, ic); }
void A__NAME__cs(struct cpu *cpu, struct arm_instr_call *ic)
{ if (cpu->cd.arm.flags & ARM_F_C) A__NAME(cpu, ic); }
void A__NAME__cc(struct cpu *cpu, struct arm_instr_call *ic)
{ if (!(cpu->cd.arm.flags & ARM_F_C)) A__NAME(cpu, ic); }
void A__NAME__mi(struct cpu *cpu, struct arm_instr_call *ic)
{ if (cpu->cd.arm.flags & ARM_F_N) A__NAME(cpu, ic); }
void A__NAME__pl(struct cpu *cpu, struct arm_instr_call *ic)
{ if (!(cpu->cd.arm.flags & ARM_F_N)) A__NAME(cpu, ic); }
void A__NAME__vs(struct cpu *cpu, struct arm_instr_call *ic)
{ if (cpu->cd.arm.flags & ARM_F_V) A__NAME(cpu, ic); }
void A__NAME__vc(struct cpu *cpu, struct arm_instr_call *ic)
{ if (!(cpu->cd.arm.flags & ARM_F_V)) A__NAME(cpu, ic); }

void A__NAME__hi(struct cpu *cpu, struct arm_instr_call *ic)
{ if (cpu->cd.arm.flags & ARM_F_C &&
!(cpu->cd.arm.flags & ARM_F_Z)) A__NAME(cpu, ic); }
void A__NAME__ls(struct cpu *cpu, struct arm_instr_call *ic)
{ if (cpu->cd.arm.flags & ARM_F_Z ||
!(cpu->cd.arm.flags & ARM_F_C)) A__NAME(cpu, ic); }
void A__NAME__ge(struct cpu *cpu, struct arm_instr_call *ic)
{ if (((cpu->cd.arm.flags & ARM_F_N)?1:0) ==
((cpu->cd.arm.flags & ARM_F_V)?1:0)) A__NAME(cpu, ic); }
void A__NAME__lt(struct cpu *cpu, struct arm_instr_call *ic)
{ if (((cpu->cd.arm.flags & ARM_F_N)?1:0) !=
((cpu->cd.arm.flags & ARM_F_V)?1:0)) A__NAME(cpu, ic); }
void A__NAME__gt(struct cpu *cpu, struct arm_instr_call *ic)
{ if (((cpu->cd.arm.flags & ARM_F_N)?1:0) ==
((cpu->cd.arm.flags & ARM_F_V)?1:0) &&
!(cpu->cd.arm.flags & ARM_F_Z)) A__NAME(cpu, ic); }
void A__NAME__le(struct cpu *cpu, struct arm_instr_call *ic)
{ if (((cpu->cd.arm.flags & ARM_F_N)?1:0) !=
((cpu->cd.arm.flags & ARM_F_V)?1:0) ||
(cpu->cd.arm.flags & ARM_F_Z)) A__NAME(cpu, ic); }


/*  Load/stores with the PC register:  */
void A__NAME_PC__eq(struct cpu *cpu, struct arm_instr_call *ic)
{ if (cpu->cd.arm.flags & ARM_F_Z) A__NAME_PC(cpu, ic); }
void A__NAME_PC__ne(struct cpu *cpu, struct arm_instr_call *ic)
{ if (!(cpu->cd.arm.flags & ARM_F_Z)) A__NAME_PC(cpu, ic); }
void A__NAME_PC__cs(struct cpu *cpu, struct arm_instr_call *ic)
{ if (cpu->cd.arm.flags & ARM_F_C) A__NAME_PC(cpu, ic); }
void A__NAME_PC__cc(struct cpu *cpu, struct arm_instr_call *ic)
{ if (!(cpu->cd.arm.flags & ARM_F_C)) A__NAME_PC(cpu, ic); }
void A__NAME_PC__mi(struct cpu *cpu, struct arm_instr_call *ic)
{ if (cpu->cd.arm.flags & ARM_F_N) A__NAME_PC(cpu, ic); }
void A__NAME_PC__pl(struct cpu *cpu, struct arm_instr_call *ic)
{ if (!(cpu->cd.arm.flags & ARM_F_N)) A__NAME_PC(cpu, ic); }
void A__NAME_PC__vs(struct cpu *cpu, struct arm_instr_call *ic)
{ if (cpu->cd.arm.flags & ARM_F_V) A__NAME_PC(cpu, ic); }
void A__NAME_PC__vc(struct cpu *cpu, struct arm_instr_call *ic)
{ if (!(cpu->cd.arm.flags & ARM_F_V)) A__NAME_PC(cpu, ic); }

void A__NAME_PC__hi(struct cpu *cpu, struct arm_instr_call *ic)
{ if (cpu->cd.arm.flags & ARM_F_C &&
!(cpu->cd.arm.flags & ARM_F_Z)) A__NAME_PC(cpu, ic); }
void A__NAME_PC__ls(struct cpu *cpu, struct arm_instr_call *ic)
{ if (cpu->cd.arm.flags & ARM_F_Z ||
!(cpu->cd.arm.flags & ARM_F_C)) A__NAME_PC(cpu, ic); }
void A__NAME_PC__ge(struct cpu *cpu, struct arm_instr_call *ic)
{ if (((cpu->cd.arm.flags & ARM_F_N)?1:0) ==
((cpu->cd.arm.flags & ARM_F_V)?1:0)) A__NAME_PC(cpu, ic); }
void A__NAME_PC__lt(struct cpu *cpu, struct arm_instr_call *ic)
{ if (((cpu->cd.arm.flags & ARM_F_N)?1:0) !=
((cpu->cd.arm.flags & ARM_F_V)?1:0)) A__NAME_PC(cpu, ic); }
void A__NAME_PC__gt(struct cpu *cpu, struct arm_instr_call *ic)
{ if (((cpu->cd.arm.flags & ARM_F_N)?1:0) ==
((cpu->cd.arm.flags & ARM_F_V)?1:0) &&
!(cpu->cd.arm.flags & ARM_F_Z)) A__NAME_PC(cpu, ic); }
void A__NAME_PC__le(struct cpu *cpu, struct arm_instr_call *ic)
{ if (((cpu->cd.arm.flags & ARM_F_N)?1:0) !=
((cpu->cd.arm.flags & ARM_F_V)?1:0) ||
(cpu->cd.arm.flags & ARM_F_Z)) A__NAME_PC(cpu, ic); }
#endif


#ifdef A__LDRD
#undef A__LDRD
#endif

#ifdef A__STRD
#undef A__STRD
#endif

