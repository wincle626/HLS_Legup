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

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <iomanip>

#include "ComponentFactory.h"
#include "GXemul.h"
#include "components/MIPS_CPUComponent.h"
#include "mips_cpu_types.h"
#include "opcodes_mips.h"

static const char* hi6_names[] = HI6_NAMES;
static const char* regnames_old[] = MIPS_OLDABI_REGISTER_NAMES;
static const char* regnames[] = MIPS_REGISTER_NAMES;
static const char* special_names[] = SPECIAL_NAMES;
static const char* special_rot_names[] = SPECIAL_ROT_NAMES;
static const char* regimm_names[] = REGIMM_NAMES;
static mips_cpu_type_def cpu_type_defs[] = MIPS_CPU_TYPE_DEFS;


static const char* regname(int i, const string& abi)
{
	if (abi == "o32")
		return regnames_old[i];
	else
		return regnames[i];
}


MIPS_CPUComponent::MIPS_CPUComponent()
	: CPUDyntransComponent("mips_cpu", "MIPS")
	, m_mips_type("5KE")	// defaults to a MIPS64 rev 2 cpu
	, m_abi("n64")
{
	m_frequency = 100e6;

	// Find (and cache) the cpu type in m_type:
	memset((void*) &m_type, 0, sizeof(m_type));
	for (size_t j=0; cpu_type_defs[j].name != NULL; j++) {
		if (m_mips_type == cpu_type_defs[j].name) {
			m_type = cpu_type_defs[j];
			break;
		}
	}

	if (m_type.name == NULL) {
		std::cerr << "Internal error: Unimplemented MIPS type?\n";
		throw std::exception();
	}

	ResetState();

	AddVariable("model", &m_mips_type);
	AddVariable("abi", &m_abi);

	AddVariable("hi", &m_hi);
	AddVariable("lo", &m_lo);

	// TODO: This only registers using the new ABI names. How should
	// this be handled? Custom "aliasing" variables?
	for (size_t i=0; i<N_MIPS_GPRS; i++)
		AddVariable(regname(i, m_abi), &m_gpr[i]);

	AddVariable("inDelaySlot", &m_inDelaySlot);
	AddVariable("delaySlotTarget", &m_delaySlotTarget);
}


refcount_ptr<Component> MIPS_CPUComponent::Create(const ComponentCreateArgs& args)
{
	// Defaults:
	ComponentCreationSettings settings;
	settings["model"] = "5KE";

	if (!ComponentFactory::GetCreationArgOverrides(settings, args))
		return NULL;

	refcount_ptr<Component> cpu = new MIPS_CPUComponent();
	if (!cpu->SetVariableValue("model", "\"" + settings["model"] + "\""))
		return NULL;

	return cpu;
}


void MIPS_CPUComponent::ResetState()
{
	// Most MIPS CPUs use 4 KB native page size.
	// However, a few use 1 KB pages; this should be supported as well.
	// TODO: Always use 1024 (worst case) on those CPUs? Or switch during
	// runtime from 4096 to 1024? (More complicated...)
	m_pageSize = 4096;

	m_hi = 0;
	m_lo = 0;
	m_scratch = 0;

	for (size_t i=0; i<N_MIPS_GPRS; i++)
		m_gpr[i] = 0;

	// MIPS CPUs are hardwired to start at 0xffffffffbfc00000:
	m_pc = MIPS_INITIAL_PC;

	// Reasonable initial stack pointer.
	m_gpr[MIPS_GPR_SP] = MIPS_INITIAL_STACK_POINTER;

	CPUDyntransComponent::ResetState();
}


bool MIPS_CPUComponent::PreRunCheckForComponent(GXemul* gxemul)
{
	if (m_gpr[MIPS_GPR_ZERO] != 0) {
		gxemul->GetUI()->ShowDebugMessage(this, "the zero register (zr) "
		    "must contain the value 0.\n");
		return false;
	}

	if (m_pc & 0x2) {
		gxemul->GetUI()->ShowDebugMessage(this, "the pc register"
		    " can not have bit 1 set!\n");
		return false;
	}

	if (Is32Bit()) {
		// All registers must be sign-extended correctly.
		if ((int64_t)m_pc != (int64_t)(int32_t)m_pc) {
			gxemul->GetUI()->ShowDebugMessage(this, "The emulated "
			    "CPU is 32-bit, but the pc register is not"
			    " a correctly sign-extended 32-bit value!\n");
			return false;
		}

		for (size_t i=1; i<N_MIPS_GPRS; i++) {
			if ((int64_t)m_gpr[i] != (int64_t)(int32_t)m_gpr[i]) {
				gxemul->GetUI()->ShowDebugMessage(this, (string)"The emulated "
				    "CPU is 32-bit, but the " + regname(i, m_abi) + " register is not"
				    " a correctly sign-extended 32-bit value!\n");
				return false;
			}
		}

		// TODO: Some more registers?
	}

	return CPUDyntransComponent::PreRunCheckForComponent(gxemul);
}


bool MIPS_CPUComponent::CheckVariableWrite(StateVariable& var, const string& oldValue)
{
	UI* ui = GetUI();

	if (m_gpr[MIPS_GPR_ZERO] != 0) {
		if (ui != NULL) {
			ui->ShowDebugMessage(this, "the zero register (zr) "
			    "must contain the value 0.\n");
		}
		return false;
	}

	if (m_mips_type != m_type.name) {
		bool found = false;
		for (size_t j=0; cpu_type_defs[j].name != NULL; j++) {
			if (m_mips_type == cpu_type_defs[j].name) {
				m_type = cpu_type_defs[j];
				found = true;
				break;
			}
		}

		if (!found) {
			if (ui != NULL) {
				stringstream ss;
				ss << "Unknown model \"" + m_mips_type + "\". Available types are:\n";
				for (size_t j=0; cpu_type_defs[j].name != NULL; j++) {
					if ((j % 6) != 0)
						ss << "\t";
					ss << cpu_type_defs[j].name;
					if ((j % 6) == 5)
						ss << "\n";
				}
				ui->ShowDebugMessage(this, ss.str());
			}
			return false;
		}
	}

	return CPUDyntransComponent::CheckVariableWrite(var, oldValue);
}


bool MIPS_CPUComponent::Is32Bit() const
{
	return m_type.isa_level == 32 || m_type.isa_level <= 2;
}


static uint64_t Trunc3264(uint64_t x, bool is32bit)
{
	return is32bit? (uint32_t)x : x;
}


static uint64_t TruncSigned3264(uint64_t x, bool is32bit)
{
	return is32bit? (int32_t)x : x;
}


void MIPS_CPUComponent::ShowRegisters(GXemul* gxemul, const vector<string>& arguments) const
{
	bool is32bit = Is32Bit();
	stringstream ss;

	ss.flags(std::ios::hex);
	ss << std::setfill('0');

	// Yuck, this is horrible. Is there some portable way to put e.g.
	// std::setw(16) into an object, and just pass that same object several
	// times?

	ss << "pc=";
	if (is32bit)
		ss << std::setw(8);
	else
		ss << std::setw(16);
	ss << Trunc3264(m_pc, is32bit);
	string symbol = GetSymbolRegistry().LookupAddress(TruncSigned3264(m_pc, is32bit), true);
	if (symbol != "")
		ss << " <" << symbol << ">";
	ss << "\n";

	ss << "hi=";
	if (is32bit)
		ss << std::setw(8);
	else
		ss << std::setw(16);
	ss << Trunc3264(m_hi, is32bit) << " lo=";
	if (is32bit)
		ss << std::setw(8);
	else
		ss << std::setw(16);
	ss << Trunc3264(m_lo, is32bit) << "\n";

	for (size_t i=0; i<N_MIPS_GPRS; i++) {
		ss << regname(i, m_abi) << "=";
		if (is32bit)
			ss << std::setw(8);
		else
			ss << std::setw(16);
		ss << Trunc3264(m_gpr[i], is32bit);
		if ((i&3) == 3)
			ss << "\n";
		else
			ss << " ";
	}

	gxemul->GetUI()->ShowDebugMessage(ss.str());
}


int MIPS_CPUComponent::FunctionTraceArgumentCount()
{
	// On old 32-bit ABIs, registers 4..7 (a0..a3) are used. On newer
	// ABIs (both 32-bit and 64-bit), registers 4..11 are used (a0..a7).

	if (m_abi == "o32")
		return 4;

	return 8;
}


int64_t MIPS_CPUComponent::FunctionTraceArgument(int n)
{
	// See comment for FunctionTraceArgumentCount above.
	return m_gpr[MIPS_GPR_A0 + n];
}


bool MIPS_CPUComponent::FunctionTraceReturnImpl(int64_t& retval)
{
	// v0 and v1 may hold return values. However, v1 is only used for
	// returning 64-bit values on old 32-bit ABIs, and 128-bit values
	// on newer ABIs, so for now I'll ignore it.

	retval = m_gpr[MIPS_GPR_V0];
	return true;
}


