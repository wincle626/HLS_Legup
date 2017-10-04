/*  GXemul: $Id: dreamcast_pvr.h,v 1.6 2006-10-31 08:27:26 debug Exp $  */
/*	$NetBSD: pvr.c,v 1.22 2006/04/12 19:38:22 jmmv Exp $	*/

#ifndef	DREAMCAST_PVR_H
#define	DREAMCAST_PVR_H

/*
 *  Note: This was pvr.c in NetBSD. It has been extended with reasonably
 *  similar symbolnames from http://www.ludd.luth.se/~jlo/dc/powervr-reg.txt.
 *
 *  There are still many things missing.
 */

/*-
 * Copyright (c) 2001 Marcus Comstedt.
 * Copyright (c) 2001 Jason R. Thorpe.
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
 *	This product includes software developed by Marcus Comstedt.
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

/*
 * Copyright (c) 1998, 1999 Tohru Nishimura.  All rights reserved.
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
 *      This product includes software developed by Tohru Nishimura
 *	for the NetBSD Project.
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

#define	PVRREG_FBSTART		0x05000000
#define	PVRREG_REGSTART		0x005f8000

#define	PVRREG_REGSIZE		0x00002000


#define	PVRREG_ID		0x00

#define	PVRREG_REVISION		0x04
#define	PVR_REVISION_MINOR_MASK	0xf
#define	PVR_REVISION_MAJOR_MASK	0xf0
#define	PVR_REVISION_MAJOR_SHIFT	4

#define	PVRREG_RESET		0x08
#define	PVR_RESET_TA		0x00000001
#define	PVR_RESET_PVR		0x00000002
#define	PVR_RESET_BUS		0x00000004

#define	PVRREG_STARTRENDER	0x14

#define	PVRREG_OB_ADDR		0x20
/*  Object Buffer start address. Bits 0..19 should always be zero.  */
#define	PVR_OB_ADDR_MASK	0x00f00000

#define	PVRREG_TILEBUF_ADDR	0x2c
#define	PVR_TILEBUF_ADDR_MASK	0x00fffff8

#define	PVRREG_SPANSORT		0x30
#define	PVR_SPANSORT_SPAN0		0x00000001
#define	PVR_SPANSORT_SPAN1		0x00000100
#define	PVR_SPANSORT_TSP_CACHE_ENABLE	0x00010000

#define	PVRREG_BRDCOLR		0x40
#define	BRDCOLR_BLUE(x)		((x) << 0)
#define	BRDCOLR_GREEN(x)	((x) << 8)
#define	BRDCOLR_RED(x)		((x) << 16)

#define	PVRREG_DIWMODE		0x44
#define	DIWMODE_DE		(1U << 0)	/* display enable */
#define	DIWMODE_SD		(1U << 1)	/* scan double enable */
#define	DIWMODE_COL(x)		((x) << 2)
#define	DIWMODE_COL_RGB555	DIWMODE_COL(0)	/* RGB555, 16-bit */
#define	DIWMODE_COL_RGB565	DIWMODE_COL(1)	/* RGB565, 16-bit */
#define	DIWMODE_COL_RGB888	DIWMODE_COL(2)	/* RGB888, 24-bit */
#define	DIWMODE_COL_ARGB888	DIWMODE_COL(3)	/* RGB888, 32-bit */
#define	DIWMODE_C		(1U << 23)	/* 2x clock enable (VGA) */
#define	DIWMODE_DE_MASK		0x00000001
#define	DIWMODE_SD_MASK		0x00000002	/*  Line double  */
#define	DIWMODE_COL_MASK	0x0000000c	/*  Pixel mode  */
#define	DIWMODE_COL_SHIFT	2
#define	DIWMODE_EX_MASK		0x00000070	/*  Extend bits  */
#define	DIWMODE_EX_SHIFT	4
#define	DIWMODE_TH_MASK		0x0000ff00	/*  ARGB8888 threshold  */
#define	DIWMODE_TH_SHIFT	8
#define	DIWMODE_SL_MASK		0x003f0000	/*  Strip Length  */
#define	DIWMODE_SL_SHIFT	16
#define	DIWMODE_SE_MASK		0x00400000	/*  Strip Buffer enabled  */
#define	DIWMODE_C_MASK		0x00800000	/*  Clock double  */

