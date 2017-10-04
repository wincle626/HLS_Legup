/*
 *  Copyright (C) 2004-2009  Anders Gavare.  All rights reserved.
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
 *  COMMENT: Standard "wdc" IDE controller
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "device.h"
#include "diskimage.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"

#include "thirdparty/wdcreg.h"


#define	DEV_WDC_LENGTH		8
#define	WDC_TICK_SHIFT		14
#define	WDC_MAX_SECTORS		512
#define	WDC_INBUF_SIZE		(512*(WDC_MAX_SECTORS+1))

extern int quiet_mode;

/*  #define debug fatal  */

struct wdc_data {
	struct interrupt irq;
	int		addr_mult;
	int		base_drive;
	int		data_debug;
	int		io_enabled;

	/*  Cached values:  */
	int		cyls[2];
	int		heads[2];
	int		sectors_per_track[2];

	unsigned char	*inbuf;
	int		inbuf_head;
	int		inbuf_tail;

	int		int_assert;

	int		write_in_progress;
	int		write_count;
	int64_t		write_offset;

	int		error;
	int		precomp;
	int		seccnt;
	int		sector;
	int		cyl_lo;
	int		cyl_hi;
	int		sectorsize;
	int		lba;
	int		drive;
	int		head;
	int		cur_command;

	int		atapi_cmd_in_progress;
	int		atapi_phase;
	struct scsi_transfer *atapi_st;
	int		atapi_len;
	size_t		atapi_received;

	unsigned char	identify_struct[512];
};


#define COMMAND_RESET	0x100


DEVICE_TICK(wdc)
{ 
	struct wdc_data *d = (struct wdc_data *) extra;

	if (d->int_assert)
		INTERRUPT_ASSERT(d->irq);
}


/*
 *  wdc_set_io_enabled():
 *
 *  Set io_enabled to zero to disable the I/O registers temporarily (e.g.
 *  used by PCI code in NetBSD to detect whether multiple controllers collide
 *  in I/O space).
 *
 *  Return value is old contents of the io_enabled variable.
 */
int wdc_set_io_enabled(struct wdc_data *d, int io_enabled)
{
	int old = d->io_enabled;
	d->io_enabled = io_enabled;
	return old;
}


/*
 *  wdc_addtoinbuf():
 *
 *  Write to the inbuf at its head, read at its tail.
 */
static void wdc_addtoinbuf(struct wdc_data *d, int c)
{
	d->inbuf[d->inbuf_head] = c;

	d->inbuf_head = (d->inbuf_head + 1) % WDC_INBUF_SIZE;
	if (d->inbuf_head == d->inbuf_tail)
		fatal("[ wdc_addtoinbuf(): WARNING! wdc inbuf overrun!"
		    " Increase WDC_MAX_SECTORS. ]\n");
}


/*
 *  wdc_get_inbuf():
 *
 *  Read from the tail of inbuf.
 */
static uint64_t wdc_get_inbuf(struct wdc_data *d)
{
	int c = d->inbuf[d->inbuf_tail];

	if (d->inbuf_head == d->inbuf_tail) {
		fatal("[ wdc: WARNING! someone is reading too much from the "
		    "wdc inbuf! ]\n");
		return (uint64_t) -1;
	}

	d->inbuf_tail = (d->inbuf_tail + 1) % WDC_INBUF_SIZE;
	return c;
}


/*
 *  wdc_initialize_identify_struct():
 */
