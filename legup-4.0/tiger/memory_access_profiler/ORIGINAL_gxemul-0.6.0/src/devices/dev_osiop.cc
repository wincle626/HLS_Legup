/*
 *  Copyright (C) 2008-2009  Anders Gavare.  All rights reserved.
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
 *  COMMENT: NCR 53C710 SCSI I/O Processor (SIOP)
 *
 *  Based on reverse-engineering OpenBSD's osiop.c, and a PDF manual:
 *  "Symbios SYM53C710 SCSI I/O Processor Technical Manual, version 3.1".
 *
 *  See e.g. openbsd/sys/dev/microcode/siop/osiop.ss for an example of what
 *  the SCRIPTS assembly language looks like (or osiop.out for the raw result).
 *
 *
 *  TODOs:
 *
 *  o)	Target mode. Right now, only Initiator mode is implemented.
 *  o)	Errors. Right now, the emulator aborts if there is a SCSI error.
 *  o)	Allow all phases to do partial transfers. Right now, only data in
 *	and data out support this (hackish).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "console.h"
#include "cpu.h"
#include "device.h"
#include "diskimage.h"
#include "interrupt.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"

#include "thirdparty/osiopreg.h"


/*  #define debug fatal  */
const int osiop_debug = 0;

static const char *phases[8] =
	{ "DATA_OUT", "DATA_IN", "COMMAND", "STATUS",
	  "RESERVED_OUT", "RESERVED_IN", "MSG_OUT", "MSG_IN" };

#define	DEV_OSIOP_LENGTH	OSIOP_NREGS
#define	OSIOP_CHIP_REVISION	2

#define	MAX_SCRIPTS_PER_CHUNK	256	/*  256 may be a reasonable value?  */

#define	OSIOP_TICK_SHIFT	17

struct osiop_data {
	struct interrupt	irq;
	int			asserted;
	
	int			scripts_running;

	/*  Current transfer:  */
	int			selected_id;
	struct scsi_transfer	*xferp;
	size_t			data_offset;

	/*  Cached emulated physical RAM page lookup:  */
	uint32_t		last_phys_page;
	uint8_t			*last_host_page;

	/*  ALU:  */
	int			carry;
	
	/*
	 *  Most of these are byte-addressed, but some are not. (Note: For
	 *  convenience, 32-bit words are stored in host byte order!)
	 */
	uint8_t			reg[OSIOP_NREGS];
};


static void osiop_free_xfer(struct osiop_data *d)
{
	if (d->xferp != NULL)
		scsi_transfer_free(d->xferp);

	d->xferp = NULL;
}


/*  Allocate memory for a new transfer.  */
static void osiop_new_xfer(struct osiop_data *d, int target_scsi_id)
{
	if (d->xferp != NULL) {
		fatal("WARNING! osiop_new_xfer(): freeing previous"
		    " transfer\n");
		osiop_free_xfer(d);
	}

	d->selected_id = target_scsi_id;
	d->xferp = scsi_transfer_alloc();
}


static int osiop_get_scsi_phase(struct osiop_data *d)
{
	return OSIOP_PHASE(d->reg[OSIOP_SOCL]);
}


static void osiop_set_scsi_phase(struct osiop_data *d, int phase)
{
	int mask = OSIOP_MSG | OSIOP_CD | OSIOP_IO;
	d->reg[OSIOP_SOCL] &= ~mask;
	d->reg[OSIOP_SOCL] |= (phase & mask);
}


/*
 *  osiop_update_sip_and_dip():
 *
 *  The SIP bit in ISTAT is basically a "summary" of whether there is
 *  a non-masked SCSI interrupt. Similarly for DIP, for DMA.
 *
 *  However, the SIP and DIP bits are set even if interrupts are
 *  masked away using DIEN and SIEN. (At least that is how I understood
 *  things from reading OpenBSD's osiop.c osiop_poll().) 
 */