int MIPS_CPUComponent::GetDyntransICshift() const
{
	bool mips16 = m_pc & 1? true : false;

	// Normal encoding: 4 bytes per instruction, i.e. shift is 2 bits.
	// MIPS16 encoding: 2 bytes per instruction, i.e. shift is 1 bit.
	return mips16? 1 : 2;
}


void (*MIPS_CPUComponent::GetDyntransToBeTranslated())(CPUDyntransComponent*, DyntransIC*) const
{
	bool mips16 = m_pc & 1? true : false;
	return mips16? instr_ToBeTranslated_MIPS16 : instr_ToBeTranslated;
}


bool MIPS_CPUComponent::VirtualToPhysical(uint64_t vaddr, uint64_t& paddr,
	bool& writable)
{
	if (Is32Bit())
		vaddr = (int32_t)vaddr;

	// TODO. For now, just return the lowest 29 bits.
	if (vaddr >= 0xffffffff80000000ULL && vaddr < 0xffffffffc0000000ULL) {
		paddr = vaddr & 0x1fffffff;
		writable = true;
		return true;
	}

	// TODO  ... or the lowest 44.
	if (vaddr >= 0xa800000000000000ULL && vaddr < 0xa8000fffffffffffULL) {
		paddr = vaddr & 0xfffffffffffULL;
		writable = true;
		return true;
	}

	return false;
}


uint64_t MIPS_CPUComponent::PCtoInstructionAddress(uint64_t pc)
{
	// MIPS16 has the lowest bit set, but the instruction is aligned as
	// if the lowest bit was 0.
	return pc & ~1;
}


size_t MIPS_CPUComponent::DisassembleInstructionMIPS16(uint64_t vaddr,
	unsigned char *instruction, vector<string>& result)
{
	// Read the instruction word:
	uint16_t iword = *((uint16_t *) instruction);
	if (m_isBigEndian)
		iword = BE16_TO_HOST(iword);
	else
		iword = LE16_TO_HOST(iword);

	// ... and add it to the result:
	char tmp[5];
	snprintf(tmp, sizeof(tmp), "%04x", iword);
	result.push_back(tmp);

	int hi5 = iword >> 11;
	int rx = (iword >> 8) & 7;
	int ry = (iword >> 5) & 7;
//	int rz = (iword >> 2) & 7;
	int imm5 = iword & 0x1f;

	// Registers are: 16 17 2 3 4 5 6 7, and T(24) and SP(29).
	if (rx <= 1)
		rx += 16;
	if (ry <= 1)
		ry += 16;

	switch (hi5) {

	case 0x14:	/*  lbu y,5(x)  */
	case 0x18:	/*  sb  y,5(x)  */
		{
			stringstream ss;
			switch (hi5) {
			case 0x14: result.push_back("lbu"); break;
			case 0x18: result.push_back("sb"); break;
			}

			int ofs = imm5;	// TODO: scaling?

			ss << regname(ry, m_abi) << "," << ofs << "(" << regname(rx, m_abi) << ")";
			result.push_back(ss.str());
		}
		break;

	default:
		{
			stringstream ss;
			ss.flags(std::ios::hex);
			ss << "unimplemented MIPS16 opcode 0x" << hi5;
			result.push_back(ss.str());
		}
		break;
	}

	return sizeof(uint16_t);
}


