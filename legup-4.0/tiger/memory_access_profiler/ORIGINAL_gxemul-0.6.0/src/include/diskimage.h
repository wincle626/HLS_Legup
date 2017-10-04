#ifndef	DISKIMAGE_H
#define	DISKIMAGE_H

/*
 *  Copyright (C) 2003-2010  Anders Gavare.  All rights reserved.
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
 *  Generic disk image functions.  (See diskimage.c for more info.)
 */

#include <stdio.h>
#include <sys/types.h>

#include "misc.h"

/*  Diskimage types:  */
#define	DISKIMAGE_SCSI		1
#define	DISKIMAGE_IDE		2
#define	DISKIMAGE_FLOPPY	3

#define	DISKIMAGE_TYPES		{ "(NONE)", "SCSI", "IDE", "FLOPPY" }


/*  512 bytes per overlay block. Don't change this.  */
#define	OVERLAY_BLOCK_SIZE	512

struct diskimage_overlay {
	char		*overlay_basename;
	FILE		*f_data;
	FILE		*f_bitmap;
};

struct diskimage {
	struct diskimage *next;
	int		type;		/*  DISKIMAGE_SCSI, etc  */
	int		id;		/*  SCSI id  */

	/*  Filename in host's file system:  */
	char		*fname;
	FILE		*f;

	/*  Overlays:  */
	int		nr_of_overlays;
	struct diskimage_overlay *overlays;

	int		chs_override;
	int		cylinders;
	int		heads;
	int		sectors_per_track;

	off_t		total_size;
	int64_t		override_base_offset;
	int		logical_block_size;

	int		writable;
	int		is_a_cdrom;
	int		is_boot_device;

	int		is_a_tape;
	uint64_t	tape_offset;
	int		tape_filenr;
	int		filemark;

	int		rpms;
	int		ncyls;
};


/*  Transfer command, sent from a SCSI controller device to a disk:  */
struct scsi_transfer {
	struct scsi_transfer	*next_free;

	/*  These should be set by the SCSI controller before the call:  */
	unsigned char		*msg_out;
	size_t			msg_out_len;
	unsigned char		*cmd;
	size_t			cmd_len;

	/*  data_out_len is set by the SCSI disk, if it needs data_out,
	    which is then filled in during a second pass in the controller.  */
	unsigned char		*data_out;
	size_t			data_out_len;
	size_t			data_out_offset;

	/*  These should be set by the SCSI (disk) device before returning:  */
	unsigned char		*data_in;
	size_t			data_in_len;
	unsigned char		*msg_in;
	size_t			msg_in_len;
	unsigned char		*status;
	size_t			status_len;
};


struct machine;


/*  diskimage_scsicmd.c:  */
struct scsi_transfer *scsi_transfer_alloc(void);
void scsi_transfer_free(struct scsi_transfer *);
void scsi_transfer_allocbuf(size_t *lenp, unsigned char **pp,
	size_t want_len, int clearflag);
int diskimage_scsicommand(struct cpu *cpu, int id, int type,
	struct scsi_transfer *);


/*  diskimage.c:  */
int64_t diskimage_getsize(struct machine *machine, int id, int type);
int64_t diskimage_get_baseoffset(struct machine *machine, int id, int type);
void diskimage_getchs(struct machine *machine, int id, int type,
	int *c, int *h, int *s);
int diskimage__internal_access(struct diskimage *d, int writeflag,
	off_t offset, unsigned char *buf, size_t len);
int diskimage_access(struct machine *machine, int id, int type, int writeflag,
	off_t offset, unsigned char *buf, size_t len);
void diskimage_add_overlay(struct diskimage *d, char *overlay_basename);
void diskimage_recalc_size(struct diskimage *d);
int diskimage_exist(struct machine *machine, int id, int type);
int diskimage_bootdev(struct machine *machine, int *typep);
int diskimage_add(struct machine *machine, char *fname);
int diskimage_getname(struct machine *machine, int id, int type,
	char *buf, size_t bufsize);
int diskimage_is_a_cdrom(struct machine *machine, int id, int type);
int diskimage_is_a_tape(struct machine *machine, int id, int type);
void diskimage_dump_info(struct machine *machine);


/*
 *  SCSI commands: 
 */
#define	SCSICMD_TEST_UNIT_READY		0x00	/*  Mandatory  */
#define	SCSICMD_REQUEST_SENSE		0x03	/*  Mandatory  */
#define	SCSICMD_INQUIRY			0x12	/*  Mandatory  */

#define	SCSICMD_READ			0x08
#define	SCSICMD_READ_10			0x28
#define	SCSICMD_WRITE			0x0a
#define	SCSICMD_WRITE_10		0x2a
#define	SCSICMD_MODE_SELECT		0x15
#define	SCSICMD_MODE_SENSE		0x1a
#define	SCSICMD_START_STOP_UNIT		0x1b
#define	SCSICMD_PREVENT_ALLOW_REMOVE	0x1e
#define	SCSICMD_MODE_SENSE10		0x5a

#define	SCSICMD_SYNCHRONIZE_CACHE	0x35

/*  SCSI block device commands:  */
#define	SCSIBLOCKCMD_READ_CAPACITY	0x25

/*  SCSI CD-ROM commands:  */
#define	SCSICDROM_READ_SUBCHANNEL	0x42
#define	SCSICDROM_READ_TOC		0x43
#define	SCSICDROM_READ_DISCINFO		0x51
#define	SCSICDROM_READ_TRACKINFO	0x52

/*  SCSI tape commands:  */
#define	SCSICMD_REWIND			0x01
#define	SCSICMD_READ_BLOCK_LIMITS	0x05
#define	SCSICMD_SPACE			0x11


#endif	/*  DISKIMAGE_H  */
