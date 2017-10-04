#ifndef	SH4_DMACREG_H
#define	SH4_DMACREG_H

/*
 *  Copyright (C) 2006-2010  Anders Gavare.  All rights reserved.
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
 *  SH4 DMAC (DMA Controller) registers, as listed in the SH-7750 and
 *  SH-7760 manuals.
 */

#define	N_SH4_DMA_CHANNELS	8	/*  4 on 7750, 8 on 7760  */

#define	SH4_SAR0	0xffa00000	/*  Source Address Register  */
#define	SH4_DAR0	0xffa00004	/*  Destination Address Register  */
#define	SH4_DMATCR0	0xffa00008	/*  Transfer Count Register  */
#define	SH4_CHCR0	0xffa0000c	/*  Channel Control Register  */

#define	SH4_SAR1	0xffa00010
#define	SH4_DAR1	0xffa00014
#define	SH4_DMATCR1	0xffa00018
#define	SH4_CHCR1	0xffa0001c

#define	SH4_SAR2	0xffa00020
#define	SH4_DAR2	0xffa00024
#define	SH4_DMATCR2	0xffa00028
#define	SH4_CHCR2	0xffa0002c

#define	SH4_SAR3	0xffa00030
#define	SH4_DAR3	0xffa00034
#define	SH4_DMATCR3	0xffa00038
#define	SH4_CHCR3	0xffa0003c

/*  TODO: Registers at 0xffa00040.  */

/*  NOTE: Channel 4 doesn't start at 0xffa00040, but 0xffa00050!  */

#define	SH4_SAR4	0xffa00050
#define	SH4_DAR4	0xffa00054
#define	SH4_DMATCR4	0xffa00058
#define	SH4_CHCR4	0xffa0005c

#define	SH4_SAR5	0xffa00060
#define	SH4_DAR5	0xffa00064
#define	SH4_DMATCR5	0xffa00068
#define	SH4_CHCR5	0xffa0006c

#define	SH4_SAR6	0xffa00070
#define	SH4_DAR6	0xffa00074
#define	SH4_DMATCR6	0xffa00078
#define	SH4_CHCR6	0xffa0007c

#define	SH4_SAR7	0xffa00080
#define	SH4_DAR7	0xffa00084
#define	SH4_DMATCR7	0xffa00088
#define	SH4_CHCR7	0xffa0008c


/*
 *  CHCR (Channel Control Register) bit definitions:
 */

#define	CHCR_SSA_MASK	0xe0000000	/*  Source Address Space Attribute Specification  */
					/*  (Only valid for PCMCIA access, in areas 5 and 6.)  */
#define	    CHCR_SSA_RESERVED				(0 << 29)
#define	    CHCR_SSA_DYNAMIC_BUS_SIZING			(1 << 29)
#define	    CHCR_SSA_8BIT_IO_SPACE			(2 << 29)
#define	    CHCR_SSA_16BIT_IO_SPACE			(3 << 29)
#define	    CHCR_SSA_8BIT_COMMON_MEMORY_SPACE		(4 << 29)
#define	    CHCR_SSA_16BIT_COMMON_MEMORY_SPACE		(5 << 29)
#define	    CHCR_SSA_8BIT_ATTRIBUTE_MEMORY_SPACE	(6 << 29)
#define	    CHCR_SSA_16BIT_ATTRIBUTE_MEMORY_SPACE	(7 << 29)
#define	CHCR_STC	0x10000000	/*  Source Address Wait Control Select  */
#define	CHCR_DSA_MASK	0x0e000000	/*  Destination Address Space Attribute Specification  */
#define	    CHCR_DSA_RESERVED				(0 << 25)
#define	    CHCR_DSA_DYNAMIC_BUS_SIZING			(1 << 25)
#define	    CHCR_DSA_8BIT_IO_SPACE			(2 << 25)
#define	    CHCR_DSA_16BIT_IO_SPACE			(3 << 25)
#define	    CHCR_DSA_8BIT_COMMON_MEMORY_SPACE		(4 << 25)
#define	    CHCR_DSA_16BIT_COMMON_MEMORY_SPACE		(5 << 25)
#define	    CHCR_DSA_8BIT_ATTRIBUTE_MEMORY_SPACE	(6 << 25)
#define	    CHCR_DSA_16BIT_ATTRIBUTE_MEMORY_SPACE	(7 << 25)
#define	CHCR_DTC	0x01000000	/*  Destination Address Wait Control Select  */
#define	CHCR_DS		0x00080000	/*  DREQ Select  */
#define	CHCR_RL		0x00040000	/*  Request Check Level  */
#define	CHCR_AM		0x00020000	/*  Acknowledge Mode  */
#define	CHCR_AL		0x00010000	/*  Acknowledge Level  */
#define	CHCR_DM		0x0000c000	/*  Destination Address Mode 1 and 0  */
#define	    CHCR_DM_FIXED	(0 << 14)	/*  Destination Address Fixed  */
#define	    CHCR_DM_INCREMENTED	(1 << 14)	/*  Destination Address Incremented  */
#define	    CHCR_DM_DECREMENTED	(2 << 14)	/*  Destination Address Decremented  */
#define	CHCR_SM		0x00003000	/*  Source Address Mode 1 and 0  */
#define	    CHCR_SM_FIXED	(0 << 12)	/*  Source Address Fixed  */
#define	    CHCR_SM_INCREMENTED	(1 << 12)	/*  Source Address Incremented  */
#define	    CHCR_SM_DECREMENTED	(2 << 12)	/*  Source Address Decremented  */
#define	CHCR_RS		0x00000f00	/*  Resource Select  */
#define	CHCR_TM		0x00000080	/*  Transmit Mode (0=cycle steal, 1=burst)  */
#define	CHCR_TS		0x00000070	/*  Transmit Size  */
#define	    CHCR_TS_8BYTE	(0 << 4)
#define	    CHCR_TS_1BYTE	(1 << 4)
#define	    CHCR_TS_2BYTE	(2 << 4)
#define	    CHCR_TS_4BYTE	(3 << 4)
#define	    CHCR_TS_32BYTE	(4 << 4)
#define	CHCR_CHSET	0x00000008	/*  Channel Setting  */
#define	CHCR_IE		0x00000004	/*  Interrupt Enable  */
#define	CHCR_TE		0x00000002	/*  Transfer End  */
#define	CHCR_TD		0x00000001	/*  DMAC Enable  */

#define	SH4_DMAOR	0xffa00040	/*  DMA operation register  */

#endif	/*  SH4_DMACREG_H  */
