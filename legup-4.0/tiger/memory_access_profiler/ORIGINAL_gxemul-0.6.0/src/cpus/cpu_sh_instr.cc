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
 *  SH instructions.
 *
 *  Individual functions should keep track of cpu->n_translated_instrs.
 *  (If no instruction was executed, then it should be decreased. If, say, 4
 *  instructions were combined into one function and executed, then it should
 *  be increased by 3.)
 */


#define	SYNCH_PC		{					\
		int low_pc = ((size_t)ic - (size_t)cpu->cd.sh.cur_ic_page) \
		    / sizeof(struct sh_instr_call);			\
		cpu->pc &= ~((SH_IC_ENTRIES_PER_PAGE-1)			\
		    << SH_INSTR_ALIGNMENT_SHIFT);			\
		cpu->pc += (low_pc << SH_INSTR_ALIGNMENT_SHIFT);	\
	}

#define	RES_INST_IF_NOT_MD						\
	if (!(cpu->cd.sh.sr & SH_SR_MD)) {				\
		SYNCH_PC;						\
		sh_exception(cpu, EXPEVT_RES_INST, 0, 0);		\
		return;							\
	}

#define	FLOATING_POINT_AVAILABLE_CHECK					\
	if (cpu->cd.sh.sr & SH_SR_FD) {					\
		/*  FPU disabled: Cause exception.  */			\
		SYNCH_PC;						\
		if (cpu->delay_slot)					\
			sh_exception(cpu, EXPEVT_FPU_SLOT_DISABLE, 0, 0);\
		else							\
			sh_exception(cpu, EXPEVT_FPU_DISABLE, 0, 0);	\
		return;							\
	}


/*
 *  nop: Nothing
 */
X(nop)
{
}


/*
 *  sleep:  Wait for interrupt
 */
X(sleep)
{
	RES_INST_IF_NOT_MD;

	/*
	 *  If there is an interrupt, then just return. Otherwise
	 *  re-run the sleep instruction (after a delay).
	 */
	if (cpu->cd.sh.int_to_assert > 0 && !(cpu->cd.sh.sr & SH_SR_BL)
	    && ((cpu->cd.sh.sr & SH_SR_IMASK) >> SH_SR_IMASK_SHIFT)
	    < cpu->cd.sh.int_level)
		return;

	cpu->cd.sh.next_ic = ic;
	cpu->is_halted = 1;
	cpu->has_been_idling = 1;

	/*
	 *  There was no interrupt. Let the host sleep for a while.
	 *
	 *  TODO:
	 *
	 *  Think about how to actually implement this usleep stuff,
	 *  in an SMP and/or timing accurate environment.
	 */

	if (cpu->machine->ncpus == 1) {
		static int x = 0;

		if ((++x) == 600) {
			usleep(10);
			x = 0;
		}

		cpu->n_translated_instrs += N_SAFE_DYNTRANS_LIMIT / 6;
	}
}


/*
 *  sett:     t = 1
 *  sets:     s = 1
 *  clrt:     t = 1
 *  clrs:     s = 1
 *  movt_rn:  rn = t
 *  clrmac:   mach = macl = 0
 *
 *  arg[1] = ptr to rn
 */
X(sett)    { cpu->cd.sh.sr |= SH_SR_T; }
X(sets)    { cpu->cd.sh.sr |= SH_SR_S; }
X(clrt)    { cpu->cd.sh.sr &= ~SH_SR_T; }
X(clrs)    { cpu->cd.sh.sr &= ~SH_SR_S; }
X(movt_rn) { reg(ic->arg[1]) = cpu->cd.sh.sr & SH_SR_T? 1 : 0; }
X(clrmac)  { cpu->cd.sh.macl = cpu->cd.sh.mach = 0; }


/*
 *  mov_rm_rn:     rn = rm
 *  neg_rm_rn:     rn = -rm
 *  negc_rm_rn:    rn = -rm - t, t = borrow
 *  not_rm_rn:     rn = ~rm
 *  swap_b_rm_rn:  rn = rm with lowest 2 bytes swapped
 *  swap_w_rm_rn:  rn = rm with high and low 16-bit words swapped
 *  exts_b_rm_rn:  rn = (int8_t) rm
 *  extu_b_rm_rn:  rn = (uint8_t) rm
 *  exts_w_rm_rn:  rn = (int16_t) rm
 *  extu_w_rm_rn:  rn = (uint16_t) rm
 *
 *  arg[0] = ptr to rm
 *  arg[1] = ptr to rn
 */
X(mov_rm_rn)    { reg(ic->arg[1]) = reg(ic->arg[0]); }
X(not_rm_rn)    { reg(ic->arg[1]) = ~reg(ic->arg[0]); }
X(neg_rm_rn)    { reg(ic->arg[1]) = -reg(ic->arg[0]); }
X(negc_rm_rn)
{
	uint64_t res = 0;
	res -= (uint64_t) reg(ic->arg[0]);
	if (cpu->cd.sh.sr & SH_SR_T)
		res --;
	if ((res >> 32) & 1)
		cpu->cd.sh.sr |= SH_SR_T;
	else
		cpu->cd.sh.sr &= ~SH_SR_T;
	reg(ic->arg[1]) = (uint32_t) res;
}
X(swap_b_rm_rn)
{
	uint32_t r = reg(ic->arg[0]);
	reg(ic->arg[1]) = (r & 0xffff0000) | ((r >> 8)&0xff) | ((r&0xff) << 8);
}
X(swap_w_rm_rn)
{
	uint32_t r = reg(ic->arg[0]);
	reg(ic->arg[1]) = (r >> 16) | (r << 16);
}
X(exts_b_rm_rn) { reg(ic->arg[1]) = (int8_t)reg(ic->arg[0]); }
X(extu_b_rm_rn) { reg(ic->arg[1]) = (uint8_t)reg(ic->arg[0]); }
X(exts_w_rm_rn) { reg(ic->arg[1]) = (int16_t)reg(ic->arg[0]); }
X(extu_w_rm_rn) { reg(ic->arg[1]) = (uint16_t)reg(ic->arg[0]); }
/*  Note: rm and rn are the same on these:  */
X(extu_b_rm)    { reg(ic->arg[1]) = (uint8_t)reg(ic->arg[1]); }
X(extu_w_rm)    { reg(ic->arg[1]) = (uint16_t)reg(ic->arg[1]); }


/*
 *  and_imm_r0:  r0 &= imm
 *  xor_imm_r0:  r0 ^= imm
 *  tst_imm_r0:  t = (r0 & imm) == 0
 *  or_imm_r0:   r0 |= imm
 *
 *  arg[0] = imm
 */
X(and_imm_r0) { cpu->cd.sh.r[0] &= ic->arg[0]; }
X(xor_imm_r0) { cpu->cd.sh.r[0] ^= ic->arg[0]; }
X(or_imm_r0)  { cpu->cd.sh.r[0] |= ic->arg[0]; }
X(tst_imm_r0)
{
	if (cpu->cd.sh.r[0] & ic->arg[0])
		cpu->cd.sh.sr &= ~SH_SR_T;
	else
		cpu->cd.sh.sr |= SH_SR_T;
}


/*
 *  xor_b_imm_r0_gbr:  mem[r0+gbr] |= imm
 *  or_b_imm_r0_gbr:   mem[r0+gbr] ^= imm
 *  and_b_imm_r0_gbr:  mem[r0+gbr] &= imm
 *
 *  arg[0] = imm
 */
X(xor_b_imm_r0_gbr)
{
	uint32_t addr = cpu->cd.sh.gbr + cpu->cd.sh.r[0];
	uint8_t *p = (uint8_t *) cpu->cd.sh.host_store[addr >> 12];

	if (p != NULL) {
		p[addr & 0xfff] ^= ic->arg[0];
	} else {
		uint8_t data;
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		   sizeof(data), MEM_READ, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
		data ^= ic->arg[0];
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		   sizeof(data), MEM_WRITE, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
	}
}
X(or_b_imm_r0_gbr)
{
	uint32_t addr = cpu->cd.sh.gbr + cpu->cd.sh.r[0];
	uint8_t *p = (uint8_t *) cpu->cd.sh.host_store[addr >> 12];

	if (p != NULL) {
		p[addr & 0xfff] |= ic->arg[0];
	} else {
		uint8_t data;
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		   sizeof(data), MEM_READ, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
		data |= ic->arg[0];
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		   sizeof(data), MEM_WRITE, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
	}
}
X(and_b_imm_r0_gbr)
{
	uint32_t addr = cpu->cd.sh.gbr + cpu->cd.sh.r[0];
	uint8_t *p = (uint8_t *) cpu->cd.sh.host_store[addr >> 12];

	if (p != NULL) {
		p[addr & 0xfff] &= ic->arg[0];
	} else {
		uint8_t data;
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		   sizeof(data), MEM_READ, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
		data &= ic->arg[0];
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		   sizeof(data), MEM_WRITE, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
	}
}


/*
 *  mov_imm_rn:  Set rn to a signed 8-bit value
 *  add_imm_rn:  Add a signed 8-bit value to Rn
 *
 *  arg[0] = int8_t imm, extended to at least int32_t
 *  arg[1] = ptr to rn
 */
X(mov_imm_rn) { reg(ic->arg[1]) = ic->arg[0]; }
X(mov_0_rn)   { reg(ic->arg[1]) = 0; }
X(add_imm_rn) { reg(ic->arg[1]) += ic->arg[0]; }
X(inc_rn)     { reg(ic->arg[1]) ++; }
X(add_4_rn)   { reg(ic->arg[1]) += 4; }
X(sub_4_rn)   { reg(ic->arg[1]) -= 4; }
X(dec_rn)     { reg(ic->arg[1]) --; }


/*
 *  mov_b_rm_predec_rn:     mov.b reg,@-Rn
 *  mov_w_rm_predec_rn:     mov.w reg,@-Rn
 *  mov_l_rm_predec_rn:     mov.l reg,@-Rn
 *  stc_l_rm_predec_rn_md:  mov.l reg,@-Rn, with MD status bit check
 *
 *  arg[0] = ptr to rm  (or other register)
 *  arg[1] = ptr to rn
 */
X(mov_b_rm_predec_rn)
{
	uint32_t addr = reg(ic->arg[1]) - sizeof(uint8_t);
	int8_t *p = (int8_t *) cpu->cd.sh.host_store[addr >> 12];
	int8_t data = reg(ic->arg[0]);
	if (p != NULL) {
		p[addr & 0xfff] = data;
		reg(ic->arg[1]) = addr;
	} else {
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		   sizeof(data), MEM_WRITE, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
		/*  The store was ok:  */
		reg(ic->arg[1]) = addr;
	}
}
X(mov_w_rm_predec_rn)
{
	uint32_t addr = reg(ic->arg[1]) - sizeof(uint16_t);
	uint16_t *p = (uint16_t *) cpu->cd.sh.host_store[addr >> 12];
	uint16_t data = reg(ic->arg[0]);

	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		data = LE16_TO_HOST(data);
	else
		data = BE16_TO_HOST(data);

	if (p != NULL) {
		p[(addr & 0xfff) >> 1] = data;
		reg(ic->arg[1]) = addr;
	} else {
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		   sizeof(data), MEM_WRITE, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
		/*  The store was ok:  */
		reg(ic->arg[1]) = addr;
	}
}
X(mov_l_rm_predec_rn)
{
	uint32_t addr = reg(ic->arg[1]) - sizeof(uint32_t);
	uint32_t *p = (uint32_t *) cpu->cd.sh.host_store[addr >> 12];
	uint32_t data = reg(ic->arg[0]);

	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		data = LE32_TO_HOST(data);
	else
		data = BE32_TO_HOST(data);

	if (p != NULL) {
		p[(addr & 0xfff) >> 2] = data;
		reg(ic->arg[1]) = addr;
	} else {
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		   sizeof(data), MEM_WRITE, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
		/*  The store was ok:  */
		reg(ic->arg[1]) = addr;
	}
}
X(stc_l_rm_predec_rn_md)
{
	uint32_t addr = reg(ic->arg[1]) - sizeof(uint32_t);
	uint32_t *p = (uint32_t *) cpu->cd.sh.host_store[addr >> 12];
	uint32_t data = reg(ic->arg[0]);

	RES_INST_IF_NOT_MD;

	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		data = LE32_TO_HOST(data);
	else
		data = BE32_TO_HOST(data);

	if (p != NULL) {
		p[(addr & 0xfff) >> 2] = data;
		reg(ic->arg[1]) = addr;
	} else {
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		   sizeof(data), MEM_WRITE, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
		/*  The store was ok:  */
		reg(ic->arg[1]) = addr;
	}
}


/*
 *  mov_l_disp_pc_rn:  Load a 32-bit value into a register,
 *                     from an immediate address relative to the pc.
 *
 *  arg[0] = offset from beginning of the current pc's page
 *  arg[1] = ptr to rn
 */
X(mov_l_disp_pc_rn)
{
	uint32_t addr = ic->arg[0] + (cpu->pc &
	    ~((SH_IC_ENTRIES_PER_PAGE-1) << SH_INSTR_ALIGNMENT_SHIFT));
	uint32_t *p = (uint32_t *) cpu->cd.sh.host_load[addr >> 12];
	uint32_t data;

	if (p != NULL) {
		data = p[(addr & 0xfff) >> 2];
	} else {
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		    sizeof(data), MEM_READ, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
	}
	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		data = LE32_TO_HOST(data);
	else
		data = BE32_TO_HOST(data);
	reg(ic->arg[1]) = data;
}


/*
 *  mova_r0:  Set r0 to an address close to the program counter.
 *
 *  arg[0] = relative offset from beginning of the current pc's page
 */
X(mova_r0)
{
	cpu->cd.sh.r[0] = ic->arg[0] + (cpu->pc &
	    ~((SH_IC_ENTRIES_PER_PAGE-1) << SH_INSTR_ALIGNMENT_SHIFT));
}


/*
 *  mov_w_disp_pc_rn:  Load a 16-bit value into a register,
 *                     from an immediate address relative to the pc.
 *
 *  arg[0] = offset from beginning of the current pc's page
 *  arg[1] = ptr to rn
 */
X(mov_w_disp_pc_rn)
{
	uint32_t addr = ic->arg[0] + (cpu->pc &
	    ~((SH_IC_ENTRIES_PER_PAGE-1) << SH_INSTR_ALIGNMENT_SHIFT));
	uint16_t *p = (uint16_t *) cpu->cd.sh.host_load[addr >> 12];
	uint16_t data;

	if (p != NULL) {
		data = p[(addr & 0xfff) >> 1];
	} else {
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		    sizeof(data), MEM_READ, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
	}

	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		data = LE16_TO_HOST(data);
	else
		data = BE16_TO_HOST(data);

	reg(ic->arg[1]) = (int16_t)data;
}


/*
 *  load_b_rm_rn:      Load an int8_t value into Rn from address Rm.
 *  load_w_rm_rn:      Load an int16_t value into Rn from address Rm.
 *  load_l_rm_rn:      Load a 32-bit value into Rn from address Rm.
 *  fmov_rm_frn:       Load a floating point value into FRn from address Rm.
 *  fmov_r0_rm_frn:    Load a floating point value into FRn from address R0+Rm.
 *  fmov_rm_postinc_frn: Load a floating point value into FRn from address Rm.
 *  mov_b_r0_rm_rn:    Load an int8_t value into Rn from address Rm + R0.
 *  mov_w_r0_rm_rn:    Load an int16_t value into Rn from address Rm + R0.
 *  mov_l_r0_rm_rn:    Load a 32-bit value into Rn from address Rm + R0.
 *  mov_l_disp_rm_rn:  Load a 32-bit value into Rn from address Rm + disp.
 *  mov_b_disp_rn_r0:  Load an int8_t from Rn+disp into R0.
 *  mov_w_disp_rn_r0:  Load an int16_t from Rn+disp into R0.
 *  mov_b_disp_gbr_r0: Load an int8_t from GBR+disp into R0.
 *  mov_w_disp_gbr_r0: Load an int16_t from GBR+disp into R0.
 *  mov_l_disp_gbr_r0: Load an int32_t from GBR+disp into R0.
 *  mov_b_arg1_postinc_to_arg0:
 *  mov_w_arg1_postinc_to_arg0:
 *  mov_l_arg1_postinc_to_arg0:
 *  mov_l_arg1_postinc_to_arg0_md:  With MD (privilege level) check.
 *  mov_l_arg1_postinc_to_arg0_fp:  With FP check.
 *
 *  arg[0] = ptr to rm   (or rm + (lo4 << 4) for disp)
 *  arg[1] = ptr to rn
 */
