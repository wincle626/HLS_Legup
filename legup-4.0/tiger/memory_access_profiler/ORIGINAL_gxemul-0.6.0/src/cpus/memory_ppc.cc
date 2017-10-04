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
 *  Included from cpu_ppc.c.
 */


/*
 *  ppc_bat():
 *
 *  BAT translation. Returns -1 if there was no BAT hit, >= 0 for a hit.
 *  (0 for access denied, 1 for read-only, and 2 for read-write access allowed.)
 */
int ppc_bat(struct cpu *cpu, uint64_t vaddr, uint64_t *return_paddr, int flags,
	int user)
{
	int i, istart = 0, iend = 8, pp;

	if (flags & FLAG_INSTR)
		iend = 4;
	else
		istart = 4;

	if (cpu->cd.ppc.bits != 32) {
		fatal("TODO: ppc_bat() for non-32-bit\n");
		exit(1);
	}
	if (cpu->cd.ppc.cpu_type.flags & PPC_601) {
		fatal("TODO: ppc_bat() for PPC 601\n");
		exit(1);
	}

	/*  Scan either the 4 instruction BATs or the 4 data BATs:  */
	for (i=istart; i<iend; i++) {
		int regnr = SPR_IBAT0U + i * 2;
		uint32_t upper = cpu->cd.ppc.spr[regnr];
		uint32_t lower = cpu->cd.ppc.spr[regnr + 1];
		uint32_t phys = lower & BAT_RPN, ebs = upper & BAT_EPI;
		uint32_t mask = ((upper & BAT_BL) << 15) | 0x1ffff;

		/*  Not valid in either supervisor or user mode?  */
		if (user && !(upper & BAT_Vu))
			continue;
		if (!user && !(upper & BAT_Vs))
			continue;

		/*  Virtual address mismatch? Then skip.  */
		if ((vaddr & ~mask) != (ebs & ~mask))
			continue;

		*return_paddr = (vaddr & mask) | (phys & ~mask);

		pp = lower & BAT_PP;
		switch (pp) {
		case BAT_PP_NONE:
			return 0;
		case BAT_PP_RO_S:
		case BAT_PP_RO:
			return (flags & FLAG_WRITEFLAG)? 0 : 1;
		default:/*  BAT_PP_RW:  */
			return 2;
		}
	}

	return -1;
}


/*
 *  get_pte_low():
 *
 *  Scan a PTE group for a cmp (compare) value.
 *
 *  Returns 1 if the value was found, and *lowp is set to the low PTE word.
 *  Returns 0 if no match was found.
 */
static int get_pte_low(struct cpu *cpu, uint64_t pteg_select,
	uint32_t *lowp, uint32_t cmp)
{
	unsigned char *d = memory_paddr_to_hostaddr(cpu->mem, pteg_select, 1);
	int i;

	for (i=0; i<8; i++) {
		uint32_t *ep = (uint32_t *) (d + (i << 3)), upper;
		upper = *ep;
		upper = BE32_TO_HOST(upper);

		/*  Valid PTE, and correct api and vsid?  */
		if (upper == cmp) {
			uint32_t lo = ep[1];
			lo = BE32_TO_HOST(lo);
			*lowp = lo;
			return 1;
		}
	}

	return 0;
}


/*
 *  ppc_vtp32():
 *
 *  Virtual to physical address translation (32-bit mode).
 *
 *  Returns 1 if a translation was found, 0 if none was found. However, finding
 *  a translation does not mean that it should be returned; there can be
 *  a permission violation. *resp is set to 0 for no access, 1 for read-only
 *  access, or 2 for read/write access.
 */
