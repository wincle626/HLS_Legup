/*  gxemul: $Id: exec_ecoff_mips.h,v 1.7 2005-03-05 12:34:02 debug Exp $
 *  Addition of some ECOFF magin numbers, and ECOFF_BADMAG has been commented
 *  out to avoid warnings on Solaris.
 */

#ifndef EXEC_ECOFF_MIPS_H
#define EXEC_ECOFF_MIPS_H

/*	$NetBSD: ecoff_machdep.h,v 1.18 2002/03/05 14:12:29 simonb Exp $	*/

/*
 * Copyright (c) 1997 Jonathan Stone
 * All rights reserved.
 *
 * Copyright (c) 1994 Adam Glass
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Adam Glass.
 * 4. The name of the Author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Adam Glass ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL Adam Glass BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#define ECOFF_LDPGSZ 4096

#define ECOFF_PAD

/*  gxemul NOTE: Original types in NetBSD were 'u_long', not uint32_t  */

#define ECOFF_MACHDEP \
        uint32_t gprmask; \
        uint32_t cprmask[4]; \
        uint32_t gp_value
#ifdef _KERNEL
#include <mips/cpu.h>		/* mips CPU architecture levels */
#define _MIPS3_OK() CPUISMIPS3
#else
#define _MIPS3_OK() 1
#endif


#define ECOFF_MAGIC_MIPSEB	0x0160	/* mips1, big-endian */
#define ECOFF_MAGIC_MIPSEL	0x0162	/* mips1, little-endian */
#define ECOFF_MAGIC_MIPSEB2	0x0163	/* mips2, big-endian  (gxemul addition) */
#define ECOFF_MAGIC_MIPSEL2	0x0166	/* mips2, little-endian  (gxemul addition) */
#define ECOFF_MAGIC_MIPSEB3	0x0140	/* mips3, big-endian  (gxemul addition) */
#define ECOFF_MAGIC_MIPSEL3	0x0142	/* mips3, little-endian */

#if 0
/*  commented out in gxemul, to avoid warnings on Solaris  */
#if BYTE_ORDER == LITTLE_ENDIAN
#define ECOFF_BADMAG(ep) \
    (!								\
	((ep)->f.f_magic == ECOFF_MAGIC_MIPSEL ||		\
	 (_MIPS3_OK() && (ep)->f.f_magic == ECOFF_MAGIC_MIPSEL3)) \
    )
#endif
#if BYTE_ORDER == BIG_ENDIAN
#define ECOFF_BADMAG(ep) ((ep)->f.f_magic != ECOFF_MAGIC_MIPSEB)
#endif
#endif	/*  gxemul  */

#define ECOFF_SEGMENT_ALIGNMENT(ep) ((ep)->a.vstamp < 23 ? 8 : 16)

#ifdef _KERNEL
struct proc;
struct exec_package;
void	cpu_exec_ecoff_setregs(struct proc *, struct exec_package *, u_long);
#endif	/* _KERNEL */


/*
 * ECOFF symbol definitions for 32-bit mips.
 * XXX 64-bit (mips3?) may be different.
 */
struct ecoff_symhdr {
	int16_t		magic;
	int16_t		vstamp;
	int32_t		ilineMax;
	int32_t		cbLine;
	int32_t		cbLineOffset;
	int32_t		idnMax;
	int32_t		cbDnOffset;
	int32_t		ipdMax;
	int32_t		cbPdOffset;
	int32_t		isymMax;
	int32_t		cbSymOffset;
	int32_t		ioptMax;
	int32_t		cbOptOffset;
	int32_t		iauxMax;
	int32_t		cbAuxOffset;
	int32_t		issMax;
	int32_t		cbSsOffset;
	int32_t		issExtMax;
	int32_t		cbSsExtOffset;
	int32_t		ifdMax;
	int32_t		cbFdOffset;
	int32_t		crfd;
	int32_t		cbRfdOffset;
	int32_t		iextMax;
	int32_t		cbExtOffset;
};

#define	MIPS_MAGIC_SYM	0x7009		/*  gxemul addition  */

/* Macro for field name used by cgd's Alpha-derived code */
#define esymMax iextMax


struct ecoff_extsym {
	uint16_t	es_flags;
	uint16_t	es_ifd;
	int32_t		es_strindex;
	int32_t		es_value;
	unsigned	es_type:6;
	unsigned	es_class:5;
	unsigned	:1;
	unsigned	es_symauxindex:20;
};

#endif	/*  EXEC_ECOFF_MIPS_H  */