X(load_b_rm_rn)
{
	uint32_t addr = reg(ic->arg[0]);
	uint8_t *p = (uint8_t *) cpu->cd.sh.host_load[addr >> 12];
	uint8_t data;

	if (p != NULL) {
		data = p[addr & 0xfff];
	} else {
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		    sizeof(data), MEM_READ, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
	}
	reg(ic->arg[1]) = (int8_t) data;
}
X(load_w_rm_rn)
{
	uint32_t addr = reg(ic->arg[0]);
	int16_t *p = (int16_t *) cpu->cd.sh.host_load[addr >> 12];
	int16_t data;

	if (p != NULL) {
		data = p[(addr & 0xfff) >> 1];
	} else {
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		    sizeof(data), MEM_READ, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
	}
	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		data = LE16_TO_HOST(data);
	else
		data = BE16_TO_HOST(data);
	reg(ic->arg[1]) = data;
}
X(load_l_rm_rn)
{
	uint32_t addr = reg(ic->arg[0]);
	uint32_t *p = (uint32_t *) cpu->cd.sh.host_load[addr >> 12];
	uint32_t data;

	if (p != NULL) {
		data = p[(addr & 0xfff) >> 2];
	} else {
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		    sizeof(data), MEM_READ, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
	}

	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		data = LE32_TO_HOST(data);
	else
		data = BE32_TO_HOST(data);
	reg(ic->arg[1]) = data;
}
X(fmov_rm_frn)
{
	uint32_t addr = reg(ic->arg[0]);
	uint32_t *p = (uint32_t *) cpu->cd.sh.host_load[addr >> 12];
	uint32_t data;

	FLOATING_POINT_AVAILABLE_CHECK;

	if (cpu->cd.sh.fpscr & SH_FPSCR_SZ) {
		fatal("fmov_rm_frn: sz=1 (register pair): TODO\n");
		exit(1);
	}

	if (p != NULL) {
		data = p[(addr & 0xfff) >> 2];
	} else {
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		    sizeof(data), MEM_READ, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
	}

	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		data = LE32_TO_HOST(data);
	else
		data = BE32_TO_HOST(data);

	reg(ic->arg[1]) = data;
}
X(fmov_r0_rm_frn)
{
	uint32_t data, addr = reg(ic->arg[0]) + cpu->cd.sh.r[0];
	uint32_t *p = (uint32_t *) cpu->cd.sh.host_load[addr >> 12];

	FLOATING_POINT_AVAILABLE_CHECK;

	if (cpu->cd.sh.fpscr & SH_FPSCR_SZ) {
		fatal("fmov_rm_frn: sz=1 (register pair): TODO\n");
		exit(1);
	}

	if (p != NULL) {
		data = p[(addr & 0xfff) >> 2];
	} else {
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		    sizeof(data), MEM_READ, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
	}

	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		data = LE32_TO_HOST(data);
	else
		data = BE32_TO_HOST(data);

	reg(ic->arg[1]) = data;
}
X(fmov_rm_postinc_frn)
{
	int d = cpu->cd.sh.fpscr & SH_FPSCR_SZ;
	uint32_t data, data2, addr = reg(ic->arg[0]);
	uint32_t *p = (uint32_t *) cpu->cd.sh.host_load[addr >> 12];
	size_t r1 = ic->arg[1];

	if (d) {
		/*  xd instead of dr?  */
		int ofs = (r1 - (size_t)&cpu->cd.sh.fr[0]) / sizeof(uint32_t);
		if (ofs & 1)
			r1 = (size_t)&cpu->cd.sh.xf[ofs & ~1];
	}

	FLOATING_POINT_AVAILABLE_CHECK;

	if (p != NULL) {
		data = p[(addr & 0xfff) >> 2];
	} else {
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		    sizeof(data), MEM_READ, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
	}

	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		data = LE32_TO_HOST(data);
	else
		data = BE32_TO_HOST(data);

	if (d) {
		/*  Double-precision load:  */
		addr += 4;
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned
		    char *)&data2, sizeof(data2), MEM_READ, CACHE_DATA))
			return;

		if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
			data2 = LE32_TO_HOST(data2);
		else
			data2 = BE32_TO_HOST(data2);
		reg(r1 + 4) = data2;
	}

	reg(r1) = data;
	reg(ic->arg[0]) = addr + sizeof(uint32_t);
}
X(mov_b_disp_gbr_r0)
{
	uint32_t addr = cpu->cd.sh.gbr + ic->arg[1];
	int8_t *p = (int8_t *) cpu->cd.sh.host_load[addr >> 12];
	int8_t data;
	if (p != NULL) {
		data = p[addr & 0xfff];
	} else {
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		    sizeof(data), MEM_READ, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
	}
	cpu->cd.sh.r[0] = data;
}
X(mov_w_disp_gbr_r0)
{
	uint32_t addr = cpu->cd.sh.gbr + ic->arg[1];
	int16_t *p = (int16_t *) cpu->cd.sh.host_load[addr >> 12];
	int16_t data;
	if (p != NULL) {
		data = p[(addr & 0xfff) >> 1];
	} else {
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		    sizeof(data), MEM_READ, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
	}
	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		data = LE16_TO_HOST(data);
	else
		data = BE16_TO_HOST(data);
	cpu->cd.sh.r[0] = data;
}
X(mov_l_disp_gbr_r0)
{
	uint32_t addr = cpu->cd.sh.gbr + ic->arg[1];
	uint32_t *p = (uint32_t *) cpu->cd.sh.host_load[addr >> 12];
	uint32_t data;
	if (p != NULL) {
		data = p[(addr & 0xfff) >> 2];
	} else {
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		    sizeof(data), MEM_READ, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
	}
	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		data = LE32_TO_HOST(data);
	else
		data = BE32_TO_HOST(data);
	cpu->cd.sh.r[0] = data;
}
X(mov_b_arg1_postinc_to_arg0)
{
	uint32_t addr = reg(ic->arg[1]);
	int8_t *p = (int8_t *) cpu->cd.sh.host_load[addr >> 12];
	int8_t data;
	if (p != NULL) {
		data = p[addr & 0xfff];
	} else {
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		    sizeof(data), MEM_READ, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
	}
	/*  The load was ok:  */
	reg(ic->arg[1]) = addr + sizeof(int8_t);
	reg(ic->arg[0]) = data;
}
X(mov_w_arg1_postinc_to_arg0)
{
	uint32_t addr = reg(ic->arg[1]);
	uint16_t *p = (uint16_t *) cpu->cd.sh.host_load[addr >> 12];
	uint16_t data;

	if (p != NULL) {
		data = p[(addr & 0xfff) >> 1];
	} else {
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		    sizeof(data), MEM_READ, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
	}

	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		data = LE16_TO_HOST(data);
	else
		data = BE16_TO_HOST(data);
	reg(ic->arg[1]) = addr + sizeof(data);
	reg(ic->arg[0]) = (int16_t)data;
}
X(mov_l_arg1_postinc_to_arg0)
{
	uint32_t addr = reg(ic->arg[1]);
	uint32_t *p = (uint32_t *) cpu->cd.sh.host_load[addr >> 12];
	uint32_t data;

	if (p != NULL) {
		data = p[(addr & 0xfff) >> 2];
	} else {
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		    sizeof(data), MEM_READ, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
	}
	/*  The load was ok:  */
	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		data = LE32_TO_HOST(data);
	else
		data = BE32_TO_HOST(data);
	reg(ic->arg[1]) = addr + sizeof(data);
	reg(ic->arg[0]) = data;
}
X(mov_l_arg1_postinc_to_arg0_md)
{
	uint32_t addr = reg(ic->arg[1]);
	uint32_t *p = (uint32_t *) cpu->cd.sh.host_load[addr >> 12];
	uint32_t data;

	RES_INST_IF_NOT_MD;

	if (p != NULL) {
		data = p[(addr & 0xfff) >> 2];
	} else {
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		    sizeof(data), MEM_READ, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
	}
	/*  The load was ok:  */
	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		data = LE32_TO_HOST(data);
	else
		data = BE32_TO_HOST(data);
	reg(ic->arg[1]) = addr + sizeof(data);

	/*  Special case when loading into the SR register:  */
	if (ic->arg[0] == (size_t)&cpu->cd.sh.sr)
		sh_update_sr(cpu, data);
	else
		reg(ic->arg[0]) = data;
}
X(mov_l_arg1_postinc_to_arg0_fp)
{
	uint32_t addr = reg(ic->arg[1]);
	uint32_t *p = (uint32_t *) cpu->cd.sh.host_load[addr >> 12];
	uint32_t data;

	FLOATING_POINT_AVAILABLE_CHECK;

	if (p != NULL) {
		data = p[(addr & 0xfff) >> 2];
	} else {
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		    sizeof(data), MEM_READ, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
	}
	/*  The load was ok:  */
	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		data = LE32_TO_HOST(data);
	else
		data = BE32_TO_HOST(data);
	reg(ic->arg[1]) = addr + sizeof(data);

	/*  Ugly special case for FPSCR:  */
	if (ic->arg[0] == (size_t)&cpu->cd.sh.fpscr)
		sh_update_fpscr(cpu, data);
	else
		reg(ic->arg[0]) = data;
}
X(mov_b_r0_rm_rn)
{
	uint32_t addr = reg(ic->arg[0]) + cpu->cd.sh.r[0];
	int8_t *p = (int8_t *) cpu->cd.sh.host_load[addr >> 12];
	int8_t data;

	if (p != NULL) {
		data = p[addr & 0xfff];
	} else {
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		    sizeof(data), MEM_READ, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
	}

	reg(ic->arg[1]) = data;
}
X(mov_w_r0_rm_rn)
{
	uint32_t addr = reg(ic->arg[0]) + cpu->cd.sh.r[0];
	int16_t *p = (int16_t *) cpu->cd.sh.host_load[addr >> 12];
	int16_t data;

	if (p != NULL) {
		data = p[(addr & 0xfff) >> 1];
	} else {
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		    sizeof(data), MEM_READ, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
	}

	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		data = LE16_TO_HOST(data);
	else
		data = BE16_TO_HOST(data);
	reg(ic->arg[1]) = data;
}
X(mov_l_r0_rm_rn)
{
	uint32_t addr = reg(ic->arg[0]) + cpu->cd.sh.r[0];
	uint32_t *p = (uint32_t *) cpu->cd.sh.host_load[addr >> 12];
	uint32_t data;

	if (p != NULL) {
		data = p[(addr & 0xfff) >> 2];
	} else {
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		    sizeof(data), MEM_READ, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
	}

	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		data = LE32_TO_HOST(data);
	else
		data = BE32_TO_HOST(data);
	reg(ic->arg[1]) = data;
}
X(mov_l_disp_rm_rn)
{
	uint32_t addr = cpu->cd.sh.r[ic->arg[0] & 0xf] +
	    ((ic->arg[0] >> 4) << 2);
	uint32_t *p = (uint32_t *) cpu->cd.sh.host_load[addr >> 12];
	uint32_t data;

	if (p != NULL) {
		data = p[(addr & 0xfff) >> 2];
	} else {
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		    sizeof(data), MEM_READ, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
	}

	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		data = LE32_TO_HOST(data);
	else
		data = BE32_TO_HOST(data);
	reg(ic->arg[1]) = data;
}
X(mov_b_disp_rn_r0)
{
	uint32_t addr = reg(ic->arg[0]) + ic->arg[1];
	uint8_t *p = (uint8_t *) cpu->cd.sh.host_load[addr >> 12];
	uint8_t data;

	if (p != NULL) {
		data = p[addr & 0xfff];
	} else {
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		    sizeof(data), MEM_READ, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
	}

	cpu->cd.sh.r[0] = (int8_t) data;
}
X(mov_w_disp_rn_r0)
{
	uint32_t addr = reg(ic->arg[0]) + ic->arg[1];
	uint16_t *p = (uint16_t *) cpu->cd.sh.host_load[addr >> 12];
	uint16_t data;

	if (p != NULL) {
		data = p[(addr & 0xfff) >> 1];
	} else {
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		    sizeof(data), MEM_READ, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
	}

	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		data = LE16_TO_HOST(data);
	else
		data = BE16_TO_HOST(data);
	cpu->cd.sh.r[0] = (int16_t) data;
}


/*
 *  mov_b_store_rm_rn:  Store Rm to address Rn (8-bit).
 *  mov_w_store_rm_rn:  Store Rm to address Rn (16-bit).
 *  mov_l_store_rm_rn:  Store Rm to address Rn (32-bit).
 *  fmov_frm_rn:        Store FRm to address Rn.
 *  fmov_frm_r0_rn:     Store FRm to address R0 + Rn.
 *  fmov_frm_predec_rn: Store FRm to address Rn - 4 (or 8), update Rn.
 *  mov_b_rm_r0_rn:     Store Rm to address Rn + R0 (8-bit).
 *  mov_w_rm_r0_rn:     Store Rm to address Rn + R0 (16-bit).
 *  mov_l_rm_r0_rn:     Store Rm to address Rn + R0 (32-bit).
 *  mov_b_r0_disp_gbr:  Store R0 to address disp + GBR (8-bit).
 *  mov_w_r0_disp_gbr:  Store R0 to address disp + GBR (16-bit).
 *  mov_l_r0_disp_gbr:  Store R0 to address disp + GBR (32-bit).
 *  mov_l_rm_disp_rn:   Store Rm to address disp + Rn.
 *  mov_b_r0_disp_rn:   Store R0 to address disp + Rn (8-bit).
 *  mov_w_r0_disp_rn:   Store R0 to address disp + Rn (16-bit).
 *
 *  arg[0] = ptr to rm
 *  arg[1] = ptr to rn    (or  Rn+(disp<<4)  for mov_l_rm_disp_rn)
 *                        (or  disp          for mov_*_r0_disp_gbr)
 */
X(mov_b_store_rm_rn)
{
	uint32_t addr = reg(ic->arg[1]);
	uint8_t *p = (uint8_t *) cpu->cd.sh.host_store[addr >> 12];
	uint8_t data = reg(ic->arg[0]);

	if (p != NULL) {
		p[addr & 0xfff] = data;
	} else {
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, &data,
		    sizeof(data), MEM_WRITE, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
	}
}
X(mov_w_store_rm_rn)
{
	uint32_t addr = reg(ic->arg[1]);
	uint16_t *p = (uint16_t *) cpu->cd.sh.host_store[addr >> 12];
	uint16_t data = reg(ic->arg[0]);

	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		data = LE16_TO_HOST(data);
	else
		data = BE16_TO_HOST(data);

	if (p != NULL) {
		p[(addr & 0xfff) >> 1] = data;
	} else {
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		    sizeof(data), MEM_WRITE, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
	}
}
X(mov_l_store_rm_rn)
{
	uint32_t addr = reg(ic->arg[1]);
	uint32_t *p = (uint32_t *) cpu->cd.sh.host_store[addr >> 12];
	uint32_t data = reg(ic->arg[0]);

	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		data = LE32_TO_HOST(data);
	else
		data = BE32_TO_HOST(data);

	if (p != NULL) {
		p[(addr & 0xfff) >> 2] = data;
	} else {
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		    sizeof(data), MEM_WRITE, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
	}
}
X(fmov_frm_rn)
{
	uint32_t addr = reg(ic->arg[1]);
	uint32_t *p = (uint32_t *) cpu->cd.sh.host_store[addr >> 12];
	uint32_t data = reg(ic->arg[0]);

	FLOATING_POINT_AVAILABLE_CHECK;

	if (cpu->cd.sh.fpscr & SH_FPSCR_SZ) {
		fatal("fmov_frm_rn: sz=1 (register pair): TODO\n");
		exit(1);
	}

	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		data = LE32_TO_HOST(data);
	else
		data = BE32_TO_HOST(data);

	if (p != NULL) {
		p[(addr & 0xfff) >> 2] = data;
	} else {
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		    sizeof(data), MEM_WRITE, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
	}
}
X(fmov_frm_r0_rn)
{
	uint32_t addr = reg(ic->arg[1]) + cpu->cd.sh.r[0];
	uint32_t *p = (uint32_t *) cpu->cd.sh.host_store[addr >> 12];
	uint32_t data = reg(ic->arg[0]);

	FLOATING_POINT_AVAILABLE_CHECK;

	if (cpu->cd.sh.fpscr & SH_FPSCR_SZ) {
		fatal("fmov_frm_r0_rn: sz=1 (register pair): TODO\n");
		exit(1);
	}

	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		data = LE32_TO_HOST(data);
	else
		data = BE32_TO_HOST(data);

	if (p != NULL) {
		p[(addr & 0xfff) >> 2] = data;
	} else {
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		    sizeof(data), MEM_WRITE, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
	}
}
X(fmov_frm_predec_rn)
{
	int d = cpu->cd.sh.fpscr & SH_FPSCR_SZ? 1 : 0;
	uint32_t data, addr = reg(ic->arg[1]) - (d? 8 : 4);
	uint32_t *p = (uint32_t *) cpu->cd.sh.host_store[addr >> 12];
	size_t r0 = ic->arg[0];

	if (d) {
		/*  xd instead of dr?  */
		int ofs0 = (r0 - (size_t)&cpu->cd.sh.fr[0]) / sizeof(uint32_t);
		if (ofs0 & 1)
			r0 = (size_t)&cpu->cd.sh.xf[ofs0 & ~1];
	}

	data = reg(r0);

	FLOATING_POINT_AVAILABLE_CHECK;

	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		data = LE32_TO_HOST(data);
	else
		data = BE32_TO_HOST(data);

	if (p != NULL) {
		p[(addr & 0xfff) >> 2] = data;
	} else {
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		    sizeof(data), MEM_WRITE, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
	}

	if (d) {
		/*  Store second single-precision floating point word:  */
		data = reg(r0 + 4);
		if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
			data = LE32_TO_HOST(data);
		else
			data = BE32_TO_HOST(data);
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr + 4, (unsigned
		    char *)&data, sizeof(data), MEM_WRITE, CACHE_DATA))
			return;
	}

	reg(ic->arg[1]) = addr;
}
X(mov_b_rm_r0_rn)
{
	uint32_t addr = reg(ic->arg[1]) + cpu->cd.sh.r[0];
	int8_t *p = (int8_t *) cpu->cd.sh.host_store[addr >> 12];
	int8_t data = reg(ic->arg[0]);
	if (p != NULL) {
		p[addr & 0xfff] = data;
	} else {
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		    sizeof(data), MEM_WRITE, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
	}
}
X(mov_w_rm_r0_rn)
{
	uint32_t addr = reg(ic->arg[1]) + cpu->cd.sh.r[0];
	uint16_t *p = (uint16_t *) cpu->cd.sh.host_store[addr >> 12];
	uint16_t data = reg(ic->arg[0]);

	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		data = LE16_TO_HOST(data);
	else
		data = BE16_TO_HOST(data);

	if (p != NULL) {
		p[(addr & 0xfff) >> 1] = data;
	} else {
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		    sizeof(data), MEM_WRITE, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
	}
}
X(mov_l_rm_r0_rn)
{
	uint32_t addr = reg(ic->arg[1]) + cpu->cd.sh.r[0];
	uint32_t *p = (uint32_t *) cpu->cd.sh.host_store[addr >> 12];
	uint32_t data = reg(ic->arg[0]);

	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		data = LE32_TO_HOST(data);
	else
		data = BE32_TO_HOST(data);

	if (p != NULL) {
		p[(addr & 0xfff) >> 2] = data;
	} else {
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		    sizeof(data), MEM_WRITE, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
	}
}
X(mov_b_r0_disp_gbr)
{
	uint32_t addr = cpu->cd.sh.gbr + ic->arg[1];
	uint8_t *p = (uint8_t *) cpu->cd.sh.host_store[addr >> 12];
	uint8_t data = cpu->cd.sh.r[0];
	if (p != NULL) {
		p[addr & 0xfff] = data;
	} else {
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		    sizeof(data), MEM_WRITE, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
	}
}
X(mov_w_r0_disp_gbr)
{
	uint32_t addr = cpu->cd.sh.gbr + ic->arg[1];
	uint16_t *p = (uint16_t *) cpu->cd.sh.host_store[addr >> 12];
	uint16_t data = cpu->cd.sh.r[0];

	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		data = LE16_TO_HOST(data);
	else
		data = BE16_TO_HOST(data);

	if (p != NULL) {
		p[(addr & 0xfff) >> 1] = data;
	} else {
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		    sizeof(data), MEM_WRITE, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
	}
}
X(mov_l_r0_disp_gbr)
{
	uint32_t addr = cpu->cd.sh.gbr + ic->arg[1];
	uint32_t *p = (uint32_t *) cpu->cd.sh.host_store[addr >> 12];
	uint32_t data = cpu->cd.sh.r[0];

	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		data = LE32_TO_HOST(data);
	else
		data = BE32_TO_HOST(data);

	if (p != NULL) {
		p[(addr & 0xfff) >> 2] = data;
	} else {
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		    sizeof(data), MEM_WRITE, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
	}
}
X(mov_l_rm_disp_rn)
{
	uint32_t addr = cpu->cd.sh.r[ic->arg[1] & 0xf] +
	    ((ic->arg[1] >> 4) << 2);
	uint32_t *p = (uint32_t *) cpu->cd.sh.host_store[addr >> 12];
	uint32_t data = reg(ic->arg[0]);

	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		data = LE32_TO_HOST(data);
	else
		data = BE32_TO_HOST(data);

	if (p != NULL) {
		p[(addr & 0xfff) >> 2] = data;
	} else {
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		    sizeof(data), MEM_WRITE, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
	}
}
X(mov_b_r0_disp_rn)
{
	uint32_t addr = reg(ic->arg[0]) + ic->arg[1];
	uint8_t *p = (uint8_t *) cpu->cd.sh.host_store[addr >> 12];
	uint8_t data = cpu->cd.sh.r[0];

	if (p != NULL) {
		p[addr & 0xfff] = data;
	} else {
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		    sizeof(data), MEM_WRITE, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
	}
}
X(mov_w_r0_disp_rn)
{
	uint32_t addr = reg(ic->arg[0]) + ic->arg[1];
	uint16_t *p = (uint16_t *) cpu->cd.sh.host_store[addr >> 12];
	uint16_t data = cpu->cd.sh.r[0];

	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		data = LE16_TO_HOST(data);
	else
		data = BE16_TO_HOST(data);

	if (p != NULL) {
		p[(addr & 0xfff) >> 1] = data;
	} else {
		SYNCH_PC;
		if (!cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *)&data,
		    sizeof(data), MEM_WRITE, CACHE_DATA)) {
			/*  Exception.  */
			return;
		}
	}
}


