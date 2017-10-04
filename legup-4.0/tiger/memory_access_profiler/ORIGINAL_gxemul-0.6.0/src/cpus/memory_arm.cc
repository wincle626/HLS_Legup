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
 *  TODO/NOTE:  The B and/or C bits could also cause the return value to
 *  be MEMORY_NOT_FULL_PAGE, to make sure it doesn't get entered into the
 *  translation arrays. TODO: Find out if this is a good thing to do.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arm_cpu_types.h"
#include "cpu.h"
#include "memory.h"
#include "misc.h"


extern int quiet_mode;


/*
 *  arm_translate_v2p():
 *
 *  Address translation with the MMU disabled. (Just treat the virtual address
 *  as a physical address.)
 */
int arm_translate_v2p(struct cpu *cpu, uint64_t vaddr64,
	uint64_t *return_paddr, int flags)
{
	*return_paddr = vaddr64 & 0xffffffff;

	return 2;
}


/*
 *  arm_check_access():
 *
 *  Helper function.  Returns 0 for no access, 1 for read-only, and 2 for
 *  read/write.
 */
static int arm_check_access(struct cpu *cpu, int ap, int dav, int user)
{
	int s, r;

	switch (dav) {
	case 0:	/*  No access at all.  */
		return 0;
	case 1:	/*  Normal access check.  */
		break;
	case 2:	fatal("arm_check_access(): 1 shouldn't be used\n");
		exit(1);
	case 3:	/*  Anything is allowed.  */
		return 2;
	}

	switch (ap) {
	case 0:	s = (cpu->cd.arm.control & ARM_CONTROL_S)? 1 : 0;
		r = (cpu->cd.arm.control & ARM_CONTROL_R)? 2 : 0;
		switch (s + r) {
		case 0:	return 0;
		case 1:	return user? 0 : 1;
		case 2:	return 1;
		}
		fatal("arm_check_access: UNPREDICTABLE s+r value!\n");
		return 0;
	case 1:	return user? 0 : 2;
	case 2:	return user? 1 : 2;
	}

	/*  "case 3":  */
	return 2;
}


/*
 *  arm_translate_v2p_mmu():
 *
 *  Don't call this function if userland_emul is non-NULL, or cpu is NULL.
 *
 *  Return values:
 *	0  Failure
 *	1  Success, the page is readable only
 *	2  Success, the page is read/write
 *
 *  If this is a 1KB page access, then the return value is ORed with
 *  MEMORY_NOT_FULL_PAGE.
 */