static int osiop_update_sip_and_dip(struct osiop_data *d)
{
	int assert = 0;

	/*  First, let's assume no interrupt assertions.  */
	d->reg[OSIOP_ISTAT] &= ~(OSIOP_ISTAT_SIP | OSIOP_ISTAT_DIP);

	/*  Any enabled SCSI interrupts?  */
	if (d->reg[OSIOP_SSTAT0])
		d->reg[OSIOP_ISTAT] |= OSIOP_ISTAT_SIP;
	if (d->reg[OSIOP_SSTAT0] & d->reg[OSIOP_SIEN])
		assert = 1;

	/*  Or enabled DMA interrupts?  */
	if (d->reg[OSIOP_DSTAT] & ~OSIOP_DIEN_RES)
		d->reg[OSIOP_ISTAT] |= OSIOP_ISTAT_DIP;
	if (d->reg[OSIOP_DSTAT] & d->reg[OSIOP_DIEN] & ~OSIOP_DIEN_RES)
		assert = 1;

	return assert;
}


/*
 *  osiop_reassert_interrupts():
 *
 *  Recalculate interrupt assertions; if either of the SIP or DIP bits
 *  in the ISTAT register is set, then we should cause an interrupt.
 *
 *  (The d->asserted stuff is to make sure we don't call INTERRUPT_DEASSERT
 *  etc. too often when not necessary. Just an optimization.)
 */
static void osiop_reassert_interrupts(struct osiop_data *d)
{
	int assert = 0;

	if (osiop_update_sip_and_dip(d))
		assert = 1;

	if (assert && !d->asserted)
		INTERRUPT_ASSERT(d->irq);
	if (!assert && d->asserted)
		INTERRUPT_DEASSERT(d->irq);

	d->asserted = assert;
}


/*  Helper: returns a word in host order, from emulated physical RAM.  */
static uint32_t read_word(struct osiop_data *d, struct cpu *cpu, uint32_t addr)
{
	uint32_t word;

	if (d->last_host_page == NULL ||
	    d->last_phys_page != (addr & 0xfffff000)) {
		d->last_phys_page = addr & 0xfffff000;
		d->last_host_page =
		    memory_paddr_to_hostaddr(cpu->mem, d->last_phys_page, 0);
	}

	word = *((uint32_t *) (d->last_host_page + (addr & 0xffc)));

	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		word = LE32_TO_HOST(word);
	else
		word = BE32_TO_HOST(word);

	return word;
}


/*  Helper: reads/writes single bytes from emulated physical RAM.  */
static uint8_t read_byte(struct osiop_data *d, struct cpu *cpu, uint32_t addr)
{
	if (d->last_host_page == NULL ||
	    d->last_phys_page != (addr & 0xfffff000)) {
		d->last_phys_page = addr & 0xfffff000;
		d->last_host_page =
		    memory_paddr_to_hostaddr(cpu->mem, d->last_phys_page, 0);
	}

	return d->last_host_page[addr & 0xfff];
}
static void write_byte(struct osiop_data *d, struct cpu *cpu, uint32_t addr, uint8_t byte)
{
	if (d->last_host_page == NULL ||
	    d->last_phys_page != (addr & 0xfffff000)) {
		d->last_phys_page = addr & 0xfffff000;
		d->last_host_page =
		    memory_paddr_to_hostaddr(cpu->mem, d->last_phys_page, 1);
	}

	d->last_host_page[addr & 0xfff] = byte;
}


/*
 *  osiop_get_next_scripts_word():
 *
 *  Reads a 32-bit word at physical memory location DSP, and returns it
 *  in host order. DSP is then advanced to point to the next instruction word.
 *
 *  (NOTE/TODO: This could be optimized to read from a host page directly,
 *  but then care must be taken when the instruction pointer (DSP) goes
 *  over a page boundary to the next page. At least the page lookup
 *  could be "cached"...)
 */
uint32_t osiop_get_next_scripts_word(struct cpu *cpu, struct osiop_data *d)
{
	uint32_t *dspp = (uint32_t*) &d->reg[OSIOP_DSP];
	uint32_t dsp = *dspp;
	uint32_t instr;

	if (dsp & 3) {
		fatal("osiop_get_next_scripts_word: unaligned DSP 0x%08x\n",
		    dsp);
		exit(1);
	}

	instr = read_word(d, cpu, dsp);

	dsp += sizeof(instr);
	*dspp = dsp;

	return instr;
}