static void wdc_initialize_identify_struct(struct cpu *cpu, struct wdc_data *d)
{
	uint64_t total_size;
	int flags, cdrom = 0;
	char namebuf[40];

	total_size = diskimage_getsize(cpu->machine, d->drive + d->base_drive,
	    DISKIMAGE_IDE);
	if (diskimage_is_a_cdrom(cpu->machine, d->drive + d->base_drive,
	    DISKIMAGE_IDE))
		cdrom = 1;

	memset(d->identify_struct, 0, sizeof(d->identify_struct));

	/*  Offsets are in 16-bit WORDS!  High byte, then low.  */

	/*  0: general flags  */
	flags = 1 << 6;	/*  Fixed  */
	if (cdrom)
		flags = 0x8580;		/*  ATAPI, CDROM, removable  */
	d->identify_struct[2 * 0 + 0] = flags >> 8;
	d->identify_struct[2 * 0 + 1] = flags;

	/*  1: nr of cylinders  */
	d->identify_struct[2 * 1 + 0] = d->cyls[d->drive] >> 8;
	d->identify_struct[2 * 1 + 1] = d->cyls[d->drive];

	/*  3: nr of heads  */
	d->identify_struct[2 * 3 + 0] = d->heads[d->drive] >> 8;
	d->identify_struct[2 * 3 + 1] = d->heads[d->drive];

	/*  6: sectors per track  */
	d->identify_struct[2 * 6 + 0] = d->sectors_per_track[d->drive] >> 8;
	d->identify_struct[2 * 6 + 1] = d->sectors_per_track[d->drive];

	/*  10-19: Serial number  */
	memcpy(&d->identify_struct[2 * 10], "#0                  ", 20);

	/*  23-26: Firmware version  */
	memcpy(&d->identify_struct[2 * 23], "1.0     ", 8);

	/*  27-46: Model number  */
	if (diskimage_getname(cpu->machine, d->drive + d->base_drive,
	    DISKIMAGE_IDE, namebuf, sizeof(namebuf))) {
		size_t i;
		for (i=0; i<sizeof(namebuf); i++)
			if (namebuf[i] == 0) {
				for (; i<sizeof(namebuf); i++)
					namebuf[i] = ' ';
				break;
			}
		memcpy(&d->identify_struct[2 * 27], namebuf, 40);
	} else
		memcpy(&d->identify_struct[2 * 27],
		    "Fake GXemul IDE disk                    ", 40);

	/*  47: max sectors per multitransfer  */
	d->identify_struct[2 * 47 + 0] = 0x80;
	d->identify_struct[2 * 47 + 1] = 128;

	/*  49: capabilities:  */
	/*  (0x200 = LBA, 0x100 = DMA support.)  */
	d->identify_struct[2 * 49 + 0] = 0;
	d->identify_struct[2 * 49 + 1] = 0;

	/*  51: PIO timing mode.  */
	d->identify_struct[2 * 51 + 0] = 0x00;	/*  ?  */
	d->identify_struct[2 * 51 + 1] = 0x00;

	/*  53: 0x02 = fields 64-70 valid, 0x01 = fields 54-58 valid  */
	d->identify_struct[2 * 53 + 0] = 0x00;
	d->identify_struct[2 * 53 + 1] = 0x02;

	/*  57-58: current capacity in sectors  */
	d->identify_struct[2 * 58 + 0] = ((total_size / 512) >> 24) % 255;
	d->identify_struct[2 * 58 + 1] = ((total_size / 512) >> 16) % 255;
	d->identify_struct[2 * 57 + 0] = ((total_size / 512) >> 8) % 255;
	d->identify_struct[2 * 57 + 1] = (total_size / 512) & 255;

	/*  60-61: total nr of addressable sectors  */
	d->identify_struct[2 * 61 + 0] = ((total_size / 512) >> 24) % 255;
	d->identify_struct[2 * 61 + 1] = ((total_size / 512) >> 16) % 255;
	d->identify_struct[2 * 60 + 0] = ((total_size / 512) >> 8) % 255;
	d->identify_struct[2 * 60 + 1] = (total_size / 512) & 255;

	/*  64: Advanced PIO mode support. 0x02 = mode4, 0x01 = mode3  */
	d->identify_struct[2 * 64 + 0] = 0x00;
	d->identify_struct[2 * 64 + 1] = 0x03;

	/*  67, 68: PIO timing  */
	d->identify_struct[2 * 67 + 0] = 0;
	d->identify_struct[2 * 67 + 1] = 120;
	d->identify_struct[2 * 68 + 0] = 0;
	d->identify_struct[2 * 68 + 1] = 120;
}


/*
 *  wdc__read():
 */
