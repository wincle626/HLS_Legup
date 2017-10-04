/*
 *  Copyright (C) 2006-2009  Anders Gavare.  All rights reserved.
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
 *  COMMENT: Dreamcast GD-ROM
 *
 *  TODO: This is just a dummy so far. It is enough for NetBSD/dreamcast to
 *  read the GD-ROM, but it shouldn't be assumed to work for anything else.
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

#include "thirdparty/dreamcast_sysasicvar.h"


/*  #define debug fatal  */

struct dreamcast_gdrom_data {
	uint8_t		busy;		/*  Busy status  */
	uint8_t		stat;		/*  Status  */
	int		cnt;		/*  Data length  */
	uint8_t		cond;

	int		cmd_count;
	uint8_t		cmd[12];

	uint8_t		*data;
	int		data_len;
	int		cur_data_offset;
	int		cur_cnt;
};

/*  Register offsets:  */
#define	GDROM_BUSY		0x18
#define	GDROM_DATA		0x80
#define	GDROM_REGX		0x84
#define	GDROM_STAT		0x8c
#define	GDROM_CNTLO		0x90
#define	GDROM_CNTHI		0x94
#define	GDROM_COND		0x9c

#define	COND_DATA_AVAIL		0x08


static void alloc_data(struct dreamcast_gdrom_data *d)
{
	d->data_len = d->cnt;

	CHECK_ALLOCATION(d->data = (uint8_t *) malloc(d->data_len));
	memset(d->data, 0, d->data_len);
}


static void handle_command(struct cpu *cpu, struct dreamcast_gdrom_data *d)
{
	int64_t sector_nr, sector_count;
	int i, res;

	debug("[ GDROM cmd: ");
	for (i=0; i<12; i++)
		debug("%02x ", d->cmd[i]);
	debug("(cnt=%i) ]\n", d->cnt);

	if (d->data != NULL)
		free(d->data);
	d->data = NULL;
	d->cur_data_offset = 0;
	d->cur_cnt = 0;

	switch (d->cmd[0]) {

	case 0x14:
		/*  Read Table-Of-Contents:  */
		if (d->cnt != 408) {
			fatal("GDROM Read TOC not 408 bytes?\n");
			exit(1);
		}
		alloc_data(d);

		/*  TODO: Fill TOC in a better way  */
		d->data[99*4] = 1;	/*  First track  */
		d->data[100*4] = 2;	/*  Last track  */

		d->data[0*4] = 0x10;	/*  Track 1  */
		d->data[1*4] = 0x10;	/*  Track 2  */
		break;

	case 0x30:
		/*  Read sectors:  */
		if (d->cmd[1] != 0x20) {
			fatal("GDROM unimplemented data format 0x%02x\n",
			    d->cmd[1]);
			exit(1);
		}
		sector_nr = d->cmd[2] * 65536 + d->cmd[3] * 256 + d->cmd[4];
		sector_count = d->cmd[8] * 65536 + d->cmd[9] * 256 + d->cmd[10];
		if (d->cnt == 0)
			d->cnt = 65536;
		alloc_data(d);
		if (sector_count * 2048 != d->data_len) {
			fatal("Huh? GDROM data_len=0x%x, but sector_count"
			    "=0x%x\n", (int)d->data_len, (int)sector_count);
			exit(1);
		}

{
if (sector_nr >= 1376810)
	sector_nr -= 1376810;
sector_nr -= 150;
if (sector_nr > 1048576)
	sector_nr -= 1048576;
/*  printf("sector nr = %i\n", (int)sector_nr);  */

if (sector_nr < 1000)
	sector_nr += (diskimage_get_baseoffset(cpu->machine, 0, DISKIMAGE_IDE)
	 / 2048);
}

		res = diskimage_access(cpu->machine, 0, DISKIMAGE_IDE,
		    0, sector_nr * 2048, d->data, d->data_len);
		if (!res) {
			fatal("GDROM: diskimage_access failed? TODO\n");
//			exit(1);
		}
		break;

	case 0x70:
		/*  Mount: (?)  */
		break;

	default:fatal("GDROM handle_command: unimplemented command 0x%02x"
		    "\n", d->cmd[0]);
		exit(1);
	}

	if (d->data != NULL)
		d->cond |= COND_DATA_AVAIL;

	if (d->cnt == 65536)
		d->cnt = 32768;

	SYSASIC_TRIGGER_EVENT(SYSASIC_EVENT_GDROM);
}