/*
 *  osiop_execute_scripts_instr():
 *
 *  Interprets a single SCRIPTS machine code instruction. Returns 1 if
 *  execution should continue, or 0 if there was an interrupt.
 *
 *  See "Symbios SYM53C710 SCSI I/O Processor Technical Manual, version 3.1"
 *  chapter 5 for details about the instruction set.
 */
int osiop_execute_scripts_instr(struct cpu *cpu, struct osiop_data *d)
{
	uint32_t *dsap = (uint32_t*) &d->reg[OSIOP_DSA];
	uint32_t *dnadp = (uint32_t*) &d->reg[OSIOP_DNAD];
	uint32_t *dbcp = (uint32_t*) &d->reg[OSIOP_DBC];
	uint32_t *dspsp = (uint32_t*) &d->reg[OSIOP_DSPS];
	uint32_t *dspp = (uint32_t*) &d->reg[OSIOP_DSP];
	uint32_t *tempp = (uint32_t*) &d->reg[OSIOP_TEMP];

	uint32_t dspOrig = *dspp;
	uint32_t instr1 = osiop_get_next_scripts_word(cpu, d);
	uint32_t instr2 = osiop_get_next_scripts_word(cpu, d);
	uint32_t dbc, target_addr = 0;
	uint8_t dcmd;
	int32_t reladdr;
	int opcode, phase, relative_addressing, table_indirect_addressing;
	int select_with_atn, scsi_ids_to_select = -1, scsi_id_to_select;
	int test_carry, compare_data, compare_phase;
	int jump_if_true, wait_for_valid_phase;
	int comparison = 0, interrupt_instead_of_branch = 0;
	
	/*
	 *  According to the 53C710 manual, chapter 5 (introduction): the first
	 *  32-bit word is always loaded into DCMD and DBC, the second into
	 *  DSPS, the third (only used by memory move instructions) is loaded
	 *  into the TEMP register.
	 */

	dcmd = d->reg[OSIOP_DCMD] = instr1 >> 24;
	dbc = *dbcp = instr1 & 0x00ffffff;
	*dspsp = instr2;

	reladdr = (instr2 << 8);
	reladdr >>= 8;

	opcode = (dcmd >> 3) & 7;
	phase = dcmd & 7;

	if (osiop_debug)
		debug("{ SCRIPTS @ 0x%08x:  0x%08x 0x%08x",
		    (int) dspOrig, (int) instr1, (int) instr2);

	switch (dcmd & 0xc0) {

	case 0x00:
		{
			int ofs1 = instr1 & 0x00ffffff;
			int ofs2 = instr2 & 0x00ffffff;
			int indirect_addressing = dcmd & 0x20;
			int table_indirect_addressing = dcmd & 0x10;
			uint32_t dsa = *dsap;
			uint32_t addr, xfer_byte_count, xfer_addr;
			int32_t tmp = ofs2 << 8;
			int res;
			size_t i;
			tmp >>= 8;
			
			opcode = (dcmd >> 3) & 1;

			switch (opcode) {
			case 0: if (osiop_debug)
					debug(": CHMOV");
				break;
			case 1: if (osiop_debug)
					debug(": MOVE");
				break;
			}

			if (indirect_addressing) {
				fatal("osiop: TODO: indirect_addressing move\n");
				exit(1);
			}

			if (!table_indirect_addressing) {
				fatal("osiop: TODO: !table_indirect_addressing move\n");
				exit(1);
			}
			
			if (osiop_debug)
				debug(" FROM %i", tmp);
			
			if (ofs1 != ofs2) {
				fatal("osiop: TODO: move ofs1!=ofs2\n");
				exit(1);
			}
			
			if (phase != osiop_get_scsi_phase(d)) {
				fatal("osiop: TODO: move: wait for phase. "
				    "phase = %i, osiop_get_scsi_phase = %i\n",
				    phase, osiop_get_scsi_phase(d));
				exit(1);
			}

			if (osiop_debug)			
				debug(" WHEN %s", phases[phase]);

			addr = dsa + tmp;
			xfer_byte_count = read_word(d, cpu, addr) & 0x00ffffff;
			xfer_addr = read_word(d, cpu, addr+4);

			switch (phase) {

			case MSG_OUT_PHASE:
				scsi_transfer_allocbuf(&d->xferp->msg_out_len,
				    &d->xferp->msg_out, xfer_byte_count, 0);

				i = 0;
				while (xfer_byte_count > 0) {
					uint8_t byte = read_byte(d, cpu, xfer_addr);
					/*  debug("  reading msg_out byte @ 0x%08x = 0x%02x\n",
					    xfer_addr, byte);  */
					d->xferp->msg_out[i++] = byte;
					xfer_addr ++;
					xfer_byte_count --;
				}
				
				osiop_set_scsi_phase(d, COMMAND_PHASE);
				break;

			case COMMAND_PHASE:
				scsi_transfer_allocbuf(&d->xferp->cmd_len,
				    &d->xferp->cmd, xfer_byte_count, 0);

				i = 0;
				while (xfer_byte_count > 0) {
					uint8_t byte = read_byte(d, cpu, xfer_addr);
					/*  debug("  reading cmd byte @ 0x%08x = 0x%02x\n",
					    xfer_addr, byte);  */
					d->xferp->cmd[i++] = byte;
					xfer_addr ++;
					xfer_byte_count --;
				}

				res = diskimage_scsicommand(cpu,
				    d->selected_id, DISKIMAGE_SCSI, d->xferp);
				if (res == 0) {
					fatal("osiop TODO: error\n");
					exit(1);
				}

				d->data_offset = 0;
				
				if (res == 2)
					osiop_set_scsi_phase(d, DATA_OUT_PHASE);
				else if (d->xferp->data_in_len > 0)
					osiop_set_scsi_phase(d, DATA_IN_PHASE);
				else
					osiop_set_scsi_phase(d, STATUS_PHASE);
				break;

			case DATA_OUT_PHASE:
				if (d->xferp->data_out == NULL)
					scsi_transfer_allocbuf(&d->xferp->data_out_len,
					    &d->xferp->data_out, d->xferp->data_out_len, 0);

				while (xfer_byte_count > 0) {
					uint8_t byte = read_byte(d, cpu, xfer_addr);
					/*  debug("  reading data_out byte @ 0x%08x = 0x%02x\n",
					    xfer_addr, byte);  */
					d->xferp->data_out[d->xferp->data_out_offset++] = byte;
					xfer_addr ++;
					xfer_byte_count --;
				}

				/*  Rerun the command to actually write out the data:  */
				res = diskimage_scsicommand(cpu,
				    d->selected_id, DISKIMAGE_SCSI, d->xferp);
				if (res == 0) {
					fatal("osiop TODO: error on rerun\n");
					exit(1);
				} else if (res == 2) {
					/*  Stay at data out phase.  */
				} else {
					osiop_set_scsi_phase(d, STATUS_PHASE);
				}
				break;

			case DATA_IN_PHASE:
				i = 0;
				while (xfer_byte_count > 0 && i + d->data_offset < d->xferp->data_in_len) {
					uint8_t byte = d->xferp->data_in[i + d->data_offset];
					i ++;
					/*  debug("  writing data_in byte @ 0x%08x = 0x%02x\n",
					    xfer_addr, byte);  */
					write_byte(d, cpu, xfer_addr, byte);
					xfer_addr ++;
					xfer_byte_count --;
				}

				d->data_offset += i;
				if (d->data_offset >= d->xferp->data_in_len)
					osiop_set_scsi_phase(d, STATUS_PHASE);
				break;

			case STATUS_PHASE:
				i = 0;
				while (xfer_byte_count > 0 && i < d->xferp->status_len) {
					uint8_t byte = d->xferp->status[i++];
					/*  debug("  writing status byte @ 0x%08x = 0x%02x\n",
					    xfer_addr, byte);  */
					write_byte(d, cpu, xfer_addr, byte);
					xfer_addr ++;
					xfer_byte_count --;
				}

				osiop_set_scsi_phase(d, MSG_IN_PHASE);
				break;

			case MSG_IN_PHASE:
				i = 0;
				while (xfer_byte_count > 0 && i < d->xferp->msg_in_len) {
					uint8_t byte = d->xferp->msg_in[i++];
					/*  debug("  writing msg_in byte @ 0x%08x = 0x%02x\n",
					    xfer_addr, byte);  */
					write_byte(d, cpu, xfer_addr, byte);
					xfer_addr ++;
					xfer_byte_count --;
				}

				/*  Done.  */
				osiop_free_xfer(d);
				break;

			default:fatal("osiop: TODO: unimplemented move, "
				    "phase=%i\n", phase);
				exit(1);
			}

			/*  Transfer complete.  */
			*dnadp = xfer_addr;
			*dbcp = xfer_byte_count;
			
			d->reg[OSIOP_DFIFO] = 0;	/*  TODO  */
		}
		break;

	case 0x40:
		/*  I/O or Read/Write  */
		relative_addressing = dcmd & 4;
		table_indirect_addressing = dcmd & 2;
		select_with_atn = dcmd & 1;
		scsi_ids_to_select = (dbc >> 16) & 0xff;

		switch (opcode) {
		case 0:	if (osiop_debug)
				debug(": SELECT");
			if (select_with_atn) {
				if (osiop_debug)
					debug(" ATN");
				d->reg[OSIOP_SOCL] |= OSIOP_ATN;
			}

			if (table_indirect_addressing) {
				uint32_t dsa = *dsap;
				uint32_t addr, word;
				int32_t tmp = dbc << 8;
				tmp >>= 8;
				addr = dsa + tmp;

				if (osiop_debug)
					debug(" FROM %i", dbc);

				word = read_word(d, cpu, addr);
				scsi_ids_to_select = (word >> 16) & 0xff;
			}

			if (scsi_ids_to_select == 0) {
				fatal("osiop: TODO: scsi_ids_to_select = 0!\n");
				exit(1);
			}

			scsi_id_to_select = 0;
			while (!(scsi_ids_to_select & 1)) {
				scsi_ids_to_select >>= 1;
				scsi_id_to_select ++;
			}

			scsi_ids_to_select &= ~1;
			if (scsi_ids_to_select != 0) {
				fatal("osiop: TODO: multiselect?\n");
				exit(1);
			}		

			if (osiop_debug)
				debug(" [SCSI ID %i]", scsi_id_to_select);

			if (relative_addressing) {
				/*  Note: Relative to _current_ DSP value, not
				    what the DSP was when the current
				    instruction was read!  */
				target_addr = reladdr + *dspp;
				if (osiop_debug)
					debug(" REL(%i)", reladdr);
			} else {
				target_addr = instr2;
				if (osiop_debug)
					debug(" 0x%08x", instr2);
			}

			if (diskimage_exist(cpu->machine,
			    scsi_id_to_select, DISKIMAGE_SCSI)) {
				osiop_new_xfer(d, scsi_id_to_select);

				/*  TODO: Just a guess, so far:  */
				osiop_set_scsi_phase(d, MSG_OUT_PHASE);
			} else {
				d->selected_id = -1;

#if 1
				/*  TODO: The scsi ID does not exist.
				    Should we simply timeout:  */
				d->scripts_running = 0;
#else
				/*  or branch to the reselect address?  */
				*(uint32_t*) &d->reg[OSIOP_DSP] = target_addr;
#endif
			}

			break;
			
		case 1:	if (osiop_debug)
				debug(": WAIT DISCONNECT");
			/*  TODO  */
			break;
		case 2:	if (osiop_debug)
				debug(": WAIT RESELECT");
			fatal("osiop: TODO: wait reselect\n");
			exit(1);
			break;
	
		case 3:	if (osiop_debug)
				debug(": SET");
			break;
		case 4:	if (osiop_debug)
				debug(": CLEAR");
			break;

		default:fatal(": UNIMPLEMENTED dcmd=0x%02x opcode=%i\n",
			    dcmd, opcode);
			exit(1);
		}

		/*  SET or CLEAR:  */
		if (opcode == 3 || opcode == 4) {
			int bit_atn = (dbc >> 3) & 1;
			int bit_ack = (dbc >> 6) & 1;
			int bit_target = (dbc >> 9) & 1;
			int bit_carry = (dbc >> 10) & 1;

			if (osiop_debug) {
				if (bit_atn)
					debug(" ATN");
				if (bit_ack)
					debug(" ACK");
				if (bit_target)
					debug(" TARGET");
				if (bit_carry)
					debug(" CARRY");
			}
					
			if (opcode == 3) {
				d->reg[OSIOP_SOCL] |= (bit_ack * OSIOP_ACK);
				d->reg[OSIOP_SOCL] |= (bit_atn * OSIOP_ATN);
				if (bit_carry)
					d->carry = 1;
				d->reg[OSIOP_SCNTL0] |=
				    (bit_target * OSIOP_SCNTL0_TRG);
			} else {
				d->reg[OSIOP_SOCL] &= ~(bit_ack * OSIOP_ACK);
				d->reg[OSIOP_SOCL] &= ~(bit_atn * OSIOP_ATN);
				if (bit_carry)
					d->carry = 0;
				d->reg[OSIOP_SCNTL0] &=
				    ~(bit_target * OSIOP_SCNTL0_TRG);
			}
		}

		break;

	case 0x80:
		/*  Transfer Control  */
		relative_addressing = (dbc >> 23) & 1;
		test_carry = (dbc >> 21) & 1;
		jump_if_true = (dbc >> 19) & 1;
		compare_data = (dbc >> 18) & 1;
		compare_phase = (dbc >> 17) & 1;
		wait_for_valid_phase = (dbc >> 16) & 1;
		
		switch (opcode) {
		case 0:	if (osiop_debug)
				debug(": JUMP");
			break;
		case 1:	if (osiop_debug)
				debug(": CALL");
			break;
		case 2:	if (osiop_debug)
				debug(": RETURN");
			break;
		case 3:	if (osiop_debug)
				debug(": INTERRUPT");
			break;
		default:fatal(": UNIMPLEMENTED dcmd=0x%02x opcode=%i\n",
			    dcmd, opcode);
			exit(1);
		}

		if (opcode == 0 || opcode == 1) {
			if (relative_addressing) {
				/*  Note: Relative to _current_ DSP value,
				    not what it was when the current instruction
				    was read!  */
				target_addr = reladdr + *dspp;
				if (osiop_debug)
					debug(" REL(%i)", reladdr);
			} else {
				target_addr = instr2;
				if (osiop_debug)
					debug(" 0x%08x", instr2);
			}
		} else {
			if (opcode == 2) {
				/*  Return:  */
				target_addr = *tempp;
			} else {
				/*  Interrupt:  */
				interrupt_instead_of_branch = 1;
			}
		}

		if (test_carry) {
			if (compare_data || compare_phase) {
				fatal("osiop: TODO: test_carry cannot be"
				    " combined with other tests!\n");
				exit(1);
			}
			
			fatal("osiop: TODO: test_carry\n");
			exit(1);
		}

		if (compare_data) {
			fatal("osiop: TODO: compare_data\n");
			exit(1);
		}

		if (compare_phase) {
			int cur_phase;

			if (osiop_debug)
				debug(" %s %s%s",
				    wait_for_valid_phase ? "WHEN" : "IF",
				    jump_if_true? "" : "NOT ",
				    phases[phase]);

			/*  TODO: wait for valid phase!  */

			cur_phase = osiop_get_scsi_phase(d);
			
			comparison = (phase == cur_phase);
		}

		if (!test_carry && !compare_data && !compare_phase)
			jump_if_true = comparison = 1;

		if ((jump_if_true && comparison) ||
		    (!jump_if_true && !comparison)) {
			/*  Perform the branch or interrupt.  */
			if (interrupt_instead_of_branch) {
				d->scripts_running = 0;
				d->reg[OSIOP_DSTAT] |= OSIOP_DSTAT_SIR;
			} else {
				*dspp = target_addr;
			}
		}

		break;

	case 0xc0:
		if (osiop_debug)
			debug(": MEMORY MOVE");
		/*  TODO: third instruction word!  */
		fatal(": TODO\n");
		exit(1);
		break;

	default:fatal(": UNIMPLEMENTED dcmd 0x%02x\n", dcmd);
		exit(1);
	}

	if (osiop_debug)
		debug(" }\n");

	return 1;
}