void wdc__read(struct cpu *cpu, struct wdc_data *d)
{
#define MAX_SECTORS_PER_CHUNK	64
	const int max_sectors_per_chunk = MAX_SECTORS_PER_CHUNK;
	unsigned char buf[512 * MAX_SECTORS_PER_CHUNK];
	int i, cyl = d->cyl_hi * 256+ d->cyl_lo;
	int count = d->seccnt? d->seccnt : 256;
	uint64_t offset = 512 * (d->sector - 1
	    + (int64_t)d->head * d->sectors_per_track[d->drive] +
	    (int64_t)d->heads[d->drive] * d->sectors_per_track[d->drive] * cyl);

#if 0
	/*  LBA:  */
	if (d->lba)
		offset = 512 * (((d->head & 0xf) << 24) + (cyl << 8)
		    + d->sector);
	printf("WDC read from offset %lli\n", (long long)offset);
#endif

	while (count > 0) {
		int to_read = count > max_sectors_per_chunk?
		    max_sectors_per_chunk : count;

		/*  TODO: result code from the read?  */

		if (d->inbuf_head + 512 * to_read <= WDC_INBUF_SIZE) {
			diskimage_access(cpu->machine, d->drive + d->base_drive,
			    DISKIMAGE_IDE, 0, offset,
			    d->inbuf + d->inbuf_head, 512 * to_read);
			d->inbuf_head += 512 * to_read;
			if (d->inbuf_head == WDC_INBUF_SIZE)
				d->inbuf_head = 0;
		} else {
			diskimage_access(cpu->machine, d->drive + d->base_drive,
			    DISKIMAGE_IDE, 0, offset, buf, 512 * to_read);
			for (i=0; i<512 * to_read; i++)
				wdc_addtoinbuf(d, buf[i]);
		}

		offset += 512 * to_read;
		count -= to_read;
	}

	d->int_assert = 1;
}


/*
 *  wdc__write():
 */
void wdc__write(struct cpu *cpu, struct wdc_data *d)
{
	int cyl = d->cyl_hi * 256+ d->cyl_lo;
	int count = d->seccnt? d->seccnt : 256;
	uint64_t offset = 512 * (d->sector - 1
	    + (int64_t)d->head * d->sectors_per_track[d->drive] +
	    (int64_t)d->heads[d->drive] * d->sectors_per_track[d->drive] * cyl);
#if 0
	/*  LBA:  */
	if (d->lba)
		offset = 512 * (((d->head & 0xf) << 24) +
		    (cyl << 8) + d->sector);
	printf("WDC write to offset %lli\n", (long long)offset);
#endif

	d->write_in_progress = d->cur_command;
	d->write_count = count;
	d->write_offset = offset;

	/*  TODO: result code?  */
}


/*
 *  status_byte():
 *
 *  Return a reasonable status byte corresponding to the controller's current
 *  state.
 */
static int status_byte(struct wdc_data *d, struct cpu *cpu)
{
	int odata = 0;
	if (diskimage_exist(cpu->machine, d->drive + d->base_drive,
	    DISKIMAGE_IDE))
		odata |= WDCS_DRDY | WDCS_DSC;
	if (d->inbuf_head != d->inbuf_tail)
		odata |= WDCS_DRQ;
	if (d->write_in_progress)
		odata |= WDCS_DRQ;
	if (d->error)
		odata |= WDCS_ERR;
	if (d->atapi_cmd_in_progress && (d->atapi_phase & WDCS_DRQ)) {
		odata |= WDCS_DRQ;
	}
	return odata;
}


DEVICE_ACCESS(wdc_altstatus)
{
	struct wdc_data *d = (struct wdc_data *) extra;
	uint64_t idata = 0, odata = 0;

	idata = data[0];

	/*  Same as the normal status byte:  */
	odata = status_byte(d, cpu);

	if (writeflag==MEM_READ)
		debug("[ wdc: read from ALTSTATUS: 0x%02x ]\n",
		    (int)odata);
	else {
		debug("[ wdc: write to ALT. CTRL: 0x%02x ]\n",
		    (int)idata);
		if (idata & WDCTL_4BIT)
			d->cur_command = COMMAND_RESET;
	}

	if (writeflag == MEM_READ)
		data[0] = odata;

	return 1;
}


/*
 *  wdc_command():
 */
