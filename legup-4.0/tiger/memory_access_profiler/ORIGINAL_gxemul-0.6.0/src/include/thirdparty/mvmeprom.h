/*	$OpenBSD: prom.h,v 1.16 2006/05/16 22:51:28 miod Exp $ */
/*
 * Copyright (c) 1998 Steve Murphree, Jr.
 * Copyright (c) 1996 Nivas Madhur
 * Copyright (c) 1995 Theo de Raadt
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __MACHINE_PROM_H__
#define __MACHINE_PROM_H__

/* BUG trap vector */
#define	MVMEPROM_VECTOR		496

#define MVMEPROM_INCHR		0x00
#define MVMEPROM_INSTAT		0x01
#define MVMEPROM_INLN		0x02
#define MVMEPROM_READSTR	0x03
#define MVMEPROM_READLN		0x04
#define MVMEPROM_DSKRD		0x10
#define MVMEPROM_DSKWR		0x11
#define MVMEPROM_DSKCFIG	0x12
#define MVMEPROM_DSKFMT		0x14
#define MVMEPROM_DSKCTRL	0x15
#define MVMEPROM_NETFOPEN	0x1b
#define MVMEPROM_NETFREAD	0x1c
#define MVMEPROM_NETCTRL	0x1d
#define MVMEPROM_OUTCHR		0x20
#define MVMEPROM_OUTSTR		0x21
#define MVMEPROM_OUTSTRCRLF	0x22
#define MVMEPROM_WRITE		0x23
#define MVMEPROM_WRITELN	0x24
#define	MVMEPROM_OUTCRLF	0x26
#define MVMEPROM_DELAY		0x43
#define MVMEPROM_RTC_RD		0x53
#define MVMEPROM_EXIT		0x63
#define MVMEPROM_GETBRDID	0x70
#define MVMEPROM_ENVIRON	0x71
#define	MVMEPROM_FORKMPU	0x100

#define NETCTRLCMD_GETETHER	1

#define ENVIRONCMD_WRITE	1
#define ENVIRONCMD_READ		2
#define ENVIRONTYPE_EOL		0
#define ENVIRONTYPE_START	1
#define ENVIRONTYPE_DISKBOOT	2
#define ENVIRONTYPE_ROMBOOT	3
#define ENVIRONTYPE_NETBOOT	4
#define ENVIRONTYPE_MEMSIZE	5

#define	FORKMPU_NOT_IDLE	-1
#define	FORKMPU_BAD_ADDRESS	-2
#define	FORKMPU_NO_MPU		-3

struct mvmeprom_netctrl {
	uint8_t		ctrl;
	uint8_t		dev;
	uint16_t	status;
	uint32_t	cmd;
	uint32_t	addr;
	uint32_t	len;
	uint32_t	flags;
};

struct mvmeprom_netfopen {
	uint8_t		ctrl;
	uint8_t		dev;
	uint16_t	status;
	char		filename[64];
};

struct mvmeprom_netfread {
	uint8_t		ctrl;
	uint8_t		dev;
	uint16_t	status;
	uint32_t	addr;
	uint16_t	bytes;
	uint16_t	blk;
	uint32_t	timeout;
};

struct prom_environ_hdr {
	uint8_t		type;
	uint8_t		len;
};

struct mvmeprom_brdid {
	uint32_t	eye_catcher;
	uint8_t		rev;
	uint8_t		month;
	uint8_t		day;
	uint8_t		year;
	uint16_t	size;
	uint16_t	rsv1;
	uint16_t	model;
	uint8_t		suffix[2];
	uint16_t	options;
	uint8_t		family;
	uint8_t		cpu;
	uint16_t	ctrlun;
	uint16_t	devlun;
	uint16_t	devtype;
	uint16_t	devnum;
	uint32_t	bug;
	uint8_t		version[4];
	uint8_t		serial[12];	/* SBC serial number */
	uint8_t		id[16];		/* SBC id */
	uint8_t		pwa[16];	/* printed wiring assembly number */
	uint8_t		speed[4];	/* cpu speed */
	uint8_t		etheraddr[6];	/* mac address, all zero if no ether */
	uint8_t		fill[2];
	uint8_t		scsiid[2];	/* local SCSI id */
	uint8_t		sysid[8];	/* system id - nothing on mvme187 */
	uint8_t		brd1_pwb[8];	/* memory board 1 pwb */
	uint8_t		brd1_serial[8];	/* memory board 1 serial */
	uint8_t		brd2_pwb[8];	/* memory board 2 pwb */
	uint8_t		brd2_serial[8];	/* memory board 2 serial */
	uint8_t		reserved[153];
	uint8_t		cksum[1];
};

struct mvmeprom_time {
        uint8_t		year_BCD;
        uint8_t		month_BCD;
        uint8_t		day_BCD;
        uint8_t		wday_BCD;
        uint8_t		hour_BCD;
        uint8_t		min_BCD;
        uint8_t		sec_BCD;
        uint8_t		cal_BCD;
};

struct mvmeprom_dskio {
	uint8_t		ctrl_lun;
	uint8_t		dev_lun;
	uint16_t	status;
	uint32_t	pbuffer;
	uint32_t	blk_num;
	uint16_t	blk_cnt;
	uint8_t	flag;
#define BUG_FILE_MARK	0x80
#define IGNORE_FILENUM	0x02
#define END_OF_FILE	0x01
	uint8_t		addr_mod;
};
#define MVMEPROM_BLOCK_SIZE	256

#endif /* __MACHINE_PROM_H__ */
