/*
 *  Copyright (C) 2003-2009  Anders Gavare.  All rights reserved.
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
 *  COMMENT: Playstation 2 SIFBIOS emulation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "console.h"
#include "cpu.h"
#include "cpu_mips.h"
#include "machine.h"
#include "memory.h"


extern int quiet_mode;


/*
 *  playstation2_sifbios_emul():
 */
int playstation2_sifbios_emul(struct cpu *cpu)
{
	int callnr;

	callnr = cpu->cd.mips.gpr[MIPS_GPR_A0];

	switch (callnr) {
	case 0:			/*  getver()  */
		cpu->cd.mips.gpr[MIPS_GPR_V0] = 0x200;		/*  TODO  */
		break;
	case 1:			/*  halt(int mode)  */
		debug("[ SIFBIOS halt(0x%"PRIx64") ]\n",
		    (uint64_t) cpu->cd.mips.gpr[MIPS_GPR_A1]);
		cpu->running = 0;
		break;
	case 2:			/*  setdve(int mode)  */
		debug("[ SIFBIOS setdve(0x%"PRIx64") ]\n",
		    (uint64_t) cpu->cd.mips.gpr[MIPS_GPR_A1]);
		break;
	case 3:			/*  putchar(int ch)  */
		/*  debug("[ SIFBIOS putchar(0x%x) ]\n",
		    (char)cpu->cd.mips.gpr[MIPS_GPR_A1]);  */
		console_putchar(cpu->machine->main_console_handle,
		    cpu->cd.mips.gpr[MIPS_GPR_A1]);
		break;
	case 4:			/*  getchar()  */
		/*  This is untested. TODO  */
		/*  debug("[ SIFBIOS getchar() ]\n";  */
		cpu->cd.mips.gpr[MIPS_GPR_V0] = 0;
		if (console_charavail(cpu->machine->main_console_handle))
			cpu->cd.mips.gpr[MIPS_GPR_V0] = console_readchar(
			    cpu->machine->main_console_handle);
		break;
	case 16:		/*  dma_init()  */
		debug("[ SIFBIOS dma_init() ]\n");
		cpu->cd.mips.gpr[MIPS_GPR_V0] = 0;		/*  TODO  */
		break;
	case 17:		/*  dma_exit()  */
		debug("[ SIFBIOS dma_exit() ]\n");
		break;
	case 32:		/*  cmd_init()  */
		debug("[ SIFBIOS cmd_init() ]\n");
		cpu->cd.mips.gpr[MIPS_GPR_V0] = 0;		/*  TODO  */
		break;
	case 33:		/*  cmd_exit()  */
		debug("[ SIFBIOS cmd_exit() ]\n");
		break;
	case 48:
		debug("[ SIFBIOS rpc_init(): TODO ]\n");
		cpu->cd.mips.gpr[MIPS_GPR_V0] = 0;		/*  TODO  */
		break;
	case 49:
		debug("[ SIFBIOS rpc_exit(): TODO ]\n");
		cpu->cd.mips.gpr[MIPS_GPR_V0] = 0;		/*  TODO  */
		break;
	case 51:
		debug("[ SIFBIOS rpc_bind(): TODO ]\n");
		cpu->cd.mips.gpr[MIPS_GPR_V0] = 1;		/*  TODO  */
		break;
	case 64:
		fatal("[ SIFBIOS SBR_IOPH_INIT(0x%"PRIx32",0x%"PRIx32",0x%"
		    PRIx32"): TODO ]\n",
		    (uint32_t) cpu->cd.mips.gpr[MIPS_GPR_A1],
		    (uint32_t) cpu->cd.mips.gpr[MIPS_GPR_A2],
		    (uint32_t) cpu->cd.mips.gpr[MIPS_GPR_A3]);

		/*
		 *  This is really really ugly:   TODO
		 *
		 *  Linux and NetBSD seem to work, but it's an ugly hack.
		 *  This should really be a callback thingy...
		 *
		 *  NetBSD has a done-word which should be set to 1.
		 *  Linux has one done-word which should be set to 1, and
		 *  one which should be set to 0.
		 *
		 *  The code as it is right now probably overwrites stuff in
		 *  memory that shouldn't be touched. Not good.
		 *
		 *  Linux: err = sbios_rpc(SBR_IOPH_INIT, NULL, &result);
		 *         err should be 0 (just as NetBSD),
		 *         and result should be set to 0 as well.
		 */
		{
			uint32_t tmpaddr;

			tmpaddr = load_32bit_word(cpu,
			    cpu->cd.mips.gpr[MIPS_GPR_A1] + 0);
			fatal("  +0: %08x\n", tmpaddr);
			tmpaddr = load_32bit_word(cpu,
			    cpu->cd.mips.gpr[MIPS_GPR_A1] + 4);
			fatal("  +4: %08x\n", tmpaddr);
			tmpaddr = load_32bit_word(cpu,
			    cpu->cd.mips.gpr[MIPS_GPR_A1] + 8);
			fatal("  +8: %08x\n", tmpaddr);
			tmpaddr = load_32bit_word(cpu,
			    cpu->cd.mips.gpr[MIPS_GPR_A1] + 12);
			fatal(" +12: %08x\n", tmpaddr);

			/*  TODO: This is probably netbsd specific  */
			tmpaddr = load_32bit_word(cpu,
			    cpu->cd.mips.gpr[MIPS_GPR_A1] + 12);
			fatal("tmpaddr 1 = 0x%08x\n", tmpaddr);

			/*  "done" word for NetBSD:  */
			store_32bit_word(cpu, tmpaddr, 1);
			/*  "done" word A for Linux:  */
			store_32bit_word(cpu, tmpaddr + 4, 1);
			/*  "done" word B for Linux:  */
			store_32bit_word(cpu,
			    cpu->cd.mips.gpr[MIPS_GPR_A1] + 0, 0);
		}
		cpu->cd.mips.gpr[MIPS_GPR_V0] = 0;
		break;
	case 65:
		fatal("[ SIFBIOS alloc iop heap(0x"PRIx32") ]\n",
		    (uint32_t)cpu->cd.mips.gpr[MIPS_GPR_A1]);

		/*
		 *  Linux uses this to allocate "heap" for the OHCI USB
		 *  controller.
		 *
		 *  TODO:  This naïve implementation does not allow for a
		 *  "free iop heap" function: :-/
		 */

		{
			uint32_t tmpaddr;
			static uint32_t return_addr = 0x1000;
				/*  0xbc000000;  */
			uint32_t size;

			tmpaddr = load_32bit_word(cpu,
			    cpu->cd.mips.gpr[MIPS_GPR_A1] + 0);
			fatal("  +0: %08x (result should be placed here)\n",
			    tmpaddr);
			tmpaddr = load_32bit_word(cpu,
			    cpu->cd.mips.gpr[MIPS_GPR_A1] + 4);
			fatal("  +4: %08x (*arg)\n", tmpaddr);
			size = load_32bit_word(cpu, tmpaddr + 0);
			fatal("      size = %08x\n", size);
			tmpaddr = load_32bit_word(cpu,
			    cpu->cd.mips.gpr[MIPS_GPR_A1] + 8);
			fatal("  +8: %08x (*func (void *, int))\n", tmpaddr);
			tmpaddr = load_32bit_word(cpu,
			    cpu->cd.mips.gpr[MIPS_GPR_A1] + 12);
			fatal(" +12: %08x (*para)\n", tmpaddr);

			/*  TODO: This is probably netbsd specific  */
			tmpaddr = load_32bit_word(cpu,
			    cpu->cd.mips.gpr[MIPS_GPR_A1] + 12);
			fatal("tmpaddr 1 = 0x%08x\n", tmpaddr);

			/*  "done" word for NetBSD:  */
			store_32bit_word(cpu, tmpaddr, 1);
			/*  "done" word A for Linux:  */
			store_32bit_word(cpu, tmpaddr + 4, 1);

			/*  Result:  */
			store_32bit_word(cpu,
			    cpu->cd.mips.gpr[MIPS_GPR_A1] + 0, return_addr);

			return_addr += size;
			/*  Round up to next page:  */
			return_addr += 4095;
			return_addr &= ~4095;
		}
		cpu->cd.mips.gpr[MIPS_GPR_V0] = 0;
		break;
	case 66:
		debug("[ SIFBIOS iopmem_free(): TODO ]\n");
		cpu->cd.mips.gpr[MIPS_GPR_V0] = 0;		/*  TODO  */
		break;
	default:
		quiet_mode = 0;
		cpu_register_dump(cpu->machine, cpu, 1, 0x1);
		printf("\n");
		fatal("Playstation 2 SIFBIOS emulation: "
		    "unimplemented call nr 0x%x\n", callnr);
		cpu->running = 0;
	}

	return 1;
}