size_t MIPS_CPUComponent::DisassembleInstruction(uint64_t vaddr, size_t maxLen,
	unsigned char *instruction, vector<string>& result)
{
	bool mips16 = m_pc & 1? true : false;
	size_t instrSize = mips16? sizeof(uint16_t) : sizeof(uint32_t);

	if (maxLen < instrSize) {
		assert(false);
		return 0;
	}

	if (mips16)
		return DisassembleInstructionMIPS16(vaddr,
		    instruction, result);

	// Read the instruction word:
	uint32_t iword = *((uint32_t *) instruction);
	if (m_isBigEndian)
		iword = BE32_TO_HOST(iword);
	else
		iword = LE32_TO_HOST(iword);

	// ... and add it to the result:
	{
		stringstream ss;
		ss.flags(std::ios::hex);
		ss << std::setfill('0') << std::setw(8) << (uint32_t) iword;
		if (PCtoInstructionAddress(m_pc) == vaddr && m_inDelaySlot)
			ss << " (delayslot)";
		result.push_back(ss.str());
	}

	int hi6 = iword >> 26;
	int rs = (iword >> 21) & 31;
	int rt = (iword >> 16) & 31;
	int rd = (iword >> 11) & 31;
	int sa = (iword >>  6) & 31;

	switch (hi6) {

	case HI6_SPECIAL:
		{
			int special6 = iword & 0x3f;
			int sub = rs;
			stringstream ss;

			switch (special6) {

			case SPECIAL_SLL:
			case SPECIAL_SRL:
			case SPECIAL_SRA:
			case SPECIAL_DSLL:
			case SPECIAL_DSRL:
			case SPECIAL_DSRA:
			case SPECIAL_DSLL32:
			case SPECIAL_DSRL32:
			case SPECIAL_DSRA32:
				if (rd == 0 && special6 == SPECIAL_SLL) {
					if (sa == 0)
						ss << "nop";
					else if (sa == 1)
						ss << "ssnop";
					else if (sa == 3)
						ss << "ehb";
					else
						ss << "nop (weird, sa="
						    << sa << ")";

					result.push_back(ss.str());
					break;
				}

				switch (sub) {
				case 0x00:
					result.push_back(
					    special_names[special6]);
					ss << regname(rd, m_abi) << "," <<
					    regname(rt, m_abi) << "," << sa;
					result.push_back(ss.str());
					break;
				case 0x01:
					result.push_back(
					    special_rot_names[special6]);
					ss << regname(rd, m_abi) << "," <<
					    regname(rt, m_abi) << "," << sa;
					result.push_back(ss.str());
					break;
				default:ss << "unimpl special, sub=" << sub;
					result.push_back(ss.str());
				}
				break;

			case SPECIAL_DSRLV:
			case SPECIAL_DSRAV:
			case SPECIAL_DSLLV:
			case SPECIAL_SLLV:
			case SPECIAL_SRAV:
			case SPECIAL_SRLV:
				sub = sa;

				switch (sub) {
				case 0x00:
					result.push_back(
					    special_names[special6]);
					ss << regname(rd, m_abi) << "," <<
					    regname(rt, m_abi) << "," <<
					    regname(rs, m_abi);
					result.push_back(ss.str());
					break;
				case 0x01:
					result.push_back(
					    special_rot_names[special6]);
					ss << regname(rd, m_abi) << "," <<
					    regname(rt, m_abi) << "," <<
					    regname(rs, m_abi);
					result.push_back(ss.str());
					break;
				default:ss << "unimpl special, sub="
					    << sub;
					result.push_back(ss.str());
				}
				break;

			case SPECIAL_JR:
				/*  .hb = hazard barrier hint on MIPS32/64
				    rev 2  */
				if ((iword >> 10) & 1)
					result.push_back("jr.hb");
				else
					result.push_back("jr");
				ss << regname(rs, m_abi);
				result.push_back(ss.str());
				break;
				
			case SPECIAL_JALR:
				/*  .hb = hazard barrier hint on
				     MIPS32/64 rev 2  */
				if ((iword >> 10) & 1)
					result.push_back("jalr.hb");
				else
					result.push_back("jalr");
				ss << regname(rd, m_abi) << "," << regname(rs, m_abi);
				result.push_back(ss.str());
				break;

			case SPECIAL_MFHI:
			case SPECIAL_MFLO:
				result.push_back(special_names[special6]);
				result.push_back(regname(rd, m_abi));
				break;

			case SPECIAL_MTLO:
			case SPECIAL_MTHI:
				result.push_back(special_names[special6]);
				result.push_back(regname(rs, m_abi));
				break;

			case SPECIAL_ADD:
			case SPECIAL_ADDU:
			case SPECIAL_SUB:
			case SPECIAL_SUBU:
			case SPECIAL_AND:
			case SPECIAL_OR:
			case SPECIAL_XOR:
			case SPECIAL_NOR:
			case SPECIAL_SLT:
			case SPECIAL_SLTU:
			case SPECIAL_DADD:
			case SPECIAL_DADDU:
			case SPECIAL_DSUB:
			case SPECIAL_DSUBU:
			case SPECIAL_MOVZ:
			case SPECIAL_MOVN:
				result.push_back(special_names[special6]);
				ss << regname(rd, m_abi) << "," <<
				    regname(rs, m_abi) << "," << regname(rt, m_abi);
				result.push_back(ss.str());
				break;

			case SPECIAL_MULT:
			case SPECIAL_MULTU:
			case SPECIAL_DMULT:
			case SPECIAL_DMULTU:
			case SPECIAL_DIV:
			case SPECIAL_DIVU:
			case SPECIAL_DDIV:
			case SPECIAL_DDIVU:
			case SPECIAL_TGE:
			case SPECIAL_TGEU:
			case SPECIAL_TLT:
			case SPECIAL_TLTU:
			case SPECIAL_TEQ:
			case SPECIAL_TNE:
				result.push_back(special_names[special6]);
				if (rd != 0) {
					if (m_type.rev == MIPS_R5900) {
						if (special6 == SPECIAL_MULT ||
						    special6 == SPECIAL_MULTU)
							ss << regname(rd, m_abi)<<",";
						else
							ss << "WEIRD_R5900_RD,";
					} else {
						ss << "WEIRD_R5900_RD,";
					}
				}

				ss << regname(rs, m_abi) << "," << regname(rt, m_abi);
				result.push_back(ss.str());
				break;

			case SPECIAL_SYNC:
				result.push_back(special_names[special6]);
				ss << ((iword >> 6) & 31);
				result.push_back(ss.str());
				break;

			case SPECIAL_SYSCALL:
			case SPECIAL_BREAK:
				result.push_back(special_names[special6]);
				if (((iword >> 6) & 0xfffff) != 0) {
					ss << ((iword >> 6) & 0xfffff);
					result.push_back(ss.str());
				}
				break;

			case SPECIAL_MFSA:
				if (m_type.rev == MIPS_R5900) {
					result.push_back("mfsa");
					result.push_back(regname(rd, m_abi));
				} else {
					result.push_back(
					    "unimplemented special 0x28");
				}
				break;

			case SPECIAL_MTSA:
				if (m_type.rev == MIPS_R5900) {
					result.push_back("mtsa");
					result.push_back(regname(rs, m_abi));
				} else {
					result.push_back(
					    "unimplemented special 0x29");
				}
				break;

			default:
				ss << "unimplemented: " <<
				    special_names[special6];
				result.push_back(ss.str());
				break;
			}
		}
		break;

	case HI6_BEQ:
	case HI6_BEQL:
	case HI6_BNE:
	case HI6_BNEL:
	case HI6_BGTZ:
	case HI6_BGTZL:
	case HI6_BLEZ:
	case HI6_BLEZL:
		{
			int imm = (int16_t) iword;
			uint64_t addr = vaddr + 4 + (imm << 2);

			stringstream ss;

			if (hi6 == HI6_BEQ && rt == MIPS_GPR_ZERO &&
			    rs == MIPS_GPR_ZERO) {
				result.push_back("b");
			} else {
				result.push_back(hi6_names[hi6]);

				switch (hi6) {
				case HI6_BEQ:
				case HI6_BEQL:
				case HI6_BNE:
				case HI6_BNEL:
					ss << regname(rt, m_abi) << ",";
				}

				ss << regname(rs, m_abi) << ",";
			}

			ss.flags(std::ios::hex | std::ios::showbase);
			ss << addr;
			result.push_back(ss.str());

			string symbol = GetSymbolRegistry().LookupAddress(addr, true);
			if (symbol != "")
				result.push_back("; <" + symbol + ">");
		}
		break;

	case HI6_ADDI:
	case HI6_ADDIU:
	case HI6_DADDI:
	case HI6_DADDIU:
	case HI6_SLTI:
	case HI6_SLTIU:
	case HI6_ANDI:
	case HI6_ORI:
	case HI6_XORI:
		{
			result.push_back(hi6_names[hi6]);

			stringstream ss;
			ss << regname(rt, m_abi) << "," << regname(rs, m_abi) << ",";
			if (hi6 == HI6_ANDI || hi6 == HI6_ORI ||
			    hi6 == HI6_XORI) {
				ss.flags(std::ios::hex | std::ios::showbase);
				ss << (uint16_t) iword;
			} else {
				ss << (int16_t) iword;
			}
			result.push_back(ss.str());
		}
		break;

	case HI6_LUI:
		{
			result.push_back(hi6_names[hi6]);

			stringstream ss;
			ss << regname(rt, m_abi) << ",";
			ss.flags(std::ios::hex | std::ios::showbase);
			ss << (uint16_t) iword;
			result.push_back(ss.str());
		}
		break;

	case HI6_LB:
	case HI6_LBU:
	case HI6_LH:
	case HI6_LHU:
	case HI6_LW:
	case HI6_LWU:
	case HI6_LD:
	case HI6_LQ_MDMX:
	case HI6_LWC1:
	case HI6_LWC2:
	case HI6_LWC3:
	case HI6_LDC1:
	case HI6_LDC2:
	case HI6_LL:
	case HI6_LLD:
	case HI6_SB:
	case HI6_SH:
	case HI6_SW:
	case HI6_SD:
	case HI6_SQ_SPECIAL3:
	case HI6_SC:
	case HI6_SCD:
	case HI6_SWC1:
	case HI6_SWC2:
	case HI6_SWC3:
	case HI6_SDC1:
	case HI6_SDC2:
	case HI6_LWL:
	case HI6_LWR:
	case HI6_LDL:
	case HI6_LDR:
	case HI6_SWL:
	case HI6_SWR:
	case HI6_SDL:
	case HI6_SDR:
		{
			if (hi6 == HI6_LQ_MDMX && m_type.rev != MIPS_R5900) {
				result.push_back("mdmx (UNIMPLEMENTED)");
				break;
			}
			if (hi6 == HI6_SQ_SPECIAL3 && m_type.rev!=MIPS_R5900) {
				result.push_back("special3 (UNIMPLEMENTED)");
				break;
			}


			int imm = (int16_t) iword;
			stringstream ss;

			/*  LWC3 is PREF in the newer ISA levels:  */
			/*  TODO: Which ISAs? IV? V? 32? 64?  */
			if (m_type.isa_level >= 4 && hi6 == HI6_LWC3) {
				result.push_back("pref");

				ss << rt << "," << imm <<
				    "(" << regname(rs, m_abi) << ")";
				result.push_back(ss.str());
				break;
			}

			result.push_back(hi6_names[hi6]);

			if (hi6 == HI6_SWC1 || hi6 == HI6_SWC2 ||
			    hi6 == HI6_SWC3 ||
			    hi6 == HI6_SDC1 || hi6 == HI6_SDC2 ||
			    hi6 == HI6_LWC1 || hi6 == HI6_LWC2 ||
			    hi6 == HI6_LWC3 ||
			    hi6 == HI6_LDC1 || hi6 == HI6_LDC2)
				ss << "r" << rt;
			else
				ss << regname(rt, m_abi);

			ss << "," << imm << "(" << regname(rs, m_abi) << ")";

			result.push_back(ss.str());
		}
		break;

	case HI6_J:
	case HI6_JAL:
		{
			result.push_back(hi6_names[hi6]);

			int imm = (iword & 0x03ffffff) << 2;
			uint64_t addr = (vaddr + 4) & ~((1 << 28) - 1);
			addr |= imm;

			stringstream ss;
			ss.flags(std::ios::hex | std::ios::showbase);
			ss << addr;
			result.push_back(ss.str());

			string symbol = GetSymbolRegistry().LookupAddress(addr, true);
			if (symbol != "")
				result.push_back("; <" + symbol + ">");
		}
		break;

	// CopX here. TODO
	// Cache
	// Special2

	case HI6_REGIMM:
		{
			int regimm5 = (iword >> 16) & 0x1f;
			int imm = (int16_t) iword;
			uint64_t addr = (vaddr + 4) + (imm << 2);

			stringstream ss;
			ss.flags(std::ios::hex | std::ios::showbase);

			switch (regimm5) {

			case REGIMM_BLTZ:
			case REGIMM_BGEZ:
			case REGIMM_BLTZL:
			case REGIMM_BGEZL:
			case REGIMM_BLTZAL:
			case REGIMM_BLTZALL:
			case REGIMM_BGEZAL:
			case REGIMM_BGEZALL:
				result.push_back(regimm_names[regimm5]);

				ss << regname(rs, m_abi) << "," << addr;
				result.push_back(ss.str());
				break;

			case REGIMM_SYNCI:
				result.push_back(regimm_names[regimm5]);

				ss << imm << "(" << regname(rs, m_abi) << ")";
				result.push_back(ss.str());
				break;

			default:
				{
					ss << "unimplemented: " <<
					    regimm_names[regimm5];
					result.push_back(ss.str());
				}
			}
		}
		break;

	default:
		{
			stringstream ss;
			ss << "unimplemented: " << hi6_names[hi6];
			result.push_back(ss.str());
		}
		break;
	}

	return instrSize;
}


string MIPS_CPUComponent::GetAttribute(const string& attributeName)
{
	if (attributeName == "stable")
		return "yes";

	if (attributeName == "description")
		return "MIPS processor.";

	return Component::GetAttribute(attributeName);
}


/*****************************************************************************/


/*
 *  b*:  Branch on condition
 *
 *  op 0: beq    rs == rt
 *  op 1: bne    rs != rt
 *  op 2: blez   rs <= 0  (signed)
 *  op 3: bgtz   rs > 0   (signed)
 *
 *  arg[0] = pointer to register rs
 *  arg[1] = pointer to register rt
 *  arg[2] = signed offset from start of current page to branch to _OR_ pointer to new instr_call
 */