#define	PVRREG_FB_RENDER_CFG	0x48
#define	  FB_RENDER_CFG_RENDER_MODE_MASK	0x07
/*	render mode:
		0: RGB0555  (2 bytes/pixel, alpha is inserted into bit15)
		1: RGB565   (2 bytes/pixel)
		2: ARGB4444 (2 bytes/pixel)
		3: ARGB1555 (2 bytes/pixel, alpha is determined by threshold)
		4: RGB888   (3 bytes/pixel)
		5: RGB0888  (4 bytes/pixel, alpha is inserted into bit24-31)
		6: ARGB8888 (4 bytes/pixel)
		7: ARGB4444 (2 bytes/pixel, same as 2?)  */
#define	  FB_RENDER_CFG_DITHER	(1 << 3)
#define	  FB_RENDER_CFG_ALPHA_MASK	0x0000ff00
#define	  FB_RENDER_CFG_ALPHA_SHFIT	8
#define	  FB_RENDER_CFG_THRESHOLD_MASK	0x00ff0000
#define	  FB_RENDER_CFG_THRESHOLD_SHFIT	8

#define	PVRREG_FB_RENDER_MODULO	0x4c
#define	FB_RENDER_MODULO_MASK	0x000001ff
/*  TODO  */

#define	PVRREG_DIWADDRL		0x50

#define	PVRREG_DIWADDRS		0x54

#define	PVRREG_DIWSIZE		0x5c
#define	DIWSIZE_DPL(x)		((x) << 0)	/* pixel data per line */
#define	DIWSIZE_LPF(x)		((x) << 10)	/* lines per field */
#define	DIWSIZE_MODULO(x)	((x) << 20)	/* words to skip + 1 */
#define	DIWSIZE_MASK		0x3ff		/*  All fields are 10 bits.  */
#define	DIWSIZE_DPL_SHIFT	0
#define	DIWSIZE_LPF_SHIFT	10
#define	DIWSIZE_MODULO_SHIFT	20

#define	PVRREG_FB_RENDER_ADDR1	0x60		/*  Odd interlace lines  */

#define	PVRREG_FB_RENDER_ADDR2	0x64		/*  Even interlace lines  */

#define	PVRREG_FB_CLIP_X	0x68		/*  horizontal pixel clipping area - 1  */
#define	PVRREG_FB_CLIP_Y	0x6c		/*  vertical pixel clipping area - 1  */
#define	  FB_CLIP_XY_MIN_MASK	  0x000007ff
#define	  FB_CLIP_XY_MAX_MASK	  0x07ff0000	/*  e.g. 640 for x  */
#define	  FB_CLIP_XY_MAX_SHIFT	  16

#define	PVRREG_SHADOW		0x74
#define	  SHADOW_INTENSITY_MASK	  0x000000ff
#define	  SHADOW_ENABLE		  (1 << 8)

#define	PVRREG_OBJECT_CLIP	0x78	/*  float, position of polygon culling  */

#define	PVRREG_OB_CFG		0x7c	/*  TODO  */

#define	PVRREG_UNKNOWN_80	0x80
#define	PVRREG_UNKNOWN_84	0x84

#define	PVRREG_BGPLANE_Z	0x88	/*  float  */

#define	PVRREG_BGPLANE_CFG	0x8c	/*  TODO  */

#define	PVRREG_ISP_CFG		0x98	/*  TODO  */

#define	PVRREG_VRAM_CFG1	0xa0
#define	VRAM_CFG1_GOOD_REFRESH_VALUE	0x20

#define	PVRREG_VRAM_CFG2	0xa4
#define	VRAM_CFG2_UNKNOWN_MAGIC	  0x0000001f

#define	PVRREG_VRAM_CFG3	0xa8
#define	VRAM_CFG3_UNKNOWN_MAGIC	  0x15d1c951

#define	PVRREG_FOG_TABLE_COL	0xb0
#define	PVRREG_FOG_VERTEX_COL	0xb4
#define	PVRREG_FOG_DENSITY	0xb8	/*  TODO  */