void wdc_command(struct cpu *cpu, struct wdc_data *d, int idata)
{
	size_t i;

	d->cur_command = idata;
	d->atapi_cmd_in_progress = 0;
	d->error = 0;

	/*
	 *  Disk images that do not exist return an ABORT error.  This also
	 *  happens with CDROM images with the WDCC_IDENTIFY command; CDROM
	 *  images must be detected with ATAPI_IDENTIFY_DEVICE instead.
	 *
	 *  TODO:  Is this correct/good behaviour?
	 */
	if (!diskimage_exist(cpu->machine, d->drive + d->base_drive,
	    DISKIMAGE_IDE)) {
		debug("[ wdc: command 0x%02x drive %i, but no disk image ]\n",
		    d->cur_command, d->drive + d->base_drive);
		d->error |= WDCE_ABRT;
		d->int_assert = 1;
		return;
	}
	if (diskimage_is_a_cdrom(cpu->machine, d->drive + d->base_drive,
	    DISKIMAGE_IDE) && d->cur_command == WDCC_IDENTIFY) {
		debug("[ wdc: IDENTIFY drive %i, but it is an ATAPI "
		    "drive ]\n", d->drive + d->base_drive);
		d->error |= WDCE_ABRT;
		d->int_assert = 1;
		return;
	}

	/*  Handle the command:  */
	switch (d->cur_command) {

	case WDCC_READ:
	case WDCC_READMULTI:
		if (!quiet_mode)
			debug("[ wdc: READ from drive %i, head %i, cyl %i, "
			    "sector %i, nsecs %i ]\n", d->drive, d->head,
			    d->cyl_hi*256+d->cyl_lo, d->sector, d->seccnt);
		wdc__read(cpu, d);
		break;

	case WDCC_WRITE:
	case WDCC_WRITEMULTI:
		if (!quiet_mode)
			debug("[ wdc: WRITE to drive %i, head %i, cyl %i, "
			    "sector %i, nsecs %i ]\n", d->drive, d->head,
			    d->cyl_hi*256+d->cyl_lo, d->sector, d->seccnt);
		wdc__write(cpu, d);
		break;

	case WDCC_IDP:	/*  Initialize drive parameters  */
		debug("[ wdc: IDP drive %i (TODO) ]\n", d->drive);
		/*  TODO  */
		d->int_assert = 1;
		break;

	case SET_FEATURES:
		debug("[ wdc: SET_FEATURES drive %i (TODO), feature 0x%02x ]\n",
		    d->drive, d->precomp);
		/*  TODO  */
		switch (d->precomp) {
		case WDSF_SET_MODE:
			debug("[ wdc: WDSF_SET_MODE drive %i, pio/dma flags "
			    "0x%02x ]\n", d->drive, d->seccnt);
			break;
		default:d->error |= WDCE_ABRT;
		}
		/*  TODO: always interrupt?  */
		d->int_assert = 1;
		break;

	case WDCC_RECAL:
		debug("[ wdc: RECAL drive %i ]\n", d->drive);
		d->int_assert = 1;
		break;

	case WDCC_IDENTIFY:
	case ATAPI_IDENTIFY_DEVICE:
		debug("[ wdc: %sIDENTIFY drive %i ]\n", d->cur_command ==
		    ATAPI_IDENTIFY_DEVICE? "ATAPI " : "", d->drive);
		wdc_initialize_identify_struct(cpu, d);
		/*  The IDENTIFY data is sent out in low/high byte order:  */
		for (i=0; i<sizeof(d->identify_struct); i+=2) {
			wdc_addtoinbuf(d, d->identify_struct[i+1]);
			wdc_addtoinbuf(d, d->identify_struct[i+0]);
		}
		d->int_assert = 1;
		break;

	case WDCC_IDLE_IMMED:
		debug("[ wdc: IDLE_IMMED drive %i ]\n", d->drive);
		/*  TODO: interrupt here?  */
		d->int_assert = 1;
		break;

	case WDCC_SETMULTI:
		debug("[ wdc: SETMULTI drive %i ]\n", d->drive);
		/*  TODO: interrupt here?  */
		d->int_assert = 1;
		break;

	case ATAPI_SOFT_RESET:
		debug("[ wdc: ATAPI_SOFT_RESET drive %i ]\n", d->drive);
		/*  TODO: interrupt here?  */
		d->int_assert = 1;
		break;

	case ATAPI_PKT_CMD:
		debug("[ wdc: ATAPI_PKT_CMD drive %i ]\n", d->drive);
		/*  TODO: interrupt here?  */
		/*  d->int_assert = 1;  */
		d->atapi_cmd_in_progress = 1;
		d->atapi_phase = PHASE_CMDOUT;
		break;

	case WDCC_DIAGNOSE:
		debug("[ wdc: WDCC_DIAGNOSE drive %i: TODO ]\n", d->drive);
		/*  TODO: interrupt here?  */
		d->int_assert = 1;
		d->error = 1;		/*  No error?  */
		break;

	/*  Unsupported commands, without warning:  */
	case WDCC_SEC_SET_PASSWORD:
	case WDCC_SEC_UNLOCK:
	case WDCC_SEC_ERASE_PREPARE:
	case WDCC_SEC_ERASE_UNIT:
	case WDCC_SEC_FREEZE_LOCK:
	case WDCC_SEC_DISABLE_PASSWORD:
		d->error |= WDCE_ABRT;
		break;

	default:/*  TODO  */
		d->error |= WDCE_ABRT;
		fatal("[ wdc: WARNING! Unimplemented command 0x%02x (drive %i,"
		    " head %i, cyl %i, sector %i, nsecs %i) ]\n",
		    d->cur_command, d->drive, d->head, d->cyl_hi*256+d->cyl_lo,
		    d->sector, d->seccnt);
	}
}


