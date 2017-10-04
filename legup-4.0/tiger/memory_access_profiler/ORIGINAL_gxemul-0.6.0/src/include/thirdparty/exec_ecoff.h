/*  gxemul: $Id: exec_ecoff.h,v 1.3 2005-03-05 12:34:02 debug Exp $  */
/*	$NetBSD: exec_ecoff.h,v 1.12 2000/11/21 00:37:56 jdolecek Exp $	*/

/*
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
 *      This product includes software developed by Adam Glass.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission
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

#ifndef	_SYS_EXEC_ECOFF_H_
#define	_SYS_EXEC_ECOFF_H_

/*  #include <machine/ecoff_machdep.h>  */
#include "exec_ecoff_mips.h"

struct ecoff_filehdr {
	uint16_t f_magic;	/* magic number */			/*  u_short  */
	uint16_t f_nscns;	/* # of sections */			/*  u_short  */
	uint32_t f_timdat;	/* time and date stamp */		/*  u_int  */
	uint32_t f_symptr;	/* file offset of symbol table */	/*  u_long  */
	uint32_t f_nsyms;	/* # of symbol table entries */		/*  u_int  */
	uint16_t f_opthdr;	/* sizeof the optional header */	/*  u_short  */
	uint16_t f_flags;	/* flags??? */				/*  u_short  */
};
						/*  original netbsd types ----^  */

struct ecoff_aouthdr {
	u_short magic;		/*  u_short  */
	u_short vstamp;		/*  u_short  */
	ECOFF_PAD
	uint32_t tsize;		/*  u_long  */
	uint32_t dsize;		/*  u_long  */
	uint32_t bsize;		/*  u_long  */
	uint32_t entry;		/*  u_long  */
	uint32_t text_start;	/*  u_long  */
	uint32_t data_start;	/*  u_long  */
	uint32_t bss_start;	/*  u_long  */
	ECOFF_MACHDEP;
};

struct ecoff_scnhdr {		/* needed for size info */
	char	 s_name[8];	/* name */
	uint32_t s_paddr;	/* physical addr? for ROMing?*/		/*  u_long  */
	uint32_t s_vaddr;	/* virtual addr? */			/*  u_long  */
	uint32_t s_size;		/* size */				/*  u_long  */
	uint32_t s_scnptr;	/* file offset of raw data */		/*  u_long  */
	uint32_t s_relptr;	/* file offset of reloc data */		/*  u_long  */
	uint32_t s_lnnoptr;	/* file offset of line data */		/*  u_long  */
	uint16_t s_nreloc;	/* # of relocation entries */		/*  u_short  */
	uint16_t s_nlnno;	/* # of line entries */			/*  u_short  */
	uint32_t s_flags;	/* flags */				/*  u_int  */
};

struct ecoff_exechdr {
	struct ecoff_filehdr f;
	struct ecoff_aouthdr a;
};

#define ECOFF_HDR_SIZE (sizeof(struct ecoff_exechdr))

#define ECOFF_OMAGIC 0407
#define ECOFF_NMAGIC 0410
#define ECOFF_ZMAGIC 0413

#define ECOFF_ROUND(value, by) \
        (((value) + (by) - 1) & ~((by) - 1))

#define ECOFF_BLOCK_ALIGN(ep, value) \
        ((ep)->a.magic == ECOFF_ZMAGIC ? ECOFF_ROUND((value), ECOFF_LDPGSZ) : \
	 (value))

#define ECOFF_TXTOFF(ep) \
        ((ep)->a.magic == ECOFF_ZMAGIC ? 0 : \
	 ECOFF_ROUND(ECOFF_HDR_SIZE + (ep)->f.f_nscns * \
		     sizeof(struct ecoff_scnhdr), ECOFF_SEGMENT_ALIGNMENT(ep)))

#define ECOFF_DATOFF(ep) \
        (ECOFF_BLOCK_ALIGN((ep), ECOFF_TXTOFF(ep) + (ep)->a.tsize))

#define ECOFF_SEGMENT_ALIGN(ep, value) \
        (ECOFF_ROUND((value), ((ep)->a.magic == ECOFF_ZMAGIC ? ECOFF_LDPGSZ : \
         ECOFF_SEGMENT_ALIGNMENT(ep))))

#ifdef _KERNEL
int	exec_ecoff_makecmds __P((struct proc *, struct exec_package *));
int	exec_ecoff_setup_stack __P((struct proc *, struct exec_package *));
int	cpu_exec_ecoff_probe __P((struct proc *, struct exec_package *));
void	cpu_exec_ecoff_setregs __P((struct proc *, struct exec_package *,
	    u_long));

int	exec_ecoff_prep_omagic __P((struct proc *, struct exec_package *,
	    struct ecoff_exechdr *, struct vnode *));
int	exec_ecoff_prep_nmagic __P((struct proc *, struct exec_package *,
	    struct ecoff_exechdr *, struct vnode *));
int	exec_ecoff_prep_zmagic __P((struct proc *, struct exec_package *,
	    struct ecoff_exechdr *, struct vnode *));

#endif /* _KERNEL */
#endif /* !_SYS_EXEC_ECOFF_H_ */