template<int op, bool samepage, bool singlestep> void MIPS_CPUComponent::instr_b(CPUDyntransComponent* cpubase, DyntransIC* ic)
{
	DYNTRANS_INSTR_HEAD(MIPS_CPUComponent)

	bool cond;

	if (op == 0)		cond = REG64(ic->arg[0]) == REG64(ic->arg[1]);	// beq
	else if (op == 1)	cond = REG64(ic->arg[0]) != REG64(ic->arg[1]);	// bne
	else if (op == 2)	cond = (int64_t)REG64(ic->arg[0]) <= 0;		// blez
	else /* op == 3 */	cond = (int64_t)REG64(ic->arg[0]) > 0;		// bgtz

	if (singlestep) {
		DYNTRANS_SYNCH_PC;

		// Prepare for the branch.
		cpu->m_inDelaySlot = true;
		cpu->m_exceptionOrAbortInDelaySlot = false;

		if (cond) {
			if (samepage) {
				std::cerr << "MIPS b instruction: samepage singlestep: should not happen.\n";
				throw std::exception();
			} else {
				cpu->m_delaySlotTarget = cpu->m_pc & ~cpu->m_dyntransPageMask;
				cpu->m_delaySlotTarget = cpu->m_delaySlotTarget + (int32_t)ic->arg[2].u32;
			}
		} else {
			cpu->m_delaySlotTarget = cpu->m_pc + 8;
		}

		cpu->m_nextIC = ic + 1;
	} else {
		// Prepare for the branch.
		cpu->m_inDelaySlot = true;
		cpu->m_exceptionOrAbortInDelaySlot = false;

		// Execute the next instruction:
		ic[1].f(cpu, ic+1);
		cpu->m_executedCycles ++;
	
		// If there was no exception, then branch:
		if (!cpu->m_exceptionOrAbortInDelaySlot) {
			if (cond) {
				if (samepage) {
					cpu->m_nextIC = (DyntransIC*) ic->arg[2].p;
				} else {
					cpu->m_pc &= ~cpu->m_dyntransPageMask;
					cpu->m_pc = cpu->m_pc + (int32_t)ic->arg[2].u32;
					cpu->DyntransPCtoPointers();
				}
			} else {
				cpu->m_nextIC = ic + 2;
			}
	
			cpu->m_inDelaySlot = false;
		}
	
		// The next instruction is now either the target of the branch
		// instruction, the instruction 2 steps after this one,
		// or the first instruction of an exception handler.
		cpu->m_exceptionOrAbortInDelaySlot = false;
	}
}


/*
 *  j, jal:  Jump [and link]
 *
 *  arg[0] = lowest 28 bits of new pc
 */
template<bool link, bool singlestep> void MIPS_CPUComponent::instr_j(CPUDyntransComponent* cpubase, DyntransIC* ic)
{
	DYNTRANS_INSTR_HEAD(MIPS_CPUComponent)

	if (link) {
		DYNTRANS_SYNCH_PC;
		cpu->m_gpr[MIPS_GPR_RA] = cpu->m_pc + 8;

		if (cpu->m_showFunctionTraceCall) {
			uint64_t saved_pc = cpu->m_pc;
			cpu->m_pc = cpu->m_pc & ~0x0fffffffUL;
			cpu->m_pc += ic->arg[0].u32;
			cpu->FunctionTraceCall();
			cpu->m_pc = saved_pc;
		}
	}

	if (singlestep) {
		// Prepare for the branch.
		cpu->m_inDelaySlot = true;
		cpu->m_exceptionOrAbortInDelaySlot = false;

		cpu->m_delaySlotTarget = cpu->m_pc & ~0x0fffffffUL;
		cpu->m_delaySlotTarget += ic->arg[0].u32;

		cpu->m_nextIC = ic + 1;
	} else {
		// Prepare for the branch.
		cpu->m_inDelaySlot = true;
		cpu->m_exceptionOrAbortInDelaySlot = false;

		// Execute the next instruction:
		ic[1].f(cpu, ic+1);
		cpu->m_executedCycles ++;

		// If there was no exception, then branch:
		if (!cpu->m_exceptionOrAbortInDelaySlot) {
			cpu->m_pc = cpu->m_pc & ~0x0fffffffUL;
			cpu->m_pc += ic->arg[0].u32;
			cpu->DyntransPCtoPointers();
	
			cpu->m_inDelaySlot = false;
		}
	
		// The next instruction is now either the target of the branch
		// instruction, the instruction 2 steps after this one,
		// or the first instruction of an exception handler.
		cpu->m_exceptionOrAbortInDelaySlot = false;
	}
}


/*
 *  jr, jalr:  Jump to register [and link]
 *
 *  arg[0] = pointer to register rs (to jump to)
 *  arg[1] = pointer to register rd (to store return address in; this is never the zero register)
 */
template<bool link, bool singlestep> void MIPS_CPUComponent::instr_jr(CPUDyntransComponent* cpubase, DyntransIC* ic)
{
	DYNTRANS_INSTR_HEAD(MIPS_CPUComponent)

	if (link) {
		DYNTRANS_SYNCH_PC;
		REG32(ic->arg[1]) = cpu->m_pc + 8;

		if (cpu->m_showFunctionTraceCall && ic->arg[1].p == &cpu->m_gpr[MIPS_GPR_RA]) {
			uint64_t saved_pc = cpu->m_pc;
			cpu->m_pc = REG64(ic->arg[0]);
			cpu->FunctionTraceCall();
			cpu->m_pc = saved_pc;
		}
	}

	if (!link && cpu->m_showFunctionTraceCall && ic->arg[0].p == &cpu->m_gpr[MIPS_GPR_RA])
		cpu->FunctionTraceReturn();

	if (singlestep) {
		// Prepare for the branch.
		cpu->m_inDelaySlot = true;
		cpu->m_exceptionOrAbortInDelaySlot = false;

		cpu->m_delaySlotTarget = REG64(ic->arg[0]);
		cpu->m_nextIC = ic + 1;
	} else {
		// Prepare for the branch.
		cpu->m_inDelaySlot = true;
		cpu->m_exceptionOrAbortInDelaySlot = false;

		// Execute the next instruction:
		ic[1].f(cpu, ic+1);
		cpu->m_executedCycles ++;

		// If there was no exception, then branch:
		if (!cpu->m_exceptionOrAbortInDelaySlot) {
			cpu->m_pc = REG64(ic->arg[0]);
			cpu->DyntransPCtoPointers();
	
			cpu->m_inDelaySlot = false;
		}
	
		// The next instruction is now either the target of the branch
		// instruction, the instruction 2 steps after this one,
		// or the first instruction of an exception handler.
		cpu->m_exceptionOrAbortInDelaySlot = false;
	}
}


DYNTRANS_INSTR(MIPS_CPUComponent,multu)
{
	DYNTRANS_INSTR_HEAD(MIPS_CPUComponent)

	uint32_t a = REG64(ic->arg[1]), b = REG64(ic->arg[2]);
	uint64_t res = (uint64_t)a * (uint64_t)b;

	cpu->m_lo = (int32_t)res;
	cpu->m_hi = (int32_t)(res >> 32);
}


DYNTRANS_INSTR(MIPS_CPUComponent,slt)
{
	REG64(ic->arg[0]) = (int64_t)REG64(ic->arg[1]) < (int64_t)REG64(ic->arg[2]);
}


DYNTRANS_INSTR(MIPS_CPUComponent,sltu)
{
	REG64(ic->arg[0]) = (uint64_t)REG64(ic->arg[1]) < (uint64_t)REG64(ic->arg[2]);
}


template<bool store, typename addressType, typename T, bool signedLoad> void MIPS_CPUComponent::instr_loadstore(CPUDyntransComponent* cpubase, DyntransIC* ic)
{
	DYNTRANS_INSTR_HEAD(MIPS_CPUComponent)

	// TODO: fast lookups

	uint64_t addr;

	if (sizeof(addressType) == sizeof(uint64_t))
		addr = REG64(ic->arg[1]) + (int32_t)ic->arg[2].u32;
	else
		addr = (int32_t) (REG64(ic->arg[1]) + (int32_t)ic->arg[2].u32);

	if (sizeof(T) > 1 && (addr & (sizeof(T)-1))) {
		std::cerr << "TODO: MIPS unaligned data access exception!\n";
		throw std::exception();
		// DYNTRANS_SYNCH_PC;
		// cpu->Exception(M88K_EXCEPTION_MISALIGNED_ACCESS, 0);
		return;
	}

	cpu->AddressSelect(addr);

	if (store) {
		T data = REG64(ic->arg[0]);
		if (!cpu->WriteData(data, cpu->m_isBigEndian? BigEndian : LittleEndian)) {
			// TODO: failed to access memory was probably an exception. Handle this!
		}
	} else {
		T data;
		if (!cpu->ReadData(data, cpu->m_isBigEndian? BigEndian : LittleEndian)) {
			// TODO: failed to access memory was probably an exception. Handle this!
		}

		if (signedLoad) {
			if (sizeof(T) == sizeof(uint32_t))
				REG64(ic->arg[0]) = (int32_t)data;
			if (sizeof(T) == sizeof(uint16_t))
				REG64(ic->arg[0]) = (int16_t)data;
			if (sizeof(T) == sizeof(uint8_t))
				REG64(ic->arg[0]) = (int8_t)data;
		} else {
			REG64(ic->arg[0]) = data;
		}
	}
}


