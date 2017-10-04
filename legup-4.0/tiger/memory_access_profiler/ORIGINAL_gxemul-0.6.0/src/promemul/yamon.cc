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
 *  COMMENT: YAMON emulation
 *
 *  (Very basic, only what is needed to get NetBSD booting.)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "console.h"
#include "cpu.h"
#include "cpu_mips.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"
#include "net.h"

#include "thirdparty/yamon.h"


/*
 *  yamon_machine_setup():
 */
void yamon_machine_setup(struct machine *machine, uint64_t env)
{
	char tmps[200];
	char macaddr[6];
	uint64_t tmpptr = env + 0x400;
	struct cpu *cpu = machine->cpus[0];

	/*
	 *  Standard YAMON environment variables:
	 *
	 *	baseboardserial
	 *	bootfile		TODO
	 *	bootprot		TODO: Non-tftp boot
	 *	bootserport
	 *	bootserver
	 *	cpuconfig		TODO
	 *	ethaddr
	 *	fpu			TODO
	 *	gateway
	 *	ipaddr			TODO: Don't hardcode!
	 *	memsize
	 *	modetty0
	 *	modetty1
	 *	prompt
	 *	start			TODO
	 *	startdelay		TODO
	 *	subnetmask		TODO: Real subnet mask
	 *	yamonrev
	 */

	add_environment_string_dual(cpu, &env, &tmpptr,
	    "baseboardserial", "0000000000");

	/*  TODO: Disk boot!  */
	add_environment_string_dual(cpu, &env, &tmpptr, "bootprot", "tftp");

	add_environment_string_dual(cpu, &env, &tmpptr, "bootserport", "tty0");

	add_environment_string_dual(cpu, &env, &tmpptr,
	    "bootserver", "10.0.0.254");

	net_generate_unique_mac(machine, (unsigned char *) macaddr);
	snprintf(tmps, sizeof(tmps), "%02x.%02x.%02x.%02x.%02x.%02x",
	    macaddr[0], macaddr[1], macaddr[2],
	    macaddr[3], macaddr[4], macaddr[5]);
	add_environment_string_dual(cpu, &env, &tmpptr, "ethaddr", tmps);

	add_environment_string_dual(cpu, &env, &tmpptr,
	    "gateway", "10.0.0.254");

	/*  TODO: Don't hardcode!  */
	add_environment_string_dual(cpu, &env, &tmpptr,
	    "ipaddr", "10.0.0.1");

	snprintf(tmps, sizeof(tmps), "0x%08x", machine->physical_ram_in_mb<<20);
	add_environment_string_dual(cpu, &env, &tmpptr, "memsize", tmps);

	add_environment_string_dual(cpu, &env, &tmpptr,
	    "modetty0", "38400,n,8,1,none");

	add_environment_string_dual(cpu, &env, &tmpptr,
	    "modetty1", "38400,n,8,1,none");

	add_environment_string_dual(cpu, &env, &tmpptr, "prompt", "YAMON");

	add_environment_string_dual(cpu, &env, &tmpptr, "yamonrev", "02.06");

	/*  TODO: Real subnet mask:  */
	add_environment_string_dual(cpu, &env, &tmpptr,
	    "subnetmask", "255.0.0.0");


	/*  FreeBSD development specific:  */
	snprintf(tmps, sizeof(tmps), "%i", machine->emulated_hz / 1000);
	add_environment_string_dual(cpu, &env, &tmpptr, "khz", tmps);

	/*  NULL terminate:  */
	tmpptr = 0;
	add_environment_string_dual(cpu, &env, &tmpptr, NULL, NULL);
}


/*
 *  yamon_emul():
 *
 *  YAMON emulation (for evbmips).
 */
int yamon_emul(struct cpu *cpu)
{
	uint32_t ofs = (cpu->pc & 0xff) + YAMON_FUNCTION_BASE;
	uint8_t ch;
	int n;
	uint32_t oid;
	uint64_t paddr, psize;

	switch (ofs) {

	case YAMON_PRINT_COUNT_OFS:
		/*
		 *  print count:
		 *	a1 = string
		 *	a2 = count
		 */
		n = 0;
		while (n < (int32_t)cpu->cd.mips.gpr[MIPS_GPR_A2]) {
			cpu->memory_rw(cpu, cpu->mem, (int32_t)cpu->cd.mips.gpr
			    [MIPS_GPR_A1] + n, &ch, sizeof(ch), MEM_READ,
			    CACHE_DATA | NO_EXCEPTIONS);
			console_putchar(cpu->machine->main_console_handle, ch);
			n++;
		}
		break;

	case YAMON_EXIT_OFS:
		/*
		 *  exit
		 */
		debug("[ yamon_emul(): exit ]\n");
		cpu->running = 0;
		break;

	/*  YAMON_FLUSH_CACHE_OFS: TODO  */
	/*  YAMON_PRINT_OFS: TODO  */
	/*  YAMON_REG_CPU_ISR_OFS: TODO  */
	/*  YAMON_DEREG_CPU_ISR_OFS: TODO  */
	/*  YAMON_REG_IC_ISR_OFS: TODO  */
	/*  YAMON_DEREG_IC_ISR_OFS: TODO  */
	/*  YAMON_REG_ESR_OFS: TODO  */
	/*  YAMON_DEREG_ESR_OFS: TODO  */

	case YAMON_GETCHAR_OFS:
		n = console_readchar(cpu->machine->main_console_handle);
		/*  Note: -1 (if no char was available) becomes 0xff:  */
		ch = n;
		cpu->memory_rw(cpu, cpu->mem, (int32_t)cpu->cd.mips.gpr[
		    MIPS_GPR_A1], &ch, sizeof(ch), MEM_WRITE,
		    CACHE_DATA | NO_EXCEPTIONS);
		break;

	case YAMON_SYSCON_READ_OFS:
		/*
		 *  syscon_read(oid [a0], addr [a1], size [a2])
		 */

		oid = cpu->cd.mips.gpr[MIPS_GPR_A0];
		paddr = cpu->cd.mips.gpr[MIPS_GPR_A1];
		psize = cpu->cd.mips.gpr[MIPS_GPR_A2];

		switch (oid) {
		case SYSCON_BOARD_CPU_CLOCK_FREQ_ID:
			if (psize == sizeof(uint32_t)) {
				uint32_t freq = cpu->machine->emulated_hz;

				debug("[ yamon_emul(): reporting CPU "
				    "frequency of %u ]\n", (unsigned int)
				    freq);

				if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
					freq = LE32_TO_HOST(freq);
				else
					freq = BE32_TO_HOST(freq);

				cpu->memory_rw(cpu, cpu->mem, (int32_t)paddr,
				    (unsigned char *) (void *) &freq, sizeof(freq), MEM_WRITE,
				    CACHE_DATA | NO_EXCEPTIONS);

				cpu->cd.mips.gpr[MIPS_GPR_V0] = 0;
			} else {
				cpu->cd.mips.gpr[MIPS_GPR_V0] = 1;
			}
			break;

		default:
			fatal("[ yamon_emul(): unimplemented object id 0x%"
			    PRIx32" ]\n", oid);
			cpu->cd.mips.gpr[MIPS_GPR_V0] = 1;
		}
		break;

	default:
		cpu_register_dump(cpu->machine, cpu, 1, 0);
		printf("\n");
		fatal("[ yamon_emul(): unimplemented yamon function 0x%"
		    PRIx32" ]\n", ofs);
		cpu->running = 0;
	}

	return 1;
}