#define	PVRREG_CLAMP_MAX	0xbc		/*  maximum color  */
#define	PVRREG_CLAMP_MIN	0xc0		/*  minimum color  */

#define	PVRREG_HPOS_IRQ		0xc8		/*  http://www.ludd.luth.se/~jlo/dc/powervr-reg.txt  */

#define	PVRREG_RASEVTPOS	0xcc		/*  vpos_irq according to powervr-reg.txt  */
#define	RASEVTPOS_POS2_MASK	0x000003ff
#define	RASEVTPOS_POS1_MASK	0x03ff0000
#define	RASEVTPOS_POS1_SHIFT	16
#define	RASEVTPOS_BOTTOM(x)	((x) << 0)
#define	RASEVTPOS_TOP(x)	((x) << 16)

#define	PVRREG_SYNCCONF		0xd0
#define	SYNCCONF_VP		(1U << 0)	/* V-sync polarity */
#define	SYNCCONF_HP		(1U << 1)	/* H-sync polarity */
#define	SYNCCONF_I		(1U << 4)	/* interlace */
#define	SYNCCONF_BC(x)		(1U << 6)	/* broadcast standard */
#define	SYNCCONF_VO		(1U << 8)	/* video output enable */
#define	SYNCCONF_VO_MASK	0x00000100
#define	SYNCCONF_BC_MASK	0x000000c0
#define	SYNCCONF_BC_SHIFT	6
#define	SYNCCONF_BC_VGA		  0
#define	SYNCCONF_BC_NTSC	  1
#define	SYNCCONF_BC_PAL		  2
#define	SYNCCONF_I_MASK		0x00000010
#define	SYNCCONF_HP_MASK	0x00000004	/*  Positive H-sync  */
#define	SYNCCONF_VP_MASK	0x00000002	/*  Positive V-sync  */

#define	PVRREG_BRDHORZ		0xd4
#define	BRDHORZ_STOP_MASK	0x0000ffff
#define	BRDHORZ_START_MASK	0xffff0000
#define	BRDHORZ_START_SHIFT	16
#define	BRDHORZ_STOP(x)		((x) << 0)
#define	BRDHORZ_START(x)	((x) << 16)

#define	PVRREG_SYNCSIZE		0xd8
#define	SYNCSIZE_H_MASK		0x0000ffff
#define	SYNCSIZE_V_MASK		0xffff0000
#define	SYNCSIZE_V_SHIFT	16
#define	SYNCSIZE_H(x)		((x) << 0)
#define	SYNCSIZE_V(x)		((x) << 16)

#define	PVRREG_BRDVERT		0xdc
#define	BRDVERT_STOP_MASK	0x0000ffff
#define	BRDVERT_START_MASK	0xffff0000
#define	BRDVERT_START_SHIFT	16
#define	BRDVERT_STOP(x)		((x) << 0)
#define	BRDVERT_START(x)	((x) << 16)

#define	PVRREG_SYNCH_WIDTH	0xe0		/*  http://www.ludd.luth.se/~jlo/dc/powervr-reg.txt  */

#define	PVRREG_TSP_CFG		0xe4		/*  http://www.ludd.luth.se/~jlo/dc/powervr-reg.txt  */
#define	    TSP_CFG_CBE		    (1 << 17)	/*  codebook enable  */
#define	    TSP_CFG_IE		    (1 << 16)	/*  index enable  */
#define	    TSP_CFG_MODULO_MASK	    0x1f	/*  modulo  */

#define	PVRREG_DIWCONF		0xe8
#define	DIWCONF_BLANK		(1U << 3)	/*  blank screen  */
#define	DIWCONF_LR		(1U << 8)	/*  low-res (320 horizontal)  */
#define	DIWCONF_MAGIC_MASK	0x003f0000
#define	DIWCONF_MAGIC		(22 << 16)

#define	PVRREG_DIWHSTRT		0xec
#define	DIWVSTRT_HPOS_MASK	0x000003ff

#define	PVRREG_DIWVSTRT		0xf0
#define	DIWVSTRT_V1_MASK	0x000003ff
#define	DIWVSTRT_V2_MASK	0x03ff0000
#define	DIWVSTRT_V2_SHIFT	16
#define	DIWVSTRT_V1(x)		((x) << 0)
#define	DIWVSTRT_V2(x)		((x) << 16)

