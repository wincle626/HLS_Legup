#ifndef MIPS_CPUCOMPONENT_H
#define	MIPS_CPUCOMPONENT_H

/*
 *  Copyright (C) 2008-2010  Anders Gavare.  All rights reserved.
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

// COMPONENT(mips_cpu)


#include "CPUDyntransComponent.h"


/***********************************************************************/

#define	MIPS_INITIAL_PC			((int32_t) 0xbfc00000)
#define	MIPS_INITIAL_STACK_POINTER	((int32_t) 0xa0008000 - 256)


/*  FPU control registers:  */
#define	N_MIPS_FCRS			32
#define	MIPS_FPU_FCIR			0
#define	MIPS_FPU_FCCR			25
#define	MIPS_FPU_FCSR			31
#define	   MIPS_FCSR_FCC0_SHIFT		   23
#define	   MIPS_FCSR_FCC1_SHIFT		   25

#define	N_MIPS_COPROCS		4

#define	N_MIPS_GPRS		32	/*  General purpose registers  */
#define	N_MIPS_FPRS		32	/*  Floating point registers  */

/*
 *  These should all be 2 characters wide:
 *
 *  NOTE: For the newer ABIs (n32 and 64-bit ABIs), registers 8..11 are used
 *  to pass arguments and are then called "a4".."a7".
 */
#define MIPS_OLDABI_REGISTER_NAMES	{ \
	"zr", "at", "v0", "v1", "a0", "a1", "a2", "a3", \
	"t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7", \
	"s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", \
	"t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"  }
#define MIPS_REGISTER_NAMES	{ \
	"zr", "at", "v0", "v1", "a0", "a1", "a2", "a3", \
	"a4", "a5", "a6", "a7", "t4", "t5", "t6", "t7", \
	"s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", \
	"t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"  }

#define	MIPS_GPR_ZERO		0		/*  zero  */
#define	MIPS_GPR_AT		1		/*  at  */
#define	MIPS_GPR_V0		2		/*  v0  */
#define	MIPS_GPR_V1		3		/*  v1  */
#define	MIPS_GPR_A0		4		/*  a0  */
#define	MIPS_GPR_A1		5		/*  a1  */
#define	MIPS_GPR_A2		6		/*  a2  */
#define	MIPS_GPR_A3		7		/*  a3  */
#define	MIPS_GPR_T0		8		/*  t0  */
#define	MIPS_GPR_T1		9		/*  t1  */
#define	MIPS_GPR_T2		10		/*  t2  */
#define	MIPS_GPR_T3		11		/*  t3  */
#define	MIPS_GPR_T4		12		/*  t4  */
#define	MIPS_GPR_T5		13		/*  t5  */
#define	MIPS_GPR_T6		14		/*  t6  */
#define	MIPS_GPR_T7		15		/*  t7  */
#define	MIPS_GPR_S0		16		/*  s0  */
#define	MIPS_GPR_S1		17		/*  s1  */
#define	MIPS_GPR_S2		18		/*  s2  */
#define	MIPS_GPR_S3		19		/*  s3  */
#define	MIPS_GPR_S4		20		/*  s4  */
#define	MIPS_GPR_S5		21		/*  s5  */
#define	MIPS_GPR_S6		22		/*  s6  */
#define	MIPS_GPR_S7		23		/*  s7  */
#define	MIPS_GPR_T8		24		/*  t8  */
#define	MIPS_GPR_T9		25		/*  t9  */
#define	MIPS_GPR_K0		26		/*  k0  */
#define	MIPS_GPR_K1		27		/*  k1  */
#define	MIPS_GPR_GP		28		/*  gp  */
#define	MIPS_GPR_SP		29		/*  sp  */
#define	MIPS_GPR_FP		30		/*  fp  */
#define	MIPS_GPR_RA		31		/*  ra  */

#define	N_HI6			64
#define	N_SPECIAL		64
#define	N_REGIMM		32


/***********************************************************************/


/*
 *  CPU type definitions:  See mips_cpu_types.h.
 */

struct mips_cpu_type_def {
	const char *	name;
	int		rev;
	int		sub;
	int		flags;
	int		exc_model;		/*  EXC3K or EXC4K  */
	int		mmu_model;		/*  MMU3K or MMU4K  */
	int		isa_level;		/*  1, 2, 3, 4, 5, 32, 64  */
	int		isa_revision;		/*  1 or 2 (for MIPS32/64)  */
	int		nr_of_tlb_entries;	/*  32, 48, 64, ...  */
	int		instrs_per_cycle;	/*  simplified, 1, 2, or 4  */
	int		picache;
	int		pilinesize;
	int		piways;
	int		pdcache;
	int		pdlinesize;
	int		pdways;
	int		scache;
	int		slinesize;
	int		sways;
};


/***********************************************************************/


/**
 * \brief A Component representing a MIPS processor.
 */
class MIPS_CPUComponent
	: public CPUDyntransComponent
{
public:
	/**
	 * \brief Constructs a MIPS_CPUComponent.
	 */
	MIPS_CPUComponent();

	/**
	 * \brief Creates a MIPS_CPUComponent.
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

	virtual string VirtualAddressAsString(uint64_t vaddr)
	{
		stringstream ss;
		ss.flags(std::ios::hex | std::ios::showbase | std::ios::right);
		if (Is32Bit())
			ss << (uint32_t)vaddr;
		else
			ss << vaddr;
		return ss.str();
	}

	virtual uint64_t PCtoInstructionAddress(uint64_t pc);

	virtual int FunctionTraceArgumentCount();
	virtual int64_t FunctionTraceArgument(int n);
	virtual bool FunctionTraceReturnImpl(int64_t& retval);

	virtual int GetDyntransICshift() const;
	virtual void (*GetDyntransToBeTranslated())(CPUDyntransComponent*, DyntransIC*) const;

	virtual void ShowRegisters(GXemul* gxemul, const vector<string>& arguments) const;

private:
	size_t DisassembleInstructionMIPS16(uint64_t vaddr,
		unsigned char *instruction, vector<string>& result);

	bool Is32Bit() const;

private:
	template<int op, bool samepage, bool singlestep> static void instr_b(CPUDyntransComponent* cpubase, DyntransIC* ic);
	template<bool link, bool singlestep> static void instr_j(CPUDyntransComponent* cpubase, DyntransIC* ic);
	template<bool link, bool singlestep> static void instr_jr(CPUDyntransComponent* cpubase, DyntransIC* ic);

	DECLARE_DYNTRANS_INSTR(multu);
	DECLARE_DYNTRANS_INSTR(slt);
	DECLARE_DYNTRANS_INSTR(sltu);

	template<bool store, typename addressType, typename T, bool signedLoad> static void instr_loadstore(CPUDyntransComponent* cpubase, DyntransIC* ic);

	void Translate(uint32_t iword, struct DyntransIC* ic);
	DECLARE_DYNTRANS_INSTR(ToBeTranslated);
	DECLARE_DYNTRANS_INSTR(ToBeTranslated_MIPS16);

private:
	/*
	 * State:
	 */
	string		m_mips_type;		// E.g. "R4400"
	string		m_abi;			// "o32", "n32", or "n64"
	uint64_t	m_gpr[N_MIPS_GPRS];
	uint64_t	m_hi;
	uint64_t	m_lo;

	uint64_t	m_scratch;		// for loads into the zero register

	/*
	 * Cached or volatile other state:
	 */
	mips_cpu_type_def	m_type;	// based on m_mips_type
};


#endif	// MIPS_CPUCOMPONENT_H

