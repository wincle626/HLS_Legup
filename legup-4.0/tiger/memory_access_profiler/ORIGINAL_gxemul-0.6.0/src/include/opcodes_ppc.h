#ifndef	OPCODES_PPC_H
#define	OPCODES_PPC_H

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
 *
 *
 *  POWER and PowerPC opcodes. These are gathered from various sources.
 *
 *  Note: The define uses the PowerPC name, not the POWER name, when they
 *  differ.
 */

#define	PPC_HI6_MULLI		0x07
#define	PPC_HI6_SUBFIC		0x08

#define	PPC_HI6_CMPLI		0x0a
#define	PPC_HI6_CMPI		0x0b
#define	PPC_HI6_ADDIC		0x0c
#define	PPC_HI6_ADDIC_DOT	0x0d
#define	PPC_HI6_ADDI		0x0e
#define	PPC_HI6_ADDIS		0x0f
#define	PPC_HI6_BC		0x10
#define	PPC_HI6_SC		0x11
#define	PPC_HI6_B		0x12
#define	PPC_HI6_19		0x13
#define	  PPC_19_MCRF		  0
#define	  PPC_19_BCLR		  16
#define	  PPC_19_RFID		  18
#define	  PPC_19_CRNOR		  33
#define	  PPC_19_RFI		  50
#define	  PPC_19_RFSVC		  82
#define	  PPC_19_CRANDC		  129
#define	  PPC_19_ISYNC		  150
#define	  PPC_19_CRXOR		  193
#define	  PPC_19_CRNAND		  225
#define	  PPC_19_CRAND		  257
#define	  PPC_19_CREQV		  289
#define	  PPC_19_CRORC		  417
#define	  PPC_19_CROR		  449
#define	  PPC_19_BCCTR		  528
#define	PPC_HI6_RLWIMI		0x14
#define	PPC_HI6_RLWINM		0x15

