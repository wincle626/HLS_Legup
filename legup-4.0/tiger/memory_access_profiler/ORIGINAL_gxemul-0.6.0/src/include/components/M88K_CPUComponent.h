#ifndef M88K_CPUCOMPONENT_H
#define	M88K_CPUCOMPONENT_H

/*
 *  Copyright (C) 2009-2010  Anders Gavare.  All rights reserved.
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

// COMPONENT(m88k_cpu)


#include "CPUDyntransComponent.h"

#include "thirdparty/m88k_psl.h"


/*  M88K CPU types:  */
struct m88k_cpu_type_def {
	const char	*name;
	int		type;
	uint32_t	pid;
};

#define M88K_PID(arn,vn) ((arn << M88K_ARN_SHIFT) | (vn << M88K_VN_SHIFT))

#define	M88K_CPU_TYPE_DEFS				{	\
	{ "88100", 88100, M88K_PID(M88K_ARN_88100,3) },		\
	{ "88110", 88110, M88K_PID(M88K_ARN_88110,0) },		\
	{ NULL,        0, 0			     }		\
	}

/*
 * Opcode names:
 */
#define	M88K_OPCODE_NAMES {						\
	"xmem.bu",  "xmem",     "ld.hu",    "ld.bu",			\
	"ld.d",     "ld",       "ld.h",     "ld.b",			\
	"st.d",     "st",       "st.h",     "st.b",			\
	"opcode0c", "opcode0d", "opcode0e", "opcode0f",			\
	"and",      "and.u",    "mask",     "mask.u",			\
	"xor",      "xor.u",    "or",       "or.u",			\
	"addu",     "subu",     "divu",     "mulu",			\
	"add",      "sub",      "div",      "cmp",			\
	"c(multi)", "f(multi)", "opcode22", "opcode23",			\
	"opcode24", "opcode25", "opcode26", "opcode27",			\
	"opcode28", "opcode29", "opcode2a", "opcode2b",			\
	"opcode2c", "opcode2d", "opcode2e", "opcode2f",			\
	"br",       "br.n",     "bsr",      "bsr.n",			\
	"bb0",      "bb0.n",    "bb1",      "bb1.n",			\
	"opcode38", "opcode39", "bcnd",     "bcnd.n",			\
	"opcode3c", "3(multi)", "tbnd",     "opcode3f" }

#define	M88K_3C_OPCODE_NAMES {						\
	"opcode3c_00", "opcode3c_01", "opcode3c_02", "opcode3c_03",	\
	"opcode3c_04", "opcode3c_05", "opcode3c_06", "opcode3c_07",	\
	"opcode3c_08", "opcode3c_09", "opcode3c_0a", "opcode3c_0b",	\
	"opcode3c_0c", "opcode3c_0d", "opcode3c_0e", "opcode3c_0f",	\
	"opcode3c_10", "opcode3c_11", "opcode3c_12", "opcode3c_13",	\
	"opcode3c_14", "opcode3c_15", "opcode3c_16", "opcode3c_17",	\
	"opcode3c_18", "opcode3c_19", "opcode3c_1a", "opcode3c_1b",	\
	"opcode3c_1c", "opcode3c_1d", "opcode3c_1e", "opcode3c_1f",	\
	"clr",         "opcode3c_21", "set",         "opcode3c_23",	\
	"ext",         "opcode3c_25", "extu",        "opcode3c_27",	\
	"mak",         "opcode3c_29", "rot",         "opcode3c_2b",	\
	"opcode3c_2c", "opcode3c_2d", "opcode3c_2e", "opcode3c_2f",	\
	"opcode3c_30", "opcode3c_31", "opcode3c_32", "opcode3c_33",	\
	"tb0",         "opcode3c_35", "tb1",         "opcode3c_37",	\
	"opcode3c_38", "opcode3c_39", "opcode3c_3a", "opcode3c_3b",	\
	"opcode3c_3c", "opcode3c_3d", "opcode3c_3e", "opcode3c_3f" }