/*
 *  add_rm_rn:  rn = rn + rm
 *  addc_rm_rn: rn = rn + rm + t
 *  and_rm_rn:  rn = rn & rm
 *  xor_rm_rn:  rn = rn ^ rm
 *  or_rm_rn:   rn = rn | rm
 *  sub_rm_rn:  rn = rn - rm
 *  subc_rm_rn: rn = rn - rm - t; t = borrow
 *  tst_rm_rn:  t = ((rm & rn) == 0)
 *  tst_rm:     t = (rm == 0)
 *  xtrct_rm_rn:  rn = (rn >> 16) | (rm << 16)
 *
 *  arg[0] = ptr to rm
 *  arg[1] = ptr to rn
 */
X(add_rm_rn) { reg(ic->arg[1]) += reg(ic->arg[0]); }
X(addc_rm_rn)
{
	uint64_t res = reg(ic->arg[1]);
	res += (uint64_t) reg(ic->arg[0]);
	if (cpu->cd.sh.sr & SH_SR_T)
		res ++;
	if ((res >> 32) & 1)
		cpu->cd.sh.sr |= SH_SR_T;
	else
		cpu->cd.sh.sr &= ~SH_SR_T;
	reg(ic->arg[1]) = (uint32_t) res;
}
X(and_rm_rn) { reg(ic->arg[1]) &= reg(ic->arg[0]); }
X(xor_rm_rn) { reg(ic->arg[1]) ^= reg(ic->arg[0]); }
X(or_rm_rn)  { reg(ic->arg[1]) |= reg(ic->arg[0]); }
X(sub_rm_rn) { reg(ic->arg[1]) -= reg(ic->arg[0]); }
X(subc_rm_rn)
{
	uint64_t res = reg(ic->arg[1]);
	res -= (uint64_t) reg(ic->arg[0]);
	if (cpu->cd.sh.sr & SH_SR_T)
		res --;
	if ((res >> 32) & 1)
		cpu->cd.sh.sr |= SH_SR_T;
	else
		cpu->cd.sh.sr &= ~SH_SR_T;
	reg(ic->arg[1]) = (uint32_t) res;
}
X(tst_rm_rn)
{
	if (reg(ic->arg[1]) & reg(ic->arg[0]))
		cpu->cd.sh.sr &= ~SH_SR_T;
	else
		cpu->cd.sh.sr |= SH_SR_T;
}
X(tst_rm)
{
	if (reg(ic->arg[0]))
		cpu->cd.sh.sr &= ~SH_SR_T;
	else
		cpu->cd.sh.sr |= SH_SR_T;
}
X(xtrct_rm_rn)
{
	uint32_t rn = reg(ic->arg[1]), rm = reg(ic->arg[0]);
	reg(ic->arg[1]) = (rn >> 16) | (rm << 16);
}


/*
 *  div0u:       Division step 0; prepare for unsigned division.
 *  div0s_rm_rn: Division step 0; prepare for signed division.
 *  div1_rm_rn:  Division step 1.
 *
 *  arg[0] = ptr to rm
 *  arg[1] = ptr to rn
 */
X(div0u)
{
	cpu->cd.sh.sr &= ~(SH_SR_Q | SH_SR_M | SH_SR_T);
}
X(div0s_rm_rn)
{
	int q = reg(ic->arg[1]) & 0x80000000;
	int m = reg(ic->arg[0]) & 0x80000000;
	uint32_t new_sr = cpu->cd.sh.sr & ~(SH_SR_Q | SH_SR_M | SH_SR_T);
	if (q)
		new_sr |= SH_SR_Q;
	if (m)
		new_sr |= SH_SR_M;
	if (m ^ q)
		new_sr |= SH_SR_T;
	cpu->cd.sh.sr = new_sr;
}
X(div1_rm_rn)
{
	uint32_t q, old_q = (cpu->cd.sh.sr & SH_SR_Q)? 1 : 0;
	uint32_t m = (cpu->cd.sh.sr & SH_SR_M)? 1 : 0;
	uint32_t t = (cpu->cd.sh.sr & SH_SR_T)? 1 : 0;
	uint32_t op1 = reg(ic->arg[0]), op2 = reg(ic->arg[1]);
	uint64_t op2_64;

	q = op2 >> 31;
	op2_64 = (uint32_t) ((op2 << 1) + t);
	if (old_q == m)
		op2_64 -= (uint64_t)op1;
	else
		op2_64 += (uint64_t)op1;
	q ^= m ^ ((op2_64 >> 32) & 1);
	t = 1 - (q ^ m);
	cpu->cd.sh.sr &= ~(SH_SR_Q | SH_SR_T);
	if (q)
		cpu->cd.sh.sr |= SH_SR_Q;
	if (t)
		cpu->cd.sh.sr |= SH_SR_T;
	reg(ic->arg[1]) = (uint32_t) op2_64;
}


/*
 *  mul_l_rm_rn:   MACL = Rm * Rn       (32-bit)
 *  muls_w_rm_rn:  MACL = Rm * Rn       (signed 16-bit * 16-bit ==> 32-bit)
 *  mulu_w_rm_rn:  MACL = Rm * Rn       (unsigned 16-bit * 16-bit ==> 32-bit)
 *  dmuls_l_rm_rn: MACH:MACL = Rm * Rn  (signed, 64-bit result)
 *  dmulu_l_rm_rn: MACH:MACL = Rm * Rn  (unsigned, 64-bit result)
 *
 *  arg[0] = ptr to rm
 *  arg[1] = ptr to rn
 */
X(mul_l_rm_rn)
{
	cpu->cd.sh.macl = reg(ic->arg[0]) * reg(ic->arg[1]);
}
X(muls_w_rm_rn)
{
	cpu->cd.sh.macl = (int32_t)(int16_t)reg(ic->arg[0]) *
	    (int32_t)(int16_t)reg(ic->arg[1]);
}
X(mulu_w_rm_rn)
{
	cpu->cd.sh.macl = (int32_t)(uint16_t)reg(ic->arg[0]) *
	    (int32_t)(uint16_t)reg(ic->arg[1]);
}
X(dmuls_l_rm_rn)
{
	uint64_t rm = (int32_t)reg(ic->arg[0]), rn = (int32_t)reg(ic->arg[1]);
	uint64_t res = rm * rn;
	cpu->cd.sh.mach = (uint32_t) (res >> 32);
	cpu->cd.sh.macl = (uint32_t) res;
}
X(dmulu_l_rm_rn)
{
	uint64_t rm = reg(ic->arg[0]), rn = reg(ic->arg[1]), res;
	res = rm * rn;
	cpu->cd.sh.mach = (uint32_t) (res >> 32);
	cpu->cd.sh.macl = (uint32_t) res;
}


/*
 *  cmpeq_imm_r0:  rn == int8_t immediate
 *  cmpeq_rm_rn:   rn == rm
 *  cmphs_rm_rn:   rn >= rm, unsigned
 *  cmpge_rm_rn:   rn >= rm, signed
 *  cmphi_rm_rn:   rn > rm, unsigned
 *  cmpgt_rm_rn:   rn > rm, signed
 *  cmppz_rn:      rn >= 0, signed
 *  cmppl_rn:      rn > 0, signed
 *  cmp_str_rm_rn: t=1 if any bytes in rm and rn match, 0 otherwise
 *
 *  arg[0] = ptr to rm   (or imm, for cmpeq_imm_r0)
 *  arg[1] = ptr to rn
 */
X(cmpeq_imm_r0)
{
	if (cpu->cd.sh.r[0] == (uint32_t)ic->arg[0])
		cpu->cd.sh.sr |= SH_SR_T;
	else
		cpu->cd.sh.sr &= ~SH_SR_T;
}
X(cmpeq_rm_rn)
{
	if (reg(ic->arg[1]) == reg(ic->arg[0]))
		cpu->cd.sh.sr |= SH_SR_T;
	else
		cpu->cd.sh.sr &= ~SH_SR_T;
}
X(cmphs_rm_rn)
{
	if (reg(ic->arg[1]) >= reg(ic->arg[0]))
		cpu->cd.sh.sr |= SH_SR_T;
	else
		cpu->cd.sh.sr &= ~SH_SR_T;
}
X(cmpge_rm_rn)
{
	if ((int32_t)reg(ic->arg[1]) >= (int32_t)reg(ic->arg[0]))
		cpu->cd.sh.sr |= SH_SR_T;
	else
		cpu->cd.sh.sr &= ~SH_SR_T;
}
X(cmphi_rm_rn)
{
	if (reg(ic->arg[1]) > reg(ic->arg[0]))
		cpu->cd.sh.sr |= SH_SR_T;
	else
		cpu->cd.sh.sr &= ~SH_SR_T;
}
X(cmpgt_rm_rn)
{
	if ((int32_t)reg(ic->arg[1]) > (int32_t)reg(ic->arg[0]))
		cpu->cd.sh.sr |= SH_SR_T;
	else
		cpu->cd.sh.sr &= ~SH_SR_T;
}
X(cmppz_rn)
{
	if ((int32_t)reg(ic->arg[1]) >= 0)
		cpu->cd.sh.sr |= SH_SR_T;
	else
		cpu->cd.sh.sr &= ~SH_SR_T;
}
X(cmppl_rn)
{
	if ((int32_t)reg(ic->arg[1]) > 0)
		cpu->cd.sh.sr |= SH_SR_T;
	else
		cpu->cd.sh.sr &= ~SH_SR_T;
}
X(cmp_str_rm_rn)
{
	uint32_t r0 = reg(ic->arg[0]), r1 = reg(ic->arg[1]);
	int t = 0;
	if ((r0 & 0xff000000) == (r1 & 0xff000000))
		t = 1;
	else if ((r0 & 0xff0000) == (r1 & 0xff0000))
		t = 1;
	else if ((r0 & 0xff00) == (r1 & 0xff00))
		t = 1;
	else if ((r0 & 0xff) == (r1 & 0xff))
		t = 1;
	if (t)
		cpu->cd.sh.sr |= SH_SR_T;
	else
		cpu->cd.sh.sr &= ~SH_SR_T;
}


/*
 *  shll_rn:  Shift rn left by 1  (t = bit that was shifted out)
 *  shlr_rn:  Shift rn right by 1 (t = bit that was shifted out)
 *  rotl_rn:  Shift rn left by 1  (t = bit that was shifted out)
 *  rotr_rn:  Shift rn right by 1 (t = bit that was shifted out)
 *  shar_rn:  Shift rn right arithmetically by 1 (t = bit that was shifted out)
 *  shllX_rn: Shift rn left logically by X bits
 *  shlrX_rn: Shift rn right logically by X bits
 *  rotcl_rn: Rotate rn left via the t bit
 *  rotcr_rn: Rotate rn right via the t bit
 *  dt_rn:    Decrease rn; t = (rn == 0)
 *
 *  arg[1] = ptr to rn
 */
X(shll_rn)
{
	uint32_t rn = reg(ic->arg[1]);
	if (rn & 0x80000000)
		cpu->cd.sh.sr |= SH_SR_T;
	else
		cpu->cd.sh.sr &= ~SH_SR_T;
	reg(ic->arg[1]) = rn << 1;
}
X(shlr_rn)
{
	uint32_t rn = reg(ic->arg[1]);
	if (rn & 1)
		cpu->cd.sh.sr |= SH_SR_T;
	else
		cpu->cd.sh.sr &= ~SH_SR_T;
	reg(ic->arg[1]) = rn >> 1;
}
X(rotl_rn)
{
	uint32_t rn = reg(ic->arg[1]), x;
	if (rn & 0x80000000) {
		x = 1;
		cpu->cd.sh.sr |= SH_SR_T;
	} else {
		x = 0;
		cpu->cd.sh.sr &= ~SH_SR_T;
	}
	reg(ic->arg[1]) = (rn << 1) | x;
}
X(rotr_rn)
{
	uint32_t rn = reg(ic->arg[1]);
	if (rn & 1)
		cpu->cd.sh.sr |= SH_SR_T;
	else
		cpu->cd.sh.sr &= ~SH_SR_T;
	reg(ic->arg[1]) = (rn >> 1) | (rn << 31);
}
X(shar_rn)
{
	int32_t rn = reg(ic->arg[1]);
	if (rn & 1)
		cpu->cd.sh.sr |= SH_SR_T;
	else
		cpu->cd.sh.sr &= ~SH_SR_T;
	reg(ic->arg[1]) = rn >> 1;
}
X(rotcl_rn)
{
	uint32_t rn = reg(ic->arg[1]), top;
	top = rn & 0x80000000;
	rn <<= 1;
	if (cpu->cd.sh.sr & SH_SR_T)
		rn ++;
	if (top)
		cpu->cd.sh.sr |= SH_SR_T;
	else
		cpu->cd.sh.sr &= ~SH_SR_T;
	reg(ic->arg[1]) = rn;
}
X(rotcr_rn)
{
	uint32_t rn = reg(ic->arg[1]), bottom;
	bottom = rn & 1;
	rn >>= 1;
	if (cpu->cd.sh.sr & SH_SR_T)
		rn |= 0x80000000;
	if (bottom)
		cpu->cd.sh.sr |= SH_SR_T;
	else
		cpu->cd.sh.sr &= ~SH_SR_T;
	reg(ic->arg[1]) = rn;
}
X(dt_rn)
{
	uint32_t rn = reg(ic->arg[1]) - 1;
	if (rn == 0)
		cpu->cd.sh.sr |= SH_SR_T;
	else
		cpu->cd.sh.sr &= ~SH_SR_T;
	reg(ic->arg[1]) = rn;
}
X(shll2_rn) { reg(ic->arg[1]) <<= 2; }
X(shll8_rn) { reg(ic->arg[1]) <<= 8; }
X(shll16_rn) { reg(ic->arg[1]) <<= 16; }
X(shlr2_rn) { reg(ic->arg[1]) >>= 2; }
X(shlr8_rn) { reg(ic->arg[1]) >>= 8; }
X(shlr16_rn) { reg(ic->arg[1]) >>= 16; }