/*****************************************************************************/


void MIPS_CPUComponent::Translate(uint32_t iword, struct DyntransIC* ic)
{
	bool singleInstructionLeft = (m_executedCycles == m_nrOfCyclesToExecute - 1);
	UI* ui = GetUI();	// for debug messages
	int requiredISA = 1;		// 1, 2, 3, 4, 32, or 64
	int requiredISArevision = 1;	// 1 or 2 (for MIPS32/64)

	int hi6 = iword >> 26;
	int rs = (iword >> 21) & 31;
	int rt = (iword >> 16) & 31;
	int rd = (iword >> 11) & 31;
	int sa = (iword >>  6) & 31;
	int32_t imm = (int16_t)iword;
	int s6 = iword & 63;
//	int s10 = (rs << 5) | sa;

	switch (hi6) {

	case HI6_SPECIAL:
		switch (s6) {

		case SPECIAL_SLL:
//		case SPECIAL_SLLV:
		case SPECIAL_SRL:
/*		case SPECIAL_SRLV:
		case SPECIAL_SRA:
		case SPECIAL_SRAV:
		case SPECIAL_DSRL:
		case SPECIAL_DSRLV:
		case SPECIAL_DSRL32:
		case SPECIAL_DSLL:
		case SPECIAL_DSLLV:
		case SPECIAL_DSLL32:
		case SPECIAL_DSRA:
		case SPECIAL_DSRAV:
		case SPECIAL_DSRA32: */
			switch (s6) {
			case SPECIAL_SLL:  ic->f = instr_shift_left_u64_u64_imm5_truncS32; break;
//			case SPECIAL_SLLV: ic->f = instr(sllv); sa = -1; break;
			case SPECIAL_SRL:  ic->f = instr_shift_right_u64_u64asu32_imm5_truncS32; break;
/*			case SPECIAL_SRLV: ic->f = instr(srlv); sa = -1; break;
			case SPECIAL_SRA:  ic->f = instr(sra); break;
			case SPECIAL_SRAV: ic->f = instr(srav); sa = -1; break;
			case SPECIAL_DSRL: ic->f = instr(dsrl); x64=1; break;
			case SPECIAL_DSRLV:ic->f = instr(dsrlv);
					   x64 = 1; sa = -1; break;
			case SPECIAL_DSRL32:ic->f= instr(dsrl); x64=1;
					   sa += 32; break;
			case SPECIAL_DSLL: ic->f = instr(dsll); x64=1; break;
			case SPECIAL_DSLLV:ic->f = instr(dsllv);
					   x64 = 1; sa = -1; break;
			case SPECIAL_DSLL32:ic->f= instr(dsll); x64=1;
					   sa += 32; break;
			case SPECIAL_DSRA: ic->f = instr(dsra); x64=1; break;
			case SPECIAL_DSRAV:ic->f = instr(dsrav);
					   x64 = 1; sa = -1; break;
			case SPECIAL_DSRA32:ic->f = instr(dsra); x64=1;
					   sa += 32; break; */
			}

			ic->arg[0].p = &m_gpr[rd];
			ic->arg[1].p = &m_gpr[rt];
			if (sa >= 0)
				ic->arg[2].u32 = sa;
			else
				ic->arg[2].p = &m_gpr[rs];

			/*  Special checks for MIPS32/64 revision 2 opcodes,
			    such as rotation instructions:  */
//			if (sa >= 0 && rs != 0x00) {
//				if (m_type.isa_level < 32 ||
//				    m_type.isa_revision < 2) {
//					static int warning_rotate = 0;
//					if (!warning_rotate &&
//					    !cpu->translation_readahead) {
//						fatal("[ WARNING! MIPS32/64 "
//						    "revision 2 rotate opcode"
//						    " used, but the %s process"
//						    "or does not implement "
//						    "such instructions. Only "
//						    "printing this "
//						    "warning once. ]\n",
//						    cpu->cd.mips.cpu_type.name);
//						warning_rotate = 1;
//					}
//					ic->f = NULL; // TODO instr(reserved);
//					break;
//				}
//
//				switch (rs) {
//				case 0x01:
//					switch (s6) {
//					case SPECIAL_SRL:	/*  ror  */
//						ic->f = NULL; //TODO instr(ror);
//						break;
//					}
//					break;
//				}
//			}

//			if (sa < 0 && (s10 & 0x1f) != 0) {
//				switch (s10 & 0x1f) {
//				/*  TODO: [d]rorv, etc.  */
//				}
//			}

			if (rd == MIPS_GPR_ZERO)
				ic->f = instr_nop;

			break;

//		case SPECIAL_ADD:
		case SPECIAL_ADDU:
//		case SPECIAL_SUB:
		case SPECIAL_SUBU:
//		case SPECIAL_DADD:
//		case SPECIAL_DADDU:
//		case SPECIAL_DSUB:
//		case SPECIAL_DSUBU:
		case SPECIAL_SLT:
		case SPECIAL_SLTU:
//		case SPECIAL_AND:
//		case SPECIAL_OR:
		case SPECIAL_XOR:
//		case SPECIAL_NOR:
//		case SPECIAL_MOVN:
//		case SPECIAL_MOVZ:
		case SPECIAL_MFHI:
		case SPECIAL_MFLO:
		case SPECIAL_MTHI:
		case SPECIAL_MTLO:
//		case SPECIAL_DIV:
//		case SPECIAL_DIVU:
//		case SPECIAL_DDIV:
//		case SPECIAL_DDIVU:
//		case SPECIAL_MULT:
		case SPECIAL_MULTU:
//		case SPECIAL_DMULT:
//		case SPECIAL_DMULTU:
//		case SPECIAL_TGE:
//		case SPECIAL_TGEU:
//		case SPECIAL_TLT:
//		case SPECIAL_TLTU:
//		case SPECIAL_TEQ:
//		case SPECIAL_TNE:
			switch (s6) {
//			case SPECIAL_ADD:   ic->f = instr(add); break;
			case SPECIAL_ADDU:  ic->f = instr_add_u64_u64_u64_truncS32; break;
//			case SPECIAL_SUB:   ic->f = instr(sub); break;
			case SPECIAL_SUBU:  ic->f = instr_sub_u64_u64_u64_truncS32; break;
//			case SPECIAL_DADD:  ic->f = instr(dadd); x64=1; break;
//			case SPECIAL_DADDU: ic->f = instr(daddu); x64=1; break;
//			case SPECIAL_DSUB:  ic->f = instr(dsub); x64=1; break;
//			case SPECIAL_DSUBU: ic->f = instr(dsubu); x64=1; break;
			case SPECIAL_SLT:   ic->f = instr_slt; break;
			case SPECIAL_SLTU:  ic->f = instr_sltu; break;
//			case SPECIAL_AND:   ic->f = instr(and); break;
//			case SPECIAL_OR:    ic->f = instr(or); break;
			case SPECIAL_XOR:   ic->f = instr_xor_u64_u64_u64; break;
//			case SPECIAL_NOR:   ic->f = instr(nor); break;
			case SPECIAL_MFHI:  ic->f = instr_mov_u64_u64; break;
			case SPECIAL_MFLO:  ic->f = instr_mov_u64_u64; break;
			case SPECIAL_MTHI:  ic->f = instr_mov_u64_u64; break;
			case SPECIAL_MTLO:  ic->f = instr_mov_u64_u64; break;
//			case SPECIAL_DIV:   ic->f = instr(div); break;
//			case SPECIAL_DIVU:  ic->f = instr(divu); break;
//			case SPECIAL_DDIV:  ic->f = instr(ddiv); x64=1; break;
//			case SPECIAL_DDIVU: ic->f = instr(ddivu); x64=1; break;
//			case SPECIAL_MULT : ic->f = instr(mult); break;
			case SPECIAL_MULTU: ic->f = instr_multu; break;
//			case SPECIAL_DMULT: ic->f = instr(dmult); x64=1; break;
//			case SPECIAL_DMULTU:ic->f = instr(dmultu); x64=1; break;
//			case SPECIAL_TGE:   ic->f = instr(tge); break;
//			case SPECIAL_TGEU:  ic->f = instr(tgeu); break;
//			case SPECIAL_TLT:   ic->f = instr(tlt); break;
//			case SPECIAL_TLTU:  ic->f = instr(tltu); break;
//			case SPECIAL_TEQ:   ic->f = instr(teq); break;
//			case SPECIAL_TNE:   ic->f = instr(tne); break;
//			case SPECIAL_MOVN:  ic->f = instr(movn); break;
//			case SPECIAL_MOVZ:  ic->f = instr(movz); break;
			}

			// NOTE: When uncommenting instruction above, make sure they
			// use the same rd, rs, rt format!

			ic->arg[0].p = &m_gpr[rd];
			ic->arg[1].p = &m_gpr[rs];
			ic->arg[2].p = &m_gpr[rt];

			switch (s6) {
			case SPECIAL_MFHI: ic->arg[1].p = &m_hi; break;
			case SPECIAL_MFLO: ic->arg[1].p = &m_lo; break;
			case SPECIAL_MTHI: ic->arg[0].p = &m_hi; break;
			case SPECIAL_MTLO: ic->arg[0].p = &m_lo; break;
			}

			//  Special cases for rd: 
			switch (s6) {
			case SPECIAL_MTHI:
			case SPECIAL_MTLO:
			case SPECIAL_DIV:
			case SPECIAL_DIVU:
			case SPECIAL_DDIV:
			case SPECIAL_DDIVU:
			case SPECIAL_MULT:
			case SPECIAL_MULTU:
			case SPECIAL_DMULT:
			case SPECIAL_DMULTU:
				if (s6 == SPECIAL_MULT && rd != MIPS_GPR_ZERO) {
					if (m_type.rev == MIPS_R5900) {
						// ic->f = instr(mult_r5900);
						ic->f = NULL;
						break;
					}
					break;
				}
				if (s6 == SPECIAL_MULTU && rd!=MIPS_GPR_ZERO) {
					if (m_type.rev == MIPS_R5900) {
						// ic->f = instr(multu_r5900);
						ic->f = NULL;
						break;
					}
				}
				if (rd != MIPS_GPR_ZERO) {
					std::cerr << "TODO: rd NON-zero\n";
					ic->f = NULL;
				}
				//  These instructions don't use rd.
				break;
			case SPECIAL_TGE:
			case SPECIAL_TGEU:
			case SPECIAL_TLT:
			case SPECIAL_TLTU:
			case SPECIAL_TEQ:
			case SPECIAL_TNE:
				//  In these instructions, rd is a 'code',
				//  only read by trap handling software.
				break;
			default:if (rd == MIPS_GPR_ZERO)
					ic->f = instr_nop;
			}

			break;

		case SPECIAL_JR:
		case SPECIAL_JALR:
			{
				void (*f_singleStepping)(CPUDyntransComponent*, struct DyntransIC*) = NULL;

				if (s6 == SPECIAL_JALR && rd == MIPS_GPR_ZERO)
					s6 = SPECIAL_JR;

				switch (s6) {
				case SPECIAL_JR:
					ic->f            = instr_jr<false, false>;
					f_singleStepping = instr_jr<false, true>;
					break;
				case SPECIAL_JALR:
					ic->f            = instr_jr<true, false>;
					f_singleStepping = instr_jr<true, true>;
					break;
				}
	
				// Only one instruction left, then we carefully single-step it.
				if (singleInstructionLeft)
					ic->f = f_singleStepping;

				ic->arg[0].p = &m_gpr[rs];
				ic->arg[1].p = &m_gpr[rd];

				if (m_inDelaySlot) {
					if (ui != NULL)
						ui->ShowDebugMessage(this, "branch in delay slot?!"
						" TODO: How should this be handled?");
	
					ic->f = NULL;
				}
			}
			break;

		default:
			if (ui != NULL) {
				stringstream ss;
				ss.flags(std::ios::hex);
				ss << "unimplemented opcode HI6_SPECIAL, s6 = 0x" << s6;
				ui->ShowDebugMessage(this, ss.str());
			}
		}
		break;

	case HI6_BEQ:
	case HI6_BNE:
//	case HI6_BEQL:
//	case HI6_BNEL:
	case HI6_BLEZ:
//	case HI6_BLEZL:
	case HI6_BGTZ:
//	case HI6_BGTZL:
		{
			void (*f_singleStepping)(CPUDyntransComponent*, struct DyntransIC*) = NULL;
			void (*samepage_function)(CPUDyntransComponent*, struct DyntransIC*) = NULL;
			bool warnAboutNonZeroRT = false;

			switch (hi6) {
			case HI6_BEQ:
				ic->f             = instr_b<0, false, false>;
				samepage_function = instr_b<0, true,  false>;
				f_singleStepping  = instr_b<0, false, true>;
				break;
			case HI6_BNE:
				ic->f             = instr_b<1, false, false>;
				samepage_function = instr_b<1, true,  false>;
				f_singleStepping  = instr_b<1, false, true>;
				break;
			case HI6_BLEZ:
				ic->f             = instr_b<2, false, false>;
				samepage_function = instr_b<2, true,  false>;
				f_singleStepping  = instr_b<2, false, true>;
				warnAboutNonZeroRT = true;
				break;
			case HI6_BGTZ:
				ic->f             = instr_b<3, false, false>;
				samepage_function = instr_b<3, true,  false>;
				f_singleStepping  = instr_b<3, false, true>;
				warnAboutNonZeroRT = true;
				break;
			}

			if (singleInstructionLeft) {
				// Only one instruction left, then we carefully single-step it.
				// (No need to optimize for samepage.)
				ic->f = f_singleStepping;
				samepage_function = NULL;
			}

			uint32_t mask = m_dyntransPageMask & ~3;	// 0xffc for 4 KB pages

			ic->arg[0].p = &m_gpr[rs];
			ic->arg[1].p = &m_gpr[rt];
			// TODO: MIPS16 offset!
			ic->arg[2].u32 = (int32_t) ( (imm << m_dyntransICshift) + (m_pc & mask) + 4 );

			if (rt != MIPS_GPR_ZERO && warnAboutNonZeroRT && ui != NULL)
				ui->ShowDebugMessage(this, "MIPS branch with rt non-zero, where it should have been zero?");

			// Is the offset from the start of the current page still
			// within the same page? Then use the samepage_function:
			if (samepage_function != NULL &&
			    ic->arg[2].u32 < (uint32_t)((m_dyntransICentriesPerPage - 1)
			    << m_dyntransICshift) && (m_pc & mask) < mask) {
				ic->arg[2].p = (m_firstIConPage +
				    ((ic->arg[2].u32 >> m_dyntransICshift)
				    & (m_dyntransICentriesPerPage - 1)));
				ic->f = samepage_function;
			}

			if (m_inDelaySlot) {
				if (ui != NULL)
					ui->ShowDebugMessage(this, "branch in delay slot?!"
					    " TODO: How should this be handled?");

				ic->f = NULL;
			}
		}
		break;

//	case HI6_ADDI:
	case HI6_ADDIU:
//	case HI6_SLTI:
//	case HI6_SLTIU:
//	case HI6_DADDI:
	case HI6_DADDIU:
	case HI6_ANDI:
	case HI6_ORI:
	case HI6_XORI:
		ic->arg[0].p = &m_gpr[rt];
		ic->arg[1].p = &m_gpr[rs];
		if (hi6 == HI6_ADDI || hi6 == HI6_ADDIU ||
		    hi6 == HI6_SLTI || hi6 == HI6_SLTIU ||
		    hi6 == HI6_DADDI || hi6 == HI6_DADDIU)
			ic->arg[2].u32 = (int16_t)iword;
		else
			ic->arg[2].u32 = (uint16_t)iword;

		switch (hi6) {
//		case HI6_ADDI:    ic->f = instr(addi); break;
		case HI6_ADDIU:   ic->f = instr_add_u64_u64_imms32_truncS32; break;
//		case HI6_SLTI:    ic->f = instr(slti); break;
//		case HI6_SLTIU:   ic->f = instr(sltiu); break;
//		case HI6_DADDI:   ic->f = instr(daddi); requiredISA = 3; break;
		case HI6_DADDIU:  ic->f = instr_add_u64_u64_imms32; requiredISA = 3; break;
		case HI6_ANDI:    ic->f = instr_and_u64_u64_immu32; break;
		case HI6_ORI:     ic->f = instr_or_u64_u64_immu32; break;
		case HI6_XORI:    ic->f = instr_xor_u64_u64_immu32; break;
		}

		if (rt == MIPS_GPR_ZERO)
			ic->f = instr_nop;
		break;

	case HI6_LUI:
		ic->f = instr_set_u64_imms32;
		ic->arg[0].p = &m_gpr[rt];
		ic->arg[1].u32 = (int32_t) (imm << 16);

		if (rt == MIPS_GPR_ZERO)
			ic->f = instr_nop;
		break;

	case HI6_J:
	case HI6_JAL:
		{
			void (*f_singleStepping)(CPUDyntransComponent*, struct DyntransIC*) = NULL;

			switch (hi6) {
			case HI6_J:
				ic->f             = instr_j<false, false>;
				f_singleStepping  = instr_j<false, true>;
				break;
			case HI6_JAL:
				ic->f             = instr_j<true, false>;
				f_singleStepping  = instr_j<true, true>;
				break;
			}

			// Only one instruction left, then we carefully single-step it.
			if (singleInstructionLeft)
				ic->f = f_singleStepping;

			ic->arg[0].u32 = (iword & 0x03ffffff) << 2;

			if (m_inDelaySlot) {
				if (ui != NULL)
					ui->ShowDebugMessage(this, "branch in delay slot?!"
					    " TODO: How should this be handled?");

				ic->f = NULL;
			}

			break;
		}

//	case HI6_LB:
//	case HI6_LBU:
	case HI6_SB:
//	case HI6_LH:
//	case HI6_LHU:
//	case HI6_SH:
	case HI6_LW:
//	case HI6_LWU:
	case HI6_SW:
//	case HI6_LD:
//	case HI6_SD:
		{
			ic->arg[0].p = &m_gpr[rt];
			ic->arg[1].p = &m_gpr[rs];
			ic->arg[2].u32 = (int32_t)imm;

			bool store = false;

			if (Is32Bit()) {
				switch (hi6) {
				case HI6_LW:  ic->f = instr_loadstore<false, int32_t,  uint32_t, true>;  break;
				case HI6_SB:  ic->f = instr_loadstore<true,  int32_t,  uint8_t,  false>; store = true; break;
				case HI6_SW:  ic->f = instr_loadstore<true,  int32_t,  uint32_t, false>; store = true; break;
				}
			} else {
				switch (hi6) {
				case HI6_LW:  ic->f = instr_loadstore<false, uint64_t, uint32_t, true>;  break;
				case HI6_SB:  ic->f = instr_loadstore<true,  uint64_t, uint8_t,  false>; store = true; break;
				case HI6_SW:  ic->f = instr_loadstore<true,  uint64_t, uint32_t, false>; store = true; break;
				}
			}

			/*  Load into the dummy scratch register, if rt = zero  */
			if (!store && rt == MIPS_GPR_ZERO)
				ic->arg[0].p = &m_scratch;
		}
		break;

	default:
		if (ui != NULL) {
			stringstream ss;
			ss.flags(std::ios::hex);
			ss << "unimplemented opcode 0x" << hi6;
			ui->ShowDebugMessage(this, ss.str());
		}
	}

	// Attempting a MIPS32 instruction on e.g. a MIPS IV CPU?
	if (requiredISA > m_type.isa_level) {
		// TODO: Cause MIPS "unimplemented instruction" exception instead.
		ic->f = NULL;

		// TODO: Only print the warning once; actual real-world code may
		// rely on this mechanism to detect cpu type, or similar.
		if (ui != NULL) {
			stringstream ss;
			ss.flags(std::ios::hex);
			ss << "instruction at 0x" << m_pc << " requires ISA level ";
			ss.flags(std::ios::dec);
			ss << requiredISA << "; this cpu supports only ISA level " <<
			    m_type.isa_level << "\n";
			ui->ShowDebugMessage(this, ss.str());
		}
	}

	// Attempting a MIPS III or IV instruction on e.g. a MIPS32 CPU?
	if ((requiredISA == 3 || requiredISA == 4) && Is32Bit()) {
		// TODO: Cause MIPS "unimplemented instruction" exception instead.
		ic->f = NULL;

		// TODO: Only print the warning once; actual real-world code may
		// rely on this mechanism to detect cpu type, or similar.
		if (ui != NULL) {
			stringstream ss;
			ss.flags(std::ios::hex);
			ss << "instruction at 0x" << m_pc << " is a 64-bit instruction,"
			    " which cannot be executed on this CPU\n";
			ui->ShowDebugMessage(this, ss.str());
		}
	}

	// Attempting a revision 2 opcode on a revision1 MIPS32/64 CPU?
	if (requiredISArevision > 1 && m_type.isa_revision) {
		// TODO: Cause MIPS "unimplemented instruction" exception instead.
		ic->f = NULL;

		// TODO: Only print the warning once; actual real-world code may
		// rely on this mechanism to detect cpu type, or similar.
		if (ui != NULL) {
			stringstream ss;
			ss.flags(std::ios::hex);
			ss << "instruction at 0x" << m_pc << " is a MIPS32/64 revision ";
			ss << requiredISArevision << " instruction; this cpu supports"
			    " only revision " << m_type.isa_revision << "\n";
			ui->ShowDebugMessage(this, ss.str());
		}
	}
}