#define	M88K_3D_OPCODE_NAMES {						\
	"opcode3d_00", "opcode3d_01", "opcode3d_02", "opcode3d_03",	\
	"opcode3d_04", "opcode3d_05", "opcode3d_06", "opcode3d_07",	\
	"opcode3d_08", "opcode3d_09", "opcode3d_0a", "opcode3d_0b",	\
	"opcode3d_0c", "opcode3d_0d", "opcode3d_0e", "opcode3d_0f",	\
	"opcode3d_10", "opcode3d_11", "opcode3d_12", "opcode3d_13",	\
	"opcode3d_14", "opcode3d_15", "opcode3d_16", "opcode3d_17",	\
	"opcode3d_18", "opcode3d_19", "opcode3d_1a", "opcode3d_1b",	\
	"opcode3d_1c", "opcode3d_1d", "opcode3d_1e", "opcode3d_1f",	\
	"opcode3d_20", "opcode3d_21", "opcode3d_22", "opcode3d_23",	\
	"opcode3d_24", "opcode3d_25", "opcode3d_26", "opcode3d_27",	\
	"opcode3d_28", "opcode3d_29", "opcode3d_2a", "opcode3d_2b",	\
	"opcode3d_2c", "opcode3d_2d", "opcode3d_2e", "opcode3d_2f",	\
	"opcode3d_30", "opcode3d_31", "opcode3d_32", "opcode3d_33",	\
	"opcode3d_34", "opcode3d_35", "opcode3d_36", "opcode3d_37",	\
	"opcode3d_38", "opcode3d_39", "opcode3d_3a", "opcode3d_3b",	\
	"opcode3d_3c", "opcode3d_3d", "opcode3d_3e", "opcode3d_3f",	\
									\
	"and",         "opcode3d_41", "opcode3d_42", "opcode3d_43",	\
	"and.c",       "opcode3d_45", "opcode3d_46", "opcode3d_47",	\
	"opcode3d_48", "opcode3d_49", "opcode3d_4a", "opcode3d_4b",	\
	"opcode3d_4c", "opcode3d_4d", "opcode3d_4e", "opcode3d_4f",	\
	"xor",         "opcode3d_51", "opcode3d_52", "opcode3d_53",	\
	"xor.c",       "opcode3d_55", "opcode3d_56", "opcode3d_57",	\
	"or",          "opcode3d_59", "opcode3d_5a", "opcode3d_5b",	\
	"or.c",        "opcode3d_5d", "opcode3d_5e", "opcode3d_5f",	\
	"addu",        "addu.co",     "addu.ci",     "addu.cio",	\
	"subu",        "subu.co",     "subu.ci",     "subu.cio",	\
	"divu",        "divu.d",      "opcode3d_6a", "opcode3d_6b",	\
	"mul",         "mulu.d",      "muls",        "opcode3d_6f",	\
	"add",         "add.co",      "add.ci",      "add.cio", 	\
	"sub",         "sub.co",      "sub.ci",      "sub.cio",		\
	"div",         "opcode3d_79", "opcode3d_7a", "opcode3d_7b",	\
	"cmp",         "opcode3d_7d", "opcode3d_7e", "opcode3d_7f",	\
									\
	"clr",         "opcode3d_81", "opcode3d_82", "opcode3d_83",	\
	"opcode3d_84", "opcode3d_85", "opcode3d_86", "opcode3d_87",	\
	"set",         "opcode3d_89", "opcode3d_8a", "opcode3d_8b",	\
	"opcode3d_8c", "opcode3d_8d", "opcode3d_8e", "opcode3d_8f",	\
	"ext",         "opcode3d_91", "opcode3d_92", "opcode3d_93",	\
	"opcode3d_94", "opcode3d_95", "opcode3d_96", "opcode3d_97",	\
	"extu",        "opcode3d_99", "opcode3d_9a", "opcode3d_9b",	\
	"opcode3d_9c", "opcode3d_9d", "opcode3d_9e", "opcode3d_9f",	\
	"mak",         "opcode3d_a1", "opcode3d_a2", "opcode3d_a3",	\
	"opcode3d_a4", "opcode3d_a5", "opcode3d_a6", "opcode3d_a7",	\
	"rot",         "opcode3d_a9", "opcode3d_aa", "opcode3d_ab",	\
	"opcode3d_ac", "opcode3d_ad", "opcode3d_ae", "opcode3d_af",	\
	"opcode3d_b0", "opcode3d_b1", "opcode3d_b2", "opcode3d_b3",	\
	"opcode3d_b4", "opcode3d_b5", "opcode3d_b6", "opcode3d_b7",	\
	"opcode3d_b8", "opcode3d_b9", "opcode3d_ba", "opcode3d_bb",	\
	"opcode3d_bc", "opcode3d_bd", "opcode3d_be", "opcode3d_bf",	\
									\
	"jmp",         "opcode3d_c1", "opcode3d_c2", "opcode3d_c3",	\
	"jmp.n",       "opcode3d_c5", "opcode3d_c6", "opcode3d_c7",	\
	"jsr",         "opcode3d_c9", "opcode3d_ca", "opcode3d_cb",	\
	"jsr.n",       "opcode3d_cd", "opcode3d_ce", "opcode3d_cf",	\
	"opcode3d_d0", "opcode3d_d1", "opcode3d_d2", "opcode3d_d3",	\
	"opcode3d_d4", "opcode3d_d5", "opcode3d_d6", "opcode3d_d7",	\
	"opcode3d_d8", "opcode3d_d9", "opcode3d_da", "opcode3d_db",	\
	"opcode3d_dc", "opcode3d_dd", "opcode3d_de", "opcode3d_df",	\
	"opcode3d_e0", "opcode3d_e1", "opcode3d_e2", "opcode3d_e3",	\
	"opcode3d_e4", "opcode3d_e5", "opcode3d_e6", "opcode3d_e7",	\
	"ff1",         "opcode3d_e9", "opcode3d_ea", "opcode3d_eb",	\
	"ff0",         "opcode3d_ed", "opcode3d_ee", "opcode3d_ef",	\
	"opcode3d_f0", "opcode3d_f1", "opcode3d_f2", "opcode3d_f3",	\
	"opcode3d_f4", "opcode3d_f5", "opcode3d_f6", "opcode3d_f7",	\
	"tbnd",        "opcode3d_f9", "opcode3d_fa", "opcode3d_fb",	\
	"opcode3d_fc", "opcode3d_fd", "opcode3d_fe", "opcode3d_ff" }

