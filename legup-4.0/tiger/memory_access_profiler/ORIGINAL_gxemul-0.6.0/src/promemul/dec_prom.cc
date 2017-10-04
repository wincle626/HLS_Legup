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
 *  COMMENT: DECstation PROM emulation
 *
 *  Implementation note: Remember that only the lowest 32 bits of GPRs are
 *  actually valid when using dyntrans with e.g. R3000 CPUs.
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
#include "diskimage.h"
#include "machine.h"
#include "machine_pmax.h"
#include "memory.h"
#include "misc.h"

#include "thirdparty/dec_prom.h"
#include "thirdparty/dec_5100.h"
#include "thirdparty/dec_kn01.h"
#include "thirdparty/dec_kn02.h"
#include "thirdparty/dec_kn03.h"


extern int quiet_mode;


/*
 *  mem_readchar():
 *
 *  Reads a byte from emulated RAM, using a MIPS register as a base address.
 *  (Helper function.)
 */
static unsigned char mem_readchar(struct cpu *cpu, int regbase, int offset)
{
	unsigned char ch;
	cpu->memory_rw(cpu, cpu->mem, (int32_t)cpu->cd.mips.gpr[regbase] +
	    offset, &ch, sizeof(ch), MEM_READ, CACHE_DATA | NO_EXCEPTIONS);
	return ch;
}


/*
 *  dec_jumptable_func():
 *
 *  The jumptable is located at the beginning of the PROM, at 0xbfc00000 + i*8,
 *  where i is the decimal function number. Many of these can be converted to
 *  an identical callback function.
 *
 *  Return value is non-zero if the vector number was converted into a callback
 *  function number, otherwise 0.
 *
 *	Vector	(dec)	Function
 *	0x0	0	reset()
 *	0x10	2	restart()
 *	0x18	3	reinit()
 *	0x30	6	open()
 *	0x38	7	read()
 *	0x58	11	lseek()
 *	0x68	13	putchar()
 *	0x88	17	printf()
 *	0x108	33	getenv2()
 */
int dec_jumptable_func(struct cpu *cpu, int vector)
{
	int i;
	static int file_opened = 0;
	static int current_file_offset = 0;

	switch (vector) {
	case 0x0:	/*  reset()  */
		/*  TODO  */
		cpu->machine->exit_without_entering_debugger = 1;
		cpu->running = 0;
		break;
	case 0x10:	/*  restart()  */
		/*  TODO  */
		cpu->machine->exit_without_entering_debugger = 1;
		cpu->running = 0;
		break;
	case 0x18:	/*  reinit()  */
		/*  TODO  */
		cpu->machine->exit_without_entering_debugger = 1;
		cpu->cd.mips.gpr[MIPS_GPR_V0] = 0;
		break;
	case 0x30:	/*  open()  */
		/*
		 *  TODO: This is just a hack to allow Sprite/pmax' bootblock
		 *  code to load /vmsprite. The filename argument (in A0)
		 *  is ignored, and a file handle value of 1 is returned.
		 */
		if (file_opened) {
			fatal("\ndec_jumptable_func(): opening more than one "
			    "file isn't supported yet.\n");
			cpu->running = 0;
		}
		file_opened = 1;
		cpu->cd.mips.gpr[MIPS_GPR_V0] = 1;
		break;
	case 0x38:	/*  read(handle, ptr, length)  */
		cpu->cd.mips.gpr[MIPS_GPR_V0] = (uint64_t) -1;
		if ((int32_t)cpu->cd.mips.gpr[MIPS_GPR_A2] > 0) {
			int disk_id = diskimage_bootdev(cpu->machine, NULL);
			int res;
			unsigned char *tmp_buf;

			CHECK_ALLOCATION(tmp_buf = (unsigned char *)
			    malloc(cpu->cd.mips.gpr[MIPS_GPR_A2]));

			res = diskimage_access(cpu->machine, disk_id,
			    DISKIMAGE_SCSI, 0, current_file_offset, tmp_buf,
			    cpu->cd.mips.gpr[MIPS_GPR_A2]);

			/*  If the transfer was successful, transfer the data
			    to emulated memory:  */
			if (res) {
				uint64_t dst = cpu->cd.mips.gpr[MIPS_GPR_A1];
				store_buf(cpu, dst, (char *)tmp_buf,
				    cpu->cd.mips.gpr[MIPS_GPR_A2]);
				cpu->cd.mips.gpr[MIPS_GPR_V0] =
				    cpu->cd.mips.gpr[MIPS_GPR_A2];
				current_file_offset +=
				    cpu->cd.mips.gpr[MIPS_GPR_A2];
			}

			free(tmp_buf);
		}
		break;
	case 0x58:	/*  lseek(handle, offset[, whence])  */
		/*  TODO  */
		if (cpu->cd.mips.gpr[MIPS_GPR_A2] == 0)
			current_file_offset = cpu->cd.mips.gpr[MIPS_GPR_A1];
		else
			fatal("WARNING! Unimplemented whence in "
			    "dec_jumptable_func()\n");
		cpu->cd.mips.gpr[MIPS_GPR_V0] = 0;
		break;
	case 0x68:	/*  putchar()  */
		console_putchar(cpu->machine->main_console_handle,
		    cpu->cd.mips.gpr[MIPS_GPR_A0]);
		break;
	case 0x88:	/*  printf()  */
		return 0x30;
	case 0x108:	/*  getenv2()  */
		return 0x64;
	default:
		cpu_register_dump(cpu->machine, cpu, 1, 0x1);
		printf("a0 points to: ");
		for (i=0; i<40; i++) {
			unsigned char ch = '\0';
			cpu->memory_rw(cpu, cpu->mem,
			    (int32_t)cpu->cd.mips.gpr[MIPS_GPR_A0] + i, &ch,
			    sizeof(ch), MEM_READ, CACHE_DATA | NO_EXCEPTIONS);
			if (ch >= ' ' && ch < 126)
				printf("%c", ch);
			else
				printf("[%02x]", ch);
		}
		printf("\n");
		fatal("PROM emulation: unimplemented JUMP TABLE vector "
		    "0x%x (decimal function %i)\n", vector, vector/8);
		cpu->running = 0;
	}

	return 0;
}


