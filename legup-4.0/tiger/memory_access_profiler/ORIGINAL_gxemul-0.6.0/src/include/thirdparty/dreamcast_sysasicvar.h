/*  GXemul: $Id: dreamcast_sysasicvar.h,v 1.2 2006-10-28 01:37:54 debug Exp $  */
/*  $NetBSD: sysasicvar.h,v 1.5 2005/12/24 23:24:00 perry Exp $  */

/*
 *  Extended with useful macros and also some more event definitions
 *  described in KOS comments by Dan Potter (kos/kernel/arch/dreamcast
 *  /hardware/asic.c).
 */

#ifndef _DREAMCAST_SYSASICVAR_H_
#define	_DREAMCAST_SYSASICVAR_H_

/*-
 * Copyright (c) 2001 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by 
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
 *        This product includes software developed by the NetBSD
 *        Foundation, Inc. and its contributors.
 * 4. Neither the name of The NetBSD Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#define	SYSASIC_BASE		0x5f6900
#define	SYSASIC_SIZE		   0x100

#define	SYSASIC_EVENT_RENDERDONE	2	/*  Render Completed  */
#define	SYSASIC_EVENT_PVR_SCANINT1	3	/*  Scanline interrupt 1  */
#define	SYSASIC_EVENT_PVR_SCANINT2	4	/*  Scanline interrupt 2  */
#define	SYSASIC_EVENT_VBLINT		5	/*  VBlank interrupt  */
#define	SYSASIC_EVENT_OPAQUEDONE	7	/*  Opaque list complete  */
#define	SYSASIC_EVENT_OPAQUEMODDONE	8	/*  Opaque modifiers complete */
#define	SYSASIC_EVENT_TRANSDONE		9	/*  Transparent list complete */
#define	SYSASIC_EVENT_TRANSMODDONE	10	/*  Trans. modifiers complete */
#define SYSASIC_EVENT_MAPLE_DMADONE	12	/*  Maple DMA complete  */
#define SYSASIC_EVENT_MAPLE_ERROR	13	/*  Maple error  */
#define	SYSASIC_EVENT_GDROM_DMA		14	/*  GD-ROM DMA Complete  */
#define	SYSASIC_EVENT_SPU_DMA		15	/*  SPU DMA Complete  */
#define	SYSASIC_EVENT_SPU_IRQ		17	/*  SPU Interrupt  */
#define	SYSASIC_EVENT_PVR_DMA		19	/*  PVR DMA Complete  */
#define	SYSASIC_EVENT_PVR_PTDONE	21	/*  Punch-through complete  */
#define SYSASIC_EVENT_GDROM		32	/*  GD-ROM Command status  */
#define SYSASIC_EVENT_AICA		33	/*  AICA (?)  */
#define SYSASIC_EVENT_8BIT		34	/*  Modem/Lan adapter  */
#define SYSASIC_EVENT_EXT		35	/*  PCI/BBA IRQ  */
#define	SYSASIC_EVENT_PRIMOUTOFMEM	66	/*  Out of primitive mem  */
#define	SYSASIC_EVENT_MATOUTOFMEM	67	/*  Out of matrix mem  */

#define	SYSASIC_EVENT_TO_ADDR(e)	(SYSASIC_BASE + 4*((e)>>5))
#define	SYSASIC_EVENT_TO_BITMASK(e)	(1 << ((e) & 31))

#define	SYSASIC_TRIGGER_EVENT(e)	{				\
	uint8_t buf[8];							\
	uint64_t tmp1 = SYSASIC_EVENT_TO_ADDR(e);			\
	uint64_t tmp2 = SYSASIC_EVENT_TO_BITMASK(e);			\
	tmp2 |= 0x100000000ULL;		/*  Internal GXemul hack  */	\
	memory_writemax64(cpu, buf, 8, tmp2);				\
	cpu->memory_rw(cpu, cpu->mem, tmp1, buf, 8,			\
	    MEM_WRITE, PHYSICAL);					\
	}


#if 0
const char *__pure sysasic_intr_string(int /*ipl*/) __attribute__((const));
void	*sysasic_intr_establish(int /*event*/, int /*ipl*/,
	    int (*ih_fun)(void *), void *);
void	sysasic_intr_disestablish(void *);
void	sysasic_intr_enable(void *, int /*on*/);
#endif

#endif /* !_DREAMCAST_SYSASICVAR_H_ */