/*  Control register names:  */
#define	N_M88K_CONTROL_REGS	64
#define	M88K_CR_NAMES	{						\
	"PID",  "PSR",  "EPSR", "SSBR",		/*   0 ..  3  */	\
	"SXIP", "SNIP", "SFIP", "VBR",		/*   4 ..  7  */	\
	"DMT0", "DMD0", "DMA0", "DMT1",		/*   8 .. 11  */	\
	"DMD1", "DMA1", "DMT2", "DMD2",		/*  12 .. 15  */	\
	"DMA2", "SR0",  "SR1",  "SR2",		/*  16 .. 19  */	\
	"SR3",  "CR21", "CR22", "CR23",		/*  20 .. 23  */	\
	"CR24", "CR25", "CR26", "CR27",		/*  24 .. 27  */	\
	"CR28", "CR29", "CR30", "CR31",		/*  28 .. 31  */	\
	"CR32", "CR33", "CR34", "CR35",		/*  32 .. 35  */	\
	"CR36", "CR37", "CR38", "CR39",		/*  36 .. 39  */	\
	"CR40", "CR41", "CR42", "CR43",		/*  40 .. 43  */	\
	"CR44", "CR45", "CR46", "CR47",		/*  44 .. 47  */	\
	"CR48", "CR49", "CR50", "CR51",		/*  48 .. 51  */	\
	"CR52", "CR53", "CR54", "CR55",		/*  52 .. 55  */	\
	"CR56", "CR57", "CR58", "CR59",		/*  56 .. 59  */	\
	"CR60", "CR61", "CR62", "CR63"		/*  60 .. 63  */	}

#define M88K_CR_PID	0
#define M88K_CR_PSR     1 
#define M88K_CR_EPSR    2 
#define M88K_CR_SSBR    3 
#define M88K_CR_SXIP    4 
#define M88K_CR_SNIP    5 
#define M88K_CR_SFIP    6 
#define M88K_CR_VBR     7 
#define M88K_CR_DMT0    8 
#define M88K_CR_DMD0    9 
#define M88K_CR_DMA0    10
#define M88K_CR_DMT1    11
#define M88K_CR_DMD1    12
#define M88K_CR_DMA1    13
#define M88K_CR_DMT2    14
#define M88K_CR_DMD2    15
#define M88K_CR_DMA2    16
#define M88K_CR_SR0     17
#define M88K_CR_SR1     18
#define M88K_CR_SR2     19
#define M88K_CR_SR3     20

