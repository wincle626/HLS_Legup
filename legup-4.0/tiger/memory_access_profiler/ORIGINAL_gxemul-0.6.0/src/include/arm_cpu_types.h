#ifndef	ARM_CPU_TYPES_H
#define	ARM_CPU_TYPES_H

/*
 *  Copyright (C) 2005-2010  Anders Gavare.  All rights reserved.
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
 */

/*  See cpu_arm.h for struct arm_cpu_type_def.  */
/*  See armreg.h for CPU_ID_xxx defines.  */

/*  TODO: Refactor these flags  */

/*  TODO: Include "ARM level", i.e. ARMv5  */


/*  Flags:  */
#define	ARM_NO_MMU		1
#define	ARM_DUAL_ENDIAN		2
#define	ARM_XSCALE		4

#include "thirdparty/armreg.h"

/*
 *  NOTE: Most of these are bogus!
 */

#define	ARM_CPU_TYPE_DEFS					      {	 \
	{ "ARM3",	CPU_ID_ARM3,	ARM_DUAL_ENDIAN,12, 1,  0, 1 }, \
	{ "ARM610",	CPU_ID_ARM600,	ARM_DUAL_ENDIAN,12, 1,  0, 1 }, \
	{ "ARM610",	CPU_ID_ARM610,	ARM_DUAL_ENDIAN,12, 1,  0, 1 }, \
	{ "ARM620",	CPU_ID_ARM620,	ARM_DUAL_ENDIAN,12, 1,  0, 1 }, \
	\
	{ "ARM700",	CPU_ID_ARM700,	0,		12, 1,  0, 1 }, \
	{ "ARM710",	CPU_ID_ARM710,	0,		12, 1,  0, 1 }, \
	{ "ARM710A",	CPU_ID_ARM710A,	0,		12, 1,  0, 1 }, \
	{ "ARM720T",	CPU_ID_ARM720T,	0,		12, 1,  0, 1 }, \
	{ "ARM740T4K",	CPU_ID_ARM740T4K,ARM_NO_MMU,	12, 1,  0, 1 }, \
	{ "ARM740T8K",	CPU_ID_ARM740T8K,ARM_NO_MMU,	13, 1,  0, 1 }, \
	{ "ARM7500",	CPU_ID_ARM7500,	0,		12, 1,  0, 1 }, \
	{ "ARM7500FE",	CPU_ID_ARM7500FE,0,		12, 1,  0, 1 }, \
	\
	{ "ARM810",	CPU_ID_ARM810,	0,		12, 1,  0, 1 }, \
	{ "ARM920T",	CPU_ID_ARM920T,	0,		14, 1, 14, 1 }, \
	{ "ARM922T",	CPU_ID_ARM922T,	0,		12, 1,  0, 1 }, \
	{ "ARM940T",	CPU_ID_ARM940T,	ARM_NO_MMU,	12, 1,  0, 1 }, \
	\
	{ "ARM946ES",	CPU_ID_ARM946ES,ARM_NO_MMU,	12, 1,  0, 1 }, \
	{ "ARM966ES",	CPU_ID_ARM966ES,ARM_NO_MMU,	12, 1,  0, 1 }, \
	{ "ARM966ESR1",	CPU_ID_ARM966ESR1,ARM_NO_MMU,	12, 1,  0, 1 }, \
	\
	{ "ARM1020E",	CPU_ID_ARM1020E,0,		12, 1,  0, 1 }, \
	{ "ARM1022ES",	CPU_ID_ARM1022ES,0,		12, 1,  0, 1 }, \
	{ "ARM1026EJS",	CPU_ID_ARM1026EJS,0,		12, 1,  0, 1 }, \
	{ "ARM1136JS",	CPU_ID_ARM1136JS,0,		12, 1,  0, 1 }, \
	{ "ARM1136JSR1",CPU_ID_ARM1136JSR1,0,		12, 1,  0, 1 }, \
	\
	{ "SA110",	CPU_ID_SA110 | 3, 0,		14, 1, 14, 1 }, \
	{ "SA1100",	CPU_ID_SA1100,	0,		14, 1, 14, 1 }, \
	{ "SA1110",	CPU_ID_SA1110,	0,		14, 1, 14, 1 }, \
	\
	{ "TI925T",	CPU_ID_TI925T,	0,		14, 1, 14, 1 }, \
	{ "IXP1200",	CPU_ID_IXP1200,	0,		14, 1, 14, 1 }, \
	{ "80200",	CPU_ID_80200,	0,		14, 1, 14, 1 }, \
	\
	{ "PXA210",	CPU_ID_PXA210,	ARM_XSCALE,	16, 1,  0, 1 }, \
	{ "PXA210A",	CPU_ID_PXA210A,	ARM_XSCALE,	16, 1,  0, 1 }, \
	{ "PXA210B",	CPU_ID_PXA210B,	ARM_XSCALE,	16, 1,  0, 1 }, \
	{ "PXA210C",	CPU_ID_PXA210C,	ARM_XSCALE,	16, 1,  0, 1 }, \
	{ "PXA250",	CPU_ID_PXA250,	ARM_XSCALE,	16, 1,  0, 1 }, \
	{ "PXA250A",	CPU_ID_PXA250A,	ARM_XSCALE,	16, 1,  0, 1 }, \
	{ "PXA250B",	CPU_ID_PXA250B,	ARM_XSCALE,	16, 1,  0, 1 }, \
	{ "PXA250C",	CPU_ID_PXA250C,	ARM_XSCALE,	16, 1,  0, 1 }, \
	{ "PXA27X",	CPU_ID_PXA27X,	ARM_XSCALE,	16, 1,  0, 1 }, \
	\
	{ "IXP425_255",	CPU_ID_IXP425_266,ARM_XSCALE,	15, 1, 15, 1 }, \
	{ "IXP425_400",	CPU_ID_IXP425_400,ARM_XSCALE,	15, 1, 15, 1 }, \
	{ "IXP425_533",	CPU_ID_IXP425_533,ARM_XSCALE,	15, 1, 15, 1 }, \
	\
	{ "80219_400",	CPU_ID_80219_400,ARM_XSCALE,	15, 1, 15, 1 }, \
	{ "80219_600",	CPU_ID_80219_600,ARM_XSCALE,	15, 1, 15, 1 }, \
	{ "80321_400",	CPU_ID_80321_400,ARM_XSCALE,	15, 1, 15, 1 }, \
	{ "80321_400_B0",CPU_ID_80321_400_B0,ARM_XSCALE,15, 1, 15, 1 }, \
	{ "80321_600",	CPU_ID_80321_600,ARM_XSCALE,	15, 1, 15, 1 }, \
	{ "80321_600_B0",CPU_ID_80321_600_B0,ARM_XSCALE,15, 1, 15, 1 }, \
	{ "80321_600_2",CPU_ID_80321_600_2,ARM_XSCALE,15, 1, 15, 1 }, \
	\
	{ NULL, 0, 0, 0,0, 0,0 } }

#endif	/*  ARM_CPU_TYPES_H  */
