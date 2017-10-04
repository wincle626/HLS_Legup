#ifndef NETBSD_IYONIX_BOOTCONFIG_H
#define NETBSD_IYONIX_BOOTCONFIG_H

/*	$NetBSD: bootconfig.h,v 1.4 2006/10/24 20:39:14 bjh21 Exp $	*/

/*
 * Copyright (c) 2002 Reinoud Zandijk.
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
 * 3. The name of the company nor the name of the author may be used to
 *    endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * boot configuration structures
 *
 */

/*  #include <arm/bootconfig.h>  */

typedef uint32_t Xu_int;		/*  GXemul  */


/* get some spare blocks ;) */
#define DRAM_BLOCKS	32
#define VRAM_BLOCKS	16

#define PHYSMEM_TYPE_GENERIC		0
#define PHYSMEM_TYPE_PROCESSOR_ONLY	1


typedef struct {
	Xu_int address;
	Xu_int pages;
	Xu_int flags;
} phys_mem;


struct bootconfig {
	Xu_int magic;
	Xu_int version;			/* version 2+ */

	u_char machine_id[4];		/* unique machine Id */
	char kernelname[80];
	char args[512];			/* 512 bytes is better than 4096 */

	Xu_int kernvirtualbase;		/* not used now */
	Xu_int kernphysicalbase;		/* not used now */
	Xu_int kernsize;
	Xu_int scratchvirtualbase;	/* not used now */
	Xu_int scratchphysicalbase;	/* not used now */
	Xu_int scratchsize;		/* not used now */

	Xu_int ksym_start;
	Xu_int ksym_end;

	Xu_int MDFvirtualbase;		/* not used yet */
	Xu_int MDFphysicalbase;		/* not used yet */
	Xu_int MDFsize;			/* not used yet */

	Xu_int display_phys;
	Xu_int display_start;
	Xu_int display_size;
	Xu_int width;
	Xu_int height;
	Xu_int log2_bpp;
	Xu_int framerate;

	char reserved[512];		/* future expansion */

	Xu_int pagesize;
	Xu_int drampages;
	Xu_int vrampages;
	Xu_int dramblocks;
	Xu_int vramblocks;

	phys_mem dram[DRAM_BLOCKS];
	phys_mem vram[VRAM_BLOCKS];

};


#define BOOTCONFIG_MAGIC     0x43112233
#define BOOTCONFIG_VERSION   	    0x2

extern struct bootconfig bootconfig;

/* End of bootconfig.h */

#endif	/*  NETBSD_IYONIX_BOOTCONFIG_H  */