/*
 *  shad: Shift Rn arithmetic left/right, as indicated by Rm. Result in Rn.
 *  shld: Shift Rn logically left/right, as indicated by Rm. Result in Rn.
 *
 *  arg[0] = ptr to rm
 *  arg[1] = ptr to rn
 */
X(shad)
{
	int32_t rn = reg(ic->arg[1]);
	int32_t rm = reg(ic->arg[0]);
	int sa = rm & 0x1f;

	if (rm >= 0)
		rn <<= sa;
	else if (sa != 0)
		rn >>= (32 - sa);
	else if (rn < 0)
		rn = -1;
	else
		rn = 0;

	reg(ic->arg[1]) = rn;
}
X(shld)
{
	uint32_t rn = reg(ic->arg[1]);
	int32_t rm = reg(ic->arg[0]);
	int sa = rm & 0x1f;

	if (rm >= 0)
		rn <<= sa;
	else if (sa != 0)
		rn >>= (32 - sa);
	else
		rn = 0;

	reg(ic->arg[1]) = rn;
}


/*
 *  bra:   Branch using PC relative immediace displacement (with delay-slot)
 *  bsr:   Like bra, but also sets PR to the return address
 *  braf:  Like bra, but using a register instead of an immediate
 *  bsrf:  Like braf, but also sets PR to the return address
 *
 *  arg[0] = immediate offset relative to start of page,
 *           or ptr to target instruction, for samepage branches
 *  arg[1] = ptr to Rn  (for braf/bsrf)
 */
X(bra)
{
	MODE_int_t target = cpu->pc & ~((SH_IC_ENTRIES_PER_PAGE-1) <<
	    SH_INSTR_ALIGNMENT_SHIFT);
	target += ic->arg[0];
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		cpu->pc = target;
		cpu->delay_slot = NOT_DELAYED;
		quick_pc_to_pointers(cpu);
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(bra_samepage)
{
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT))
		cpu->cd.sh.next_ic = (struct sh_instr_call *) ic->arg[0];
	cpu->delay_slot = NOT_DELAYED;
}
X(bsr)
{
	MODE_int_t target = cpu->pc & ~((SH_IC_ENTRIES_PER_PAGE-1) <<
	    SH_INSTR_ALIGNMENT_SHIFT);
	uint32_t old_pc;
	SYNCH_PC;
	old_pc = cpu->pc;
	target += ic->arg[0];
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		cpu->cd.sh.pr = old_pc + 4;
		cpu->pc = target;
		cpu->delay_slot = NOT_DELAYED;
		quick_pc_to_pointers(cpu);
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(bsr_samepage)
{
	uint32_t old_pc;
	SYNCH_PC;
	old_pc = cpu->pc;
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		cpu->cd.sh.pr = old_pc + 4;
		cpu->cd.sh.next_ic = (struct sh_instr_call *) ic->arg[0];
	}
	cpu->delay_slot = NOT_DELAYED;
}
X(braf_rn)
{
	MODE_int_t target = cpu->pc & ~((SH_IC_ENTRIES_PER_PAGE-1) <<
	    SH_INSTR_ALIGNMENT_SHIFT);
	target += ic->arg[0] + reg(ic->arg[1]);
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		cpu->pc = target;
		cpu->delay_slot = NOT_DELAYED;
		quick_pc_to_pointers(cpu);
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(bsrf_rn)
{
	MODE_int_t target = cpu->pc & ~((SH_IC_ENTRIES_PER_PAGE-1) <<
	    SH_INSTR_ALIGNMENT_SHIFT);
	uint32_t old_pc;
	SYNCH_PC;
	old_pc = cpu->pc;
	target += ic->arg[0] + reg(ic->arg[1]);
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		cpu->cd.sh.pr = old_pc + 4;
		cpu->pc = target;
		cpu->delay_slot = NOT_DELAYED;
		quick_pc_to_pointers(cpu);
	} else
		cpu->delay_slot = NOT_DELAYED;
}


/*
 *  bt: Branch if true
 *  bf: Branch if false
 *  bt/s: Branch if true (with delay-slot)
 *  bf/s: Branch if false (with delay-slot)
 *
 *  arg[0] = immediate offset relative to start of page
 *  arg[1] = for samepage functions, the new instruction pointer
 */
