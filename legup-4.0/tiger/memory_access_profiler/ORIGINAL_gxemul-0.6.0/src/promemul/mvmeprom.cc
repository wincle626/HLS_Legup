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
 *  COMMENT: MVME PROM emulation
 *
 *  For mvme88k emulation.
 *
 *  TODO: Perhaps this could be reused for mvme68k emulation too?
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "console.h"
#include "cpu.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"

#include "thirdparty/mvmeprom.h"


#define	MVMEPROM_BRDID_ADDR	0x1100


/*
 *  mvmeprom_init():
 */
void mvmeprom_init(struct machine *machine)
{
	struct cpu *cpu = machine->cpus[0];
	struct mvmeprom_brdid mvmeprom_brdid;
	uint32_t vbr = 0x00000000;
	int model = 0x187;

	switch (machine->machine_subtype) {
	case MACHINE_MVME88K_187:  model = 0x187; break;
	case MACHINE_MVME88K_188:  model = 0x188; break;
	case MACHINE_MVME88K_197:  model = 0x197; break;
	}

	cpu->cd.m88k.cr[M88K_CR_VBR] = vbr;

	/*  A magic prom call instruction, followed by an 'rte':  */
	store_32bit_word(cpu, vbr + 8 * MVMEPROM_VECTOR, M88K_PROM_INSTR);
	store_32bit_word(cpu, vbr + 8 * MVMEPROM_VECTOR + 4, 0xf400fc00);

	/*  brdid struct:  */
	memset(&mvmeprom_brdid, 0, sizeof(mvmeprom_brdid));
	store_16bit_word_in_host(cpu, (unsigned char *)
	    &(mvmeprom_brdid.model), model);
	mvmeprom_brdid.speed[0] = '3';	/*  33 MHz, for now  */
	mvmeprom_brdid.speed[1] = '3';
	mvmeprom_brdid.speed[2] = '0';
	mvmeprom_brdid.speed[3] = '0';
	store_buf(cpu, MVMEPROM_BRDID_ADDR,
	    (char *)&mvmeprom_brdid, sizeof(struct mvmeprom_brdid));
}


/*
 *  mvmeprom_emul():
 *
 *  For MVME88K:
 *
 *  Input:
 *	r9 = requested function number
 *	r2 = first argument (for functions that take arguments)
 *
 *  Output:
 *	r2 = result
 */
int mvmeprom_emul(struct cpu *cpu)
{
	int func = cpu->cd.m88k.r[9];

	switch (func) {

	case MVMEPROM_OUTCHR:
		console_putchar(cpu->machine->main_console_handle,
		    cpu->cd.m88k.r[2]);
		break;

	case MVMEPROM_OUTCRLF:
		console_putchar(cpu->machine->main_console_handle, '\n');
		break;

	case MVMEPROM_EXIT:
		fatal("[ MVME PROM: exit ]\n");
		cpu->running = 0;
		break;

	case MVMEPROM_GETBRDID:
		/*  Return a pointer in r2 to a mvmeprom_brdid struct.  */
		cpu->cd.m88k.r[2] = MVMEPROM_BRDID_ADDR;
		break;

	default:
		cpu_register_dump(cpu->machine, cpu, 1, 0);
		cpu_register_dump(cpu->machine, cpu, 0, 1);
		fatal("[ MVME PROM emulation: unimplemented function 0x%"
		    PRIx32" ]\n", func);
		cpu->running = 0;
		return 0;
	}

	return 1;
}