/*
 *  osiop_execute_scripts():
 *
 *  Interprets SCRIPTS machine code by reading one instruction word at a time,
 *  and executing it.
 */
void osiop_execute_scripts(struct cpu *cpu, struct osiop_data *d)
{
	int n = 0;

	if (osiop_debug)
		debug("{ SCRIPTS start }\n");

	while (d->scripts_running && n < MAX_SCRIPTS_PER_CHUNK &&
	    osiop_execute_scripts_instr(cpu, d))
		n++;

	if (osiop_debug)	
		debug("{ SCRIPTS end }\n");
}


DEVICE_TICK(osiop)
{
	struct osiop_data *d = (struct osiop_data *) extra;

	if (d->scripts_running)
		osiop_execute_scripts(cpu, d);

	osiop_reassert_interrupts(d);
}


DEVICE_ACCESS(osiop)
{
	uint64_t idata = 0, odata = 0;
	uint8_t oldreg = 0;
	struct osiop_data *d = (struct osiop_data *) extra;
	int origofs = relative_addr;
	int non1lenOk = 0;

	idata = memory_readmax64(cpu, data, len);

	/*  Make relative_addr suit addresses in osiopreg.h:  */
	if (cpu->byte_order == EMUL_BIG_ENDIAN) {
		relative_addr =
		    (relative_addr & ~3) |
		    (3 - (relative_addr & 3));
	}

	if (len == sizeof(uint32_t)) {
		/*
		 *  NOTE: These are stored in HOST byte order!
		 */
		switch (origofs) {
		case OSIOP_DSA:
		case OSIOP_TEMP:
		case OSIOP_DBC:
		case OSIOP_DNAD:
		case OSIOP_DSP:
		case OSIOP_DSPS:
		case OSIOP_SCRATCH:
		case OSIOP_ADDER:
			relative_addr = origofs;
			non1lenOk = 1;

			uint32_t *p = (uint32_t*) &d->reg[origofs];

			if (writeflag == MEM_WRITE)
				*p = idata;
			else
				odata = *p;
			
			break;
		}
	} else {
		/*  Byte access:  */
		oldreg = d->reg[relative_addr];

		if (writeflag == MEM_WRITE)
			d->reg[relative_addr] = idata;
		else
			odata = oldreg;
	}

	switch (relative_addr) {

	case OSIOP_SCNTL0:
	case OSIOP_SCNTL1:
		break;

	case OSIOP_SIEN:
		/*  Used by OpenBSD/mvme88k during probing:  */
		if (len == 4)
			non1lenOk = 1;
		if (writeflag == MEM_WRITE)
			osiop_reassert_interrupts(d);
		break;

	case OSIOP_SCID:
		if (idata != oldreg) {
			fatal("osiop TODO: attempt to change SCID?\n");
			exit(1);
		}
		break;

	case OSIOP_SBDL:
		if (writeflag == MEM_WRITE)
			fatal("[ osiop: SBDL set to 0x%x, but should be "
			    "read-only! ]\n", (int) idata);
		break;

	case OSIOP_SBCL:
		if (writeflag == MEM_WRITE) {
			if (osiop_debug)
				debug("[ osiop: SBCL set to 0x%x ]\n", (int) idata);
		}
		break;

	case OSIOP_DSTAT:
		/*  Cleared when read. Most likely not writable.  */
		odata = d->reg[OSIOP_DSTAT];
		d->reg[OSIOP_DSTAT] = OSIOP_DSTAT_DFE;
		osiop_reassert_interrupts(d);
		break;

	case OSIOP_SSTAT0:
		/*  Cleared when read. Most likely not writable.  */
		odata = d->reg[OSIOP_SSTAT0];
		d->reg[OSIOP_SSTAT0] = 0;
		osiop_reassert_interrupts(d);
		break;

	case OSIOP_SSTAT1:
	case OSIOP_SSTAT2:
		break;
		
	case OSIOP_DSA:
		if (writeflag == MEM_WRITE) {
			if (osiop_debug)
				debug("[ osiop: DSA set to 0x%x ]\n", (int) idata);
		}
		break;

	case OSIOP_CTEST0:
	case OSIOP_CTEST1:
	case OSIOP_CTEST2:
	case OSIOP_CTEST3:
	case OSIOP_CTEST4:
	case OSIOP_CTEST5:
	case OSIOP_CTEST6:
	case OSIOP_CTEST7:
		break;

	case OSIOP_TEMP:
		if (writeflag == MEM_WRITE) {
			if (osiop_debug)
				debug("[ osiop: TEMP set to 0x%x ]\n", (int) idata);
		}
		break;

	case OSIOP_DFIFO:
		break;

	case OSIOP_ISTAT:
		if (writeflag == MEM_WRITE) {
			if ((idata & 0x3f) != 0x00) {
				fatal("osiop TODO: istat 0x%x\n", (int) idata);
				exit(1);
			}

			d->reg[relative_addr] = idata &
			    ~(OSIOP_ISTAT_ABRT | OSIOP_ISTAT_RST);
			osiop_reassert_interrupts(d);
		}
		break;

	case OSIOP_CTEST8:
		odata = (odata & 0xf) | (OSIOP_CHIP_REVISION << 4);
		break;

	case OSIOP_DBC:
		break;

	case OSIOP_DSP:
		if (writeflag == MEM_WRITE) {
			if (osiop_debug)
				debug("[ osiop: DSP set to 0x%x ]\n", (int) idata);

			d->scripts_running = 1;
			osiop_execute_scripts(cpu, d);
			osiop_reassert_interrupts(d);
		}
		break;

	case OSIOP_DSPS:
	case OSIOP_SCRATCH:
		break;

	case OSIOP_DMODE:
		break;

	case OSIOP_DIEN:
		if (writeflag == MEM_WRITE)
			osiop_reassert_interrupts(d);
		break;

	case OSIOP_DWT:
		break;

	case OSIOP_DCNTL:
		if (writeflag == MEM_WRITE) {
			if (idata & OSIOP_DCNTL_SSM) {
				fatal("osiop TODO: SSM\n");
				exit(1);
			}
			if (idata & OSIOP_DCNTL_LLM) {
				fatal("osiop TODO: LLM\n");
				exit(1);
			}
			if (idata & OSIOP_DCNTL_STD) {
				fatal("osiop TODO: STD\n");
				exit(1);
			}
		}
		break;

	default:
		if (writeflag == MEM_READ) {
			fatal("[ osiop: read from 0x%02lx ]\n",
			    (long)relative_addr);
		} else {
			fatal("[ osiop: write to  0x%02lx: 0x%02x ]\n",
			    (long)relative_addr, (int)idata);
		}

		exit(1);
	}

	if (len != 1 && !non1lenOk) {
		fatal("[ osiop: TODO: len != 1, addr 0x%0x ]\n",
		    (int)relative_addr);
		exit(1);
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVINIT(osiop)
{
	struct osiop_data *d;

	CHECK_ALLOCATION(d = (struct osiop_data *) malloc(sizeof(struct osiop_data)));
	memset(d, 0, sizeof(struct osiop_data));

	/*  Set up initial device register values:  */
	d->reg[OSIOP_SCID] = OSIOP_SCID_VALUE(7);

	/*  OpenBSD's osiop_checkintr needs this:  */
	d->reg[OSIOP_CTEST1] = OSIOP_CTEST1_FMT;

	INTERRUPT_CONNECT(devinit->interrupt_path, d->irq);

	memory_device_register(devinit->machine->memory, "osiop",
	    devinit->addr, DEV_OSIOP_LENGTH,
	    dev_osiop_access, d, DM_DEFAULT, NULL);

	machine_add_tickfunction(devinit->machine,
	    dev_osiop_tick, d, OSIOP_TICK_SHIFT);

	return 1;
}