X(bt)
{
	if (cpu->cd.sh.sr & SH_SR_T) {
		cpu->pc &= ~((SH_IC_ENTRIES_PER_PAGE-1) <<
		    SH_INSTR_ALIGNMENT_SHIFT);
		cpu->pc += ic->arg[0];
		quick_pc_to_pointers(cpu);
	}
}
X(bf)
{
	if (!(cpu->cd.sh.sr & SH_SR_T)) {
		cpu->pc &= ~((SH_IC_ENTRIES_PER_PAGE-1) <<
		    SH_INSTR_ALIGNMENT_SHIFT);
		cpu->pc += ic->arg[0];
		quick_pc_to_pointers(cpu);
	}
}
X(bt_samepage)
{
	if (cpu->cd.sh.sr & SH_SR_T)
		cpu->cd.sh.next_ic = (struct sh_instr_call *) ic->arg[1];
}
X(bf_samepage)
{
	if (!(cpu->cd.sh.sr & SH_SR_T))
		cpu->cd.sh.next_ic = (struct sh_instr_call *) ic->arg[1];
}
X(bt_s)
{
	MODE_int_t target = cpu->pc & ~((SH_IC_ENTRIES_PER_PAGE-1) <<
	    SH_INSTR_ALIGNMENT_SHIFT);
	int cond = cpu->cd.sh.sr & SH_SR_T;
	target += ic->arg[0];
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		cpu->delay_slot = NOT_DELAYED;
		if (cond) {
			cpu->pc = target;
			quick_pc_to_pointers(cpu);
		} else
			cpu->cd.sh.next_ic ++;
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(bf_s)
{
	MODE_int_t target = cpu->pc & ~((SH_IC_ENTRIES_PER_PAGE-1) <<
	    SH_INSTR_ALIGNMENT_SHIFT);
	int cond = !(cpu->cd.sh.sr & SH_SR_T);
	target += ic->arg[0];
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		cpu->delay_slot = NOT_DELAYED;
		if (cond) {
			cpu->pc = target;
			quick_pc_to_pointers(cpu);
		} else
			cpu->cd.sh.next_ic ++;
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(bt_s_samepage)
{
	int cond = cpu->cd.sh.sr & SH_SR_T;
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		cpu->delay_slot = NOT_DELAYED;
		if (cond)
			cpu->cd.sh.next_ic =
			    (struct sh_instr_call *) ic->arg[1];
		else
			cpu->cd.sh.next_ic ++;
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(bf_s_samepage)
{
	int cond = !(cpu->cd.sh.sr & SH_SR_T);
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		cpu->delay_slot = NOT_DELAYED;
		if (cond)
			cpu->cd.sh.next_ic =
			    (struct sh_instr_call *) ic->arg[1];
		else
			cpu->cd.sh.next_ic ++;
	} else
		cpu->delay_slot = NOT_DELAYED;
}


/*
 *  jmp_rn: Jump to Rn
 *  jsr_rn: Jump to Rn, store return address in PR.
 *
 *  arg[0] = ptr to rn
 */
X(jmp_rn)
{
	MODE_int_t target = reg(ic->arg[0]);
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		cpu->pc = target;
		cpu->delay_slot = NOT_DELAYED;
		quick_pc_to_pointers(cpu);
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(jmp_rn_trace)
{
	MODE_int_t target = reg(ic->arg[0]);
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		cpu->pc = target;
#if 0
		/*  NOTE: Jmp works like both a return, and a subroutine
		    call.  */
		cpu_functioncall_trace_return(cpu);
		cpu_functioncall_trace(cpu, cpu->pc);
#endif
		cpu->delay_slot = NOT_DELAYED;
		quick_pc_to_pointers(cpu);
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(jsr_rn)
{
	MODE_int_t target = reg(ic->arg[0]), retaddr;
	cpu->delay_slot = TO_BE_DELAYED;
	retaddr = cpu->pc & ~((SH_IC_ENTRIES_PER_PAGE-1) <<
	    SH_INSTR_ALIGNMENT_SHIFT);
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	cpu->cd.sh.pr = retaddr + (int32_t)ic->arg[1];
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		cpu->pc = target;
		cpu->delay_slot = NOT_DELAYED;
		quick_pc_to_pointers(cpu);
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(jsr_rn_trace)
{
	MODE_int_t target = reg(ic->arg[0]), retaddr;
	cpu->delay_slot = TO_BE_DELAYED;
	retaddr = cpu->pc & ~((SH_IC_ENTRIES_PER_PAGE-1) <<
	    SH_INSTR_ALIGNMENT_SHIFT);
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	cpu->cd.sh.pr = retaddr + (int32_t)ic->arg[1];
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		cpu->pc = target;
		cpu_functioncall_trace(cpu, cpu->pc);
		cpu->delay_slot = NOT_DELAYED;
		quick_pc_to_pointers(cpu);
	} else
		cpu->delay_slot = NOT_DELAYED;
}


/*
 *  rts: Jump to PR.
 */
X(rts)
{
	MODE_int_t target = cpu->cd.sh.pr;
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		cpu->pc = target;
		cpu->delay_slot = NOT_DELAYED;
		quick_pc_to_pointers(cpu);
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(rts_trace)
{
	MODE_int_t target = cpu->cd.sh.pr;
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		cpu->pc = target;
		cpu_functioncall_trace_return(cpu);
		cpu->delay_slot = NOT_DELAYED;
		quick_pc_to_pointers(cpu);
	} else
		cpu->delay_slot = NOT_DELAYED;
}


/*
 *  rte:  Return from exception.
 */
X(rte)
{
	RES_INST_IF_NOT_MD;

	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		cpu->pc = cpu->cd.sh.spc;
		cpu->delay_slot = NOT_DELAYED;
		sh_update_sr(cpu, cpu->cd.sh.ssr);
		quick_pc_to_pointers(cpu);
	} else
		cpu->delay_slot = NOT_DELAYED;
}


/*
 *  ldtlb:  Load UTLB entry.
 */
X(ldtlb)
{
	uint32_t old_hi, old_lo;
	int urc = (cpu->cd.sh.mmucr & SH4_MMUCR_URC_MASK)
	    >> SH4_MMUCR_URC_SHIFT;

	RES_INST_IF_NOT_MD;

	old_hi = cpu->cd.sh.utlb_hi[urc];
	old_lo = cpu->cd.sh.utlb_lo[urc];

	cpu->cd.sh.utlb_hi[urc] = cpu->cd.sh.pteh;
	cpu->cd.sh.utlb_lo[urc] = cpu->cd.sh.ptel;

	/*  Invalidate the old mapping, if it belonged to the same ASID:  */
	if ((old_hi & SH4_PTEH_ASID_MASK) ==
	    (cpu->cd.sh.pteh & SH4_PTEH_ASID_MASK)) {
		if ((old_lo & SH4_PTEL_SZ_MASK) == SH4_PTEL_SZ_4K)
			cpu->invalidate_translation_caches(cpu,
			    old_hi & 0xfffff000, INVALIDATE_VADDR);
		else
			cpu->invalidate_translation_caches(cpu,
			    0, INVALIDATE_ALL);
	}
}


/*
 *  copy_privileged_register: Copy normal into privileged register, or vice
 *                            versa, after checking the MD status bit.
 *
 *  arg[0] = ptr to source register
 *  arg[1] = ptr to destination register
 */
X(copy_privileged_register)
{
	RES_INST_IF_NOT_MD;
	reg(ic->arg[1]) = reg(ic->arg[0]);
}


/*
 *  ldc_rm_sr:      Copy Rm into SR, after checking the MD status bit.
 *
 *  arg[1] = ptr to rm
 */
X(ldc_rm_sr)
{
	RES_INST_IF_NOT_MD;
	sh_update_sr(cpu, reg(ic->arg[1]));

#if 0
/*  NOTE: This code causes NetBSD/landisk to get past a point where it
    otherwise hangs, but it causes Linux/Dreamcast to bug out instead. :/  */

	if (!(cpu->cd.sh.sr & SH_SR_BL) && cpu->cd.sh.int_to_assert > 0 &&
	    ( (cpu->cd.sh.sr & SH_SR_IMASK) >> SH_SR_IMASK_SHIFT)
	    < cpu->cd.sh.int_level) {
		/*  Cause interrupt immediately, by dropping out of the
		    main dyntrans loop:  */
		cpu->cd.sh.next_ic = &nothing_call;
	}
#endif
}


/*
 *  trapa:  Immediate trap.
 *
 *  arg[0] = imm << 2
 */
X(trapa)
{
	SYNCH_PC;

	if (cpu->delay_slot) {
		sh_exception(cpu, EXPEVT_SLOT_INST, 0, 0);
		return;
	}

	cpu->cd.sh.tra = ic->arg[0];
	sh_exception(cpu, EXPEVT_TRAPA, 0, 0);
}


/*
 *  copy_fp_register:   Copy a register into another, with FP avail check.
 *  lds_rm_fpscr:       Copy Rm into FPSCR.
 *
 *  arg[0] = ptr to source
 *  arg[1] = ptr to destination
 */
X(copy_fp_register)
{
	FLOATING_POINT_AVAILABLE_CHECK;
	reg(ic->arg[1]) = reg(ic->arg[0]);
}
X(lds_rm_fpscr)
{
	FLOATING_POINT_AVAILABLE_CHECK;
	sh_update_fpscr(cpu, reg(ic->arg[1]));
}


/*
 *  fmov_frm_frn:  Copy one floating-point register (or pair) to another.
 *
 *  arg[0] = ptr to source float register or pair
 *  arg[1] = ptr to destination float register or pair
 */
X(fmov_frm_frn)
{
	size_t r0, r1;
	int ofs0, ofs1;

	FLOATING_POINT_AVAILABLE_CHECK;

	/*  Simplest case, single-precision:  */
	if (!(cpu->cd.sh.fpscr & SH_FPSCR_SZ)) {
		reg(ic->arg[1]) = reg(ic->arg[0]);
		return;
	}

	/*  Double-precision:  */
	r0 = ic->arg[0]; r1 = ic->arg[1];
	ofs0 = (r0 - (size_t)&cpu->cd.sh.fr[0]) / sizeof(uint32_t);
	ofs1 = (r1 - (size_t)&cpu->cd.sh.fr[0]) / sizeof(uint32_t);
	if (ofs0 & 1)
		r0 = (size_t)&cpu->cd.sh.xf[ofs0 & ~1];
	if (ofs1 & 1)
		r1 = (size_t)&cpu->cd.sh.xf[ofs1 & ~1];

	reg(r1) = reg(r0);
	reg(r1 + 4) = reg(r0 + 4);
}


/*
 *  float_fpul_frn:  Load FPUL into float register.
 *
 *  arg[0] = ptr to float register, or float register pair
 */
X(float_fpul_frn)
{
	int32_t fpul = cpu->cd.sh.fpul;

	FLOATING_POINT_AVAILABLE_CHECK;

	if (cpu->cd.sh.fpscr & SH_FPSCR_PR) {
		/*  Double-precision, using a pair of registers:  */
		uint64_t ieee = ieee_store_float_value(fpul, IEEE_FMT_D, 0);
		reg(ic->arg[0]) = (uint32_t) (ieee >> 32);
		reg(ic->arg[0] + sizeof(uint32_t)) = (uint32_t) ieee;
	} else {
		/*  Single-precision:  */
		uint32_t ieee = ieee_store_float_value(fpul, IEEE_FMT_S, 0);
		reg(ic->arg[0]) = (uint32_t) ieee;
	}
}


/*
 *  ftrc_frm_fpul:  Truncate a float register into FPUL.
 *
 *  arg[0] = ptr to float register, or float register pair
 */
X(ftrc_frm_fpul)
{
	struct ieee_float_value op1;

	FLOATING_POINT_AVAILABLE_CHECK;

	if (cpu->cd.sh.fpscr & SH_FPSCR_PR) {
		/*  Double-precision, using a pair of registers:  */
		int64_t r1 = ((uint64_t)reg(ic->arg[0]) << 32) +
		    reg(ic->arg[0] + sizeof(uint32_t));
		ieee_interpret_float_value(r1, &op1, IEEE_FMT_D);
		cpu->cd.sh.fpul = (int32_t) op1.f;
	} else {
		/*  Single-precision:  */
		ieee_interpret_float_value(reg(ic->arg[0]), &op1, IEEE_FMT_S);
		cpu->cd.sh.fpul = (int32_t) op1.f;
	}
}


/*
 *  fcnvsd_fpul_drn:  Convert single-precision to double-precision.
 *  fcnvds_drm_fpul:  Convert double-precision to single-precision.
 *
 *  arg[0] = ptr to destination (double- or single-precision float)
 */
X(fcnvsd_fpul_drn)
{
	struct ieee_float_value op1;
	int64_t ieee;

	FLOATING_POINT_AVAILABLE_CHECK;

	ieee_interpret_float_value(cpu->cd.sh.fpul, &op1, IEEE_FMT_S);
	cpu->cd.sh.fpul = (int32_t) op1.f;

	/*  Store double-precision result:  */
	ieee = ieee_store_float_value(op1.f, IEEE_FMT_D, 0);
	reg(ic->arg[0]) = (uint32_t) (ieee >> 32);
	reg(ic->arg[0] + sizeof(uint32_t)) = (uint32_t) ieee;
}
X(fcnvds_drm_fpul)
{
	struct ieee_float_value op1;
	int64_t r1;

	FLOATING_POINT_AVAILABLE_CHECK;

	r1 = reg(ic->arg[0] + sizeof(uint32_t)) +
	    ((uint64_t)reg(ic->arg[0]) << 32);
	ieee_interpret_float_value(r1, &op1, IEEE_FMT_D);

	cpu->cd.sh.fpul = ieee_store_float_value(op1.f, IEEE_FMT_S, 0);
}


/*
 *  fsca_fpul_drn:  Sinus/cosinus approximation.
 *
 *  Note: This is an interesting instruction. It is not included in the SH4
 *  manual. Some googling indicated that this might be an SH4X instruction.
 *  On the other hand, it is used by Dreamcast code (and the Dreamcast has an
 *  SH4), and a cvs comment for gdb said that this is an SH4 instruction, not
 *  an SH4A instruction. Well well...
 *
 *  arg[0] = ptr to single-precision float register pair
 */
X(fsca_fpul_drn)
{
	double fpul = ((double) (int32_t)cpu->cd.sh.fpul) / 32768.0;

	FLOATING_POINT_AVAILABLE_CHECK;

	reg(ic->arg[0]) = ieee_store_float_value(sin(fpul), IEEE_FMT_S, 0);
	reg(ic->arg[0] + sizeof(uint32_t)) =
	    ieee_store_float_value(cos(fpul), IEEE_FMT_S, 0);
}


/*
 *  fipr_fvm_fvn:  Vector * vector  =>  vector
 *
 *  arg[0] = ptr to FVm
 *  arg[1] = ptr to FVn
 *
 *  Result of adding all   FR{m+i} * FR{n+i}  where i=0..3
 *  is stored in FR{n+3}.
 */
X(fipr_fvm_fvn)
{
	struct ieee_float_value frn0, frn1, frn2, frn3;
	struct ieee_float_value frm0, frm1, frm2, frm3;

	FLOATING_POINT_AVAILABLE_CHECK;

	ieee_interpret_float_value(reg(ic->arg[0] + 0), &frm0, IEEE_FMT_S);
	ieee_interpret_float_value(reg(ic->arg[0] + 4), &frm1, IEEE_FMT_S);
	ieee_interpret_float_value(reg(ic->arg[0] + 8), &frm2, IEEE_FMT_S);
	ieee_interpret_float_value(reg(ic->arg[0] + 12), &frm3, IEEE_FMT_S);
	ieee_interpret_float_value(reg(ic->arg[1] + 0), &frn0, IEEE_FMT_S);
	ieee_interpret_float_value(reg(ic->arg[1] + 4), &frn1, IEEE_FMT_S);
	ieee_interpret_float_value(reg(ic->arg[1] + 8), &frn2, IEEE_FMT_S);
	ieee_interpret_float_value(reg(ic->arg[1] + 12), &frn3, IEEE_FMT_S);

	frn3.f =
	    frm0.f * frn0.f + frm1.f * frn1.f +
	    frm2.f * frn2.f + frm3.f * frn3.f;

	reg(ic->arg[1] + 12) = ieee_store_float_value(frn3.f, IEEE_FMT_S, 0);
}


/*
 *  ftrv_xmtrx_fvn:  Matrix * vector  ==>  vector
 *
 *  arg[0] = ptr to FVn
 */
X(ftrv_xmtrx_fvn)
{
	int i;
	struct ieee_float_value xmtrx[16], frn[4];
	double frnp0 = 0.0, frnp1 = 0.0, frnp2 = 0.0, frnp3 = 0.0;

	ieee_interpret_float_value(reg(ic->arg[0] + 0), &frn[0], IEEE_FMT_S);
	ieee_interpret_float_value(reg(ic->arg[0] + 4), &frn[1], IEEE_FMT_S);
	ieee_interpret_float_value(reg(ic->arg[0] + 8), &frn[2], IEEE_FMT_S);
	ieee_interpret_float_value(reg(ic->arg[0] + 12), &frn[3], IEEE_FMT_S);

	for (i=0; i<16; i++)
		ieee_interpret_float_value(cpu->cd.sh.xf[i],
		    &xmtrx[i], IEEE_FMT_S);

	for (i=0; i<4; i++)
		frnp0 += xmtrx[i*4].f * frn[i].f;

	for (i=0; i<4; i++)
		frnp1 += xmtrx[i*4 + 1].f * frn[i].f;

	for (i=0; i<4; i++)
		frnp2 += xmtrx[i*4 + 2].f * frn[i].f;

	for (i=0; i<4; i++)
		frnp3 += xmtrx[i*4 + 3].f * frn[i].f;

	reg(ic->arg[0] + 0) = ieee_store_float_value(frnp0, IEEE_FMT_S, 0);
	reg(ic->arg[0] + 4) = ieee_store_float_value(frnp1, IEEE_FMT_S, 0);
	reg(ic->arg[0] + 8) = ieee_store_float_value(frnp2, IEEE_FMT_S, 0);
	reg(ic->arg[0] + 12) = ieee_store_float_value(frnp3, IEEE_FMT_S, 0);
}


/*
 *  fldi:  Load immediate (0.0 or 1.0) into floating point register.
 *  fneg:  Negate a floating point register
 *  fabs:  Get the absolute value of a floating point register
 *  fsqrt: Calculate square root
 *  fsrra: Calculate 1 / (square root)
 *
 *  arg[0] = ptr to fp register
 *  arg[1] = (uint32_t) immediate value (for fldi)
 */
X(fldi_frn)
{
	FLOATING_POINT_AVAILABLE_CHECK;
	reg(ic->arg[0]) = ic->arg[1];
}
X(fneg_frn)
{
	FLOATING_POINT_AVAILABLE_CHECK;
	/*  Note: This also works for double-precision.  */
	reg(ic->arg[0]) ^= 0x80000000;
}
X(fabs_frn)
{
	FLOATING_POINT_AVAILABLE_CHECK;
	/*  Note: This also works for double-precision.  */
	reg(ic->arg[0]) &= 0x7fffffff;
}
X(fsqrt_frn)
{
	struct ieee_float_value op1;

	FLOATING_POINT_AVAILABLE_CHECK;

	if (cpu->cd.sh.fpscr & SH_FPSCR_PR) {
		/*  Double-precision:  */
		int64_t r1, ieee;
		r1 = reg(ic->arg[0] + sizeof(uint32_t)) +
		    ((uint64_t)reg(ic->arg[0]) << 32);
		ieee_interpret_float_value(r1, &op1, IEEE_FMT_D);
		ieee = ieee_store_float_value(sqrt(op1.f), IEEE_FMT_D, 0);
		reg(ic->arg[0]) = (uint32_t) (ieee >> 32);
		reg(ic->arg[0] + sizeof(uint32_t)) = (uint32_t) ieee;
	} else {
		/*  Single-precision:  */
		int32_t ieee, r1 = reg(ic->arg[0]);
		ieee_interpret_float_value(r1, &op1, IEEE_FMT_S);
		ieee = ieee_store_float_value(sqrt(op1.f), IEEE_FMT_S, 0);
		reg(ic->arg[0]) = ieee;
	}
}
X(fsrra_frn)
{
	// I'm guessing that this is 1/sqrt. That's how it is described at
	// http://yam.20to4.net/dreamcast/hints/index.html at least.

	struct ieee_float_value op1;

	FLOATING_POINT_AVAILABLE_CHECK;

	if (cpu->cd.sh.fpscr & SH_FPSCR_PR) {
		/*  Double-precision:  */
		fatal("Double-precision fsrra? TODO\n");
		exit(1);
	} else {
		/*  Single-precision:  */
		int32_t ieee, r1 = reg(ic->arg[0]);
		ieee_interpret_float_value(r1, &op1, IEEE_FMT_S);
		ieee = ieee_store_float_value(1.0f / sqrt(op1.f),
		    IEEE_FMT_S, 0);
		reg(ic->arg[0]) = ieee;
	}
}


/*
 *  fadd_frm_frn:     Floating point addition.
 *  fsub_frm_frn:     Floating point subtraction.
 *  fmul_frm_frn:     Floating point multiplication.
 *  fdiv_frm_frn:     Floating point division.
 *  fmac_fr0_frm_frn: Multiply-and-accumulate.
 *  fcmp_eq_frm_frn:  Floating point greater-than comparison.
 *  fcmp_gt_frm_frn:  Floating point greater-than comparison.
 *
 *  arg[0] = ptr to float register FRm
 *  arg[1] = ptr to float register FRn
 */
X(fadd_frm_frn)
{
	struct ieee_float_value op1, op2;

	FLOATING_POINT_AVAILABLE_CHECK;

	if (cpu->cd.sh.fpscr & SH_FPSCR_PR) {
		/*  Double-precision, using a pair of registers:  */
		int64_t r1, r2, ieee;
		double result;

		r1 = reg(ic->arg[0] + sizeof(uint32_t)) +
		    ((uint64_t)reg(ic->arg[0]) << 32);
		r2 = reg(ic->arg[1] + sizeof(uint32_t)) +
		    ((uint64_t)reg(ic->arg[1]) << 32);
		ieee_interpret_float_value(r1, &op1, IEEE_FMT_D);
		ieee_interpret_float_value(r2, &op2, IEEE_FMT_D);

		result = op2.f + op1.f;
		ieee = ieee_store_float_value(result, IEEE_FMT_D, 0);
		reg(ic->arg[1]) = (uint32_t) (ieee >> 32);
		reg(ic->arg[1] + sizeof(uint32_t)) = (uint32_t) ieee;
	} else {
		/*  Single-precision:  */
		uint32_t r1, r2, ieee;
		double result;

		r1 = reg(ic->arg[0]);
		r2 = reg(ic->arg[1]);
		ieee_interpret_float_value(r1, &op1, IEEE_FMT_S);
		ieee_interpret_float_value(r2, &op2, IEEE_FMT_S);

		result = op2.f + op1.f;
		ieee = ieee_store_float_value(result, IEEE_FMT_S, 0);
		reg(ic->arg[1]) = (uint32_t) ieee;
	}
}
X(fsub_frm_frn)
{
	struct ieee_float_value op1, op2;

	FLOATING_POINT_AVAILABLE_CHECK;

	if (cpu->cd.sh.fpscr & SH_FPSCR_PR) {
		/*  Double-precision, using a pair of registers:  */
		int64_t r1, r2, ieee;
		double result;
		r1 = reg(ic->arg[0] + sizeof(uint32_t)) +
		    ((uint64_t)reg(ic->arg[0]) << 32);
		r2 = reg(ic->arg[1] + sizeof(uint32_t)) +
		    ((uint64_t)reg(ic->arg[1]) << 32);
		ieee_interpret_float_value(r1, &op1, IEEE_FMT_D);
		ieee_interpret_float_value(r2, &op2, IEEE_FMT_D);
		result = op2.f - op1.f;
		ieee = ieee_store_float_value(result, IEEE_FMT_D, 0);
		reg(ic->arg[1]) = (uint32_t) (ieee >> 32);
		reg(ic->arg[1] + sizeof(uint32_t)) = (uint32_t) ieee;
	} else {
		/*  Single-precision:  */
		uint32_t r1, r2, ieee;
		double result;
		r1 = reg(ic->arg[0]);
		r2 = reg(ic->arg[1]);
		ieee_interpret_float_value(r1, &op1, IEEE_FMT_S);
		ieee_interpret_float_value(r2, &op2, IEEE_FMT_S);
		result = op2.f - op1.f;
		ieee = ieee_store_float_value(result, IEEE_FMT_S, 0);
		reg(ic->arg[1]) = (uint32_t) ieee;
	}
}
X(fmul_frm_frn)
{
	struct ieee_float_value op1, op2;

	FLOATING_POINT_AVAILABLE_CHECK;

	if (cpu->cd.sh.fpscr & SH_FPSCR_PR) {
		/*  Double-precision, using a pair of registers:  */
		int64_t r1, r2, ieee;
		double result;

		r1 = reg(ic->arg[0] + sizeof(uint32_t)) +
		    ((uint64_t)reg(ic->arg[0]) << 32);
		r2 = reg(ic->arg[1] + sizeof(uint32_t)) +
		    ((uint64_t)reg(ic->arg[1]) << 32);
		ieee_interpret_float_value(r1, &op1, IEEE_FMT_D);
		ieee_interpret_float_value(r2, &op2, IEEE_FMT_D);

		result = op2.f * op1.f;
		ieee = ieee_store_float_value(result, IEEE_FMT_D, 0);
		reg(ic->arg[1]) = (uint32_t) (ieee >> 32);
		reg(ic->arg[1] + sizeof(uint32_t)) = (uint32_t) ieee;
	} else {
		/*  Single-precision:  */
		uint32_t r1, r2, ieee;
		double result;

		r1 = reg(ic->arg[0]);
		r2 = reg(ic->arg[1]);
		ieee_interpret_float_value(r1, &op1, IEEE_FMT_S);
		ieee_interpret_float_value(r2, &op2, IEEE_FMT_S);

		result = op2.f * op1.f;
		ieee = ieee_store_float_value(result, IEEE_FMT_S, 0);
		reg(ic->arg[1]) = (uint32_t) ieee;
	}
}
X(fdiv_frm_frn)
{
	struct ieee_float_value op1, op2;

	FLOATING_POINT_AVAILABLE_CHECK;

	if (cpu->cd.sh.fpscr & SH_FPSCR_PR) {
		/*  Double-precision, using a pair of registers:  */
		int64_t r1, r2, ieee;
		double result;

		r1 = reg(ic->arg[0] + sizeof(uint32_t)) +
		    ((uint64_t)reg(ic->arg[0]) << 32);
		r2 = reg(ic->arg[1] + sizeof(uint32_t)) +
		    ((uint64_t)reg(ic->arg[1]) << 32);
		ieee_interpret_float_value(r1, &op1, IEEE_FMT_D);
		ieee_interpret_float_value(r2, &op2, IEEE_FMT_D);

		if (op1.f != 0.0)
			result = op2.f / op1.f;
		else
			result = 0.0;

		ieee = ieee_store_float_value(result, IEEE_FMT_D, 0);

		reg(ic->arg[1]) = (uint32_t) (ieee >> 32);
		reg(ic->arg[1] + sizeof(uint32_t)) = (uint32_t) ieee;
	} else {
		/*  Single-precision:  */
		uint32_t r1, r2, ieee;
		double result;

		r1 = reg(ic->arg[0]);
		r2 = reg(ic->arg[1]);
		ieee_interpret_float_value(r1, &op1, IEEE_FMT_S);
		ieee_interpret_float_value(r2, &op2, IEEE_FMT_S);

		if (op1.f != 0.0)
			result = op2.f / op1.f;
		else
			result = 0.0;

		ieee = ieee_store_float_value(result, IEEE_FMT_S, 0);

		reg(ic->arg[1]) = (uint32_t) ieee;
	}
}
X(fmac_fr0_frm_frn)
{
	struct ieee_float_value op1, op2, op0;
	int32_t r1, r2, fr0 = cpu->cd.sh.fr[0], ieee;

	FLOATING_POINT_AVAILABLE_CHECK;

	r1 = reg(ic->arg[0]), r2 = reg(ic->arg[1]);
	ieee_interpret_float_value(fr0, &op0, IEEE_FMT_S);
	ieee_interpret_float_value(r1, &op1, IEEE_FMT_S);
	ieee_interpret_float_value(r2, &op2, IEEE_FMT_S);
	ieee = ieee_store_float_value(op0.f * op1.f + op2.f, IEEE_FMT_S, 0);
	reg(ic->arg[1]) = ieee;
}
X(fcmp_eq_frm_frn)
{
	struct ieee_float_value op1, op2;

	FLOATING_POINT_AVAILABLE_CHECK;

	if (cpu->cd.sh.fpscr & SH_FPSCR_PR) {
		/*  Double-precision, using a pair of registers:  */
		int64_t r1, r2;
		r1 = reg(ic->arg[0] + sizeof(uint32_t)) +
		    ((uint64_t)reg(ic->arg[0]) << 32);
		r2 = reg(ic->arg[1] + sizeof(uint32_t)) +
		    ((uint64_t)reg(ic->arg[1]) << 32);
		ieee_interpret_float_value(r1, &op1, IEEE_FMT_D);
		ieee_interpret_float_value(r2, &op2, IEEE_FMT_D);
	} else {
		/*  Single-precision:  */
		uint32_t r1 = reg(ic->arg[0]), r2 = reg(ic->arg[1]);
		ieee_interpret_float_value(r1, &op1, IEEE_FMT_S);
		ieee_interpret_float_value(r2, &op2, IEEE_FMT_S);
	}

	if (op2.f == op1.f)
		cpu->cd.sh.sr |= SH_SR_T;
	else
		cpu->cd.sh.sr &= ~SH_SR_T;
}
X(fcmp_gt_frm_frn)
{
	struct ieee_float_value op1, op2;

	FLOATING_POINT_AVAILABLE_CHECK;

	if (cpu->cd.sh.fpscr & SH_FPSCR_PR) {
		/*  Double-precision, using a pair of registers:  */
		int64_t r1, r2;
		r1 = reg(ic->arg[0] + sizeof(uint32_t)) +
		    ((uint64_t)reg(ic->arg[0]) << 32);
		r2 = reg(ic->arg[1] + sizeof(uint32_t)) +
		    ((uint64_t)reg(ic->arg[1]) << 32);
		ieee_interpret_float_value(r1, &op1, IEEE_FMT_D);
		ieee_interpret_float_value(r2, &op2, IEEE_FMT_D);
	} else {
		/*  Single-precision:  */
		uint32_t r1 = reg(ic->arg[0]), r2 = reg(ic->arg[1]);
		ieee_interpret_float_value(r1, &op1, IEEE_FMT_S);
		ieee_interpret_float_value(r2, &op2, IEEE_FMT_S);
	}

	if (op2.f > op1.f)
		cpu->cd.sh.sr |= SH_SR_T;
	else
		cpu->cd.sh.sr &= ~SH_SR_T;
}


/*
 *  frchg:  Change floating-point register banks.
 *  fschg:  Change floating-point register size.
 */
X(frchg)
{
	FLOATING_POINT_AVAILABLE_CHECK;
	sh_update_fpscr(cpu, cpu->cd.sh.fpscr ^ SH_FPSCR_FR);
}
X(fschg)
{
	FLOATING_POINT_AVAILABLE_CHECK;
	sh_update_fpscr(cpu, cpu->cd.sh.fpscr ^ SH_FPSCR_SZ);
}


/*
 *  pref_rn:  Prefetch.
 *
 *  arg[1] = ptr to Rn
 */
X(pref_rn)
{
	uint32_t addr = reg(ic->arg[1]), extaddr;
	int sq_nr, ofs;

	if (addr < 0xe0000000 || addr >= 0xe4000000)
		return;

	/*  Send Store Queue contents to external memory:  */
	extaddr = addr & 0x03ffffe0;
	sq_nr = addr & 0x20? 1 : 0;

	if (cpu->cd.sh.mmucr & SH4_MMUCR_AT) {
		fatal("Store Queue to external memory, when "
		    "MMU enabled: TODO\n");
		exit(1);
	}

	if (sq_nr == 0)
		extaddr |= (((cpu->cd.sh.qacr0 >> 2) & 7) << 26);
	else
		extaddr |= (((cpu->cd.sh.qacr1 >> 2) & 7) << 26);

	/*  fatal("extaddr = 0x%08x\n", extaddr);  */

	SYNCH_PC;
	for (ofs = 0; ofs < 32; ofs += sizeof(uint32_t)) {
		uint32_t word;
		cpu->memory_rw(cpu, cpu->mem, 0xe0000000 + ofs
		    + sq_nr * 0x20, (unsigned char *)
		    &word, sizeof(word), MEM_READ, PHYSICAL);
		cpu->memory_rw(cpu, cpu->mem, extaddr+ofs, (unsigned char *)
		    &word, sizeof(word), MEM_WRITE, PHYSICAL);
	}
}


/*
 *  tas_b_rn: Test-and-Set.
 *
 *  arg[1] = ptr to Rn
 */
X(tas_b_rn)
{
	uint32_t addr = reg(ic->arg[1]);
	uint8_t byte, newbyte;

	SYNCH_PC;

	if (!cpu->memory_rw(cpu, cpu->mem, addr, &byte, 1, MEM_READ,
	   CACHE_DATA)) {
		/*  Exception.  */
		return;
	}

	newbyte = byte | 0x80;

	if (!cpu->memory_rw(cpu, cpu->mem, addr, &newbyte, 1, MEM_WRITE,
	   CACHE_DATA)) {
		/*  Exception.  */
		return;
	}

	if (byte == 0)
		cpu->cd.sh.sr |= SH_SR_T;
	else
		cpu->cd.sh.sr &= ~SH_SR_T;
}


/*
 *  prom_emul:
 */
X(prom_emul)
{
	uint32_t old_pc;
	SYNCH_PC;
	old_pc = cpu->pc;

	switch (cpu->machine->machine_type) {
	case MACHINE_DREAMCAST:
		dreamcast_emul(cpu);
		break;
	case MACHINE_LANDISK:
		sh_ipl_g_emul(cpu);
		break;
	default:
		fatal("SH prom_emul: unimplemented machine type.\n");
		exit(1);
	}

	if (!cpu->running) {
		cpu->n_translated_instrs --;
		cpu->cd.sh.next_ic = &nothing_call;
	} else if ((uint32_t)cpu->pc != old_pc) {
		/*  The PC value was changed by the PROM call.  */
		quick_pc_to_pointers(cpu);
	}
}


/*****************************************************************************/


X(end_of_page)
{
	/*  Update the PC:  (offset 0, but on the next page)  */
	cpu->pc &= ~((SH_IC_ENTRIES_PER_PAGE-1) <<
	    SH_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (SH_IC_ENTRIES_PER_PAGE << SH_INSTR_ALIGNMENT_SHIFT);

	/*  end_of_page doesn't count as an executed instruction:  */
	cpu->n_translated_instrs --;

	/*
	 *  Find the new physpage and update translation pointers.
	 *
	 *  Note: This may cause an exception, if e.g. the new page is
	 *  not accessible.
	 */
	quick_pc_to_pointers(cpu);

	/*  Simple jump to the next page (if we are lucky):  */
	if (cpu->delay_slot == NOT_DELAYED)
		return;

	/*
	 *  If we were in a delay slot, and we got an exception while doing
	 *  quick_pc_to_pointers, then return. The function which called
	 *  end_of_page should handle this case.
	 */
	if (cpu->delay_slot == EXCEPTION_IN_DELAY_SLOT)
		return;

	/*
	 *  Tricky situation; the delay slot is on the next virtual page.
	 *  Calling to_be_translated will translate one instruction manually,
	 *  execute it, and then discard it.
	 */
	/*  fatal("[ end_of_page: delay slot across page boundary! ]\n");  */

	instr(to_be_translated)(cpu, cpu->cd.sh.next_ic);

	/*  The instruction in the delay slot has now executed.  */
	/*  fatal("[ end_of_page: back from executing the delay slot, %i ]\n",
	    cpu->delay_slot);  */

	/*  Find the physpage etc of the instruction in the delay slot
	    (or, if there was an exception, the exception handler):  */
	quick_pc_to_pointers(cpu);
}


X(end_of_page2)
{
	/*  Synchronize PC on the _second_ instruction on the next page:  */
	int low_pc = ((size_t)ic - (size_t)cpu->cd.sh.cur_ic_page)
	    / sizeof(struct sh_instr_call);
	cpu->pc &= ~((SH_IC_ENTRIES_PER_PAGE-1)
	    << SH_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << SH_INSTR_ALIGNMENT_SHIFT);

	/*  This doesn't count as an executed instruction.  */
	cpu->n_translated_instrs --;

	quick_pc_to_pointers(cpu);

	if (cpu->delay_slot == NOT_DELAYED)
		return;

	fatal("end_of_page2: fatal error, we're in a delay slot\n");
	exit(1);
}


/*****************************************************************************/


/*
 *  sh_instr_to_be_translated():
 *
 *  Translate an instruction word into an sh_instr_call. ic is filled in with
 *  valid data for the translated instruction, or a "nothing" instruction if
 *  there was a translation failure. The newly translated instruction is then
 *  executed.
 */
X(to_be_translated)
{
	uint32_t addr, low_pc, iword;
	unsigned char *page;
	unsigned char ib[2];
	int main_opcode, isize = sizeof(ib);
	int in_crosspage_delayslot = 0, r8, r4, lo4, lo8;
	void (*samepage_function)(struct cpu *, struct sh_instr_call *);

	/*  Figure out the (virtual) address of the instruction:  */
	low_pc = ((size_t)ic - (size_t)cpu->cd.sh.cur_ic_page)
	    / sizeof(struct sh_instr_call);

	/*  Special case for branch with delayslot on the next page:  */
	if (cpu->delay_slot == TO_BE_DELAYED && low_pc == 0) {
		/*  fatal("[ delay-slot translation across page "
		    "boundary ]\n");  */
		in_crosspage_delayslot = 1;
	}

	addr = cpu->pc & ~((SH_IC_ENTRIES_PER_PAGE-1)
	    << SH_INSTR_ALIGNMENT_SHIFT);
	addr += (low_pc << SH_INSTR_ALIGNMENT_SHIFT);
	cpu->pc = (MODE_int_t)addr;
	addr &= ~((1 << SH_INSTR_ALIGNMENT_SHIFT) - 1);

	/*  Read the instruction word from memory:  */
	page = cpu->cd.sh.host_load[(uint32_t)addr >> 12];

	if (page != NULL) {
		/*  fatal("TRANSLATION HIT!\n");  */
		memcpy(ib, page + (addr & 0xfff), isize);
	} else {
		/*  fatal("TRANSLATION MISS!\n");  */
		if (!cpu->memory_rw(cpu, cpu->mem, addr, ib,
		    isize, MEM_READ, CACHE_INSTRUCTION)) {
			fatal("to_be_translated(): read failed: TODO\n");
			goto bad;
		}
	}

	{
		uint16_t *p = (uint16_t *) ib;
		iword = *p;
	}

	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		iword = LE16_TO_HOST(iword);
	else
		iword = BE16_TO_HOST(iword);

	main_opcode = iword >> 12;
	r8 = (iword >> 8) & 0xf;
	r4 = (iword >> 4) & 0xf;
	lo8 = iword & 0xff;
	lo4 = iword & 0xf;


#define DYNTRANS_TO_BE_TRANSLATED_HEAD
#include "cpu_dyntrans.cc"
#undef  DYNTRANS_TO_BE_TRANSLATED_HEAD


	/*
	 *  Translate the instruction:
	 */

	/*  Default args. for many instructions:  */
	ic->arg[0] = (size_t)&cpu->cd.sh.r[r4];	/* m */
	ic->arg[1] = (size_t)&cpu->cd.sh.r[r8];	/* n */

	switch (main_opcode) {

	case 0x0:
		if (lo4 == 0x4) {
			/*  MOV.B Rm,@(R0,Rn)  */
			ic->f = instr(mov_b_rm_r0_rn);
		} else if (lo4 == 0x5) {
			/*  MOV.W Rm,@(R0,Rn)  */
			ic->f = instr(mov_w_rm_r0_rn);
		} else if (lo4 == 0x6) {
			/*  MOV.L Rm,@(R0,Rn)  */
			ic->f = instr(mov_l_rm_r0_rn);
		} else if (lo4 == 0x7) {
			/*  MUL.L Rm,Rn  */
			ic->f = instr(mul_l_rm_rn);
		} else if (iword == 0x000b) {
			if (cpu->machine->show_trace_tree)
				ic->f = instr(rts_trace);
			else
				ic->f = instr(rts);
		} else if (lo4 == 0xc) {
			/*  MOV.B @(R0,Rm),Rn  */
			ic->f = instr(mov_b_r0_rm_rn);
		} else if (lo4 == 0xd) {
			/*  MOV.W @(R0,Rm),Rn  */
			ic->f = instr(mov_w_r0_rm_rn);
		} else if (lo4 == 0xe) {
			/*  MOV.L @(R0,Rm),Rn  */
			ic->f = instr(mov_l_r0_rm_rn);
		} else if (iword == 0x0008) {
			/*  CLRT  */
			ic->f = instr(clrt);
		} else if (iword == 0x0018) {
			/*  SETT  */
			ic->f = instr(sett);
		} else if (iword == 0x0019) {
			/*  DIV0U  */
			ic->f = instr(div0u);
		} else if (iword == 0x001b) {
			/*  SLEEP  */
			ic->f = instr(sleep);
		} else if (iword == 0x0028) {
			/*  CLRMAC  */
			ic->f = instr(clrmac);
		} else if (iword == 0x002b) {
			/*  RTE  */
			ic->f = instr(rte);
		} else if (iword == 0x0038) {
			/*  LDTLB  */
			ic->f = instr(ldtlb);
		} else if (iword == 0x0048) {
			/*  CLRS  */
			ic->f = instr(clrs);
		} else if (iword == 0x0058) {
			/*  SETS  */
			ic->f = instr(sets);
		} else if ((lo8 & 0x8f) == 0x82) {
			/*  STC Rm_BANK, Rn  */
			ic->f = instr(copy_privileged_register);
			ic->arg[0] = (size_t)&cpu->cd.sh.r_bank[(lo8 >> 4) & 7];
		} else if (iword == SH_INVALID_INSTR) {
			/*  PROM emulation (GXemul specific)  */
			ic->f = instr(prom_emul);
		} else {
			switch (lo8) {
			case 0x02:	/*  STC SR,Rn  */
				ic->f = instr(copy_privileged_register);
				ic->arg[0] = (size_t)&cpu->cd.sh.sr;
				break;
			case 0x03:	/*  BSRF Rn  */
				ic->f = instr(bsrf_rn);
				ic->arg[0] = (int32_t) (addr &
				    ((SH_IC_ENTRIES_PER_PAGE-1)
				    << SH_INSTR_ALIGNMENT_SHIFT) & ~1) + 4;
				/*  arg[1] is Rn  */
				break;
			case 0x09:	/*  NOP  */
				ic->f = instr(nop);
				if (iword & 0x0f00) {
					if (!cpu->translation_readahead)
						fatal("Unimplemented NOP"
						    " variant?\n");
					goto bad;
				}
				break;
			case 0x0a:	/*  STS MACH,Rn  */
				ic->f = instr(mov_rm_rn);
				ic->arg[0] = (size_t)&cpu->cd.sh.mach;
				break;
			case 0x12:	/*  STC GBR,Rn  */
				ic->f = instr(mov_rm_rn);
				ic->arg[0] = (size_t)&cpu->cd.sh.gbr;
				break;
			case 0x1a:	/*  STS MACL,Rn  */
				ic->f = instr(mov_rm_rn);
				ic->arg[0] = (size_t)&cpu->cd.sh.macl;
				break;
			case 0x22:	/*  STC VBR,Rn  */
				ic->f = instr(copy_privileged_register);
				ic->arg[0] = (size_t)&cpu->cd.sh.vbr;
				break;
			case 0x23:	/*  BRAF Rn  */
				ic->f = instr(braf_rn);
				ic->arg[0] = (int32_t) (addr &
				    ((SH_IC_ENTRIES_PER_PAGE-1)
				    << SH_INSTR_ALIGNMENT_SHIFT) & ~1) + 4;
				/*  arg[1] is Rn  */
				break;
			case 0x29:	/*  MOVT Rn  */
				ic->f = instr(movt_rn);
				break;
			case 0x2a:	/*  STS PR,Rn  */
				ic->f = instr(mov_rm_rn);
				ic->arg[0] = (size_t)&cpu->cd.sh.pr;
				break;
			case 0x32:	/*  STC SSR,Rn  */
				ic->f = instr(copy_privileged_register);
				ic->arg[0] = (size_t)&cpu->cd.sh.ssr;
				break;
			case 0x42:	/*  STC SPC,Rn  */
				ic->f = instr(copy_privileged_register);
				ic->arg[0] = (size_t)&cpu->cd.sh.spc;
				break;
			case 0x5a:	/*  STS FPUL,Rn  */
				ic->f = instr(copy_fp_register);
				ic->arg[0] = (size_t)&cpu->cd.sh.fpul;
				ic->arg[1] = (size_t)&cpu->cd.sh.r[r8];
				break;
			case 0x6a:	/*  STS FPSCR,Rn  */
				ic->f = instr(copy_fp_register);
				ic->arg[0] = (size_t)&cpu->cd.sh.fpscr;
				ic->arg[1] = (size_t)&cpu->cd.sh.r[r8];
				break;
			case 0x83:	/*  PREF @Rn  */
				ic->f = instr(pref_rn);
				break;
			case 0x93:	/*  OCBI @Rn  */
				/*  Treat as nop for now:  */
				/*  TODO: Implement this.  */
				ic->f = instr(nop);
				break;
			case 0xa3:	/*  OCBP @Rn  */
				/*  Treat as nop for now:  */
				/*  TODO: Implement this.  */
				ic->f = instr(nop);
				break;
			case 0xb3:	/*  OCBWB @Rn  */
				/*  Treat as nop for now:  */
				/*  TODO: Implement this.  */
				ic->f = instr(nop);
				break;
			case 0xc3:	/*  MOVCA.L R0,@Rn  */
				/*  Treat as nop for now:  */
				/*  TODO: Implement this.  */
				ic->f = instr(nop);
				break;
			case 0xfa:	/*  STC DBR,Rn  */
				ic->f = instr(copy_privileged_register);
				ic->arg[0] = (size_t)&cpu->cd.sh.dbr;
				ic->arg[1] = (size_t)&cpu->cd.sh.r[r8];
				break;
			default:if (!cpu->translation_readahead)
					fatal("Unimplemented opcode 0x%x,"
					    "0x%03x\n", main_opcode,
					    iword & 0xfff);
				goto bad;
			}
		}
		break;

	case 0x1:
		ic->f = instr(mov_l_rm_disp_rn);
		ic->arg[1] = r8 + (lo4 << 4);
		break;

	case 0x2:
		switch (lo4) {
		case 0x0:	/*  MOV.B Rm,@Rn  */
			ic->f = instr(mov_b_store_rm_rn);
			break;
		case 0x1:	/*  MOV.W Rm,@Rn  */
			ic->f = instr(mov_w_store_rm_rn);
			break;
		case 0x2:	/*  MOV.L Rm,@Rn  */
			ic->f = instr(mov_l_store_rm_rn);
			break;
		case 0x4:	/*  MOV.B Rm,@-Rn  */
			ic->f = instr(mov_b_rm_predec_rn);
			break;
		case 0x5:	/*  MOV.W Rm,@-Rn  */
			ic->f = instr(mov_w_rm_predec_rn);
			break;
		case 0x6:	/*  MOV.L Rm,@-Rn  */
			ic->f = instr(mov_l_rm_predec_rn);
			break;
		case 0x7:	/*  DIV0S Rm,Rn  */
			ic->f = instr(div0s_rm_rn);
			break;
		case 0x8:	/*  TST Rm,Rn  */
			ic->f = instr(tst_rm_rn);
			if (r8 == r4)
				ic->f = instr(tst_rm);
			break;
		case 0x9:	/*  AND Rm,Rn  */
			ic->f = instr(and_rm_rn);
			break;
		case 0xa:	/*  XOR Rm,Rn  */
			ic->f = instr(xor_rm_rn);
			break;
		case 0xb:	/*  OR Rm,Rn  */
			ic->f = instr(or_rm_rn);
			break;
		case 0xc:	/*  CMP/STR Rm,Rn  */
			ic->f = instr(cmp_str_rm_rn);
			break;
		case 0xd:	/*  XTRCT Rm,Rn  */
			ic->f = instr(xtrct_rm_rn);
			break;
		case 0xe:	/*  MULU.W Rm,Rn  */
			ic->f = instr(mulu_w_rm_rn);
			break;
		case 0xf:	/*  MULS.W Rm,Rn  */
			ic->f = instr(muls_w_rm_rn);
			break;
		default:if (!cpu->translation_readahead)
				fatal("Unimplemented opcode 0x%x,0x%x\n",
				    main_opcode, lo4);
			goto bad;
		}
		break;

	case 0x3:
		switch (lo4) {
		case 0x0:	/*  CMP/EQ Rm,Rn  */
			ic->f = instr(cmpeq_rm_rn);
			break;
		case 0x2:	/*  CMP/HS Rm,Rn  */
			ic->f = instr(cmphs_rm_rn);
			break;
		case 0x3:	/*  CMP/GE Rm,Rn  */
			ic->f = instr(cmpge_rm_rn);
			break;
		case 0x4:	/*  DIV1 Rm,Rn  */
			ic->f = instr(div1_rm_rn);
			break;
		case 0x5:	/*  DMULU.L Rm,Rn  */
			ic->f = instr(dmulu_l_rm_rn);
			break;
		case 0x6:	/*  CMP/HI Rm,Rn  */
			ic->f = instr(cmphi_rm_rn);
			break;
		case 0x7:	/*  CMP/GT Rm,Rn  */
			ic->f = instr(cmpgt_rm_rn);
			break;
		case 0x8:	/*  SUB Rm,Rn  */
			ic->f = instr(sub_rm_rn);
			break;
		case 0xa:	/*  SUBC Rm,Rn  */
			ic->f = instr(subc_rm_rn);
			break;
		case 0xc:	/*  ADD Rm,Rn  */
			ic->f = instr(add_rm_rn);
			break;
		case 0xd:	/*  DMULS.L Rm,Rn  */
			ic->f = instr(dmuls_l_rm_rn);
			break;
		case 0xe:	/*  ADDC Rm,Rn  */
			ic->f = instr(addc_rm_rn);
			break;
		default:if (!cpu->translation_readahead)
				fatal("Unimplemented opcode 0x%x,0x%x\n",
				    main_opcode, lo4);
			goto bad;
		}
		break;

	case 0x4:
		if (lo4 == 0xc) {
			ic->f = instr(shad);
		} else if (lo4 == 0xd) {
			ic->f = instr(shld);
		} else if ((lo8 & 0x8f) == 0x83) {
			/*  STC.L Rm_BANK,@-Rn  */
			ic->f = instr(stc_l_rm_predec_rn_md);
			ic->arg[0] = (size_t)&cpu->cd.sh.r_bank[
			    (lo8 >> 4) & 7];	/* m */
		} else if ((lo8 & 0x8f) == 0x87) {
			/*   LDC.L @Rm+,Rn_BANK  */
			ic->f = instr(mov_l_arg1_postinc_to_arg0_md);
			ic->arg[0] = (size_t)&cpu->cd.sh.r_bank[(lo8 >> 4) & 7];
		} else if ((lo8 & 0x8f) == 0x8e) {
			/*  LDC Rm, Rn_BANK  */
			ic->f = instr(copy_privileged_register);
			ic->arg[0] = (size_t)&cpu->cd.sh.r[r8];
			ic->arg[1] = (size_t)&cpu->cd.sh.r_bank[(lo8 >> 4) & 7];
		} else {
			switch (lo8) {
			case 0x00:	/*  SHLL Rn  */
				ic->f = instr(shll_rn);
				break;
			case 0x01:	/*  SHLR Rn  */
				ic->f = instr(shlr_rn);
				break;
			case 0x02:	/*  STS.L MACH,@-Rn  */
				ic->f = instr(mov_l_rm_predec_rn);
				ic->arg[0] = (size_t)&cpu->cd.sh.mach;
				break;
			case 0x03:	/*  STC.L SR,@-Rn  */
				ic->f = instr(stc_l_rm_predec_rn_md);
				ic->arg[0] = (size_t)&cpu->cd.sh.sr;
				break;
			case 0x04:	/*  ROTL Rn  */
				ic->f = instr(rotl_rn);
				break;
			case 0x05:	/*  ROTR Rn  */
				ic->f = instr(rotr_rn);
				break;
			case 0x06:	/*  LDS.L @Rm+,MACH  */
				ic->f = instr(mov_l_arg1_postinc_to_arg0);
				ic->arg[0] = (size_t)&cpu->cd.sh.mach;
				break;
			case 0x07:	/*  LDC.L @Rm+,SR  */
				ic->f = instr(mov_l_arg1_postinc_to_arg0_md);
				ic->arg[0] = (size_t)&cpu->cd.sh.sr;
				break;
			case 0x08:	/*  SHLL2 Rn  */
				ic->f = instr(shll2_rn);
				break;
			case 0x09:	/*  SHLR2 Rn  */
				ic->f = instr(shlr2_rn);
				break;
			case 0x0b:	/*  JSR @Rn  */
				if (cpu->machine->show_trace_tree)
					ic->f = instr(jsr_rn_trace);
				else
					ic->f = instr(jsr_rn);
				ic->arg[0] = (size_t)&cpu->cd.sh.r[r8];	/* n */
				ic->arg[1] = (addr & 0xffe) + 4;
				break;
			case 0x0e:	/*  LDC Rm,SR  */
				ic->f = instr(ldc_rm_sr);
				break;
			case 0x10:	/*  DT Rn  */
				ic->f = instr(dt_rn);
				break;
			case 0x11:	/*  CMP/PZ Rn  */
				ic->f = instr(cmppz_rn);
				break;
			case 0x12:	/*  STS.L MACL,@-Rn  */
				ic->f = instr(mov_l_rm_predec_rn);
				ic->arg[0] = (size_t)&cpu->cd.sh.macl;
				break;
			case 0x13:	/*  STC.L GBR,@-Rn  */
				ic->f = instr(mov_l_rm_predec_rn);
				ic->arg[0] = (size_t)&cpu->cd.sh.gbr;
				break;
			case 0x15:	/*  CMP/PL Rn  */
				ic->f = instr(cmppl_rn);
				break;
			case 0x16:	/*  LDS.L @Rm+,MACL  */
				ic->f = instr(mov_l_arg1_postinc_to_arg0);
				ic->arg[0] = (size_t)&cpu->cd.sh.macl;
				break;
			case 0x17:	/*  LDC.L @Rm+,GBR  */
				ic->f = instr(mov_l_arg1_postinc_to_arg0);
				ic->arg[0] = (size_t)&cpu->cd.sh.gbr;
				break;
			case 0x18:	/*  SHLL8 Rn  */
				ic->f = instr(shll8_rn);
				break;
			case 0x19:	/*  SHLR8 Rn  */
				ic->f = instr(shlr8_rn);
				break;
			case 0x1b:	/*  TAS.B @Rn  */
				ic->f = instr(tas_b_rn);
				break;
			case 0x1e:	/*  LDC Rm,GBR  */
				ic->f = instr(mov_rm_rn);
				ic->arg[0] = (size_t)&cpu->cd.sh.r[r8];	/* m */
				ic->arg[1] = (size_t)&cpu->cd.sh.gbr;
				break;
			case 0x20:	/*  SHAL Rn  */
				ic->f = instr(shll_rn);  /*  NOTE: shll  */
				break;
			case 0x21:	/*  SHAR Rn  */
				ic->f = instr(shar_rn);
				break;
			case 0x22:	/*  STS.L PR,@-Rn  */
				ic->f = instr(mov_l_rm_predec_rn);
				ic->arg[0] = (size_t)&cpu->cd.sh.pr;	/* m */
				ic->arg[1] = (size_t)&cpu->cd.sh.r[r8];	/* n */
				break;
			case 0x23:	/*  STC.L VBR,@-Rn  */
				ic->f = instr(stc_l_rm_predec_rn_md);
				ic->arg[0] = (size_t)&cpu->cd.sh.vbr;
				break;
			case 0x24:	/*  ROTCL Rn  */
				ic->f = instr(rotcl_rn);
				break;
			case 0x25:	/*  ROTCR Rn  */
				ic->f = instr(rotcr_rn);
				break;
			case 0x26:	/*  LDS.L @Rm+,PR  */
				ic->f = instr(mov_l_arg1_postinc_to_arg0);
				ic->arg[0] = (size_t)&cpu->cd.sh.pr;
				break;
			case 0x27:	/*  LDC.L @Rm+,VBR  */
				ic->f = instr(mov_l_arg1_postinc_to_arg0_md);
				ic->arg[0] = (size_t)&cpu->cd.sh.vbr;
				break;
			case 0x28:	/*  SHLL16 Rn  */
				ic->f = instr(shll16_rn);
				break;
			case 0x29:	/*  SHLR16 Rn  */
				ic->f = instr(shlr16_rn);
				break;
			case 0x2a:	/*  LDS Rm,PR  */
				ic->f = instr(mov_rm_rn);
				ic->arg[0] = (size_t)&cpu->cd.sh.r[r8];	/* m */
				ic->arg[1] = (size_t)&cpu->cd.sh.pr;
				break;
			case 0x2b:	/*  JMP @Rn  */
				if (cpu->machine->show_trace_tree)
					ic->f = instr(jmp_rn_trace);
				else
					ic->f = instr(jmp_rn);
				ic->arg[0] = (size_t)&cpu->cd.sh.r[r8];	/* n */
				ic->arg[1] = (addr & 0xffe) + 4;
				break;
			case 0x2e:	/*  LDC Rm,VBR  */
				ic->f = instr(copy_privileged_register);
				ic->arg[0] = (size_t)&cpu->cd.sh.r[r8];	/* m */
				ic->arg[1] = (size_t)&cpu->cd.sh.vbr;
				break;
			case 0x33:	/*  STC.L SSR,@-Rn  */
				ic->f = instr(stc_l_rm_predec_rn_md);
				ic->arg[0] = (size_t)&cpu->cd.sh.ssr;
				break;
			case 0x37:	/*  LDC.L @Rm+,SSR  */
				ic->f = instr(mov_l_arg1_postinc_to_arg0_md);
				ic->arg[0] = (size_t)&cpu->cd.sh.ssr;
				break;
			case 0x3e:	/*  LDC rm,SSR  */
				ic->f = instr(copy_privileged_register);
				ic->arg[0] = (size_t)&cpu->cd.sh.r[r8];	/* m */
				ic->arg[1] = (size_t)&cpu->cd.sh.ssr;
				break;
			case 0x43:	/*  STC.L SPC,@-Rn  */
				ic->f = instr(stc_l_rm_predec_rn_md);
				ic->arg[0] = (size_t)&cpu->cd.sh.spc;
				break;
			case 0x47:	/*  LDC.L @Rm+,SPC  */
				ic->f = instr(mov_l_arg1_postinc_to_arg0_md);
				ic->arg[0] = (size_t)&cpu->cd.sh.spc;
				break;
			case 0x4e:	/*  LDC rm,SPC  */
				ic->f = instr(copy_privileged_register);
				ic->arg[0] = (size_t)&cpu->cd.sh.r[r8];	/* m */
				ic->arg[1] = (size_t)&cpu->cd.sh.spc;
				break;
			case 0x52:	/*  STS.L FPUL,@-Rn  */
				ic->f = instr(mov_l_rm_predec_rn);
				ic->arg[0] = (size_t)&cpu->cd.sh.fpul;
				ic->arg[1] = (size_t)&cpu->cd.sh.r[r8];	/* n */
				break;
			case 0x56:	/*  LDS.L @Rm+,FPUL  */
				ic->f = instr(mov_l_arg1_postinc_to_arg0_fp);
				ic->arg[0] = (size_t)&cpu->cd.sh.fpul;
				break;
			case 0x5a:	/*  LDS Rm,FPUL  */
				ic->f = instr(copy_fp_register);
				ic->arg[0] = (size_t)&cpu->cd.sh.r[r8];	/* m */
				ic->arg[1] = (size_t)&cpu->cd.sh.fpul;
				break;
			case 0x62:	/*  STS.L FPSCR,@-Rn  */
				ic->f = instr(mov_l_rm_predec_rn);
				ic->arg[0] = (size_t)&cpu->cd.sh.fpscr;
				ic->arg[1] = (size_t)&cpu->cd.sh.r[r8];	/* n */
				break;
			case 0x66:	/*  LDS.L @Rm+,FPSCR  */
				/*  Note: Loading into FPSCR is a specia
				    case (need to call sh_update_fpsrc()).  */
				ic->f = instr(mov_l_arg1_postinc_to_arg0_fp);
				ic->arg[0] = (size_t)&cpu->cd.sh.fpscr;
				break;
			case 0x6a:	/*  LDS Rm,FPSCR  */
				ic->f = instr(lds_rm_fpscr);
				/*  arg 1 = R8 = Rm  */
				break;
			case 0xfa:	/*  LDC Rm,DBR  */
				ic->f = instr(copy_privileged_register);
				ic->arg[0] = (size_t)&cpu->cd.sh.r[r8];
				ic->arg[1] = (size_t)&cpu->cd.sh.dbr;
				break;
			default:if (!cpu->translation_readahead)
					fatal("Unimplemented opcode 0x%x,"
					    "0x%02x\n", main_opcode, lo8);
				goto bad;
			}
		}
		break;

	case 0x5:
		ic->f = instr(mov_l_disp_rm_rn);
		ic->arg[0] = r4 + (lo4 << 4);
		break;

	case 0x6:
		switch (lo4) {
		case 0x0:	/*  MOV.B @Rm,Rn  */
			ic->f = instr(load_b_rm_rn);
			break;
		case 0x1:	/*  MOV.W @Rm,Rn  */
			ic->f = instr(load_w_rm_rn);
			break;
		case 0x2:	/*  MOV.L @Rm,Rn  */
			ic->f = instr(load_l_rm_rn);
			break;
		case 0x3:	/*  MOV Rm,Rn  */
			ic->f = instr(mov_rm_rn);
			break;
		case 0x4:	/*  MOV.B @Rm+,Rn  */
			ic->f = instr(mov_b_arg1_postinc_to_arg0);
			/*  Note: Order  */
			ic->arg[1] = (size_t)&cpu->cd.sh.r[r4];	/* m */
			ic->arg[0] = (size_t)&cpu->cd.sh.r[r8];	/* n */
			break;
		case 0x5:	/*  MOV.W @Rm+,Rn  */
			ic->f = instr(mov_w_arg1_postinc_to_arg0);
			/*  Note: Order  */
			ic->arg[1] = (size_t)&cpu->cd.sh.r[r4];	/* m */
			ic->arg[0] = (size_t)&cpu->cd.sh.r[r8];	/* n */
			break;
		case 0x6:	/*  MOV.L @Rm+,Rn  */
			ic->f = instr(mov_l_arg1_postinc_to_arg0);
			/*  Note: Order  */
			ic->arg[1] = (size_t)&cpu->cd.sh.r[r4];	/* m */
			ic->arg[0] = (size_t)&cpu->cd.sh.r[r8];	/* n */
			break;
		case 0x7:	/*  NOT Rm,Rn  */
			ic->f = instr(not_rm_rn);
			break;
		case 0x8:	/*  SWAP.B Rm,Rn  */
			ic->f = instr(swap_b_rm_rn);
			break;
		case 0x9:	/*  SWAP.W Rm,Rn  */
			ic->f = instr(swap_w_rm_rn);
			break;
		case 0xa:	/*  NEGC Rm,Rn  */
			ic->f = instr(negc_rm_rn);
			break;
		case 0xb:	/*  NEG Rm,Rn  */
			ic->f = instr(neg_rm_rn);
			break;
		case 0xc:	/*  EXTU.B Rm,Rn  */
			ic->f = instr(extu_b_rm_rn);
			if (r8 == r4)
				ic->f = instr(extu_b_rm);
			break;
		case 0xd:	/*  EXTU.W Rm,Rn  */
			ic->f = instr(extu_w_rm_rn);
			if (r8 == r4)
				ic->f = instr(extu_w_rm);
			break;
		case 0xe:	/*  EXTS.B Rm,Rn  */
			ic->f = instr(exts_b_rm_rn);
			break;
		case 0xf:	/*  EXTS.W Rm,Rn  */
			ic->f = instr(exts_w_rm_rn);
			break;
		default:if (!cpu->translation_readahead)
				fatal("Unimplemented opcode 0x%x,0x%x\n",
				    main_opcode, lo4);
			goto bad;
		}
		break;

	case 0x7:	/*  ADD #imm,Rn  */
		ic->f = instr(add_imm_rn);
		ic->arg[0] = (int8_t)lo8;
		ic->arg[1] = (size_t)&cpu->cd.sh.r[r8];		/* n */
		if (lo8 == 1)
			ic->f = instr(inc_rn);
		if (lo8 == 4)
			ic->f = instr(add_4_rn);
		if (lo8 == 0xfc)
			ic->f = instr(sub_4_rn);
		if (lo8 == 0xff)
			ic->f = instr(dec_rn);
		break;

	case 0x8:
		/*  Displacement from beginning of page = default arg 0.  */
		ic->arg[0] = (int8_t)lo8 * 2 +
		    (addr & ((SH_IC_ENTRIES_PER_PAGE-1)
		    << SH_INSTR_ALIGNMENT_SHIFT) & ~1) + 4;
		samepage_function = NULL;

		switch (r8) {
		case 0x0:	/*  MOV.B R0,@(disp,Rn)  */
			ic->f = instr(mov_b_r0_disp_rn);
			ic->arg[0] = (size_t)&cpu->cd.sh.r[r4];	/* n */
			ic->arg[1] = lo4;
			break;
		case 0x1:	/*  MOV.W R0,@(disp,Rn)  */
			ic->f = instr(mov_w_r0_disp_rn);
			ic->arg[0] = (size_t)&cpu->cd.sh.r[r4];	/* n */
			ic->arg[1] = lo4 * 2;
			break;
		case 0x4:	/*  MOV.B @(disp,Rn),R0  */
			ic->f = instr(mov_b_disp_rn_r0);
			ic->arg[0] = (size_t)&cpu->cd.sh.r[r4];	/* n */
			ic->arg[1] = lo4;
			break;
		case 0x5:	/*  MOV.W @(disp,Rn),R0  */
			ic->f = instr(mov_w_disp_rn_r0);
			ic->arg[0] = (size_t)&cpu->cd.sh.r[r4];	/* n */
			ic->arg[1] = lo4 * 2;
			break;
		case 0x8:	/*  CMP/EQ #imm,R0  */
			ic->f = instr(cmpeq_imm_r0);
			ic->arg[0] = (int8_t)lo8;
			break;
		case 0x9:	/*  BT (disp,PC)  */
			ic->f = instr(bt);
			samepage_function = instr(bt_samepage);
			break;
		case 0xb:	/*  BF (disp,PC)  */
			ic->f = instr(bf);
			samepage_function = instr(bf_samepage);
			break;
		case 0xd:	/*  BT/S (disp,PC)  */
			ic->f = instr(bt_s);
			samepage_function = instr(bt_s_samepage);
			break;
		case 0xf:	/*  BF/S (disp,PC)  */
			ic->f = instr(bf_s);
			samepage_function = instr(bf_s_samepage);
			break;
		default:if (!cpu->translation_readahead)
				fatal("Unimplemented opcode 0x%x,0x%x\n",
				    main_opcode, r8);
			goto bad;
		}

		/*  samepage branches:  */
		if (samepage_function != NULL && ic->arg[0] < 0x1000 &&
		    (addr & 0xfff) < 0xffe) {
			ic->arg[1] = (size_t) (cpu->cd.sh.cur_ic_page +
			    (ic->arg[0] >> SH_INSTR_ALIGNMENT_SHIFT));
			ic->f = samepage_function;
		}

		break;

	case 0x9:	/*  MOV.W @(disp,PC),Rn  */
		ic->f = instr(mov_w_disp_pc_rn);
		ic->arg[0] = lo8 * 2 + (addr & ((SH_IC_ENTRIES_PER_PAGE-1)
		    << SH_INSTR_ALIGNMENT_SHIFT) & ~1) + 4;

		/*  If the word is reachable from the same page as the
		    current address, then optimize it as a mov_imm_rn:  */
		if (ic->arg[0] < 0x1000 && page != NULL) {
			uint16_t *p = (uint16_t *) page;
			uint16_t data = p[ic->arg[0] >> 1];
			if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
				data = LE16_TO_HOST(data);
			else
				data = BE16_TO_HOST(data);
			ic->f = instr(mov_imm_rn);
			ic->arg[0] = (int16_t) data;
		}
		break;

	case 0xa:	/*  BRA disp  */
	case 0xb:	/*  BSR disp  */
		samepage_function = NULL;

		switch (main_opcode) {
		case 0xa:
			ic->f = instr(bra);
			samepage_function = instr(bra_samepage);
			break;
		case 0xb:
			ic->f = instr(bsr);
			samepage_function = instr(bsr_samepage);
			break;
		}

		ic->arg[0] = (int32_t) ( (addr & ((SH_IC_ENTRIES_PER_PAGE-1)
		    << SH_INSTR_ALIGNMENT_SHIFT) & ~1) + 4 +
		    (((int32_t)(int16_t)((iword & 0xfff) << 4)) >> 3) );

		/*  samepage branches:  */
		if (samepage_function != NULL && ic->arg[0] < 0x1000 &&
		    (addr & 0xfff) < 0xffe) {
			ic->arg[0] = (size_t) (cpu->cd.sh.cur_ic_page +
			    (ic->arg[0] >> SH_INSTR_ALIGNMENT_SHIFT));
			ic->f = samepage_function;
		}
		break;

	case 0xc:
		switch (r8) {
		case 0x0:
			ic->f = instr(mov_b_r0_disp_gbr);
			ic->arg[1] = lo8;
			break;
		case 0x1:
			ic->f = instr(mov_w_r0_disp_gbr);
			ic->arg[1] = lo8 << 1;
			break;
		case 0x2:
			ic->f = instr(mov_l_r0_disp_gbr);
			ic->arg[1] = lo8 << 2;
			break;
		case 0x3:
			ic->f = instr(trapa);
			ic->arg[0] = lo8 << 2;
			break;
		case 0x4:
			ic->f = instr(mov_b_disp_gbr_r0);
			ic->arg[1] = lo8;
			break;
		case 0x5:
			ic->f = instr(mov_w_disp_gbr_r0);
			ic->arg[1] = lo8 << 1;
			break;
		case 0x6:
			ic->f = instr(mov_l_disp_gbr_r0);
			ic->arg[1] = lo8 << 2;
			break;
		case 0x7:	/*  MOVA @(disp,pc),R0  */
			ic->f = instr(mova_r0);
			ic->arg[0] = lo8 * 4 + (addr &
			    ((SH_IC_ENTRIES_PER_PAGE-1)
			    << SH_INSTR_ALIGNMENT_SHIFT) & ~3) + 4;
			break;
		case 0x8:	/*  TST #imm,R0  */
			ic->f = instr(tst_imm_r0);
			ic->arg[0] = lo8;
			break;
		case 0x9:	/*  AND #imm,R0  */
			ic->f = instr(and_imm_r0);
			ic->arg[0] = lo8;
			break;
		case 0xa:	/*  XOR #imm,R0  */
			ic->f = instr(xor_imm_r0);
			ic->arg[0] = lo8;
			break;
		case 0xb:	/*  OR #imm,R0  */
			ic->f = instr(or_imm_r0);
			ic->arg[0] = lo8;
			break;
		case 0xd:	/*  AND.B #imm,@(R0,GBR)  */
			ic->f = instr(and_b_imm_r0_gbr);
			ic->arg[0] = lo8;
			break;
		case 0xe:	/*  XOR.B #imm,@(R0,GBR)  */
			ic->f = instr(xor_b_imm_r0_gbr);
			ic->arg[0] = lo8;
			break;
		case 0xf:	/*  OR.B #imm,@(R0,GBR)  */
			ic->f = instr(or_b_imm_r0_gbr);
			ic->arg[0] = lo8;
			break;
		default:if (!cpu->translation_readahead)
				fatal("Unimplemented opcode 0x%x,0x%x\n",
				    main_opcode, r8);
			goto bad;
		}
		break;

	case 0xd:	/*  MOV.L @(disp,PC),Rn  */
		ic->f = instr(mov_l_disp_pc_rn);
		ic->arg[0] = lo8 * 4 + (addr & ((SH_IC_ENTRIES_PER_PAGE-1)
		    << SH_INSTR_ALIGNMENT_SHIFT) & ~3) + 4;

		/*  If the word is reachable from the same page as the
		    current address, then optimize it as a mov_imm_rn:  */
		if (ic->arg[0] < 0x1000 && page != NULL) {
			uint32_t *p = (uint32_t *) page;
			uint32_t data = p[ic->arg[0] >> 2];
			if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
				data = LE32_TO_HOST(data);
			else
				data = BE32_TO_HOST(data);
			ic->f = instr(mov_imm_rn);
			ic->arg[0] = data;
		}
		break;

	case 0xe:	/*  MOV #imm,Rn  */
		ic->f = instr(mov_imm_rn);
		ic->arg[0] = (int8_t)lo8;
		ic->arg[1] = (size_t)&cpu->cd.sh.r[r8];	/* n */
		if (lo8 == 0)
			ic->f = instr(mov_0_rn);
		break;

	case 0xf:
		if (lo4 == 0x0) {
			/*  FADD FRm,FRn  */
			ic->f = instr(fadd_frm_frn);
			ic->arg[0] = (size_t)&cpu->cd.sh.fr[r4];
			ic->arg[1] = (size_t)&cpu->cd.sh.fr[r8];
		} else if (lo4 == 0x1) {
			/*  FSUB FRm,FRn  */
			ic->f = instr(fsub_frm_frn);
			ic->arg[0] = (size_t)&cpu->cd.sh.fr[r4];
			ic->arg[1] = (size_t)&cpu->cd.sh.fr[r8];
		} else if (lo4 == 0x2) {
			/*  FMUL FRm,FRn  */
			ic->f = instr(fmul_frm_frn);
			ic->arg[0] = (size_t)&cpu->cd.sh.fr[r4];
			ic->arg[1] = (size_t)&cpu->cd.sh.fr[r8];
		} else if (lo4 == 0x3) {
			/*  FDIV FRm,FRn  */
			ic->f = instr(fdiv_frm_frn);
			ic->arg[0] = (size_t)&cpu->cd.sh.fr[r4];
			ic->arg[1] = (size_t)&cpu->cd.sh.fr[r8];
		} else if (lo4 == 0x4) {
			/*  FCMP/EQ FRm,FRn  */
			ic->f = instr(fcmp_eq_frm_frn);
			ic->arg[0] = (size_t)&cpu->cd.sh.fr[r4];
			ic->arg[1] = (size_t)&cpu->cd.sh.fr[r8];
		} else if (lo4 == 0x5) {
			/*  FCMP/GT FRm,FRn  */
			ic->f = instr(fcmp_gt_frm_frn);
			ic->arg[0] = (size_t)&cpu->cd.sh.fr[r4];
			ic->arg[1] = (size_t)&cpu->cd.sh.fr[r8];
		} else if (lo4 == 0x6) {
			/*  FMOV @(R0,Rm),FRn  */
			ic->f = instr(fmov_r0_rm_frn);
			ic->arg[0] = (size_t)&cpu->cd.sh.r[r4];  /* m */
			ic->arg[1] = (size_t)&cpu->cd.sh.fr[r8];
		} else if (lo4 == 0x7) {
			/*  FMOV FRm,@(R0,Rn)  */
			ic->f = instr(fmov_frm_r0_rn);
			ic->arg[0] = (size_t)&cpu->cd.sh.fr[r4];  /* m */
			ic->arg[1] = (size_t)&cpu->cd.sh.r[r8];
		} else if (lo4 == 0x8) {
			/*  FMOV @Rm,FRn  */
			ic->f = instr(fmov_rm_frn);
			ic->arg[0] = (size_t)&cpu->cd.sh.r[r4];  /* m */
			ic->arg[1] = (size_t)&cpu->cd.sh.fr[r8];
		} else if (lo4 == 0x9) {
			/*  FMOV @Rm+,FRn  */
			ic->f = instr(fmov_rm_postinc_frn);
			ic->arg[0] = (size_t)&cpu->cd.sh.r[r4];  /* m */
			ic->arg[1] = (size_t)&cpu->cd.sh.fr[r8];
		} else if (lo4 == 0xa) {
			/*  FMOV FRm,@Rn  */
			ic->f = instr(fmov_frm_rn);
			ic->arg[0] = (size_t)&cpu->cd.sh.fr[r4];  /* m */
			ic->arg[1] = (size_t)&cpu->cd.sh.r[r8];
		} else if (lo4 == 0xb) {
			/*  FMOV FRm,@-Rn  */
			ic->f = instr(fmov_frm_predec_rn);
			ic->arg[0] = (size_t)&cpu->cd.sh.fr[r4];  /* m */
			ic->arg[1] = (size_t)&cpu->cd.sh.r[r8];
		} else if (lo4 == 0xc) {
			/*  FMOV FRm,FRn  */
			ic->f = instr(fmov_frm_frn);
			ic->arg[0] = (size_t)&cpu->cd.sh.fr[r4];
			ic->arg[1] = (size_t)&cpu->cd.sh.fr[r8];
		} else if (lo8 == 0x0d) {
			/*  FSTS FPUL,FRn  */
			ic->f = instr(copy_fp_register);
			ic->arg[0] = (size_t)&cpu->cd.sh.fpul;
			ic->arg[1] = (size_t)&cpu->cd.sh.fr[r8];
		} else if (lo8 == 0x1d) {
			/*  FLDS FRn,FPUL  */
			ic->f = instr(copy_fp_register);
			ic->arg[0] = (size_t)&cpu->cd.sh.fr[r8];
			ic->arg[1] = (size_t)&cpu->cd.sh.fpul;
		} else if (lo8 == 0x2d) {
			/*  FLOAT FPUL,FRn  */
			ic->f = instr(float_fpul_frn);
			ic->arg[0] = (size_t)&cpu->cd.sh.fr[r8];
		} else if (lo8 == 0x3d) {
			/*  FTRC FRm,FPUL  */
			ic->f = instr(ftrc_frm_fpul);
			ic->arg[0] = (size_t)&cpu->cd.sh.fr[r8];
		} else if (lo8 == 0x4d) {
			/*  FNEG FRn  */
			ic->f = instr(fneg_frn);
			ic->arg[0] = (size_t)&cpu->cd.sh.fr[r8];
		} else if (lo8 == 0x5d) {
			/*  FABS FRn  */
			ic->f = instr(fabs_frn);
			ic->arg[0] = (size_t)&cpu->cd.sh.fr[r8];
		} else if (lo8 == 0x6d) {
			/*  FSQRT FRn  */
			ic->f = instr(fsqrt_frn);
			ic->arg[0] = (size_t)&cpu->cd.sh.fr[r8];
		} else if (lo8 == 0x7d) {
			/*  FSRRA FRn  */
			ic->f = instr(fsrra_frn);
			ic->arg[0] = (size_t)&cpu->cd.sh.fr[r8];
		} else if (lo8 == 0x8d) {
			/*  FLDI0 FRn  */
			ic->f = instr(fldi_frn);
			ic->arg[0] = (size_t)&cpu->cd.sh.fr[r8];
			ic->arg[1] = 0x00000000;
		} else if (lo8 == 0x9d) {
			/*  FLDI1 FRn  */
			ic->f = instr(fldi_frn);
			ic->arg[0] = (size_t)&cpu->cd.sh.fr[r8];
			ic->arg[1] = 0x3f800000;
		} else if ((iword & 0x01ff) == 0x00ad) {
			/*  FCNVSD FPUL,DRn  */
			ic->f = instr(fcnvsd_fpul_drn);
			ic->arg[0] = (size_t)&cpu->cd.sh.fr[r8];
		} else if ((iword & 0x01ff) == 0x00bd) {
			/*  FCNVDS DRm,FPUL  */
			ic->f = instr(fcnvds_drm_fpul);
			ic->arg[0] = (size_t)&cpu->cd.sh.fr[r8];
		} else if (lo8 == 0xed) {
			/*  FIPR FVm,FVn  */
			ic->f = instr(fipr_fvm_fvn);
			ic->arg[0] = (size_t)&cpu->cd.sh.fr[r8 & 0xc];  /* m */
			ic->arg[1] = (size_t)&cpu->cd.sh.fr[(r8&3)*4];  /* n */
		} else if ((iword & 0x01ff) == 0x00fd) {
			/*  FSCA FPUL,DRn  */
			ic->f = instr(fsca_fpul_drn);
			ic->arg[0] = (size_t)&cpu->cd.sh.fr[r8];
		} else if (iword == 0xf3fd) {
			/*  FSCHG  */
			ic->f = instr(fschg);
		} else if (iword == 0xfbfd) {
			/*  FRCHG  */
			ic->f = instr(frchg);
		} else if ((iword & 0xf3ff) == 0xf1fd) {
			/*  FTRV XMTRX, FVn  */
			ic->f = instr(ftrv_xmtrx_fvn);
			ic->arg[0] = (size_t)&cpu->cd.sh.fr[r8 & 0xc];
		} else if (lo4 == 0xe) {
			/*  FMAC FR0,FRm,FRn  */
			ic->f = instr(fmac_fr0_frm_frn);
			ic->arg[0] = (size_t)&cpu->cd.sh.fr[r4];
			ic->arg[1] = (size_t)&cpu->cd.sh.fr[r8];
		} else {
			if (!cpu->translation_readahead)
				fatal("Unimplemented opcode 0x%x,0x%02x\n",
				    main_opcode, lo8);
			goto bad;
		}
		break;

	default:if (!cpu->translation_readahead)
			fatal("Unimplemented main opcode 0x%x\n", main_opcode);
		goto bad;
	}


#define	DYNTRANS_TO_BE_TRANSLATED_TAIL
#include "cpu_dyntrans.cc"
#undef	DYNTRANS_TO_BE_TRANSLATED_TAIL
}