/*  MVME197 extended control registers:  */
#define	M88K_CR_NAMES_197	{					\
	"PID",  "PSR",  "EPSR", "SSBR",		/*   0 ..  3  */	\
	"EXIP", "ENIP", "SFIP", "VBR",		/*   4 ..  7  */	\
	"DMT0", "DMD0", "DMA0", "DMT1",		/*   8 .. 11  */	\
	"DMD1", "DMA1", "DMT2", "DMD2",		/*  12 .. 15  */	\
	"SRX", "SR0",  "SR1",  "SR2",		/*  16 .. 19  */	\
	"SR3",  "CR21", "CR22", "CR23",		/*  20 .. 23  */	\
	"CR24", "ICMD", "ICTL", "ISAR",		/*  24 .. 27  */	\
	"ISAP", "IUAP", "IIR",  "IBP",		/*  28 .. 31  */	\
	"IPPU", "IPPL", "ISR",  "ILAR",		/*  32 .. 35  */	\
	"IPAR", "CR37", "CR38", "CR39",		/*  36 .. 39  */	\
	"DCMD", "DCTL", "DSAR", "DSAP",		/*  40 .. 43  */	\
	"DUAP", "DIR",  "DBP",  "DPPU",		/*  44 .. 47  */	\
	"DPPL", "DSR",  "DLAR", "DPAR",		/*  48 .. 51  */	\
	"CR52", "CR53", "CR54", "CR55",		/*  52 .. 55  */	\
	"CR56", "CR57", "CR58", "CR59",		/*  56 .. 59  */	\
	"CR60", "CR61", "CR62", "CR63"		/*  60 .. 63  */	}

#define M88K_CR_EXIP    4
#define M88K_CR_ENIP    5
#define M88K_CR_SRX     16
#define M88K_CR_ICMD    25
#define M88K_CR_ICTL    26
#define M88K_CR_ISAR    27
#define M88K_CR_ISAP    28
#define M88K_CR_IUAP    29
#define M88K_CR_IIR     30
#define M88K_CR_IBP     31
#define M88K_CR_IPPU    32
#define M88K_CR_IPPL    33
#define M88K_CR_ISR     34
#define M88K_CR_ILAR    35
#define M88K_CR_IPAR    36
#define M88K_CR_DCMD    40
#define M88K_CR_DCTL    41
#define M88K_CR_DSAR    42
#define M88K_CR_DSAP    43
#define M88K_CR_DUAP    44
#define M88K_CR_DIR     45
#define M88K_CR_DBP     46
#define M88K_CR_DPPU    47
#define M88K_CR_DPPL    48
#define M88K_CR_DSR     49
#define M88K_CR_DLAR    50
#define M88K_CR_DPAR    51


#define	N_M88K_FPU_CONTROL_REGS		64

#define M88K_FPCR_FPECR		0
#define	M88K_FPECR_FDVZ			(1 << 3)
#define	M88K_FPECR_FUNIMP		(1 << 6)
/*  ... TODO: more  */

// Dyntrans:
#define	M88K_INSTR_ALIGNMENT_SHIFT	2
#define	M88K_IC_ENTRIES_PER_PAGE	1024	// always 4 KB pages

// M88K registers:
#define	N_M88K_REGS		32
#define	M88K_ZERO_REG		0	// r0: always zero
#define	M88K_RETURN_REG		1	// r1: the return address on function calls
#define	M88K_RETURN_VALUE_REG	2	// r2: the return value, from function calls
#define	M88K_FIRST_ARG_REG	2	// r2..r9: eight standard arguments to functions
#define	M88K_STACKPOINTER_REG	31	// r31: commonly used as stack pointer