#define	PPC_HI6_RLWNM		0x17
#define	PPC_HI6_ORI		0x18
#define	PPC_HI6_ORIS		0x19
#define	PPC_HI6_XORI		0x1a
#define	PPC_HI6_XORIS		0x1b
#define	PPC_HI6_ANDI_DOT	0x1c
#define	PPC_HI6_ANDIS_DOT	0x1d
#define	PPC_HI6_30		0x1e
#define	  PPC_30_RLDICL		  0x0
#define	  PPC_30_RLDICR		  0x1
#define	  PPC_30_RLDIMI		  0x3
#define	PPC_HI6_31		0x1f
#define	  PPC_31_CMP		  0
#define	  PPC_31_TW		  4
#define	  PPC_31_SUBFC		  8
#define	  PPC_31_ADDC		  10
#define	  PPC_31_MULHWU		  11
#define	  PPC_31_MFCR		  19
#define	  PPC_31_LWARX		  20
#define	  PPC_31_LWZX		  23
#define	  PPC_31_SLW		  24
#define	  PPC_31_CNTLZW		  26
#define	  PPC_31_SLD		  27
#define	  PPC_31_AND		  28
#define	  PPC_31_CMPL		  32
#define	  PPC_31_SUBF		  40
#define	  PPC_31_DCBST		  54
#define	  PPC_31_LWZUX		  55
#define	  PPC_31_ANDC		  60
#define	  PPC_31_TD		  68
#define	  PPC_31_MULHW		  75
#define	  PPC_31_MFMSR		  83
#define	  PPC_31_LDARX		  84
#define	  PPC_31_DCBF		  86
#define	  PPC_31_LBZX		  87
#define	  PPC_31_LVX		  103
#define	  PPC_31_NEG		  104
#define	  PPC_31_CLF		  118
#define	  PPC_31_LBZUX		  119
#define	  PPC_31_NOR		  124
#define	  PPC_31_SUBFE		  136
#define	  PPC_31_ADDE		  138
#define	  PPC_31_MTCRF		  144
#define	  PPC_31_MTMSR		  146
#define	  PPC_31_STDX		  149
#define	  PPC_31_STWCX_DOT	  150
#define	  PPC_31_STWX		  151
#define	  PPC_31_WRTEEI		  163
#define	  PPC_31_MTMSRD		  178
#define	  PPC_31_STDUX		  181
#define	  PPC_31_STWUX		  183
#define	  PPC_31_SUBFZE		  200
#define	  PPC_31_ADDZE		  202
#define	  PPC_31_MTSR		  210
#define	  PPC_31_STDCX_DOT	  214
#define	  PPC_31_STBX		  215
#define	  PPC_31_STVX		  231
#define	  PPC_31_SUBFME		  232
#define	  PPC_31_ADDME		  234
#define	  PPC_31_MULLW		  235
#define	  PPC_31_MTSRIN		  242
#define	  PPC_31_DCBTST		  246
#define	  PPC_31_STBUX		  247
#define	  PPC_31_ADD		  266
#define	  PPC_31_DCBT		  278
#define	  PPC_31_LHZX		  279
#define	  PPC_31_EQV		  284
#define	  PPC_31_TLBIE		  306
#define	  PPC_31_LHZUX		  311
#define	  PPC_31_XOR		  316
#define	  PPC_31_MFSPR		  339
#define	  PPC_31_LHAX		  343
#define	  PPC_31_LVXL		  359
#define	  PPC_31_TLBIA		  370
#define	  PPC_31_MFTB		  371
#define	  PPC_31_LHAUX		  375
#define	  PPC_31_STHX		  407
#define	  PPC_31_ORC		  412
#define	  PPC_31_SLBIE		  434
#define	  PPC_31_STHUX		  439
#define	  PPC_31_OR		  444
#define	  PPC_31_DCCCI		  454
#define	  PPC_31_DIVWU		  459
#define	  PPC_31_MTSPR		  467
#define	  PPC_31_DCBI		  470
#define	  PPC_31_NAND		  476
#define	  PPC_31_STVXL		  487
#define	  PPC_31_DIVW		  491
#define	  PPC_31_SLBIA		  498
#define	  PPC_31_CLI		  502
#define	  PPC_31_SUBFCO		  520
#define	  PPC_31_ADDCO		  522
#define	  PPC_31_LWBRX		  534
#define	  PPC_31_LFSX		  535
#define	  PPC_31_SRW		  536
#define	  PPC_31_SUBFO		  552
#define	  PPC_31_TLBSYNC	  566
#define	  PPC_31_MFSR		  595
#define	  PPC_31_LSWI		  597
#define	  PPC_31_SYNC		  598
#define	  PPC_31_LFDX		  599
#define	  PPC_31_NEGO		  616
#define	  PPC_31_DCLST		  630
#define	  PPC_31_SUBFEO		  648
#define	  PPC_31_ADDEO		  650
#define	  PPC_31_MFSRIN		  659
#define	  PPC_31_STWBRX		  662
#define	  PPC_31_STFSX		  663
#define	  PPC_31_SUBFZEO	  712
#define	  PPC_31_ADDZEO		  714
#define	  PPC_31_STSWI		  725
#define	  PPC_31_STFDX		  727
#define	  PPC_31_SUBFMEO	  744
#define	  PPC_31_ADDMEO		  746
#define	  PPC_31_MULLWO		  747
#define	  PPC_31_ADDO		  778
#define	  PPC_31_LHBRX		  790
#define	  PPC_31_SRAW		  792
#define	  PPC_31_DSSALL		  822
#define	  PPC_31_SRAWI		  824
#define	  PPC_31_EIEIO		  854
#define	  PPC_31_TLBSX_DOT	  914
#define	  PPC_31_STHBRX		  918
#define	  PPC_31_EXTSH		  922
#define	  PPC_31_EXTSB		  954
#define	  PPC_31_ICCCI		  966
#define	  PPC_31_DIVWUO		  971
#define	  PPC_31_TLBLD		  978
#define	  PPC_31_ICBI		  982
#define	  PPC_31_EXTSW		  986
#define	  PPC_31_DIVWO		  1003
#define	  PPC_31_TLBLI		  1010
#define	  PPC_31_DCBZ		  1014
#define	PPC_HI6_LWZ		0x20
#define	PPC_HI6_LWZU		0x21
#define	PPC_HI6_LBZ		0x22
#define	PPC_HI6_LBZU		0x23
#define	PPC_HI6_STW		0x24
#define	PPC_HI6_STWU		0x25
#define	PPC_HI6_STB		0x26
#define	PPC_HI6_STBU		0x27
#define	PPC_HI6_LHZ		0x28
#define	PPC_HI6_LHZU		0x29
#define	PPC_HI6_LHA		0x2a
#define	PPC_HI6_LHAU		0x2b
#define	PPC_HI6_STH		0x2c
#define	PPC_HI6_STHU		0x2d
#define	PPC_HI6_LMW		0x2e
#define	PPC_HI6_STMW		0x2f
#define	PPC_HI6_LFS		0x30

#define	PPC_HI6_LFD		0x32

#define	PPC_HI6_STFS		0x34

#define	PPC_HI6_STFD		0x36

#define	PPC_HI6_LD		0x3a
#define	PPC_HI6_59		0x3b
#define	  PPC_59_FDIVS		  18
#define	  PPC_59_FSUBS		  20
#define	  PPC_59_FADDS		  21
#define	  PPC_59_FMULS		  25
#define	  PPC_59_FMADDS		  29

#define	PPC_HI6_STD		0x3e
#define	PPC_HI6_63		0x3f
#define	  PPC_63_FCMPU		  0
#define	  PPC_63_FRSP		  12
#define	  PPC_63_FCTIWZ		  15
#define	  PPC_63_FDIV		  18
#define	  PPC_63_FSUB		  20
#define	  PPC_63_FADD		  21
#define	  PPC_63_FMUL		  25
#define	  PPC_63_FMSUB		  28
#define	  PPC_63_FMADD		  29
#define	  PPC_63_FNEG		  40
#define	  PPC_63_FMR		  72
#define	  PPC_63_FNABS		  136
#define	  PPC_63_FABS		  264
#define	  PPC_63_MFFS		  583
#define	  PPC_63_MTFSF		  711

#endif	/*  OPCODES_PPC_H  */