int arm_translate_v2p_mmu(struct cpu *cpu, uint64_t vaddr64,
	uint64_t *return_paddr, int flags)
{
	unsigned char *q;
	uint32_t addr, d=0, d2 = (uint32_t)(int32_t)-1, ptba, vaddr = vaddr64;
	int instr = flags & FLAG_INSTR;
	int writeflag = (flags & FLAG_WRITEFLAG)? 1 : 0;
	int useraccess = flags & MEMORY_USER_ACCESS;
	int no_exceptions = flags & FLAG_NOEXCEPTIONS;
	int user = (cpu->cd.arm.cpsr & ARM_FLAG_MODE) == ARM_MODE_USR32;
	int domain, dav, ap0,ap1,ap2,ap3, ap = 0, access = 0;
	int fs = 2;		/*  fault status (2 = terminal exception)  */
	int subpage = 0;

	if (useraccess)
		user = 1;

	addr = ((vaddr & 0xfff00000ULL) >> 18);

	if (cpu->cd.arm.translation_table == NULL ||
	    cpu->cd.arm.ttb != cpu->cd.arm.last_ttb) {
		cpu->cd.arm.translation_table = memory_paddr_to_hostaddr(
		    cpu->mem, cpu->cd.arm.ttb & 0x0fffffff, 0);
		cpu->cd.arm.last_ttb = cpu->cd.arm.ttb;
	}

	if (cpu->cd.arm.translation_table != NULL) {
		d = *(uint32_t *)(cpu->cd.arm.translation_table + addr);
#ifdef HOST_LITTLE_ENDIAN
		if (cpu->byte_order == EMUL_BIG_ENDIAN)
#else
		if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
#endif
			d = ((d & 0xff) << 24) | ((d & 0xff00) << 8) |
			    ((d & 0xff0000) >> 8) | ((d & 0xff000000) >> 24);
	}

	/*  Get the domain from the descriptor, and the Domain Access Value:  */
	domain = (d >> 5) & 15;
	dav = (cpu->cd.arm.dacr >> (domain * 2)) & 3;

	switch (d & 3) {

	case 0:	domain = 0;
		fs = FAULT_TRANS_S;
		goto exception_return;

	case 1:	/*  Course Pagetable:  */
		if (dav == 0) {
			fs = FAULT_DOMAIN_P;
			goto exception_return;
		}
		ptba = d & 0xfffffc00;
		addr = ptba + ((vaddr & 0x000ff000) >> 10);

		q = memory_paddr_to_hostaddr(cpu->mem, addr & 0x0fffffff, 0);
		if (q == NULL) {
			printf("arm memory blah blah adfh asfg asdgasdg\n");
			exit(1);
		}
		d2 = *(uint32_t *)(q);
#ifdef HOST_LITTLE_ENDIAN
		if (cpu->byte_order == EMUL_BIG_ENDIAN)
#else
		if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
#endif
			d2 = ((d2 & 0xff) << 24) | ((d2 & 0xff00) << 8) |
			     ((d2 & 0xff0000) >> 8) | ((d2 & 0xff000000) >> 24);

		switch (d2 & 3) {
		case 0:	fs = FAULT_TRANS_P;
			goto exception_return;
		case 1:	/*  16KB page:  */
			ap = (d2 >> 4) & 255;
			switch (vaddr & 0x0000c000) {
			case 0x4000:	ap >>= 2; break;
			case 0x8000:	ap >>= 4; break;
			case 0xc000:	ap >>= 6; break;
			}
			ap &= 3;
			*return_paddr = (d2 & 0xffff0000)|(vaddr & 0x0000ffff);
			break;
		case 3:	if (cpu->cd.arm.cpu_type.flags & ARM_XSCALE) {
				/*  4KB page (Xscale)  */
				subpage = 0;
			} else {
				/*  1KB page  */
				subpage = 1;
				ap = (d2 >> 4) & 3;
				*return_paddr = (d2 & 0xfffffc00) |
				    (vaddr & 0x000003ff);
				break;
			}
			/*  NOTE: Fall-through for XScale!  */
		case 2:	/*  4KB page:  */
			ap3 = (d2 >> 10) & 3;
			ap2 = (d2 >>  8) & 3;
			ap1 = (d2 >>  6) & 3;
			ap0 = (d2 >>  4) & 3;
			switch (vaddr & 0x00000c00) {
			case 0x000: ap = ap0; break;
			case 0x400: ap = ap1; break;
			case 0x800: ap = ap2; break;
			default:    ap = ap3;
			}
			/*  NOTE: Ugly hack for XScale:  */
			if ((d2 & 3) == 3) {
				/*  Treated as 4KB page:  */
				ap = ap0;
			} else {
				if (ap0 != ap1 || ap0 != ap2 || ap0 != ap3)
					subpage = 1;
			}
			*return_paddr = (d2 & 0xfffff000)|(vaddr & 0x00000fff);
			break;
		}
		access = arm_check_access(cpu, ap, dav, user);
		if (access > writeflag)
			return access | (subpage? MEMORY_NOT_FULL_PAGE : 0);
		fs = FAULT_PERM_P;
		goto exception_return;

	case 2:	/*  Section descriptor:  */
		if (dav == 0) {
			fs = FAULT_DOMAIN_S;
			goto exception_return;
		}
		*return_paddr = (d & 0xfff00000) | (vaddr & 0x000fffff);
		ap = (d >> 10) & 3;
		access = arm_check_access(cpu, ap, dav, user);
		if (access > writeflag)
			return access;
		fs = FAULT_PERM_S;
		goto exception_return;

	default:fatal("TODO: descriptor for vaddr 0x%08x: 0x%08x ("
		    "unimplemented type %i)\n", vaddr, d, d&3);
		exit(1);
	}

exception_return:
	if (no_exceptions)
		return 0;

	if (!quiet_mode) {
		fatal("{ arm memory fault: vaddr=0x%08x domain=%i dav=%i ap=%i "
		    "access=%i user=%i", (int)vaddr, domain, dav, ap,
		    access, user);
		fatal(" d=0x%08x d2=0x%08x pc=0x%08x }\n", d, d2, (int)cpu->pc);
	}

	if (instr)
		arm_exception(cpu, ARM_EXCEPTION_PREF_ABT);
	else {
		cpu->cd.arm.far = vaddr;
		cpu->cd.arm.fsr = (domain << 4) | fs;
		arm_exception(cpu, ARM_EXCEPTION_DATA_ABT);
	}

	return 0;
}