#define	M88K_CMP_HS	0x00000800
#define	M88K_CMP_LO	0x00000400
#define	M88K_CMP_LS	0x00000200
#define	M88K_CMP_HI	0x00000100
#define	M88K_CMP_GE	0x00000080
#define	M88K_CMP_LT	0x00000040
#define	M88K_CMP_LE	0x00000020
#define	M88K_CMP_GT	0x00000010
#define	M88K_CMP_NE	0x00000008
#define	M88K_CMP_EQ	0x00000004

/*  Exception numbers:  */
#define	M88K_EXCEPTION_RESET				0
#define	M88K_EXCEPTION_INTERRUPT			1
#define	M88K_EXCEPTION_INSTRUCTION_ACCESS		2
#define	M88K_EXCEPTION_DATA_ACCESS			3
#define	M88K_EXCEPTION_MISALIGNED_ACCESS		4
#define	M88K_EXCEPTION_UNIMPLEMENTED_OPCODE		5
#define	M88K_EXCEPTION_PRIVILEGE_VIOLATION		6
#define	M88K_EXCEPTION_BOUNDS_CHECK_VIOLATION		7
#define	M88K_EXCEPTION_ILLEGAL_INTEGER_DIVIDE		8
#define	M88K_EXCEPTION_INTEGER_OVERFLOW			9
#define	M88K_EXCEPTION_ERROR				10
#define	M88K_EXCEPTION_SFU1_PRECISE			114
#define	M88K_EXCEPTION_SFU1_IMPRECISE			115
#define	M88K_EXCEPTION_USER_TRAPS_START			128


/*
 *  GXemul-specific instructions:
 */

/*  A reserved/unimplemented instruction, used for PROM calls:  */
#define	M88K_PROM_INSTR			0xf400fc92

/*  An instruction which aborts before doing anything:  */
#define	M88K_FAIL_EARLY_INSTR		0xf400fc93

/*  An instruction which aborts after increasing r1:  */
#define	M88K_FAIL_LATE_INSTR		0xf400fc94


/*
 *  M88200/88204 CMMU:
 */

#define	MAX_M8820X_CMMUS		8
#define	M8820X_LENGTH			0x1000
#define	N_M88200_BATC_REGS		10
#define	N_M88200_PATC_ENTRIES		56
#define	M8820X_PATC_SUPERVISOR_BIT	0x00000001

struct m8820x_cmmu {
	uint32_t	reg[M8820X_LENGTH / sizeof(uint32_t)];
	uint32_t	batc[N_M88200_BATC_REGS];
	uint32_t	patc_v_and_control[N_M88200_PATC_ENTRIES];
	uint32_t	patc_p_and_supervisorbit[N_M88200_PATC_ENTRIES];
	int		patc_update_index;
};



/***********************************************************************/

/**
 * \brief A Component representing a Motorola 88000 processor.
 *
 * The only two implementations there were of the 88K architecture were
 * 88100 and 88110. GXemul only supports 88100 emulation so far.
 */