DYNTRANS_INSTR(MIPS_CPUComponent,ToBeTranslated)
{
	DYNTRANS_INSTR_HEAD(MIPS_CPUComponent)

	cpu->DyntransToBeTranslatedBegin(ic);

	uint32_t iword;
	if (cpu->DyntransReadInstruction(iword))
		cpu->Translate(iword, ic);

	if (cpu->m_inDelaySlot && ic->f == NULL)
		ic->f = instr_abort;

	cpu->DyntransToBeTranslatedDone(ic);
}


DYNTRANS_INSTR(MIPS_CPUComponent,ToBeTranslated_MIPS16)
{
	DYNTRANS_INSTR_HEAD(MIPS_CPUComponent)

	cpu->DyntransToBeTranslatedBegin(ic);

	uint16_t iword;
	if (cpu->DyntransReadInstruction(iword)) {
		// TODO: Recode
		UI* ui = cpu->GetUI();
		if (ui != NULL) {
			stringstream ss;
			ss.flags(std::ios::hex);
			ss << "TODO: recode MIPS16 => regular MIPS instruction\n";
			ui->ShowDebugMessage(cpu, ss.str());
		}

		//cpu->Translate(iword, ic);
	}

	cpu->DyntransToBeTranslatedDone(ic);
}


/*****************************************************************************/


