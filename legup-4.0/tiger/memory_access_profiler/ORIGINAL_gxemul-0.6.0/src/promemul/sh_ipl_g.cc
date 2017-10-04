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
 *  COMMENT: SH-IPL+G emulation
 *
 *  Very basic, only what is needed to get OpenBSD/landisk booting.
 *  (SH-IPL+G stands for SuperH Initial Program Loader + GDB stub.)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "cpu.h"
#include "cpu_sh.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"

#include "thirdparty/sh4_exception.h"


/*
 *  sh_ipl_g_emul_init():
 */
void sh_ipl_g_emul_init(struct machine *machine)
{
	struct cpu *cpu = machine->cpus[0];

	cpu->cd.sh.vbr = 0x8c000000;
	store_16bit_word(cpu, 0x8c000100, SH_INVALID_INSTR);
	store_16bit_word(cpu, 0x8c000102, 0x002b);  	/*  rte  */
	store_16bit_word(cpu, 0x8c000104, 0x0009);  	/*  nop  */
}


/*
 *  sh_ipl_g_promcall():
 *
 *  SH-IPL+G PROM call emulation.
 */
static int sh_ipl_g_promcall(struct cpu *cpu)
{
	/*
	 *  SH-IPL+G call numbers are in R0:
	 *
	 *  NOTE: r_bank[0], since this is in a trap handler!
	 */
	switch (cpu->cd.sh.r_bank[0]) {

	case 4:	/*  Get memory size.  */
		cpu->cd.sh.r_bank[0] = 64 * 1048576;
		/*  Note: cpu->machine->physical_ram_in_mb * 1048576
		    would be more correct, but physical_ram_in_mb is
		    set to 2 for landisk emulation...  */
		break;

	default:cpu_register_dump(cpu->machine, cpu, 1, 0);
		printf("\n");
		fatal("[ SH-IPL+G PROM emulation: unimplemented function 0x%"
		    PRIx32" ]\n", cpu->cd.sh.r_bank[0]);
		cpu->running = 0;
		return 0;
	}

	return 1;
}


/*
 *  sh_ipl_g_emul():
 */
int sh_ipl_g_emul(struct cpu *cpu)
{
	/*  SH-IPL+G calls are "trapa #63":  */
	if (cpu->cd.sh.expevt == EXPEVT_TRAPA &&
	    cpu->cd.sh.tra == 0xfc) {
		return sh_ipl_g_promcall(cpu);
	} else {
		cpu_register_dump(cpu->machine, cpu, 1, 0);
		printf("\n");
		fatal("[ SH-IPL+G PROM emulation: expevt=0x%x, "
		    " tra=0x%x ]\n", (int)cpu->cd.sh.expevt,
		    (int)cpu->cd.sh.tra);
		cpu->running = 0;
		return 0;
	}
}