class M88K_CPUComponent
	: public CPUDyntransComponent
{
public:
	/**
	 * \brief Constructs a M88K_CPUComponent.
	 */
	M88K_CPUComponent();

	/**
	 * \brief Creates a M88K_CPUComponent.
	 */
	static refcount_ptr<Component> Create(const ComponentCreateArgs& args);

	static string GetAttribute(const string& attributeName);

	virtual void ResetState();

	virtual bool PreRunCheckForComponent(GXemul* gxemul);

	virtual size_t DisassembleInstruction(uint64_t vaddr, size_t maxlen,
		unsigned char *instruction, vector<string>& result);


	/********************************************************************/

	static void RunUnitTests(int& nSucceeded, int& nFailures);

protected:
	virtual bool CheckVariableWrite(StateVariable& var, const string& oldValue);

	virtual bool VirtualToPhysical(uint64_t vaddr, uint64_t& paddr,
	    bool& writable);

	virtual int FunctionTraceArgumentCount() { return 8; }
	virtual int64_t FunctionTraceArgument(int n) { return m_r[M88K_FIRST_ARG_REG + n]; }
	virtual bool FunctionTraceReturnImpl(int64_t& retval) { retval = m_r[M88K_RETURN_VALUE_REG]; return true; }

	virtual int GetDyntransICshift() const;
	virtual void (*GetDyntransToBeTranslated())(CPUDyntransComponent*, DyntransIC*) const;

	virtual void ShowRegisters(GXemul* gxemul, const vector<string>& arguments) const;

private:
	void Exception(int vector, int is_trap);

	void stcr(int cr, uint32_t value, bool is_rte);

	void m88k_cmp(struct DyntransIC *ic, uint32_t y);
	void m88k_extu(struct DyntransIC *ic, int w, int o);
	void m88k_ext(struct DyntransIC *ic, int w, int o);
	void m88k_mak(struct DyntransIC *ic, int w, int o);

	DECLARE_DYNTRANS_INSTR(cmp);
	DECLARE_DYNTRANS_INSTR(cmp_imm);
	DECLARE_DYNTRANS_INSTR(extu);
	DECLARE_DYNTRANS_INSTR(extu_imm);
	DECLARE_DYNTRANS_INSTR(ext);
	DECLARE_DYNTRANS_INSTR(ext_imm);
	DECLARE_DYNTRANS_INSTR(mak);
	DECLARE_DYNTRANS_INSTR(mak_imm);
	DECLARE_DYNTRANS_INSTR(divu_imm);
	DECLARE_DYNTRANS_INSTR(mulu_imm);
	DECLARE_DYNTRANS_INSTR(bsr);
	DECLARE_DYNTRANS_INSTR(bsr_samepage);
	DECLARE_DYNTRANS_INSTR(bsr_functioncalltrace);
	DECLARE_DYNTRANS_INSTR(bsr_n);
	DECLARE_DYNTRANS_INSTR(bsr_n_functioncalltrace);
	DECLARE_DYNTRANS_INSTR(bsr_n_functioncalltrace_singlestep);
	template<bool n, int op, bool singlestep> static void instr_bcnd(CPUDyntransComponent* cpubase, DyntransIC* ic);
	template<bool one, bool samepage> static void instr_bb(CPUDyntransComponent* cpubase, DyntransIC* ic);
	template<bool one> static void instr_bb_n(CPUDyntransComponent* cpubase, DyntransIC* ic);
	template<bool one> static void instr_bb_n_singlestep(CPUDyntransComponent* cpubase, DyntransIC* ic);
	DECLARE_DYNTRANS_INSTR(jmp);
	DECLARE_DYNTRANS_INSTR(jmp_n);
	DECLARE_DYNTRANS_INSTR(jmp_n_functioncalltrace);
	DECLARE_DYNTRANS_INSTR(jmp_n_functioncalltrace_singlestep);
	DECLARE_DYNTRANS_INSTR(ldcr);
	DECLARE_DYNTRANS_INSTR(stcr);
	template<bool one> static void instr_tb(CPUDyntransComponent* cpubase, DyntransIC* ic);
	template<bool store, typename T, bool doubleword, bool regofs, bool scaled, bool signedLoad> static void instr_loadstore(CPUDyntransComponent* cpubase, DyntransIC* ic);
	template<int scaleFactor> static void instr_lda(CPUDyntransComponent* cpubase, DyntransIC* ic);

	void Translate(uint32_t iword, struct DyntransIC* ic);
	DECLARE_DYNTRANS_INSTR(ToBeTranslated);

	// For unit tests:
	DECLARE_DYNTRANS_INSTR(fail_early);
	DECLARE_DYNTRANS_INSTR(fail_late);

private:
	/*
	 * State:
	 */
	string			m_m88k_type;	// E.g. "88100"
	uint32_t		m_initial_r31;	// Initial stack pointer.

	// General-Purpose Registers:
	//
	// 32 (N_M88K_REGS) registers, plus one which is always zero. (This
	// is to support st.d with d = r31. ld.d with d=r31 is converted to
	// just ld.
	uint32_t		m_r[N_M88K_REGS+1];

	// Destination scratch register for non-nop instructions with d=r0.
	// (Not serialized.)
	uint32_t		m_zero_scratch;

	// Control Registers:
	uint32_t		m_cr[N_M88K_CONTROL_REGS];

	// Floating Point Control registers:
	uint32_t		m_fcr[N_M88K_FPU_CONTROL_REGS];

	/*
	 * Cached other state:
	 */
	m88k_cpu_type_def	m_type;	// based on m_m88k_type
};


#endif	// M88K_CPUCOMPONENT_H