#define	PVRREG_SCALER_CFG	0xf4

#define	PVRREG_PALETTE_CFG	0x108
#define	PVR_PALETTE_CFG_MODE_MASK	0x3
#define	PVR_PALETTE_CFG_MODE_ARGB1555	 0x0
#define	PVR_PALETTE_CFG_MODE_RGB565	 0x1
#define	PVR_PALETTE_CFG_MODE_ARGB4444	 0x2
#define	PVR_PALETTE_CFG_MODE_ARGB8888	 0x3

#define	PVRREG_SYNC_STAT	0x10c
#define	PVR_SYNC_STAT_VPOS_MASK			0x000003ff
#define	PVR_SYNC_STAT_INTERLACE_FIELD_EVEN	0x00000400
#define	PVR_SYNC_STAT_HBLANK			0x00001000
#define	PVR_SYNC_STAT_VBLANK			0x00002000

#define	PVRREG_MAGIC_110	0x110
#define	  MAGIC_110_VALUE	  0x93f39

#define	PVRREG_TA_LUMINANCE	0x118	/*  todo  */

#define	PVRREG_TA_OPB_START	0x124
#define	TA_OPB_START_MASK	0x00ffff80

#define	PVRREG_TA_OB_START	0x128
#define	TA_OB_START_MASK	0x00fffff8

#define	PVRREG_TA_OPB_END	0x12c
#define	TA_OPB_END_MASK		0x00ffff80

#define	PVRREG_TA_OB_END	0x130
#define	TA_OB_END_MASK		0x00fffff8

#define	PVRREG_TA_OPB_POS	0x134
#define	TA_OPB_POS_MASK		0x00ffff80

#define	PVRREG_TA_OB_POS	0x138
#define	TA_OB_POS_MASK		0x00fffff8

#define	PVRREG_TILEBUF_SIZE	0x13c
#define	TILEBUF_SIZE_HEIGHT_MASK  0xffff0000
#define	TILEBUF_SIZE_HEIGHT_SHIFT 16
#define	TILEBUF_SIZE_WIDTH_MASK   0x0000ffff

#define	PVRREG_TA_OPB_CFG	0x140
#define	TA_OPB_CFG_OPAQUEPOLY_MASK	0x00000003
#define	TA_OPB_CFG_OPAQUEMOD_MASK	0x00000030
#define	TA_OPB_CFG_OPAQUEMOD_SHIFT	         4
#define	TA_OPB_CFG_TRANSPOLY_MASK	0x00000300
#define	TA_OPB_CFG_TRANSPOLY_SHIFT	         8
#define	TA_OPB_CFG_TRANSMOD_MASK	0x00003000
#define	TA_OPB_CFG_TRANSMOD_SHIFT	        12
#define	TA_OPB_CFG_PUNCHTHROUGH_MASK	0x00030000
#define	TA_OPB_CFG_PUNCHTHROUGH_SHIFT	        16
#define	TA_OPB_CFG_OPBDIR		0x00100000

#define	PVRREG_TA_INIT		0x144
#define	PVR_TA_INIT		0x80000000

#define	PVRREG_YUV_ADDR		0x148
#define	PVR_YUV_ADDR_MASK	0x00ffffe0

#define	PVRREG_YUV_CFG1		0x14c
/*  TODO  */

#define	PVRREG_YUV_STAT		0x150
/*  Nr of currently converted 16x16 macro blocks.  */
#define	PVR_YUV_STAT_BLOCKS_MASK 0x1fff

#define	PVRREG_TA_OPL_REINIT	0x160
#define	PVR_TA_OPL_REINIT	0x80000000

#define	PVRREG_TA_OPL_INIT	0x164
/*  Start of Object Pointer List allocation in VRAM.  */
#define	PVR_TA_OPL_INIT_MASK	0x00ffff80

#define	PVRREG_FOG_TABLE	0x0200
#define	PVR_FOG_TABLE_SIZE	0x0200

#define	PVRREG_PALETTE		0x1000
#define	PVR_PALETTE_SIZE	0x1000

#endif	/*  DREAMCAST_PVR_H  */