DEVICE_ACCESS(wdc)
{
	struct wdc_data *d = (struct wdc_data *) extra;
	uint64_t idata = 0, odata = 0;
	int i;

	relative_addr /= d->addr_mult;

	if (!d->io_enabled)
		goto ret;

	if (writeflag == MEM_WRITE) {
		if (relative_addr == wd_data)
			idata = memory_readmax64(cpu, data, len);
		else {
			if (len != 1)
				fatal("[ wdc: WARNING! non-8-bit access! ]\n");
			idata = data[0];
		}
	}

	switch (relative_addr) {

	case wd_data:	/*  0: data  */
		if (writeflag == MEM_READ) {
			odata = wdc_get_inbuf(d);

			if (cpu->byte_order == EMUL_LITTLE_ENDIAN) {
				if (len >= 2)
					odata += (wdc_get_inbuf(d) << 8);
				if (len == 4) {
					odata += (wdc_get_inbuf(d) << 16);
					odata += (wdc_get_inbuf(d) << 24);
				}
			} else {
				if (len >= 2)
					odata = (odata << 8) + wdc_get_inbuf(d);
				if (len == 4) {
					odata = (odata << 8) + wdc_get_inbuf(d);
					odata = (odata << 8) + wdc_get_inbuf(d);
				}
			}

			if (d->data_debug) {
				const char *s = "0x%04"PRIx64" ]\n";
				if (len == 1)
					s = "0x%02"PRIx64" ]\n";
				if (len == 4)
					s = "0x%08"PRIx64" ]\n";
				if (len == 8)
					s = "0x%016"PRIx64" ]\n";
				debug("[ wdc: read from DATA: ");
				debug(s, (uint64_t) odata);
			}

			if (d->atapi_cmd_in_progress) {
				d->atapi_len -= len;
				d->atapi_received += len;
				if (d->atapi_len == 0) {
					if (d->atapi_received < d->atapi_st->
					    data_in_len) {
						d->atapi_phase = PHASE_DATAIN;
						d->atapi_len = d->atapi_st->
						    data_in_len -
						    d->atapi_received;
						if (d->atapi_len > 32768)
							d->atapi_len = 0;
					} else
						d->atapi_phase =
						    PHASE_COMPLETED;
					d->int_assert = 1;
				}
			} else {
#if 0
				if (d->inbuf_tail != d->inbuf_head)
#else
				if (d->inbuf_tail != d->inbuf_head &&
				    ((d->inbuf_tail - d->inbuf_head) % 512)
				    == 0)
#endif
					d->int_assert = 1;
			}
		} else {
			int inbuf_len;
			if (d->data_debug) {
				const char *s = "0x%04"PRIx64" ]\n";
				if (len == 1)
					s = "0x%02"PRIx64" ]\n";
				if (len == 4)
					s = "0x%08"PRIx64" ]\n";
				if (len == 8)
					s = "0x%016"PRIx64" ]\n";
				debug("[ wdc: write to DATA: ");
				debug(s, (uint64_t) idata);
			}
			if (!d->write_in_progress &&
			    !d->atapi_cmd_in_progress) {
				fatal("[ wdc: write to DATA, but not "
				    "expecting any? (len=%i): 0x%08lx ]\n",
				    (int)len, (long)idata);
			}

			if (cpu->byte_order == EMUL_LITTLE_ENDIAN) {
				switch (len) {
				case 4:	wdc_addtoinbuf(d, idata & 0xff);
					wdc_addtoinbuf(d, (idata >> 8) & 0xff);
					wdc_addtoinbuf(d, (idata >> 16) & 0xff);
					wdc_addtoinbuf(d, (idata >> 24) & 0xff);
					break;
				case 2:	wdc_addtoinbuf(d, idata & 0xff);
					wdc_addtoinbuf(d, (idata >> 8) & 0xff);
					break;
				case 1:	wdc_addtoinbuf(d, idata); break;
				default:fatal("wdc: unimplemented write "
					    "len %i\n", len);
					exit(1);
				}
			} else {
				switch (len) {
				case 4:	wdc_addtoinbuf(d, (idata >> 24) & 0xff);
					wdc_addtoinbuf(d, (idata >> 16) & 0xff);
					wdc_addtoinbuf(d, (idata >> 8) & 0xff);
					wdc_addtoinbuf(d, idata & 0xff);
					break;
				case 2:	wdc_addtoinbuf(d, (idata >> 8) & 0xff);
					wdc_addtoinbuf(d, idata & 0xff);
					break;
				case 1:	wdc_addtoinbuf(d, idata); break;
				default:fatal("wdc: unimplemented write "
					    "len %i\n", len);
					exit(1);
				}
			}

			inbuf_len = d->inbuf_head - d->inbuf_tail;
			while (inbuf_len < 0)
				inbuf_len += WDC_INBUF_SIZE;

			if (d->atapi_cmd_in_progress && inbuf_len == 12) {
				unsigned char *scsi_cmd;
				int x = 0, res;

				CHECK_ALLOCATION(scsi_cmd = (unsigned char *) malloc(12));

				if (d->atapi_st != NULL)
					scsi_transfer_free(d->atapi_st);
				d->atapi_st = scsi_transfer_alloc();

				debug("[ wdc: ATAPI command ]\n");

				while (inbuf_len > 0) {
					scsi_cmd[x++] = wdc_get_inbuf(d);
					inbuf_len --;
				}

				d->atapi_st->cmd = scsi_cmd;
				d->atapi_st->cmd_len = 12;

				if (scsi_cmd[0] == SCSIBLOCKCMD_READ_CAPACITY
				    || scsi_cmd[0] == SCSICMD_READ_10
				    || scsi_cmd[0] == SCSICMD_MODE_SENSE10)
					d->atapi_st->cmd_len = 10;

				res = diskimage_scsicommand(cpu,
				    d->drive + d->base_drive, DISKIMAGE_IDE,
				    d->atapi_st);

				if (res == 0) {
					fatal("WDC: ATAPI scsi error?\n");
					exit(1);
				}

				d->atapi_len = 0;
				d->atapi_received = 0;

				if (res == 1) {
					if (d->atapi_st->data_in != NULL) {
						int i;
						d->atapi_phase = PHASE_DATAIN;
						d->atapi_len = d->atapi_st->
						    data_in_len;
						for (i=0; i<d->atapi_len; i++)
							wdc_addtoinbuf(d,
							    d->atapi_st->
							    data_in[i]);
						if (d->atapi_len > 32768)
							d->atapi_len = 32768;
					} else {
						d->atapi_phase =
						    PHASE_COMPLETED;
					}
				} else {
					fatal("wdc atapi Dataout? TODO\n");
					d->atapi_phase = PHASE_DATAOUT;
					exit(1);
				}

				d->int_assert = 1;
			}

			if (( d->write_in_progress == WDCC_WRITEMULTI &&
			    inbuf_len % (512 * d->write_count) == 0)
			    ||
			    ( d->write_in_progress == WDCC_WRITE &&
			    inbuf_len % 512 == 0) ) {
				int count = (d->write_in_progress ==
				    WDCC_WRITEMULTI)? d->write_count : 1;
				unsigned char *buf, *b;

				CHECK_ALLOCATION(buf = (unsigned char *) malloc(512 * count));
				b = buf;

				if (d->inbuf_tail+512*count <= WDC_INBUF_SIZE) {
					b = d->inbuf + d->inbuf_tail;
					d->inbuf_tail = (d->inbuf_tail + 512
					    * count) % WDC_INBUF_SIZE;
				} else {
					for (i=0; i<512 * count; i++)
						buf[i] = wdc_get_inbuf(d);
				}

				diskimage_access(cpu->machine,
				    d->drive + d->base_drive, DISKIMAGE_IDE, 1,
				    d->write_offset, b, 512 * count);

				d->write_count -= count;
				d->write_offset += 512 * count;

				d->int_assert = 1;

				if (d->write_count == 0)
					d->write_in_progress = 0;

				free(buf);
			}
		}
		break;

	case wd_error:	/*  1: error (r), precomp (w)  */
		if (writeflag == MEM_READ) {
			odata = d->error;
			debug("[ wdc: read from ERROR: 0x%02x ]\n", (int)odata);
		} else {
			d->precomp = idata;
			debug("[ wdc: write to PRECOMP: 0x%02x ]\n",(int)idata);
		}
		break;

	case wd_seccnt:	/*  2: sector count (or "ireason" for ATAPI)  */
		if (writeflag == MEM_READ) {
			odata = d->seccnt;
			if (d->atapi_cmd_in_progress) {
				odata = d->atapi_phase & (WDCI_CMD | WDCI_IN);
			}
			debug("[ wdc: read from SECCNT: 0x%02x ]\n",(int)odata);
		} else {
			d->seccnt = idata;
			debug("[ wdc: write to SECCNT: 0x%02x ]\n", (int)idata);
		}
		break;

	case wd_sector:	/*  3: first sector  */
		if (writeflag == MEM_READ) {
			odata = d->sector;
			debug("[ wdc: read from SECTOR: 0x%02x ]\n",(int)odata);
		} else {
			d->sector = idata;
			debug("[ wdc: write to SECTOR: 0x%02x ]\n", (int)idata);
		}
		break;

	case wd_cyl_lo:	/*  4: cylinder low  */
		if (writeflag == MEM_READ) {
			odata = d->cyl_lo;
			if (d->cur_command == COMMAND_RESET &&
			    diskimage_is_a_cdrom(cpu->machine,
			    d->drive + d->base_drive, DISKIMAGE_IDE))
				odata = 0x14;
			if (d->atapi_cmd_in_progress) {
				int x = d->atapi_len;
				if (x > 32768)
					x = 32768;
				odata = x & 255;
			}
			debug("[ wdc: read from CYL_LO: 0x%02x ]\n",(int)odata);
		} else {
			d->cyl_lo = idata;
			debug("[ wdc: write to CYL_LO: 0x%02x ]\n", (int)idata);
		}
		break;

	case wd_cyl_hi:	/*  5: cylinder high  */
		if (writeflag == MEM_READ) {
			odata = d->cyl_hi;
			if (d->cur_command == COMMAND_RESET &&
			    diskimage_is_a_cdrom(cpu->machine,
			    d->drive + d->base_drive, DISKIMAGE_IDE))
				odata = 0xeb;
			if (d->atapi_cmd_in_progress) {
				int x = d->atapi_len;
				if (x > 32768)
					x = 32768;
				odata = (x >> 8) & 255;
			}
			debug("[ wdc: read from CYL_HI: 0x%02x ]\n",(int)odata);
		} else {
			d->cyl_hi = idata;
			debug("[ wdc: write to CYL_HI: 0x%02x ]\n", (int)idata);
		}
		break;

	case wd_sdh:	/*  6: sectorsize/drive/head  */
		if (writeflag==MEM_READ) {
			odata = (d->sectorsize << 6) + (d->lba << 5) +
			    (d->drive << 4) + (d->head);
			debug("[ wdc: read from SDH: 0x%02x (sectorsize %i,"
			    " lba=%i, drive %i, head %i) ]\n", (int)odata,
			    d->sectorsize, d->lba, d->drive, d->head);
		} else {
			d->sectorsize = (idata >> 6) & 3;
			d->lba   = (idata >> 5) & 1;
			d->drive = (idata >> 4) & 1;
			d->head  = idata & 0xf;
			debug("[ wdc: write to SDH: 0x%02x (sectorsize %i,"
			    " lba=%i, drive %i, head %i) ]\n", (int)idata,
			    d->sectorsize, d->lba, d->drive, d->head);
		}
		break;

	case wd_command:	/*  7: command or status  */
		if (writeflag==MEM_READ) {
			odata = status_byte(d, cpu);
			if (!quiet_mode)
				debug("[ wdc: read from STATUS: 0x%02x ]\n",
				    (int)odata);
			INTERRUPT_DEASSERT(d->irq);
			d->int_assert = 0;
		} else {
			debug("[ wdc: write to COMMAND: 0x%02x ]\n",(int)idata);
			wdc_command(cpu, d, idata);
		}
		break;

	default:
		if (writeflag==MEM_READ)
			debug("[ wdc: read from 0x%02x ]\n",
			    (int)relative_addr);
		else
			debug("[ wdc: write to  0x%02x: 0x%02x ]\n",
			    (int)relative_addr, (int)idata);
	}

	/*  Assert interrupt, if necessary:  */
	dev_wdc_tick(cpu, extra);

ret:
	if (writeflag == MEM_READ) {
		if (relative_addr == wd_data)
			memory_writemax64(cpu, data, len, odata);
		else
			data[0] = odata;
	}

	return 1;
}