DEVICE_ACCESS(dreamcast_gdrom)
{
	struct dreamcast_gdrom_data *d = (struct dreamcast_gdrom_data *) extra;
	uint64_t idata = 0, odata = 0;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	switch (relative_addr) {

	case GDROM_BUSY:
		if (writeflag == MEM_READ) {
			odata = d->busy;
		} else {
			fatal("[ Write to GDROM_BUSY? ]\n");
/*			exit(1);  */
		}
		break;

	case GDROM_DATA:
		if (len != sizeof(uint16_t)) {
			fatal("Non-16bit GDROM data access? TODO\n");
			exit(1);
		}

		if (writeflag == MEM_READ) {
			if (!(d->cond & COND_DATA_AVAIL)) {
				fatal("Read from GDROM_DATA when no data"
				    " is available? TODO\n");
				exit(1);
			}
			if (d->cur_data_offset < d->data_len) {
				odata = d->data[d->cur_data_offset ++];
				odata |= (d->data[d->cur_data_offset ++] << 8);
				d->cur_cnt += 2;
				if (d->cur_cnt >= d->cnt) {
					if (d->cur_data_offset >= d->data_len) {
						d->cond &= ~COND_DATA_AVAIL;
					} else {
						d->cnt = d->data_len -
						    d->cur_data_offset;
						d->cur_cnt = 0;
					}
					SYSASIC_TRIGGER_EVENT(
					    SYSASIC_EVENT_GDROM);
				}
			} else {
				fatal("Read too much from GDROM_DATA\n");
				exit(1);
			}
		} else {
			if (d->busy & 0x08) {
				if (d->cmd_count >= 12) {
					fatal("Too much GDROM_DATA?\n");
					exit(1);
				}
				/*  Add data to cmd:  */
				d->cmd[d->cmd_count++] = idata;
				d->cmd[d->cmd_count++] = idata >> 8;
				if (d->cmd_count == 12) {
					d->busy &= ~0x08;
					handle_command(cpu, d);
				}
			} else {
				fatal("Write to GDROM_DATA, but not waiting"
				    " for data?\n");
				exit(1);
			}
		}
		break;

	case GDROM_REGX:
		if (writeflag == MEM_READ) {
			fatal("Read from GDROM_REGX?\n");
			exit(1);
		} else {
			/*  NetBSD/dreamcast writes 0 here.  */
			if (idata != 0) {
				fatal("[ Write to GDROM_REGX? ]\n");
				/*  exit(1);  */
			}
		}
		break;

	case GDROM_STAT:
		if (writeflag == MEM_READ) {
			odata = d->stat;
		} else {
			fatal("[ Write to GDROM_STAT? ]\n");
/*			exit(1);  */
		}
		break;

	case GDROM_CNTLO:
		if (writeflag == MEM_READ) {
			odata = d->cnt & 0xff;
		} else {
			d->cnt = (d->cnt & 0xff00) | (idata & 0xff);
		}
		break;

	case GDROM_CNTHI:
		if (writeflag == MEM_READ) {
			odata = (d->cnt >> 8) & 0xff;
		} else {
			d->cnt = (d->cnt & 0x00ff) | ((idata & 0xff) << 8);
		}
		break;

	case GDROM_COND:
		if (writeflag == MEM_READ) {
			odata = d->cond;
		} else {
			d->cond = idata;

			/*
			 *  NetBSD/dreamcast writes 0xa0 to GDROM_COND to
			 *  start a command. It expects (BUSY & 0x88) to
			 *  be 0x08 after writing to GDROM_COND, and STAT
			 *  to be not equal to 6. NetBSD then sends 6
			 *  16-bit data words to GDROM_DATA.
			 */
			if (idata == 0xa0) {
				d->stat = 0;	/*  TODO  */
				d->busy |= 0x08;
				d->cmd_count = 0;
			} else if (idata == 0xef) {
				fatal("dreamcast_gdrom: ROM: TODO\n");
			} else {
				fatal("dreamcast_gdrom: unimplemented "
				    "GDROM_COND = 0x%02x\n", (int)idata);
				exit(1);
			}
		}
		break;

	default:if (writeflag == MEM_READ) {
			fatal("[ dreamcast_gdrom: read from addr 0x%x ]\n",
			    (int)relative_addr);
		} else {
			fatal("[ dreamcast_gdrom: write to addr 0x%x: 0x%x ]\n",
			    (int)relative_addr, (int)idata);
		}
/*		exit(1);  */
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVINIT(dreamcast_gdrom)
{
	struct dreamcast_gdrom_data *d;

	CHECK_ALLOCATION(d = (struct dreamcast_gdrom_data *) malloc(sizeof(struct dreamcast_gdrom_data)));
	memset(d, 0, sizeof(struct dreamcast_gdrom_data));

	memory_device_register(devinit->machine->memory, devinit->name,
	    0x005f7000, 0x100, dev_dreamcast_gdrom_access, d,
	    DM_DEFAULT, NULL);

	return 1;
}