#ifdef WITHUNITTESTS

#include "ComponentFactory.h"

static void Test_MIPS_CPUComponent_IsStable()
{
	UnitTest::Assert("the MIPS_CPUComponent should be stable",
	    ComponentFactory::HasAttribute("mips_cpu", "stable"));
}

static void Test_MIPS_CPUComponent_Create()
{
	refcount_ptr<Component> cpu =
	    ComponentFactory::CreateComponent("mips_cpu");
	UnitTest::Assert("component was not created?", !cpu.IsNULL());

	const StateVariable * p = cpu->GetVariable("pc");
	UnitTest::Assert("cpu has no pc state variable?", p != NULL);
	UnitTest::Assert("initial pc", p->ToString(), "0xffffffffbfc00000");
}

static void Test_MIPS_CPUComponent_IsCPU()
{
	refcount_ptr<Component> mips_cpu =
	    ComponentFactory::CreateComponent("mips_cpu");
	CPUComponent* cpu = mips_cpu->AsCPUComponent();
	UnitTest::Assert("mips_cpu is not a CPUComponent?", cpu != NULL);
}

static void Test_MIPS_CPUComponent_DefaultModel()
{
	refcount_ptr<Component> cpu =
	    ComponentFactory::CreateComponent("mips_cpu");

	// 5KE is a good default model (MIPS64 rev 2 ISA)
	UnitTest::Assert("wrong default model",
	    cpu->GetVariable("model")->ToString(), "5KE");
}

static void Test_MIPS_CPUComponent_ModelChange()
{
	refcount_ptr<Component> cpu =
	    ComponentFactory::CreateComponent("mips_cpu");

	cpu->SetVariableValue("model", "\"R2000\"");
	UnitTest::Assert("model change was not applied",
	    cpu->GetVariable("model")->ToString(), "R2000");

	cpu->SetVariableValue("model", "\"R1000\"");
	UnitTest::Assert("model change was applied when it should NOT have been",
	    cpu->GetVariable("model")->ToString(), "R2000");
}

static void Test_MIPS_CPUComponent_Disassembly_Basic()
{
	refcount_ptr<Component> mips_cpu =
	    ComponentFactory::CreateComponent("mips_cpu");
	CPUComponent* cpu = mips_cpu->AsCPUComponent();

	vector<string> result;
	size_t len;
	unsigned char instruction[sizeof(uint32_t)];
	// This assumes that the default endianness is BigEndian...
	instruction[0] = 0x27;
	instruction[1] = 0xbd;
	instruction[2] = 0xff;
	instruction[3] = 0xd8;

	len = cpu->DisassembleInstruction(0x12345678, sizeof(uint32_t),
	    instruction, result);

	UnitTest::Assert("disassembled instruction was wrong length?", len, 4);
	UnitTest::Assert("disassembly result incomplete?", result.size(), 3);
	UnitTest::Assert("disassembly result[0]", result[0], "27bdffd8");
	UnitTest::Assert("disassembly result[1]", result[1], "addiu");
	UnitTest::Assert("disassembly result[2]", result[2], "sp,sp,-40");
}