/*
 *  decstation_prom_emul():
 *
 *  DECstation PROM emulation.
 *
 *  Callback functions:
 *	0x0c	strcmp()
 *	0x14	strlen()
 *	0x24	getchar()
 *	0x28	gets()
 *	0x2c	puts()
 *	0x30	printf()
 *	0x38	iopoll()
 *	0x54	bootinit()
 *	0x58	bootread()
 *	0x64	getenv()
 *	0x6c	slot_address()
 *	0x70	wbflush()
 *	0x7c	clear_cache()
 *	0x80	getsysid()
 *	0x84	getbitmap()
 *	0x88	disableintr()
 *	0x8c	enableintr()
 *	0x9c	halt()
 *	0xa4	gettcinfo()
 *	0xa8	execute_cmd()
 *	0xac	rex()
 */
int decstation_prom_emul(struct cpu *cpu)
{
	int i, j, ch, argreg, argdata;
	int vector = cpu->pc & 0xfff;
	int callback = (cpu->pc & 0xf000)? 1 : 0;
	unsigned char buf[100];
	unsigned char ch1, ch2, ch3;
	uint64_t tmpaddr, slot_base = 0x10000000, slot_size = 0;

	if (!callback) {
		vector = dec_jumptable_func(cpu, vector);
		if (vector == 0)
			return 1;
	} else {
		/*  Vector number is n*4, PC points to n*8.  */
		vector /= 2;
	}

	switch (vector) {
	case 0x0c:		/*  strcmp():  */
		i = j = 0;
		do {
			ch1 = mem_readchar(cpu, MIPS_GPR_A0, i++);
			ch2 = mem_readchar(cpu, MIPS_GPR_A1, j++);
		} while (ch1 == ch2 && ch1 != '\0');

		/*  If ch1=='\0', then strings are equal.  */
		if (ch1 == '\0')
			cpu->cd.mips.gpr[MIPS_GPR_V0] = 0;
		if ((signed char)ch1 > (signed char)ch2)
			cpu->cd.mips.gpr[MIPS_GPR_V0] = 1;
		if ((signed char)ch1 < (signed char)ch2)
			cpu->cd.mips.gpr[MIPS_GPR_V0] = (uint64_t) -1;
		break;
	case 0x14:		/*  strlen():  */
		i = 0;
		do {
			ch2 = mem_readchar(cpu, MIPS_GPR_A0, i++);
		} while (ch2 != 0);
		cpu->cd.mips.gpr[MIPS_GPR_V0] = i - 1;
		break;
	case 0x24:		/*  getchar()  */
		/*  debug("[ DEC PROM getchar() ]\n");  */
		cpu->cd.mips.gpr[MIPS_GPR_V0] = console_readchar(
		    cpu->machine->main_console_handle);
		break;
	case 0x28:		/*  gets()  */
		/*  debug("[ DEC PROM gets() ]\n");  */
		tmpaddr = cpu->cd.mips.gpr[MIPS_GPR_A0];
		i = 0;

		/*  TODO: Make this not hang (block) the entire emulator  */

		do {
			while ((ch = console_readchar(
			    cpu->machine->main_console_handle)) < 1)
				;
			if (ch == '\r')
				ch = '\n';
			ch2 = ch;

			if (ch == '\b') {
				if (i > 0) {
					console_putchar(cpu->machine->
					    main_console_handle, ch2);
					console_putchar(cpu->machine->
					    main_console_handle, ' ');
					console_putchar(cpu->machine->
					    main_console_handle, ch2);
				}
			} else
				console_putchar(cpu->machine->
				    main_console_handle, ch2);

			fflush(stdout);

			if (ch == '\n') {
				/*  It seems that trailing newlines
				    are not included in the buffer.  */
			} else if (ch != '\b') {
				cpu->memory_rw(cpu, cpu->mem, (int32_t)
				    cpu->cd.mips.gpr[MIPS_GPR_A0] + i,
				    &ch2, sizeof(ch2), MEM_WRITE,
				    CACHE_DATA | NO_EXCEPTIONS);
				i++;
			} else {
				if (i > 0)
					i--;
			}
		} while (ch2 != '\n');

		/*  Trailing nul-byte:  */
		ch2 = '\0';
		cpu->memory_rw(cpu, cpu->mem, (int32_t)
		    cpu->cd.mips.gpr[MIPS_GPR_A0] + i, &ch2, sizeof(ch2),
		    MEM_WRITE, CACHE_DATA | NO_EXCEPTIONS);

		/*  Return the input argument:  */
		cpu->cd.mips.gpr[MIPS_GPR_V0] = cpu->cd.mips.gpr[MIPS_GPR_A0];
		break;
	case 0x2c:		/*  puts()  */
		i = 0;
		while ((ch = mem_readchar(cpu, MIPS_GPR_A0, i++)) != '\0')
			console_putchar(cpu->machine->main_console_handle, ch);
		console_putchar(cpu->machine->main_console_handle, '\n');
		cpu->cd.mips.gpr[MIPS_GPR_V0] = 0;
		break;
	case 0x30:		/*  printf()  */
		if (cpu->machine->register_dump ||
		    cpu->machine->instruction_trace)
			debug("PROM printf(0x%08lx): \n",
			    (long)cpu->cd.mips.gpr[MIPS_GPR_A0]);

		i = 0; ch = -1; argreg = MIPS_GPR_A1;
		while (ch != '\0') {
			char printfbuf[8000];
			size_t x;

			printfbuf[0] = printfbuf[sizeof(printfbuf)-1] = '\0';

			ch = mem_readchar(cpu, MIPS_GPR_A0, i++);
			switch (ch) {
			case '%':
				ch = '0';
				while (ch >= '0' && ch <= '9')
					ch = mem_readchar(cpu,
					    MIPS_GPR_A0, i++);

				switch (ch) {
				case '%':
					strlcpy(printfbuf, "%%",
					    sizeof(printfbuf));
					break;
				case 'c':
				case 'd':
				case 's':
				case 'x':
					/*  Get argument:  */
					if (argreg > MIPS_GPR_A3) {
#if 1
						/*  Linux booters seem to go
						    over the edge sometimes: */
						ch = '\0';
						strlcpy(printfbuf, "[...]\n",
						    sizeof(printfbuf));
#else
						printf("[ decstation_prom_emul"
						    "(): too many arguments ]");
						/*  This reuses the last arg,
						    which is utterly incorrect.
						    (TODO)  */
						argreg = MIPS_GPR_A3;
#endif
					}

					ch2 = argdata =
					    cpu->cd.mips.gpr[argreg];

					switch (ch) {
					case 'c':
						snprintf(printfbuf, sizeof(
						    printfbuf), "%c", ch2);
						break;
					case 'd':
						snprintf(printfbuf, sizeof(
						    printfbuf), "%d", argdata);
						break;
					case 'x':
						snprintf(printfbuf, sizeof(
						    printfbuf), "%x", argdata);
						break;
					case 's':
						/*  Print a "%s" string.  */
						j = 0; ch3 = '\n';
						while (ch2) {
							ch2 = mem_readchar(cpu,
							    argreg, j++);
							if (ch2) {
								snprintf(
								    printfbuf +
								    strlen(
								    printfbuf),
								    sizeof(
								    printfbuf)-
								    1-strlen(
								    printfbuf),
								    "%c", ch2);
								ch3 = ch2;
							}
						}
						break;
					}
					argreg ++;
					break;
				default:
					printf("[ unknown printf format char"
					    " '%c' ]", ch);
				}
				break;
			case '\0':
				break;
			default:
				snprintf(printfbuf, sizeof(printfbuf),
				    "%c", ch);
			}

			printfbuf[sizeof(printfbuf)-1] = '\0';

			for (x=0; x<strlen(printfbuf); x++)
				console_putchar(cpu->machine->
				    main_console_handle, printfbuf[x]);
		}
		if (cpu->machine->register_dump ||
		    cpu->machine->instruction_trace)
			debug("\n");
		fflush(stdout);
		cpu->cd.mips.gpr[MIPS_GPR_V0] = 0;
		break;
	case 0x54:		/*  bootinit()  */
		/*  debug("[ DEC PROM bootinit(0x%08x): TODO ]\n",
		    (int)cpu->cd.mips.gpr[MIPS_GPR_A0]);  */
		cpu->cd.mips.gpr[MIPS_GPR_V0] = 0;
		break;
	case 0x58:		/*  bootread(int b, void *buffer, int n)  */
		/*
		 *  Read data from the boot device.
		 *  b is a sector number (512 bytes per sector),
		 *  buffer is the destination address, and n
		 *  is the number of _bytes_ to read.
		 *
		 *  TODO: Return value? NetBSD thinks that 0 is ok.
		 */
		debug("[ DEC PROM bootread(0x%x, 0x%08x, 0x%x) ]\n",
		    (int)cpu->cd.mips.gpr[MIPS_GPR_A0],
		    (int)cpu->cd.mips.gpr[MIPS_GPR_A1],
		    (int)cpu->cd.mips.gpr[MIPS_GPR_A2]);

		cpu->cd.mips.gpr[MIPS_GPR_V0] = 0;

		if ((int32_t)cpu->cd.mips.gpr[MIPS_GPR_A2] > 0) {
			int disk_id = diskimage_bootdev(cpu->machine, NULL);
			int res;
			unsigned char *tmp_buf;

			CHECK_ALLOCATION(tmp_buf = (unsigned char *)
			    malloc(cpu->cd.mips.gpr[MIPS_GPR_A2]));

			res = diskimage_access(cpu->machine, disk_id,
			    DISKIMAGE_SCSI, 0,
			    cpu->cd.mips.gpr[MIPS_GPR_A0] * 512, tmp_buf,
			    cpu->cd.mips.gpr[MIPS_GPR_A2]);

			/*  If the transfer was successful, transfer the data
			    to emulated memory:  */
			if (res) {
				uint64_t dst = cpu->cd.mips.gpr[MIPS_GPR_A1];
				if (dst < 0x80000000ULL)
					dst |= 0x80000000;

				store_buf(cpu, dst, (char *)tmp_buf,
				    cpu->cd.mips.gpr[MIPS_GPR_A2]);
				cpu->cd.mips.gpr[MIPS_GPR_V0] =
				    cpu->cd.mips.gpr[MIPS_GPR_A2];
			}

			free(tmp_buf);
		}
		break;
	case 0x64:		/*  getenv()  */
		/*  Find the environment variable given by a0:  */
		for (i=0; i<(int)sizeof(buf); i++)
			cpu->memory_rw(cpu, cpu->mem, (int32_t)
			    cpu->cd.mips.gpr[MIPS_GPR_A0] + i, &buf[i],
			    sizeof(char), MEM_READ, CACHE_DATA | NO_EXCEPTIONS);
		buf[sizeof(buf)-1] = '\0';
		debug("[ DEC PROM getenv(\"%s\") ]\n", buf);
		for (i=0; i<0x1000; i++) {
			/*  Matching string at offset i?  */
			int nmatches = 0;
			for (j=0; j<(int32_t)strlen((char *)buf); j++) {
				cpu->memory_rw(cpu, cpu->mem, (int32_t)
				    (DEC_PROM_STRINGS + i + j), &ch2,
				    sizeof(char), MEM_READ, CACHE_DATA |
				    NO_EXCEPTIONS);
				if (ch2 == buf[j])
					nmatches++;
			}
			cpu->memory_rw(cpu, cpu->mem, (int32_t)(DEC_PROM_STRINGS
			    + i + strlen((char *)buf)), &ch2, sizeof(char),
			    MEM_READ, CACHE_DATA | NO_EXCEPTIONS);
			if (nmatches == (int)strlen((char *)buf) && ch2=='=') {
				cpu->cd.mips.gpr[MIPS_GPR_V0] =
				    DEC_PROM_STRINGS + i +
				    strlen((char *)buf) + 1;
				return 1;
			}
		}
		/*  Return NULL if string wasn't found.  */
		fatal("[ DEC PROM getenv(\"%s\"): WARNING: Not in "
		    "environment! ]\n", buf);
		cpu->cd.mips.gpr[MIPS_GPR_V0] = 0;
		break;
	case 0x6c:		/*  ulong slot_address(int sn)  */
		debug("[ DEC PROM slot_address(%i) ]\n",
		    (int)cpu->cd.mips.gpr[MIPS_GPR_A0]);
		/*  TODO:  This is too hardcoded.  */
		/*  TODO 2:  Should these be physical or virtual addresses?  */
		switch (cpu->machine->machine_subtype) {
		case MACHINE_DEC_3MAX_5000:
			slot_base = KN02_PHYS_TC_0_START;/*  0x1e000000  */
			slot_size = 4*1048576;		 /*  4 MB  */
			break;
		case MACHINE_DEC_3MIN_5000:
			slot_base = 0x10000000;
			slot_size = 0x4000000;		/*  64 MB  */
			break;
		case MACHINE_DEC_3MAXPLUS_5000:
			slot_base = 0x1e000000;
			slot_size = 0x800000;		/*  8 MB  */
			break;
		case MACHINE_DEC_MAXINE_5000:
			slot_base = 0x10000000;
			slot_size = 0x4000000;		/*  64 MB  */
			break;
		default:
			fatal("warning: DEC PROM slot_address() "
			    "unimplemented for this machine type\n");
		}
		cpu->cd.mips.gpr[MIPS_GPR_V0] = (int64_t)(int32_t)
		    (0x80000000 + slot_base + slot_size *
		    cpu->cd.mips.gpr[MIPS_GPR_A0]);
		break;
	case 0x70:		/*  wbflush()  */
		debug("[ DEC PROM wbflush(): TODO ]\n");
		cpu->cd.mips.gpr[MIPS_GPR_V0] = 0;
		break;
	case 0x7c:		/*  clear_cache(addr, len)  */
		debug("[ DEC PROM clear_cache(0x%x,%i) ]\n",
		    (uint32_t)cpu->cd.mips.gpr[MIPS_GPR_A0],
		    (int)cpu->cd.mips.gpr[MIPS_GPR_A1]);
		/*  TODO  */
		cpu->cd.mips.gpr[MIPS_GPR_V0] = 0;	/*  ?  */
		break;
	case 0x80:		/*  getsysid()  */
		/*  debug("[ DEC PROM getsysid() ]\n");  */
		/*  TODO:  why did I add the 0x82 stuff???  */
		cpu->cd.mips.gpr[MIPS_GPR_V0] = ((uint32_t)0x82 << 24)
		    + (cpu->machine->machine_subtype << 16) + (0x3 << 8);
		cpu->cd.mips.gpr[MIPS_GPR_V0] =
		    (int64_t)(int32_t)cpu->cd.mips.gpr[MIPS_GPR_V0];
		break;
	case 0x84:		/*  getbitmap()  */
		debug("[ DEC PROM getbitmap(0x%08x) ]\n",
		    (int)cpu->cd.mips.gpr[MIPS_GPR_A0]);
		store_buf(cpu, cpu->cd.mips.gpr[MIPS_GPR_A0],
		    (char *)cpu->machine->md.pmax->memmap,
		    sizeof(struct dec_memmap));
		cpu->cd.mips.gpr[MIPS_GPR_V0] =
		    sizeof(cpu->machine->md.pmax->memmap->bitmap);
		break;
	case 0x88:		/*  disableintr()  */
		debug("[ DEC PROM disableintr(): TODO ]\n");
		cpu->cd.mips.gpr[MIPS_GPR_V0] = 0;
		break;
	case 0x8c:		/*  enableintr()  */
		debug("[ DEC PROM enableintr(): TODO ]\n");
		cpu->cd.mips.gpr[MIPS_GPR_V0] = 0;
		break;
	case 0x9c:		/*  halt()  */
		debug("[ DEC PROM halt() ]\n");
		cpu->machine->exit_without_entering_debugger = 1;
		cpu->running = 0;
		break;
	case 0xa4:		/*  gettcinfo()  */
		/*
		 *  These are just bogus values...  TODO
		 *
		 *   0:  revision
		 *   4:  clock period in nano seconds
		 *   8:  slot size in megabytes  TODO: not same for all models!
		 *  12:  I/O timeout in cycles
		 *  16:  DMA address range in megabytes
		 *  20:  maximum DMA burst length
		 *  24:  turbochannel parity (yes = 1)
		 *  28:  reserved
		 */
		store_32bit_word(cpu, DEC_PROM_TCINFO +  0, 0);
		store_32bit_word(cpu, DEC_PROM_TCINFO +  4, 50);
		store_32bit_word(cpu, DEC_PROM_TCINFO +  8, 4);
		store_32bit_word(cpu, DEC_PROM_TCINFO + 12, 10);
		store_32bit_word(cpu, DEC_PROM_TCINFO + 16, 1);
		store_32bit_word(cpu, DEC_PROM_TCINFO + 20, 100);
		store_32bit_word(cpu, DEC_PROM_TCINFO + 24, 0);
		store_32bit_word(cpu, DEC_PROM_TCINFO + 28, 0);
		cpu->cd.mips.gpr[MIPS_GPR_V0] = DEC_PROM_TCINFO;
		break;
	case 0xa8:		/*  int execute_cmd(char *)  */
		i = 0;
		while ((ch = mem_readchar(cpu, MIPS_GPR_A0, i++)) != '\0')
			console_putchar(cpu->machine->main_console_handle, ch);
		console_putchar(cpu->machine->main_console_handle, '\n');
		cpu->cd.mips.gpr[MIPS_GPR_V0] = 0;
		break;
	case 0xac:		/*  rex()  */
		debug("[ DEC PROM rex('%c') ]\n",
		    (int)cpu->cd.mips.gpr[MIPS_GPR_A0]);
		switch ((int32_t)cpu->cd.mips.gpr[MIPS_GPR_A0]) {
		case 'h':
			debug("DEC PROM: rex('h') ==> halt\n");
			cpu->machine->exit_without_entering_debugger = 1;
			cpu->running = 0;
			break;
		case 'b':
			debug("DEC PROM: rex('b') ==> reboot: TODO "
			    "(halting CPU instead)\n");
			cpu->machine->exit_without_entering_debugger = 1;
			cpu->running = 0;
			break;
		default:
			fatal("DEC prom emulation: unknown rex() a0=0x%"PRIx64
			    " ('%c')\n",
			    (int64_t) cpu->cd.mips.gpr[MIPS_GPR_A0],
			    (char) cpu->cd.mips.gpr[MIPS_GPR_A0]);
			cpu->running = 0;
		}
		break;
	default:
		cpu_register_dump(cpu->machine, cpu, 1, 0x1);
		printf("a0 points to: ");
		for (i=0; i<40; i++) {
			unsigned char ch = '\0';
			cpu->memory_rw(cpu, cpu->mem, (int32_t)
			    cpu->cd.mips.gpr[MIPS_GPR_A0] + i, &ch,
			    sizeof(ch), MEM_READ, CACHE_DATA | NO_EXCEPTIONS);
			if (ch >= ' ' && ch < 126)
				printf("%c", ch);
			else
				printf("[%02x]", ch);
		}
		printf("\n");
		fatal("PROM emulation: unimplemented callback vector 0x%x\n",
		    vector);
		cpu->running = 0;
	}

	return 1;
}