DEVINIT(wdc)
{
	struct wdc_data *d;
	uint64_t alt_status_addr;
	int i, tick_shift = WDC_TICK_SHIFT;

	CHECK_ALLOCATION(d = (struct wdc_data *) malloc(sizeof(struct wdc_data)));
	memset(d, 0, sizeof(struct wdc_data));

	INTERRUPT_CONNECT(devinit->interrupt_path, d->irq);
	d->addr_mult  = devinit->addr_mult;
	d->data_debug = 1;
	d->io_enabled = 1;
	d->error      = 1;

	d->inbuf = (unsigned char *) zeroed_alloc(WDC_INBUF_SIZE);

	/*  base_drive = 0 for the primary controller, 2 for the secondary.  */
	d->base_drive = 0;
	if ((devinit->addr & 0xfff) == 0x170)
		d->base_drive = 2;

	alt_status_addr = devinit->addr + 0x206;

	/*  Special hacks for individual machines:  */
	switch (devinit->machine->machine_type) {
	case MACHINE_MACPPC:
		alt_status_addr = devinit->addr + 0x160;
		break;
	case MACHINE_HPCMIPS:
		/*  TODO: Fix  */
		if (devinit->addr == 0x14000180)
			alt_status_addr = 0x14000386;
		break;
	case MACHINE_IQ80321:
		alt_status_addr = devinit->addr + 0x402;
		break;
	}

	/*  Get disk geometries:  */
	for (i=0; i<2; i++)
		if (diskimage_exist(devinit->machine, d->base_drive +i,
		    DISKIMAGE_IDE))
			diskimage_getchs(devinit->machine, d->base_drive + i,
			    DISKIMAGE_IDE, &d->cyls[i], &d->heads[i],
			    &d->sectors_per_track[i]);

	memory_device_register(devinit->machine->memory, "wdc_altstatus",
	    alt_status_addr, 2, dev_wdc_altstatus_access, d, DM_DEFAULT, NULL);
	memory_device_register(devinit->machine->memory, devinit->name,
	    devinit->addr, DEV_WDC_LENGTH * devinit->addr_mult, dev_wdc_access,
	    d, DM_DEFAULT, NULL);

	machine_add_tickfunction(devinit->machine, dev_wdc_tick,
	    d, tick_shift);

	devinit->return_ptr = d;

	return 1;
}