static void Test_MIPS_CPUComponent_Execute_DelayBranchWithValidInstruction()
{
	GXemul gxemul;
	gxemul.GetCommandInterpreter().RunCommand("add testmips");

	refcount_ptr<Component> cpu = gxemul.GetRootComponent()->LookupPath("root.machine0.mainbus0.cpu0");
	UnitTest::Assert("huh? no cpu?", !cpu.IsNULL());

	AddressDataBus* bus = cpu->AsAddressDataBus();
	UnitTest::Assert("cpu should be addressable", bus != NULL);

	uint32_t data32 = 0x10000111;	// b 0xffffffff80004448
	bus->AddressSelect(0xffffffff80004000ULL);
	bus->WriteData(data32, BigEndian);

	data32 = 0x27bdffd8; // Something valid, addiu sp,sp,-40
	bus->AddressSelect(0xffffffff80004004ULL);
	bus->WriteData(data32, BigEndian);

	cpu->SetVariableValue("pc", "0xffffffff80004000");
	UnitTest::Assert("setting pc failed?", cpu->GetVariable("pc")->ToInteger(), 0xffffffff80004000ULL);
	UnitTest::Assert("sp before execute", cpu->GetVariable("sp")->ToInteger(), 0xffffffffa0007f00ULL);

	// This tests that execute 2 steps will execute both the delay branch
	// and the delay slot instruction.
	gxemul.SetRunState(GXemul::Running);
	gxemul.Execute(2);

	UnitTest::Assert("pc should have changed", cpu->GetVariable("pc")->ToInteger(), 0xffffffff80004448ULL);
	UnitTest::Assert("sp should have changed", cpu->GetVariable("sp")->ToInteger(), 0xffffffffa0007ed8ULL);
}

static void Test_MIPS_CPUComponent_Execute_DelayBranchWithValidInstruction_SingleStepping()
{
	GXemul gxemul;
	gxemul.GetCommandInterpreter().RunCommand("add testmips");

	refcount_ptr<Component> cpu = gxemul.GetRootComponent()->LookupPath("root.machine0.mainbus0.cpu0");
	UnitTest::Assert("huh? no cpu?", !cpu.IsNULL());

	AddressDataBus* bus = cpu->AsAddressDataBus();
	UnitTest::Assert("cpu should be addressable", bus != NULL);

	uint32_t data32 = 0x10000111;	// b 0xffffffff80004448
	bus->AddressSelect(0xffffffff80004000ULL);
	bus->WriteData(data32, BigEndian);

	data32 = 0x27bdffd8; // Something valid, addiu sp,sp,-40
	bus->AddressSelect(0xffffffff80004004ULL);
	bus->WriteData(data32, BigEndian);

	cpu->SetVariableValue("pc", "0xffffffff80004000");
	UnitTest::Assert("setting pc failed?", cpu->GetVariable("pc")->ToInteger(), 0xffffffff80004000ULL);
	UnitTest::Assert("sp before execute", cpu->GetVariable("sp")->ToInteger(), 0xffffffffa0007f00ULL);

	// This tests that execute 2 steps will execute both the delay branch
	// and the delay slot instruction.
	gxemul.SetRunState(GXemul::SingleStepping);
	gxemul.Execute(1);

	// Should now be in the delay slot.
	UnitTest::Assert("pc should have changed 1", cpu->GetVariable("pc")->ToInteger(), 0xffffffff80004004ULL);
	UnitTest::Assert("delay slot", cpu->GetVariable("inDelaySlot")->ToString(), "true");
	UnitTest::Assert("sp should not yet have changed", cpu->GetVariable("sp")->ToInteger(), 0xffffffffa0007f00ULL);

	gxemul.SetRunState(GXemul::SingleStepping);
	gxemul.Execute(1);

	UnitTest::Assert("pc should have changed 2", cpu->GetVariable("pc")->ToInteger(), 0xffffffff80004448ULL);
	UnitTest::Assert("delay slot after branch", cpu->GetVariable("inDelaySlot")->ToString(), "false");
	UnitTest::Assert("sp should have changed", cpu->GetVariable("sp")->ToInteger(), 0xffffffffa0007ed8ULL);
}

static void Test_MIPS_CPUComponent_Execute_DelayBranchWithValidInstruction_RunTwoTimes()
{
	GXemul gxemul;
	gxemul.GetCommandInterpreter().RunCommand("add testmips");

	refcount_ptr<Component> cpu = gxemul.GetRootComponent()->LookupPath("root.machine0.mainbus0.cpu0");
	UnitTest::Assert("huh? no cpu?", !cpu.IsNULL());

	AddressDataBus* bus = cpu->AsAddressDataBus();
	UnitTest::Assert("cpu should be addressable", bus != NULL);

	uint32_t data32 = 0x10000111;	// b 0xffffffff80004448
	bus->AddressSelect(0xffffffff80004000ULL);
	bus->WriteData(data32, BigEndian);

	data32 = 0x27bdffd8; // Something valid, addiu sp,sp,-40
	bus->AddressSelect(0xffffffff80004004ULL);
	bus->WriteData(data32, BigEndian);

	cpu->SetVariableValue("pc", "0xffffffff80004000");
	UnitTest::Assert("setting pc failed?", cpu->GetVariable("pc")->ToInteger(), 0xffffffff80004000ULL);
	UnitTest::Assert("sp before execute", cpu->GetVariable("sp")->ToInteger(), 0xffffffffa0007f00ULL);

	// This tests that execute 1 step with normal running, two times, will
	// execute both the delay branch and the delay slot instruction correctly.
	gxemul.SetRunState(GXemul::Running);
	gxemul.Execute(1);

	// Should now be in the delay slot.
	UnitTest::Assert("pc should have changed 1", cpu->GetVariable("pc")->ToInteger(), 0xffffffff80004004ULL);
	UnitTest::Assert("delay slot", cpu->GetVariable("inDelaySlot")->ToString(), "true");
	UnitTest::Assert("sp should not yet have changed", cpu->GetVariable("sp")->ToInteger(), 0xffffffffa0007f00ULL);

	gxemul.SetRunState(GXemul::Running);
	gxemul.Execute(1);

	UnitTest::Assert("pc should have changed 2", cpu->GetVariable("pc")->ToInteger(), 0xffffffff80004448ULL);
	UnitTest::Assert("delay slot after branch", cpu->GetVariable("inDelaySlot")->ToString(), "false");
	UnitTest::Assert("sp should have changed", cpu->GetVariable("sp")->ToInteger(), 0xffffffffa0007ed8ULL);
}

static void Test_MIPS_CPUComponent_Execute_DelayBranchWithFault()
{
	GXemul gxemul;
	gxemul.GetCommandInterpreter().RunCommand("add testmips");

	refcount_ptr<Component> cpu = gxemul.GetRootComponent()->LookupPath("root.machine0.mainbus0.cpu0");
	UnitTest::Assert("huh? no cpu?", !cpu.IsNULL());

	AddressDataBus* bus = cpu->AsAddressDataBus();
	UnitTest::Assert("cpu should be addressable", bus != NULL);

	uint32_t data32 = 0x10000111;	// b 0xffffffff80004048
	bus->AddressSelect(0xffffffff80004000ULL);
	bus->WriteData(data32, BigEndian);

	data32 = 0xffffffff; // Something invalid.
	bus->AddressSelect(0xffffffff80004004ULL);
	bus->WriteData(data32, BigEndian);

	cpu->SetVariableValue("pc", "0xffffffff80004000");
	UnitTest::Assert("setting pc failed?", cpu->GetVariable("pc")->ToInteger(), 0xffffffff80004000ULL);
	UnitTest::Assert("should not be in delay slot before execution", cpu->GetVariable("inDelaySlot")->ToString(), "false");

	// This tests that execute 100 steps will only execute 1, if the instruction
	// in the delay slot fails.
	gxemul.SetRunState(GXemul::Running);
	gxemul.Execute(100);

	UnitTest::Assert("pc should have increased one step", cpu->GetVariable("pc")->ToInteger(), 0xffffffff80004004ULL);
	UnitTest::Assert("should be in delay slot after execution", cpu->GetVariable("inDelaySlot")->ToString(), "true");

	gxemul.SetRunState(GXemul::SingleStepping);
	gxemul.Execute(1);

	UnitTest::Assert("pc should not have increased", cpu->GetVariable("pc")->ToInteger(), 0xffffffff80004004ULL);
	UnitTest::Assert("should still be in delay slot", cpu->GetVariable("inDelaySlot")->ToString(), "true");
}

UNITTESTS(MIPS_CPUComponent)
{
	UNITTEST(Test_MIPS_CPUComponent_IsStable);
	UNITTEST(Test_MIPS_CPUComponent_Create);
	UNITTEST(Test_MIPS_CPUComponent_IsCPU);
	UNITTEST(Test_MIPS_CPUComponent_DefaultModel);
	UNITTEST(Test_MIPS_CPUComponent_ModelChange);

	// Disassembly:
	UNITTEST(Test_MIPS_CPUComponent_Disassembly_Basic);

	// Dyntrans execution:
	UNITTEST(Test_MIPS_CPUComponent_Execute_DelayBranchWithValidInstruction);
	UNITTEST(Test_MIPS_CPUComponent_Execute_DelayBranchWithValidInstruction_SingleStepping);
	UNITTEST(Test_MIPS_CPUComponent_Execute_DelayBranchWithValidInstruction_RunTwoTimes);
	UNITTEST(Test_MIPS_CPUComponent_Execute_DelayBranchWithFault);
}

#endif