static int ppc_vtp32(struct cpu *cpu, uint32_t vaddr, uint64_t *return_paddr,
	int *resp, uint64_t msr, int writeflag, int instr)
{
	int srn = (vaddr >> 28) & 15, api = (vaddr >> 22) & PTE_API;
	int access, key, match;
	uint32_t vsid = cpu->cd.ppc.sr[srn] & 0x00ffffff;
	uint64_t sdr1 = cpu->cd.ppc.spr[SPR_SDR1], htaborg;
	uint32_t hash1, hash2, pteg_select, tmp;
	uint32_t lower_pte = 0, cmp;

	htaborg = sdr1 & 0xffff0000UL;

	/*  Primary hash:  */
	hash1 = (vsid & 0x7ffff) ^ ((vaddr >> 12) & 0xffff);
	tmp = (hash1 >> 10) & (sdr1 & 0x1ff);
	pteg_select = htaborg & 0xfe000000;
	pteg_select |= ((hash1 & 0x3ff) << 6);
	pteg_select |= (htaborg & 0x01ff0000) | (tmp << 16);
	cpu->cd.ppc.spr[SPR_HASH1] = pteg_select;
	cmp = cpu->cd.ppc.spr[instr? SPR_ICMP : SPR_DCMP] =
	    PTE_VALID | api | (vsid << PTE_VSID_SHFT);
	match = get_pte_low(cpu, pteg_select, &lower_pte, cmp);

	/*  Secondary hash:  */
	hash2 = hash1 ^ 0x7ffff;
	tmp = (hash2 >> 10) & (sdr1 & 0x1ff);
	pteg_select = htaborg & 0xfe000000;
	pteg_select |= ((hash2 & 0x3ff) << 6);
	pteg_select |= (htaborg & 0x01ff0000) | (tmp << 16);
	cpu->cd.ppc.spr[SPR_HASH2] = pteg_select;
	if (!match) {
		cmp |= PTE_HID;
		match = get_pte_low(cpu, pteg_select, &lower_pte, cmp);
	}

	*resp = 0;

	if (!match)
		return 0;

	/*  Non-executable, or Guarded page?  */
	if (instr && cpu->cd.ppc.sr[srn] & SR_NOEXEC)
		return 1;
	if (instr && lower_pte & PTE_G)
		return 1;

	access = lower_pte & PTE_PP;
	*return_paddr = (lower_pte & PTE_RPGN) | (vaddr & ~PTE_RPGN);

	key = (cpu->cd.ppc.sr[srn] & SR_PRKEY && msr & PPC_MSR_PR) ||
	    (cpu->cd.ppc.sr[srn] & SR_SUKEY && !(msr & PPC_MSR_PR));

	if (key) {
		switch (access) {
		case 1:
		case 3:	*resp = writeflag? 0 : 1;
			break;
		case 2:	*resp = 2;
			break;
		}
	} else {
		switch (access) {
		case 3:	*resp = writeflag? 0 : 1;
			break;
		default:*resp = 2;
		}
	}

	return 1;
}


/*
 *  ppc_translate_v2p():
 *
 *  Don't call this function if userland_emul is non-NULL, or cpu is NULL.
 *
 *  Return values:
 *	0  Failure
 *	1  Success, the page is readable only
 *	2  Success, the page is read/write
 */
int ppc_translate_v2p(struct cpu *cpu, uint64_t vaddr,
	uint64_t *return_paddr, int flags)
{
	int instr = flags & FLAG_INSTR, res = 0, match, user;
	int writeflag = flags & FLAG_WRITEFLAG? 1 : 0;
	uint64_t msr;

	reg_access_msr(cpu, &msr, 0, 0);
	user = msr & PPC_MSR_PR? 1 : 0;

	if (cpu->cd.ppc.bits == 32)
		vaddr &= 0xffffffff;

	if ((instr && !(msr & PPC_MSR_IR)) || (!instr && !(msr & PPC_MSR_DR))) {
		*return_paddr = vaddr;
		return 2;
	}

	if (cpu->cd.ppc.cpu_type.flags & PPC_601) {
		fatal("ppc_translate_v2p(): TODO: 601\n");
		exit(1);
	}

	/*  Try the BATs first:  */
	if (cpu->cd.ppc.bits == 32) {
		res = ppc_bat(cpu, vaddr, return_paddr, flags, user);
		if (res > 0)
			return res;
		if (res == 0) {
			fatal("[ TODO: BAT exception ]\n");
			exit(1);
		}
	}

	/*  Virtual to physical translation:  */
	if (cpu->cd.ppc.bits == 32) {
		match = ppc_vtp32(cpu, vaddr, return_paddr, &res, msr,
		    writeflag, instr);
		if (match && res > 0)
			return res;
	} else {
		/*  htaborg = sdr1 & 0xfffffffffffc0000ULL;  */
		fatal("TODO: ppc 64-bit translation\n");
		exit(1);
	}


	/*
	 *  No match? Then cause an exception.
	 *
	 *  PPC603: cause a software TLB reload exception.
	 *  All others: cause a DSI or ISI.
	 */

	if (flags & FLAG_NOEXCEPTIONS)
		return 0;

	if (!quiet_mode)
		fatal("[ memory_ppc: exception! vaddr=0x%"PRIx64" pc=0x%"PRIx64
		    " instr=%i user=%i wf=%i ]\n", (uint64_t) vaddr,
		    (uint64_t) cpu->pc, instr, user, writeflag);

	if (cpu->cd.ppc.cpu_type.flags & PPC_603) {
		cpu->cd.ppc.spr[instr? SPR_IMISS : SPR_DMISS] = vaddr;

		msr |= PPC_MSR_TGPR;
		reg_access_msr(cpu, &msr, 1, 0);

		ppc_exception(cpu, instr? 0x10 : (writeflag? 0x12 : 0x11));
	} else {
		if (!instr) {
			cpu->cd.ppc.spr[SPR_DAR] = vaddr;
			cpu->cd.ppc.spr[SPR_DSISR] = match?
			    DSISR_PROTECT : DSISR_NOTFOUND;
			if (writeflag)
				cpu->cd.ppc.spr[SPR_DSISR] |= DSISR_STORE;
		}
		ppc_exception(cpu, instr?
		    PPC_EXCEPTION_ISI : PPC_EXCEPTION_DSI);
	}

	return 0;
}

