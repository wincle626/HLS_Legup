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

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <iomanip>

#include "ComponentFactory.h"
#include "GXemul.h"
#include "components/M88K_CPUComponent.h"

static const char* opcode_names[] = M88K_OPCODE_NAMES;
static const char* opcode_names_3c[] = M88K_3C_OPCODE_NAMES;
static const char* opcode_names_3d[] = M88K_3D_OPCODE_NAMES;
static m88k_cpu_type_def cpu_type_defs[] = M88K_CPU_TYPE_DEFS;

static const char *memop[4] = { ".d", "", ".h", ".b" };

static const char *m88k_cr_names[] = M88K_CR_NAMES;
//static const char *m88k_cr_197_names[] = M88K_CR_NAMES_197;

static const char *m88k_cr_name(int i)
{
	const char **cr_names = m88k_cr_names;

	// TODO: Is this really MVME197 specific? Or 88110?
	//if (cpu->machine->machine_subtype == MACHINE_MVME88K_197)
	//	cr_names = m88k_cr_197_names;

	return cr_names[i];
}


M88K_CPUComponent::M88K_CPUComponent()
	: CPUDyntransComponent("m88k_cpu", "Motorola 88000")
	, m_m88k_type("88100")
{
	m_frequency = 50e6;	// 50 MHz

	// Find (and cache) the cpu type in m_type:
	memset((void*) &m_type, 0, sizeof(m_type));
	for (size_t j=0; cpu_type_defs[j].name != NULL; j++) {
		if (m_m88k_type == cpu_type_defs[j].name) {
			m_type = cpu_type_defs[j];
			break;
		}
	}

	if (m_type.name == NULL) {
		std::cerr << "Internal error: Unimplemented M88K type?\n";
		throw std::exception();
	}

	AddVariable("model", &m_m88k_type);

	for (size_t i=0; i<N_M88K_REGS; i++) {
		stringstream ss;
		ss << "r" << i;
		AddVariable(ss.str(), &m_r[i]);
	}

	for (size_t i=0; i<N_M88K_CONTROL_REGS; i++) {
		stringstream ss;
		ss << "cr" << i;
		AddVariable(ss.str(), &m_cr[i]);
	}

	for (size_t i=0; i<N_M88K_FPU_CONTROL_REGS; i++) {
		stringstream ss;
		ss << "fcr" << i;
		AddVariable(ss.str(), &m_fcr[i]);
	}

	m_initial_r31 = 0x00000000;
	AddVariable("initial_r31", &m_initial_r31);

	AddVariable("inDelaySlot", &m_inDelaySlot);
	AddVariable("delaySlotTarget", &m_delaySlotTarget);

	ResetState();
}


refcount_ptr<Component> M88K_CPUComponent::Create(const ComponentCreateArgs& args)
{
	// Defaults:
	ComponentCreationSettings settings;
	settings["model"] = "88100";
	settings["r31"] = "0x00000000";

	if (!ComponentFactory::GetCreationArgOverrides(settings, args))
		return NULL;

	// Create the CPU...
	refcount_ptr<Component> cpu = new M88K_CPUComponent();

	// ... and apply settings:
	if (!cpu->SetVariableValue("model", "\"" + settings["model"] + "\""))
		return NULL;

	if (!cpu->SetVariableValue("initial_r31", settings["r31"]))
		return NULL;
	if (!cpu->SetVariableValue("r31", settings["r31"]))
		return NULL;

	return cpu;
}


void M88K_CPUComponent::ResetState()
{
	m_pageSize = 4096;

	// r0 .. r31 and the extra "r32/r0" zero register:
	for (size_t i=0; i<N_M88K_REGS+1; i++)
		m_r[i] = 0;

	// ... but change r31 to the initial stack pointer value:
	m_r[M88K_STACKPOINTER_REG] = m_initial_r31;

	for (size_t i=0; i<N_M88K_CONTROL_REGS; i++)
		m_cr[i] = 0;

	for (size_t i=0; i<N_M88K_FPU_CONTROL_REGS; i++)
		m_fcr[i] = 0;

	m_pc = 0;

	// Set the Processor ID:
	m_cr[M88K_CR_PID] = m_type.pid | M88K_PID_MC;

	// Start in supervisor mode, with interrupts disabled.
	m_cr[M88K_CR_PSR] = M88K_PSR_MODE | M88K_PSR_IND;
	if (!m_isBigEndian)
		m_cr[M88K_CR_PSR] |= M88K_PSR_BO;

	CPUDyntransComponent::ResetState();
}


bool M88K_CPUComponent::PreRunCheckForComponent(GXemul* gxemul)
{
	if (m_r[M88K_ZERO_REG] != 0) {
		gxemul->GetUI()->ShowDebugMessage(this, "the r0 register "
		    "must contain the value 0.\n");
		return false;
	}

	if (m_pc > (uint64_t)0xffffffff) {
		gxemul->GetUI()->ShowDebugMessage(this, "the pc register "
		    "must be a 32-bit value.\n");
		return false;
	}

	if (m_pc & 0x2) {
		gxemul->GetUI()->ShowDebugMessage(this, "the pc register must have"
		    " its lower two bits clear!\n");
		return false;
	}

	if (m_r[N_M88K_REGS] != 0) {
		gxemul->GetUI()->ShowDebugMessage(this, "internal error: the "
		    "register following r31 must mimic the r0 register.\nIf"
		    " you encounter this message, please write a bug report!\n");
		return false;
	}

	return CPUDyntransComponent::PreRunCheckForComponent(gxemul);
}


bool M88K_CPUComponent::CheckVariableWrite(StateVariable& var, const string& oldValue)
{
	UI* ui = GetUI();

	if (m_r[M88K_ZERO_REG] != 0) {
		if (ui != NULL) {
			ui->ShowDebugMessage(this, "the zero register (r0) "
			    "must contain the value 0.\n");
		}
		return false;
	}

	if (m_m88k_type != m_type.name) {
		bool found = false;
		for (size_t j=0; cpu_type_defs[j].name != NULL; j++) {
			if (m_m88k_type == cpu_type_defs[j].name) {
				m_type = cpu_type_defs[j];
				found = true;
				break;
			}
		}

		if (!found) {
			if (ui != NULL) {
				stringstream ss;
				ss << "Unknown model \"" + m_m88k_type + "\". Available types are:\n";
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


void M88K_CPUComponent::ShowRegisters(GXemul* gxemul, const vector<string>& arguments) const
{
	bool done = false;

	stringstream ss;
	ss.flags(std::ios::hex);

	if (arguments.size() == 0 ||
	    find(arguments.begin(), arguments.end(), "r") != arguments.end()) {
		ss << "   pc = 0x" << std::setfill('0') << std::setw(8) << m_pc;

		string symbol = GetSymbolRegistry().LookupAddress(m_pc, true);
		if (symbol != "")
			ss << "  <" << symbol << ">";
		ss << "\n";

		for (size_t i=0; i<N_M88K_REGS; i++) {
			stringstream regname;
			regname << "r" << i;
		
			ss << std::setfill(' ');
			ss << std::setw(5) << regname.str() << " = 0x";
			ss << std::setfill('0') << std::setw(8) << m_r[i];
			if ((i&3) == 3)
				ss << "\n";
			else
				ss << "  ";
		}

		done = true;
	}

	if (find(arguments.begin(), arguments.end(), "cr") != arguments.end()) {
		for (size_t i=0; i<N_M88K_CONTROL_REGS; i++) {
			stringstream regname;
			regname << "cr" << i;
		
			ss << std::setfill(' ');
			ss << std::setw(5) << regname.str() << " = 0x";
			ss << std::setfill('0') << std::setw(8) << m_cr[i];
			if ((i&3) == 3)
				ss << "\n";
			else
				ss << "  ";
		}

		done = true;
	}

	if (find(arguments.begin(), arguments.end(), "crn") != arguments.end()) {
		for (size_t i=0; i<N_M88K_CONTROL_REGS; i++) {
			ss << std::setfill(' ');
			ss << std::setw(5) << m88k_cr_name(i) << " = 0x";
			ss << std::setfill('0') << std::setw(8) << m_cr[i];
			if ((i&3) == 3)
				ss << "\n";
			else
				ss << "  ";
		}

		done = true;
	}

	if (find(arguments.begin(), arguments.end(), "fcr") != arguments.end()) {
		for (size_t i=0; i<N_M88K_FPU_CONTROL_REGS; i++) {
			stringstream regname;
			regname << "fcr" << i;
		
			ss << std::setfill(' ');
			ss << std::setw(5) << regname.str() << " = 0x";
			ss << std::setfill('0') << std::setw(8) << m_fcr[i];
			if ((i&3) == 3)
				ss << "\n";
			else
				ss << "  ";
		}

		done = true;
	}

	if (!done) {
		ss << "M88K usage: .registers [r] [cr] [crn] [fcr]\n"
		    "r   = pc and general purpose registers  (default)\n"
		    "cr  = control registers\n"
		    "crn = control registers with symbolic names instead of crX\n"
		    "fcr = floating point control registers\n";
	}

	gxemul->GetUI()->ShowDebugMessage(ss.str());
}


int M88K_CPUComponent::GetDyntransICshift() const
{
	// 4 bytes per instruction, i.e. shift is 2 bits.
	return M88K_INSTR_ALIGNMENT_SHIFT;
}


void (*M88K_CPUComponent::GetDyntransToBeTranslated())(CPUDyntransComponent*, DyntransIC*) const
{
	return instr_ToBeTranslated;
}


bool M88K_CPUComponent::VirtualToPhysical(uint64_t vaddr, uint64_t& paddr,
	bool& writable)
{
	// TODO. For now, just return paddr = vaddr.

	paddr = vaddr & 0xffffffff;
	writable = true;
	return true;
}


void M88K_CPUComponent::Exception(int vector, int is_trap)
{
	std::cerr << "TODO: M88K exception\n";
	throw std::exception();
}


size_t M88K_CPUComponent::DisassembleInstruction(uint64_t vaddr, size_t maxLen,
	unsigned char *instruction, vector<string>& result)
{
	const size_t instrSize = sizeof(uint32_t);

	if (maxLen < instrSize) {
		assert(false);
		return 0;
	}

	// Read the instruction word:
	uint32_t iw = *((uint32_t *) instruction);
	if (m_isBigEndian)
		iw = BE32_TO_HOST(iw);
	else
		iw = LE32_TO_HOST(iw);

	// ... and add it to the result:
	{
		stringstream ss;
		ss.flags(std::ios::hex);
		ss << std::setfill('0') << std::setw(8) << (uint32_t) iw;
		if (m_pc == vaddr && m_inDelaySlot)
			ss << " (delayslot)";
		result.push_back(ss.str());
	}

	uint32_t op26   = (iw >> 26) & 0x3f;
	uint32_t op11   = (iw >> 11) & 0x1f;
	uint32_t op10   = (iw >> 10) & 0x3f;
	uint32_t d      = (iw >> 21) & 0x1f;
	uint32_t s1     = (iw >> 16) & 0x1f;
	uint32_t s2     =  iw        & 0x1f;
	uint32_t op3d   = (iw >>  8) & 0xff;
	uint32_t imm16  = iw & 0xffff;
	uint32_t w5     = (iw >>  5) & 0x1f;
	uint32_t cr6    = (iw >>  5) & 0x3f;
	int32_t  d16    = ((int16_t) (iw & 0xffff)) * 4;
	int32_t  d26    = ((int32_t)((iw & 0x03ffffff) << 6)) >> 4;

	switch (op26) {

	case 0x00:	/*  xmem.bu  */
	case 0x01:	/*  xmem     */
	case 0x02:	/*  ld.hu    */
	case 0x03:	/*  ld.bu    */
	case 0x04:	/*  ld.d     */
	case 0x05:	/*  ld       */
	case 0x06:	/*  ld.h     */
	case 0x07:	/*  ld.b     */
	case 0x08:	/*  st.d     */
	case 0x09:	/*  st       */
	case 0x0a:	/*  st.h     */
	case 0x0b:	/*  st.b     */
	case 0x10:	/*  and     */
	case 0x11:	/*  and.u   */
	case 0x12:	/*  mask    */
	case 0x13:	/*  mask.u  */
	case 0x14:	/*  xor     */
	case 0x15:	/*  xor.u   */
	case 0x16:	/*  or      */
	case 0x17:	/*  or.u    */
	case 0x18:	/*  addu    */
	case 0x19:	/*  subu    */
	case 0x1a:	/*  divu    */
	case 0x1b:	/*  mulu    */
	case 0x1c:	/*  add    */
	case 0x1d:	/*  sub    */
	case 0x1e:	/*  div    */
	case 0x1f:	/*  cmp    */
		if (iw == 0x00000000) {
			result.push_back("-");
		} else {
			// Two registers (d, s1) and an immediate.
			result.push_back(opcode_names[op26]);

			stringstream ss;
			ss << "r" << d << ",r" << s1;
			ss.flags(std::ios::hex | std::ios::showbase);
			ss << "," << imm16;
			result.push_back(ss.str());
		}
		break;

	case 0x20:
		if ((iw & 0x001ff81f) == 0x00004000) {
			result.push_back("ldcr");
			stringstream ss;
			ss << "r" << d << ",cr" << cr6;
			result.push_back(ss.str());

			stringstream comment;
			comment << "; cr" << cr6 << " = " << m88k_cr_name(cr6);
			result.push_back(comment.str());
		} else if ((iw & 0x001ff81f) == 0x00004800) {
			result.push_back("fldcr");
			stringstream ss;
			ss << "r" << d << ",fcr" << cr6;
			result.push_back(ss.str());
		} else if ((iw & 0x03e0f800) == 0x00008000) {
			result.push_back("stcr");
			stringstream ss;
			ss << "r" << s1 << ",cr" << cr6;
			result.push_back(ss.str());
			if (s1 != s2)
				result.push_back("; Weird encoding: s1 != s2");

			stringstream comment;
			comment << "; cr" << cr6 << " = " << m88k_cr_name(cr6);
			result.push_back(comment.str());
		} else if ((iw & 0x03e0f800) == 0x00008800) {
			result.push_back("fstcr");
			stringstream ss;
			ss << "r" << s1 << ",fcr" << cr6;
			result.push_back(ss.str());
			if (s1 != s2)
				result.push_back("; Weird encoding: s1 != s2");
		} else if ((iw & 0x0000f800) == 0x0000c000) {
			result.push_back("xcr");
			stringstream ss;
			ss << "r" << d << ",r" << s1 << ",cr" << cr6;
			result.push_back(ss.str());
			if (s1 != s2)
				result.push_back("; Weird encoding: s1 != s2");

			stringstream comment;
			comment << "; cr" << cr6 << " = " << m88k_cr_name(cr6);
			result.push_back(comment.str());
		} else if ((iw & 0x0000f800) == 0x0000c800) {
			result.push_back("fxcr");
			stringstream ss;
			ss << "r" << d << ",r" << s1 << ",fcr" << cr6;
			result.push_back(ss.str());
			if (s1 != s2)
				result.push_back("; Weird encoding: s1 != s2");
		} else {
			result.push_back("unimpl_0x20_variant");
		}
		break;

	case 0x21:
		switch (op11) {
		case 0x00:	/*  fmul  */
		case 0x05:	/*  fadd  */
		case 0x06:	/*  fsub  */
		case 0x07:	/*  fcmp  */
		case 0x0e:	/*  fdiv  */
			{
				stringstream ss;
				switch (op11) {
				case 0x00: ss << "fmul"; break;
				case 0x05: ss << "fadd"; break;
				case 0x06: ss << "fsub"; break;
				case 0x07: ss << "fcmp"; break;
				case 0x0e: ss << "fdiv"; break;
				}
				ss << "." <<
				    (((iw >> 5) & 1)? "d" : "s") <<
				    (((iw >> 9) & 1)? "d" : "s") <<
				    (((iw >> 7) & 1)? "d" : "s");
				result.push_back(ss.str());

				stringstream ss2;
				ss2 << "r" << d << ",r" << s1 << ",r" << s2;
				result.push_back(ss2.str());
			}
			break;
		case 0x04:	/*  flt  */
			{
				stringstream ss;
				switch (op11) {
				case 0x04: ss << "flt"; break;
				}
				ss << "." << (((iw >> 5) & 1)? "d" : "s") << "s";
				result.push_back(ss.str());

				stringstream ss2;
				ss2 << "r" << d << ",r" << s2;
				result.push_back(ss2.str());
			}
			break;
		case 0x09:	/*  int  */
		case 0x0a:	/*  nint  */
		case 0x0b:	/*  trnc  */
			{
				stringstream ss;
				switch (op11) {
				case 0x09: ss << "int"; break;
				case 0x0a: ss << "nint"; break;
				case 0x0b: ss << "trnc"; break;
				}
				ss << ".s" << (((iw >> 7) & 1)? "d" : "s");
				result.push_back(ss.str());

				stringstream ss2;
				ss2 << "r" << d << ",r" << s2;
				result.push_back(ss2.str());
			}
			break;
		default:{
				stringstream ss;
				ss << "unimpl_0x21, op11=" << op11;
				result.push_back(ss.str());
			}
		}
		break;

	case 0x30:	/*  br  */
	case 0x31:	/*  br.n  */
	case 0x32:	/*  bsr  */
	case 0x33:	/*  bsr.n  */
		{
			result.push_back(opcode_names[op26]);

			stringstream ss;
			ss.flags(std::ios::hex | std::ios::showbase);
			ss << ((uint32_t) (vaddr + d26));
			result.push_back(ss.str());

			string symbol = GetSymbolRegistry().LookupAddress(
			    (uint32_t) (vaddr + d26), true);
			if (symbol != "")
				result.push_back("; <" + symbol + ">");
		}
		break;

	case 0x34:	/*  bb0    */
	case 0x35:	/*  bb0.n  */
	case 0x36:	/*  bb1    */
	case 0x37:	/*  bb1.n  */
	case 0x3a:	/*  bcnd    */
	case 0x3b:	/*  bcnd.n  */
		{
			result.push_back(opcode_names[op26]);

			stringstream ss;
			if (op26 == 0x3a || op26 == 0x3b) {
				/*  Attempt to decode bcnd condition:  */
				switch (d) {
				case 0x1: ss << "gt0"; break;
				case 0x2: ss << "eq0"; break;
				case 0x3: ss << "ge0"; break;
				case 0x7: ss << "not_maxneg"; break;
				case 0x8: ss << "maxneg"; break;
				case 0xc: ss << "lt0"; break;
				case 0xd: ss << "ne0"; break;
				case 0xe: ss << "le0"; break;
				default:  ss << "unk_" << d;
				}
			} else {
				ss << d;
			}

			ss << ",r" << s1 << ",";

			ss.flags(std::ios::hex | std::ios::showbase);
			ss << ((uint32_t) (vaddr + d16));
			result.push_back(ss.str());

			string symbol = GetSymbolRegistry().LookupAddress(
			    (uint32_t) (vaddr + d16), true);
			if (symbol != "")
				result.push_back("; <" + symbol + ">");
		}

		break;

	case 0x3c:
		if ((iw & 0x0000f000)==0x1000 || (iw & 0x0000f000)==0x2000) {
			/*  Load/store:  */
			stringstream ss;
			ss << ((iw & 0x0000f000) == 0x1000? "ld" : "st");

			switch (iw & 0x00000c00) {
			case 0x000: ss << ".d"; break;
			case 0x400: break;
			case 0x800: ss << ".x"; break;
			default: ss << ".UNIMPLEMENTED";
			}

			if (iw & 0x100)
				ss << ".usr";
			if (iw & 0x80)
				ss << ".wt";

			result.push_back(ss.str());

			stringstream ss2;
			ss2 << "r" << d << ",r" << s1;
			if (iw & 0x200)
				ss2 << "[r" << s2 << "]";
			else
				ss2 << ",r" << s2;

			result.push_back(ss2.str());
		} else switch (op10) {
		case 0x20:	/*  clr  */
		case 0x22:	/*  set  */
		case 0x24:	/*  ext  */
		case 0x26:	/*  extu  */
		case 0x28:	/*  mak  */
		case 0x2a:	/*  rot  */
			/*  Two-register plus bit position/length:  */
			{
				result.push_back(opcode_names_3c[op10]);

				stringstream ss;
				ss << "r" << d << ",r" << s1 << ",";

				/*  Don't include w5 for the rot instruction:  */
				if (op10 != 0x2a)
					ss << w5;

				/*  Note: o5 = s2:  */
				ss << "<" << s2 << ">";

				result.push_back(ss.str());
			}
			break;
		case 0x34:	/*  tb0  */
		case 0x36:	/*  tb1  */
			/*  B5 bit index, register, plus 9-bit immediate vector:  */
			{
				result.push_back(opcode_names_3c[op10]);

				stringstream ss;
				ss << d << ",r" << s1 << ",";
				ss.flags(std::ios::hex | std::ios::showbase);
				ss << (iw & 0x1ff);
				result.push_back(ss.str());
			}
			break;
		default:{
				stringstream ss;
				ss << "unimpl_" << opcode_names_3c[op10];
				result.push_back(ss.str());
			}
		}
		break;

	case 0x3d:
		if ((iw & 0xf000) <= 0x3fff) {
			/*  Load, Store, xmem, and lda:  */
			stringstream op;
			
			switch (iw & 0xf000) {
			case 0x2000: op << "st"; break;
			case 0x3000: op << "lda"; break;
			default:     if ((iw & 0xf800) >= 0x0800)
					op << "ld";
				     else
					op << "xmem";
			}
			
			if ((iw & 0xf000) >= 0x1000) {
				/*  ld, st, lda  */
				op << memop[(iw >> 10) & 3];
			} else if ((iw & 0xf800) == 0x0000) {
				/*  xmem  */
				if (!(iw & 0x400))
					op << ".bu";
			} else {
				/*  ld  */
				if ((iw & 0xf00) < 0xc00)
					op << ".hu";
				else
					op << ".bu";
			}
			
			if (iw & 0x100)
				op << ".usr";
			if (iw & 0x80)
				op << ".wt";

			result.push_back(op.str());

			stringstream ss;
			ss << "r" << d << ",r" << s1;
			if (iw & 0x200)
				ss << "[r" << s2 << "]";
			else
				ss << ",r" << s2;

			result.push_back(ss.str());
		} else switch (op3d) {
		case 0x40:	/*  and  */
		case 0x44:	/*  and.c  */
		case 0x50:	/*  xor  */
		case 0x54:	/*  xor.c  */
		case 0x58:	/*  or  */
		case 0x5c:	/*  or.c  */
		case 0x60:	/*  addu  */
		case 0x61:	/*  addu.co  */
		case 0x62:	/*  addu.ci  */
		case 0x63:	/*  addu.cio  */
		case 0x64:	/*  subu  */
		case 0x65:	/*  subu.co  */
		case 0x66:	/*  subu.ci  */
		case 0x67:	/*  subu.cio  */
		case 0x68:	/*  divu  */
		case 0x69:	/*  divu.d  */
		case 0x6c:	/*  mul  */
		case 0x6d:	/*  mulu.d  */
		case 0x6e:	/*  muls  */
		case 0x70:	/*  add  */
		case 0x71:	/*  add.co  */
		case 0x72:	/*  add.ci  */
		case 0x73:	/*  add.cio  */
		case 0x74:	/*  sub  */
		case 0x75:	/*  sub.co  */
		case 0x76:	/*  sub.ci  */
		case 0x77:	/*  sub.cio  */
		case 0x78:	/*  div  */
		case 0x7c:	/*  cmp  */
		case 0x80:	/*  clr  */
		case 0x88:	/*  set  */
		case 0x90:	/*  ext  */
		case 0x98:	/*  extu  */
		case 0xa0:	/*  mak  */
		case 0xa8:	/*  rot  */
			/*  Three-register opcodes:  */
			{
				result.push_back(opcode_names_3d[op3d]);

				stringstream ss;
				ss << "r" << d << ",r" << s1 << ",r" << s2;
				result.push_back(ss.str());
			}
			break;
		case 0xc0:	/*  jmp  */
		case 0xc4:	/*  jmp.n  */
		case 0xc8:	/*  jsr  */
		case 0xcc:	/*  jsr.n  */
			/*  One-register jump opcodes:  */
			{
				result.push_back(opcode_names_3d[op3d]);

				stringstream ss;
				ss << "(r" << s2 << ")";
				result.push_back(ss.str());
			}
			break;
		case 0xe8:	/*  ff1  */
		case 0xec:	/*  ff0  */
			/*  Two-register opcodes d,s2:  */
			{
				result.push_back(opcode_names_3d[op3d]);

				stringstream ss;
				ss << "r" << d << ",r" << s2;
				result.push_back(ss.str());
			}
			break;
		case 0xf8:	/*  tbnd  */
			/*  Two-register opcodes s1,s2:  */
			{
				result.push_back(opcode_names_3d[op3d]);

				stringstream ss;
				ss << "r" << s1 << ",r" << s2;
				result.push_back(ss.str());
			}
			break;
		case 0xfc:
			switch (iw & 0xff) {
			case 0x00:
				result.push_back("rte");
				break;
			case 0x01:
			case 0x02:
			case 0x03:
				{
					stringstream ss;
					ss << "illop" << (iw & 0xff);
					result.push_back(ss.str());
				}
				break;
			case (M88K_PROM_INSTR & 0xff):
				result.push_back("gxemul_prom_call");
				break;
			case (M88K_FAIL_EARLY_INSTR & 0xff):
				result.push_back("gxemul_fail_early");
				break;
			case (M88K_FAIL_LATE_INSTR & 0xff):
				result.push_back("gxemul_fail_late");
				break;
			default:{
					stringstream ss;
					ss << "unimpl_3d_0xfc_" << (iw & 0xff);
					result.push_back(ss.str());
				}
			}
			break;
		default:{
				stringstream ss;
				ss << "unimpl_" << opcode_names_3d[op3d];
				result.push_back(ss.str());
			}
		}
		break;

	case 0x3e:	/*  tbnd  */
		{
			result.push_back(opcode_names[op26]);

			stringstream ss;
			ss << "r" << s1;
			ss.flags(std::ios::hex | std::ios::showbase);
			ss << "," << imm16;
			result.push_back(ss.str());
		}
		break;

	default:
		{
			stringstream ss;
			ss << "unimpl_" << opcode_names[op26];
			result.push_back(ss.str());
		}
		break;
	}

	return instrSize;
}


string M88K_CPUComponent::GetAttribute(const string& attributeName)
{
	if (attributeName == "stable")
		return "yes";

	if (attributeName == "description")
		return "Motorola 88000 processor.";

	return Component::GetAttribute(attributeName);
}


/*****************************************************************************/


void M88K_CPUComponent::stcr(int cr, uint32_t value, bool is_rte)
{
	uint32_t old = m_cr[cr];

	switch (cr) {

	case M88K_CR_PSR:	/*  Processor Status Regoster  */
		if ((!m_isBigEndian && !(value & M88K_PSR_BO)) ||
		    (m_isBigEndian && (value & M88K_PSR_BO))) {
			std::cerr << "TODO: attempt to change endianness by flipping"
			    " the endianness bit in the PSR. How should this"
			    " be handled? Aborting.\n";
			std::cerr << "TODO: abort in a nicer way\n";
			throw std::exception();
		}

		if (!is_rte && (old & M88K_PSR_MODE) && !(value & M88K_PSR_MODE)) {
			UI* ui = GetUI();
			if (ui != NULL) {
				ui->ShowDebugMessage(this, "m88k stcr: WARNING! the PSR_MODE bit is being"
				    " cleared; this should be done using the RTE "
				    "instruction only, according to the M88100 "
				    "manual! Continuing anyway.\n");
			}
		}

		if (value & M88K_PSR_MXM) {
			std::cerr << "m88k stcr: TODO: MXM support\n";
			std::cerr << "TODO: abort in a nicer way\n";
			throw std::exception();
		}

		if ((old & M88K_PSR_MODE) != (value & M88K_PSR_MODE)) {
//			cpu->invalidate_translation_caches(
//			    cpu, 0, INVALIDATE_ALL);
			std::cerr << "m88k stcr: TODO: PSR mode switch.\n";
			std::cerr << "TODO: abort in a nicer way\n";
			throw std::exception();
		}

		m_cr[cr] = value;
		break;

	case M88K_CR_EPSR:
		m_cr[cr] = value;
		break;

	case M88K_CR_SXIP:
	case M88K_CR_SNIP:
	case M88K_CR_SFIP:
		m_cr[cr] = value;
		break;

	case M88K_CR_SSBR:	/*  Shadow ScoreBoard Register  */
		if (value & 1) {
			UI* ui = GetUI();
			if (ui != NULL)
				ui->ShowDebugMessage(this, "WARNING! bit 0 non-zero when writing to SSBR\n");
		}

		m_cr[cr] = value;
		break;

	case M88K_CR_VBR:
		if (value & 0x00000fff) {
			UI* ui = GetUI();
			if (ui != NULL)
				ui->ShowDebugMessage(this, "WARNING! bits 0..11 non-zero when writing to VBR\n");
		}

		m_cr[cr] = value;
		break;

	case M88K_CR_DMT0:
	case M88K_CR_DMT1:
	case M88K_CR_DMT2:
		m_cr[cr] = value;
		break;

	case M88K_CR_SR0:	/*  Supervisor Storage Registers 0..3  */
	case M88K_CR_SR1:
	case M88K_CR_SR2:
	case M88K_CR_SR3:
		m_cr[cr] = value;
		break;

	default:std::cerr << "m88k stcr: UNIMPLEMENTED cr = " << cr << "\n";
		std::cerr << "TODO: abort in a nicer way\n";
		throw std::exception();
	}
}


/*
 *  cmp_imm:  Compare S1 with immediate value.
 *  cmp:      Compare S1 with S2.
 *
 *  arg[0] = pointer to register d
 *  arg[1] = pointer to register s1
 *  arg[2] = pointer to register s2 or imm
 */
void M88K_CPUComponent::m88k_cmp(struct DyntransIC *ic, uint32_t y)
{
	uint32_t x = REG32(ic->arg[1]);
	uint32_t r;

	if (x == y) {
		r = M88K_CMP_HS | M88K_CMP_LS | M88K_CMP_GE
		  | M88K_CMP_LE | M88K_CMP_EQ;
	} else {
		if (x > y)
			r = M88K_CMP_NE | M88K_CMP_HS | M88K_CMP_HI;
		else
			r = M88K_CMP_NE | M88K_CMP_LO | M88K_CMP_LS;
		if ((int32_t)x > (int32_t)y)
			r |= M88K_CMP_GE | M88K_CMP_GT;
		else
			r |= M88K_CMP_LT | M88K_CMP_LE;
	}

	REG32(ic->arg[0]) = r;
}


DYNTRANS_INSTR(M88K_CPUComponent,cmp)
{
	DYNTRANS_INSTR_HEAD(M88K_CPUComponent)
	cpu->m88k_cmp(ic, REG32(ic->arg[2]));
}


DYNTRANS_INSTR(M88K_CPUComponent,cmp_imm)
{
	DYNTRANS_INSTR_HEAD(M88K_CPUComponent)
	cpu->m88k_cmp(ic, ic->arg[2].u32);
}


/*
 *  extu_imm:  Extract bits, unsigned, immediate W<O>.
 *  extu:      Extract bits, unsigned, W<O> taken from register s2.
 *  ext_imm:   Extract bits, signed, immediate W<O>.
 *  ext:       Extract bits, signed, W<O> taken from register s2.
 *
 *  arg[0] = pointer to register d
 *  arg[1] = pointer to register s1
 *  arg[2] = pointer to register s2 or 10 bits wwwwwooooo
 */
void M88K_CPUComponent::m88k_extu(struct DyntransIC *ic, int w, int o)
{
	uint32_t x = REG32(ic->arg[1]) >> o;
	if (w != 0) {
		x <<= (32-w);
		x >>= (32-w);
	}

	REG32(ic->arg[0]) = x;
}
void M88K_CPUComponent::m88k_ext(struct DyntransIC *ic, int w, int o)
{
	int32_t x = REG32(ic->arg[1]);
	x >>= o;	/*  signed (arithmetic) shift  */
	if (w != 0) {
		x <<= (32-w);
		x >>= (32-w);
	}

	REG32(ic->arg[0]) = x;
}
DYNTRANS_INSTR(M88K_CPUComponent,extu_imm)
{
	DYNTRANS_INSTR_HEAD(M88K_CPUComponent)
	cpu->m88k_extu(ic, ic->arg[2].u32 >> 5, ic->arg[2].u32 & 0x1f);
}
DYNTRANS_INSTR(M88K_CPUComponent,extu)
{
	DYNTRANS_INSTR_HEAD(M88K_CPUComponent)
	cpu->m88k_extu(ic, (REG32(ic->arg[2]) >> 5) & 0x1f, REG32(ic->arg[2]) & 0x1f);
}
DYNTRANS_INSTR(M88K_CPUComponent,ext_imm)
{
	DYNTRANS_INSTR_HEAD(M88K_CPUComponent)
	cpu->m88k_ext(ic, ic->arg[2].u32 >> 5, ic->arg[2].u32 & 0x1f);
}
DYNTRANS_INSTR(M88K_CPUComponent,ext)
{
	DYNTRANS_INSTR_HEAD(M88K_CPUComponent)
	cpu->m88k_ext(ic, (REG32(ic->arg[2]) >> 5) & 0x1f, REG32(ic->arg[2]) & 0x1f);
}


/*
 *  mak:      Make bit field, W<O> taken from register s2.
 *  mak_imm:  Make bit field, immediate W<O>.
 *
 *  arg[0] = pointer to register d
 *  arg[1] = pointer to register s1
 *  arg[2] = pointer to register s2 or immediate.
 */
void M88K_CPUComponent::m88k_mak(struct DyntransIC *ic, int w, int o)
{
	uint32_t x = REG32(ic->arg[1]);
	if (w != 0) {
		x <<= (32-w);
		x >>= (32-w);
	}

	REG32(ic->arg[0]) = x << o;
}


DYNTRANS_INSTR(M88K_CPUComponent,mak)
{
	DYNTRANS_INSTR_HEAD(M88K_CPUComponent)
	cpu->m88k_mak(ic, (REG32(ic->arg[2]) >> 5) & 0x1f, REG32(ic->arg[2]) & 0x1f);
}


DYNTRANS_INSTR(M88K_CPUComponent,mak_imm)
{
	DYNTRANS_INSTR_HEAD(M88K_CPUComponent)
	cpu->m88k_mak(ic, ic->arg[2].u32 >> 5, ic->arg[2].u32 & 0x1f);
}


/*
 *  divu_imm:  d = s1 / immediate
 *  mulu_imm:  d = s1 * immediate
 *
 *  arg[0] = pointer to register d
 *  arg[1] = pointer to register s1
 *  arg[2] = immediate.
 */
DYNTRANS_INSTR(M88K_CPUComponent,divu_imm)
{
	DYNTRANS_INSTR_HEAD(M88K_CPUComponent)

	// TODO: 88100 only, not 88110:
	if (cpu->m_cr[M88K_CR_PSR] & M88K_PSR_SFD1) {
		DYNTRANS_SYNCH_PC;
		cpu->m_fcr[M88K_FPCR_FPECR] = M88K_FPECR_FUNIMP;
		cpu->Exception(M88K_EXCEPTION_SFU1_PRECISE, 0);
	} else if (ic->arg[2].u32 == 0) {
		DYNTRANS_SYNCH_PC;
		cpu->Exception(M88K_EXCEPTION_ILLEGAL_INTEGER_DIVIDE, 0);
	} else {
		REG32(ic->arg[0]) = REG32(ic->arg[1]) / ic->arg[2].u32;
	}
}
DYNTRANS_INSTR(M88K_CPUComponent,mulu_imm)
{
	DYNTRANS_INSTR_HEAD(M88K_CPUComponent)

	// TODO: 88100 only, not 88110:
	if (cpu->m_cr[M88K_CR_PSR] & M88K_PSR_SFD1) {
		DYNTRANS_SYNCH_PC;
		cpu->m_fcr[M88K_FPCR_FPECR] = M88K_FPECR_FUNIMP;
		cpu->Exception(M88K_EXCEPTION_SFU1_PRECISE, 0);
	} else {
		REG32(ic->arg[0]) = REG32(ic->arg[1]) * ic->arg[2].u32;
	}
}


DYNTRANS_INSTR(M88K_CPUComponent,bsr)
{
	DYNTRANS_INSTR_HEAD(M88K_CPUComponent)

	cpu->m_pc &= ~((M88K_IC_ENTRIES_PER_PAGE-1) << M88K_INSTR_ALIGNMENT_SHIFT);
	cpu->m_r[M88K_RETURN_REG] = cpu->m_pc + ic->arg[2].u32;

	cpu->m_pc = (uint32_t) (cpu->m_pc + ic->arg[1].u32);
	cpu->DyntransPCtoPointers();
}


DYNTRANS_INSTR(M88K_CPUComponent,bsr_samepage)
{
	DYNTRANS_INSTR_HEAD(M88K_CPUComponent)

	cpu->m_r[M88K_RETURN_REG] = (cpu->m_pc & ~((M88K_IC_ENTRIES_PER_PAGE-1)
	    << M88K_INSTR_ALIGNMENT_SHIFT)) + ic->arg[2].u32;
	cpu->m_nextIC = (struct DyntransIC *) ic->arg[0].p;
}


DYNTRANS_INSTR(M88K_CPUComponent,bsr_functioncalltrace)
{
	DYNTRANS_INSTR_HEAD(M88K_CPUComponent)

	cpu->m_pc &= ~((M88K_IC_ENTRIES_PER_PAGE-1) << M88K_INSTR_ALIGNMENT_SHIFT);
	cpu->m_r[M88K_RETURN_REG] = cpu->m_pc + ic->arg[2].u32;

	cpu->m_pc = (uint32_t) (cpu->m_pc + ic->arg[1].u32);

	bool continueExecution = cpu->FunctionTraceCall();
	cpu->DyntransPCtoPointers();

	if (!continueExecution)
		cpu->m_nextIC = &cpu->m_abortIC;
}


DYNTRANS_INSTR(M88K_CPUComponent,bsr_n)
{
	DYNTRANS_INSTR_HEAD(M88K_CPUComponent)

	uint32_t startOfPage = cpu->m_pc & ~((M88K_IC_ENTRIES_PER_PAGE-1) << M88K_INSTR_ALIGNMENT_SHIFT);
	cpu->m_r[M88K_RETURN_REG] = startOfPage + ic->arg[2].u32;

	// Prepare for the branch.
	cpu->m_exceptionOrAbortInDelaySlot = false;
	cpu->m_inDelaySlot = true;
	cpu->m_delaySlotTarget = (uint32_t) (startOfPage + ic->arg[1].u32);

	// Execute the next instruction:
	ic[1].f(cpu, ic+1);
	cpu->m_executedCycles ++;

	// If there was no exception, then branch:
	if (!cpu->m_exceptionOrAbortInDelaySlot) {
		cpu->m_pc = (uint32_t) (startOfPage + ic->arg[1].u32);
		cpu->DyntransPCtoPointers();

		cpu->m_inDelaySlot = false;
	}

	// The next instruction is now either the target of the branch
	// instruction, or the first instruction of an exception handler.
	cpu->m_exceptionOrAbortInDelaySlot = false;
}


DYNTRANS_INSTR(M88K_CPUComponent,bsr_n_functioncalltrace)
{
	DYNTRANS_INSTR_HEAD(M88K_CPUComponent)

	uint32_t startOfPage = cpu->m_pc & ~((M88K_IC_ENTRIES_PER_PAGE-1) << M88K_INSTR_ALIGNMENT_SHIFT);
	cpu->m_r[M88K_RETURN_REG] = startOfPage + ic->arg[2].u32;

	// Prepare for the branch.
	cpu->m_inDelaySlot = true;
	cpu->m_exceptionOrAbortInDelaySlot = false;

	// Execute the next instruction:
	ic[1].f(cpu, ic+1);
	cpu->m_executedCycles ++;

	// If there was no exception, then branch:
	if (!cpu->m_exceptionOrAbortInDelaySlot) {
		cpu->m_pc = (uint32_t) (startOfPage + ic->arg[1].u32);

		bool continueExecution = cpu->FunctionTraceCall();
		cpu->DyntransPCtoPointers();

		cpu->m_inDelaySlot = false;

		if (!continueExecution)
			cpu->m_nextIC = &cpu->m_abortIC;
	}

	// The next instruction is now either the target of the branch
	// instruction, or the first instruction of an exception handler.
	cpu->m_exceptionOrAbortInDelaySlot = false;
}


// Note: This IC function is used both when function call trace is enabled
// and disabled. (Ok, since it is only used when singlestepping.)
DYNTRANS_INSTR(M88K_CPUComponent,bsr_n_functioncalltrace_singlestep)
{
	DYNTRANS_INSTR_HEAD(M88K_CPUComponent)

	// Prepare for the delayed branch.
	cpu->m_inDelaySlot = true;
	cpu->m_exceptionOrAbortInDelaySlot = false;

	cpu->m_pc &= ~((M88K_IC_ENTRIES_PER_PAGE-1) << M88K_INSTR_ALIGNMENT_SHIFT);
	cpu->m_r[M88K_RETURN_REG] = cpu->m_pc + ic->arg[2].u32;

	uint32_t old_pc = cpu->m_pc;

	cpu->m_pc = (uint32_t) (cpu->m_pc + ic->arg[1].u32);

	if (cpu->m_showFunctionTraceCall)
		cpu->FunctionTraceCall();

	cpu->m_delaySlotTarget = cpu->m_pc;

	// make m_nextIC (and pc!) point to the next instruction:
	cpu->m_nextIC = ic + 1;
	cpu->m_pc = old_pc;	// at least the same page... not necessarily more correct than that.
}


/*
 *  bcnd, bcnd.n:  Branch on condition
 *
 *  arg[0] = pointer to register s1
 *  arg[2] = offset from start of current page to branch to _OR_ pointer to new instr_call
 */
template<bool n, int op, bool singlestep> void M88K_CPUComponent::instr_bcnd(CPUDyntransComponent* cpubase, DyntransIC* ic)
{
	DYNTRANS_INSTR_HEAD(M88K_CPUComponent)

	bool cond;
	if (op == 1)		cond = ((int32_t)REG32(ic->arg[0]) > 0);		// gt0
	else if (op == 2)	cond = ((int32_t)REG32(ic->arg[0]) == 0);		// eq0
	else if (op == 3)	cond = ((int32_t)REG32(ic->arg[0]) >= 0);		// ge0
	else if (op == 7)	cond = ((uint32_t)REG32(ic->arg[0]) != 0x80000000UL);	// not_maxneg
	else if (op == 8)	cond = ((uint32_t)REG32(ic->arg[0]) == 0x80000000UL);	// maxneg
	else if (op == 12)	cond = ((int32_t)REG32(ic->arg[0]) < 0);		// lt0
	else if (op == 13)	cond = ((int32_t)REG32(ic->arg[0]) != 0);		// ne0
	else /* op == 14 */	cond = ((int32_t)REG32(ic->arg[0]) <= 0);		// le0

	if (n) {
		if (singlestep) {
			DYNTRANS_SYNCH_PC;
		
			// Prepare for the branch.
			cpu->m_inDelaySlot = true;
			cpu->m_exceptionOrAbortInDelaySlot = false;

			if (cond) {
				cpu->m_delaySlotTarget = cpu->m_pc & ~((M88K_IC_ENTRIES_PER_PAGE-1) << M88K_INSTR_ALIGNMENT_SHIFT);
				cpu->m_delaySlotTarget = (uint32_t) (cpu->m_delaySlotTarget + ic->arg[2].u32);
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
					cpu->m_pc &= ~((M88K_IC_ENTRIES_PER_PAGE-1) << M88K_INSTR_ALIGNMENT_SHIFT);
					cpu->m_pc = (uint32_t) (cpu->m_pc + ic->arg[2].u32);
					cpu->DyntransPCtoPointers();
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
	} else {
		// bcnd without the .n flag:
		if (cond) {
			cpu->m_pc &= ~((M88K_IC_ENTRIES_PER_PAGE-1) << M88K_INSTR_ALIGNMENT_SHIFT);
			cpu->m_pc = (uint32_t) (cpu->m_pc + ic->arg[2].u32);
			cpu->DyntransPCtoPointers();
		} else {
			// m_nextIC should already point to the next ic.
		}
	}
}


/*
 *  bb0, bb1:  Branch if a bit in a register is 0 or 1
 *  bb0.n, bb1.n:  Branch if a bit in a register is 0 or 1 with delay slot
 *
 *  arg[0] = pointer to register s1
 *  arg[1] = uint32_t bitmask to test
 *  arg[2] = offset from start of current page to branch to _OR_ pointer to new instr_call
 */
template<bool one, bool samepage> void M88K_CPUComponent::instr_bb(CPUDyntransComponent* cpubase, DyntransIC* ic)
{
	DYNTRANS_INSTR_HEAD(M88K_CPUComponent)

 	bool bit = REG32(ic->arg[0]) & ic->arg[1].u32;
	if (bit == one) {
		if (samepage) {
			cpu->m_nextIC = (DyntransIC*) ic->arg[2].p;
		} else {
			cpu->m_pc &= ~((M88K_IC_ENTRIES_PER_PAGE-1) << M88K_INSTR_ALIGNMENT_SHIFT);
			cpu->m_pc = (uint32_t) (cpu->m_pc + ic->arg[2].u32);
			cpu->DyntransPCtoPointers();
		}
	}
}


template<bool one> void M88K_CPUComponent::instr_bb_n(CPUDyntransComponent* cpubase, DyntransIC* ic)
{
	DYNTRANS_INSTR_HEAD(M88K_CPUComponent)

 	bool bit = REG32(ic->arg[0]) & ic->arg[1].u32;

	// Prepare for the branch.
	cpu->m_inDelaySlot = true;
	cpu->m_exceptionOrAbortInDelaySlot = false;

	// Execute the next instruction:
	ic[1].f(cpu, ic+1);
	cpu->m_executedCycles ++;

	// If there was no exception, then branch:
	if (!cpu->m_exceptionOrAbortInDelaySlot) {
		if (bit == one) {
			cpu->m_pc &= ~((M88K_IC_ENTRIES_PER_PAGE-1) << M88K_INSTR_ALIGNMENT_SHIFT);
			cpu->m_pc = (uint32_t) (cpu->m_pc + ic->arg[2].u32);
			cpu->DyntransPCtoPointers();
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


template<bool one> void M88K_CPUComponent::instr_bb_n_singlestep(CPUDyntransComponent* cpubase, DyntransIC* ic)
{
	DYNTRANS_INSTR_HEAD(M88K_CPUComponent)

 	bool bit = REG32(ic->arg[0]) & ic->arg[1].u32;

	DYNTRANS_SYNCH_PC;

	// Prepare for the branch.
	cpu->m_inDelaySlot = true;
	cpu->m_exceptionOrAbortInDelaySlot = false;

	if (bit == one) {
		cpu->m_delaySlotTarget = cpu->m_pc & ~((M88K_IC_ENTRIES_PER_PAGE-1) << M88K_INSTR_ALIGNMENT_SHIFT);
		cpu->m_delaySlotTarget = (uint32_t) (cpu->m_delaySlotTarget + ic->arg[2].u32);
	} else {
		cpu->m_delaySlotTarget = cpu->m_pc + 8;
	}

	cpu->m_nextIC = ic + 1;
}


DYNTRANS_INSTR(M88K_CPUComponent,jmp)
{
	DYNTRANS_INSTR_HEAD(M88K_CPUComponent)
	bool continueExecution = true;

	if (cpu->m_showFunctionTraceCall && ic->arg[2].p == &cpu->m_r[M88K_RETURN_REG])
		continueExecution = cpu->FunctionTraceReturn();

	cpu->m_pc = REG32(ic->arg[2]);
	cpu->DyntransPCtoPointers();

	if (!continueExecution)
		cpu->m_nextIC = &cpu->m_abortIC;
}


DYNTRANS_INSTR(M88K_CPUComponent,jmp_n)
{
	DYNTRANS_INSTR_HEAD(M88K_CPUComponent)

	// Prepare for the branch.
	cpu->m_inDelaySlot = true;
	cpu->m_exceptionOrAbortInDelaySlot = false;
	uint32_t branchTarget = REG32(ic->arg[2]);

	// Execute the next instruction:
	ic[1].f(cpu, ic+1);
	cpu->m_executedCycles ++;

	// If there was no exception, then branch:
	if (!cpu->m_exceptionOrAbortInDelaySlot) {
		cpu->m_pc = branchTarget;
		cpu->DyntransPCtoPointers();

		cpu->m_inDelaySlot = false;
	}

	// The next instruction is now either the target of the branch
	// instruction, the instruction 2 steps after this one,
	// or the first instruction of an exception handler.
	cpu->m_exceptionOrAbortInDelaySlot = false;
}


DYNTRANS_INSTR(M88K_CPUComponent,jmp_n_functioncalltrace)
{
	DYNTRANS_INSTR_HEAD(M88K_CPUComponent)

	// Prepare for the branch.
	cpu->m_inDelaySlot = true;
	cpu->m_exceptionOrAbortInDelaySlot = false;
	uint32_t branchTarget = REG32(ic->arg[2]);

	// Execute the next instruction:
	ic[1].f(cpu, ic+1);
	cpu->m_executedCycles ++;

	// If there was no exception, then branch:
	if (!cpu->m_exceptionOrAbortInDelaySlot) {
		bool continueExecution = true;
		if (cpu->m_showFunctionTraceCall && ic->arg[2].p == &cpu->m_r[M88K_RETURN_REG])
			continueExecution = cpu->FunctionTraceReturn();

		cpu->m_pc = branchTarget;
		cpu->DyntransPCtoPointers();

		cpu->m_inDelaySlot = false;

		if (!continueExecution)
			cpu->m_nextIC = &cpu->m_abortIC;
	}

	// The next instruction is now either the target of the branch
	// instruction, the instruction 2 steps after this one,
	// or the first instruction of an exception handler.
	cpu->m_exceptionOrAbortInDelaySlot = false;
}


// Note: This IC function is used both when function call trace is enabled
// and disabled. (Ok, since it is only used when singlestepping.)
DYNTRANS_INSTR(M88K_CPUComponent,jmp_n_functioncalltrace_singlestep)
{
	DYNTRANS_INSTR_HEAD(M88K_CPUComponent)

	if (cpu->m_showFunctionTraceCall && ic->arg[2].p == &cpu->m_r[M88K_RETURN_REG])
		cpu->FunctionTraceReturn();

	// Prepare for the delayed branch.
	cpu->m_inDelaySlot = true;
	cpu->m_exceptionOrAbortInDelaySlot = false;

	cpu->m_delaySlotTarget = REG32(ic->arg[2]);

	// m_nextIC already points to the next instruction
}


/*
 *  ldcr:   Load value from a control register, store in register d.
 *
 *  arg[0] = pointer to register d
 *  arg[1] = 6-bit control register number
 */
DYNTRANS_INSTR(M88K_CPUComponent,ldcr)
{
	DYNTRANS_INSTR_HEAD(M88K_CPUComponent)

	if (cpu->m_cr[M88K_CR_PSR] & M88K_PSR_MODE) {
		int cr = ic->arg[1].u32;
		REG32(ic->arg[0]) = cpu->m_cr[cr];
	} else {
		DYNTRANS_SYNCH_PC;
		cpu->Exception(M88K_EXCEPTION_PRIVILEGE_VIOLATION, 0);
	}
}


/*
 *  stcr:   Store value from register s1 into a control register.
 *
 *  arg[0] = pointer to register s1
 *  arg[1] = 6-bit control register number
 */
DYNTRANS_INSTR(M88K_CPUComponent,stcr)
{
	DYNTRANS_INSTR_HEAD(M88K_CPUComponent)

	DYNTRANS_SYNCH_PC;

	if (!(cpu->m_cr[M88K_CR_PSR] & M88K_PSR_MODE)) {
		cpu->Exception(M88K_EXCEPTION_PRIVILEGE_VIOLATION, 0);
		return;
	}

	cpu->stcr(ic->arg[1].u32, REG32(ic->arg[0]), false);

	cpu->m_nextIC = ic + 1;
}


/*
 *  tb0, tb1:  Trap on bit Clear/Set
 *
 *  arg[0] = bitmask to check (e.g. 0x00020000 for bit 17)
 *  arg[1] = pointer to register s1
 *  arg[2] = 9-bit vector number
 */
template<bool one> void M88K_CPUComponent::instr_tb(CPUDyntransComponent* cpubase, DyntransIC* ic)
{
	DYNTRANS_INSTR_HEAD(M88K_CPUComponent)

	if (!(cpu->m_cr[M88K_CR_PSR] & M88K_PSR_MODE)
	    && ic->arg[2].u32 < M88K_EXCEPTION_USER_TRAPS_START) {
		DYNTRANS_SYNCH_PC;
		cpu->Exception(M88K_EXCEPTION_PRIVILEGE_VIOLATION, 0);
		return;
	}

	bool bit = (REG32(ic->arg[1]) & ic->arg[0].u32) > 0;
	if (bit == one) {
		DYNTRANS_SYNCH_PC;
		cpu->Exception(ic->arg[2].u32, 1);
	}
}


/*
 *  lda:  d = s1 + s2 * scaleFactor
 *
 *  arg[0] = pointer to register d
 *  arg[1] = pointer to register s1
 *  arg[2] = pointer to register s2
 */
template<int scaleFactor> void M88K_CPUComponent::instr_lda(CPUDyntransComponent* cpubase, DyntransIC* ic)
{
	REG32(ic->arg[0]) = REG32(ic->arg[1]) + scaleFactor * REG32(ic->arg[2]);
}


/*
 *  Loads and stores:
 *
 *  arg[0] = pointer to register d
 *  arg[1] = pointer to register s1
 *  arg[2] = pointer to register s2  or  uint16_t offset
 */
template<bool store, typename T, bool doubleword, bool regofs, bool scaled, bool signedLoad> void M88K_CPUComponent::instr_loadstore(CPUDyntransComponent* cpubase, DyntransIC* ic)
{
	DYNTRANS_INSTR_HEAD(M88K_CPUComponent)

	// TODO: fast lookups
	// TODO: usr access

	// TODO: place in M88K's "ongoing memory transaction" registers!

	uint32_t addr = REG32(ic->arg[1]) +
	    (scaled? (doubleword? sizeof(uint64_t) : sizeof(T)) : 1) *
	    (regofs? REG32(ic->arg[2]) : ic->arg[2].u32);

	if (sizeof(T) > 1 && (addr & (sizeof(T)-1))) {
		DYNTRANS_SYNCH_PC;
		cpu->Exception(M88K_EXCEPTION_MISALIGNED_ACCESS, 0);
		return;
	}

	cpu->AddressSelect(addr);

	if (store) {
		T data = REG32(ic->arg[0]);
		if (!cpu->WriteData(data, cpu->m_isBigEndian? BigEndian : LittleEndian)) {
			// TODO: failed to access memory was probably an exception. Handle this!
		}
	} else {
		T data;
		if (!cpu->ReadData(data, cpu->m_isBigEndian? BigEndian : LittleEndian)) {
			// TODO: failed to access memory was probably an exception. Handle this!
		}

		if (signedLoad) {
			if (sizeof(T) == sizeof(uint16_t))
				data = (int16_t)data;
			if (sizeof(T) == sizeof(uint8_t))
				data = (int8_t)data;
		}

		REG32(ic->arg[0]) = data;
	}

	// Special handling of second word in a double-word read or write:
	if (doubleword) {
		if (store) {
			uint32_t data2 = (* (((uint32_t*)(ic->arg[0].p)) + 1) );
			cpu->AddressSelect(addr + sizeof(uint32_t));
			if (!cpu->WriteData(data2, cpu->m_isBigEndian? BigEndian : LittleEndian)) {
				// TODO: failed to access memory was probably an exception. Handle this!
			}
		} else {
			uint32_t data2;
			cpu->AddressSelect(addr + sizeof(uint32_t));
			if (!cpu->ReadData(data2, cpu->m_isBigEndian? BigEndian : LittleEndian)) {
				// TODO: failed to access memory was probably an exception. Handle this!
			}

			(* (((uint32_t*)(ic->arg[0].p)) + 1) ) = data2;
		}
	}
}


/*****************************************************************************/


/*
 *  For unit tests:
 *
 *  fail_early: Results in an abort before doing anything.
 *  fail_late: Results in an abort after increasing r1.
 */
DYNTRANS_INSTR(M88K_CPUComponent,fail_early)
{
	DYNTRANS_INSTR_HEAD(M88K_CPUComponent)

	// Point to this instruction...
	DYNTRANS_SYNCH_PC;

	// We didn't actually do anything in this instruction.
	cpu->m_executedCycles --;

	// ... and then abort.
	cpu->m_nextIC = &cpu->m_abortIC;
	if (cpu->m_inDelaySlot)
		cpu->m_exceptionOrAbortInDelaySlot = true;
}

DYNTRANS_INSTR(M88K_CPUComponent,fail_late)
{
	DYNTRANS_INSTR_HEAD(M88K_CPUComponent)

	// Do something...
	cpu->m_r[1] ++;

	// Point to next instruction...
	DYNTRANS_SYNCH_PC;
	cpu->m_pc += sizeof(uint32_t);

	// ... and abort.
	cpu->m_nextIC = &cpu->m_abortIC;
	if (cpu->m_inDelaySlot)
		cpu->m_exceptionOrAbortInDelaySlot = true;
}


/*****************************************************************************/


void M88K_CPUComponent::Translate(uint32_t iw, struct DyntransIC* ic)
{
	bool singleInstructionLeft = (m_executedCycles == m_nrOfCyclesToExecute - 1);
	UI* ui = GetUI();	// for debug messages

	uint32_t op26   = (iw >> 26) & 0x3f;
//	uint32_t op11   = (iw >> 11) & 0x1f;
	uint32_t op10   = (iw >> 10) & 0x3f;
	uint32_t d      = (iw >> 21) & 0x1f;
	uint32_t s1     = (iw >> 16) & 0x1f;
	uint32_t s2     =  iw        & 0x1f;
//	uint32_t op3d   = (iw >>  8) & 0xff;
	uint32_t imm16  = iw & 0xffff;
//	uint32_t w5     = (iw >>  5) & 0x1f;
	uint32_t cr6    = (iw >>  5) & 0x3f;
	int32_t  d16    = ((int16_t) (iw & 0xffff)) * 4;
	int32_t  d26    = ((int32_t)((iw & 0x03ffffff) << 6)) >> 4;

	switch (op26) {

	case 0x02:	/*  ld.hu  */
	case 0x03:	/*  ld.bu  */
	case 0x04:	/*  ld.d   */
	case 0x05:	/*  ld     */
	case 0x06:	/*  ld.h   */
	case 0x07:	/*  ld.b   */
	case 0x08:	/*  st.d   */
	case 0x09:	/*  st     */
	case 0x0a:	/*  st.h   */
	case 0x0b:	/*  st.b   */
		{
			bool store = op26 >= 0x08;
			int opsize = 0;

			ic->arg[0].p = &m_r[d];
			ic->arg[1].p = &m_r[s1];
			ic->arg[2].u32 = imm16;

			switch (op26) {
			case 0x02: ic->f = instr_loadstore<false, uint16_t, false, false, false, false>; opsize = 1; break;
			case 0x03: ic->f = instr_loadstore<false, uint8_t,  false, false, false, false>; opsize = 0; break;
			case 0x04: ic->f = instr_loadstore<false, uint32_t, true,  false, false, false>; opsize = 3; break;
			case 0x05: ic->f = instr_loadstore<false, uint32_t, false, false, false, false>; opsize = 2; break;
			case 0x06: ic->f = instr_loadstore<false, uint16_t, false, false, false, true>;  opsize = 1; break;
			case 0x07: ic->f = instr_loadstore<false, uint8_t,  false, false, false, true>;  opsize = 0; break;
			case 0x08: ic->f = instr_loadstore<true,  uint32_t, true,  false, false, false>; opsize = 3; break;
			case 0x09: ic->f = instr_loadstore<true,  uint32_t, false, false, false, false>; opsize = 2; break;
			case 0x0a: ic->f = instr_loadstore<true,  uint16_t, false, false, false, false>; opsize = 1; break;
			case 0x0b: ic->f = instr_loadstore<true,  uint8_t,  false, false, false, false>; opsize = 0; break;
			}

			if (opsize == 3 && d == 31) {
				// m88k load/store of register pair r31/r0 is not
				// yet implemented: TODO: figure out how to deal with this.
				ic->f = NULL;
				break;
			}

			// Loads into the zero register => load into scratch register.
			// According to the MC88110 manual: special cache operation
			// "(touch, allocate, or flush) may be performed". (TODO)
			if (!store && d == M88K_ZERO_REG && ic->f != NULL)
				ic->arg[0].p = &m_zero_scratch;
		}
		break;

	case 0x10:	/*  and    immu32  */
	case 0x11:	/*  and.u  immu32  */
	case 0x12:	/*  mask   immu32  */
	case 0x13:	/*  mask.u immu32  */
	case 0x14:	/*  xor    immu32  */
	case 0x15:	/*  xor.u  immu32  */
	case 0x16:	/*  or     immu32  */
	case 0x17:	/*  or.u   immu32  */
	case 0x18:	/*  addu   immu32  */
	case 0x19:	/*  subu   immu32  */
	case 0x1a:	/*  divu   immu32  */
	case 0x1b:	/*  mulu   immu32  */
	case 0x1f:	/*  cmp    immu32  */
		{
			int shift = 0;
			switch (op26) {
			case 0x10: ic->f = instr_and_u32_u32_immu32; break; // Note (see below): and only ands upper or lower part!
			case 0x11: ic->f = instr_and_u32_u32_immu32; shift = 16; break;
			case 0x12: ic->f = instr_and_u32_u32_immu32; break; // Note: mask is implemented using and
			case 0x13: ic->f = instr_and_u32_u32_immu32; shift = 16; break;
			case 0x14: ic->f = instr_xor_u32_u32_immu32; break;
			case 0x15: ic->f = instr_xor_u32_u32_immu32; shift = 16; break;
			case 0x16: ic->f = instr_or_u32_u32_immu32; break;
			case 0x17: ic->f = instr_or_u32_u32_immu32; shift = 16; break;
			case 0x18: ic->f = instr_add_u32_u32_immu32; break;
			case 0x19: ic->f = instr_sub_u32_u32_immu32; break;
			case 0x1a: ic->f = instr_divu_imm; break;
			case 0x1b: ic->f = instr_mulu_imm; break;
	//		case 0x1c: ic->f = instr(add_imm); break;
	//		case 0x1d: ic->f = instr(sub_imm); break;
	//		case 0x1e: ic->f = instr(div_imm); break;
			case 0x1f: ic->f = instr_cmp_imm; break;
			}

			ic->arg[0].p = &m_r[d];
			ic->arg[1].p = &m_r[s1];
			ic->arg[2].u32 = imm16 << shift;

			// The 'and' instruction only ands bits in the upper or
			// lower parts of the word; the 'mask' instruction works
			// on the whole register.
			if (op26 == 0x10)
				ic->arg[2].u32 |= 0xffff0000;
			if (op26 == 0x11)
				ic->arg[2].u32 |= 0x0000ffff;

			if (d == M88K_ZERO_REG)
				ic->f = instr_nop;
		}
		break;

	case 0x20:
		if ((iw & 0x001ff81f) == 0x00004000) {
			ic->f = instr_ldcr;
			ic->arg[0].p = &m_r[d];
			ic->arg[1].u32 = cr6;
			if (d == M88K_ZERO_REG)
				ic->arg[0].p = &m_zero_scratch;
//		} else if ((iword & 0x001ff81f) == 0x00004800) {
//			ic->f = instr(fldcr);
//			ic->arg[0] = (size_t) &cpu->cd.m88k.r[d];
//			ic->arg[1] = cr6;
//			if (d == M88K_ZERO_REG)
//				ic->arg[0] = (size_t)
//				    &cpu->cd.m88k.zero_scratch;
		} else if ((iw & 0x03e0f800) == 0x00008000) {
			ic->f = instr_stcr;
			ic->arg[0].p = &m_r[s1];
			ic->arg[1].u32 = cr6;
			if (s1 != s2) {
				ic->f = NULL;
				if (ui != NULL) {
					stringstream ss;
					ss.flags(std::ios::hex);
					ss << "stcr with s1 != s2? TODO: how "
					    "should this be handled? s1=0x"
					    << s1 << ", s2=0x" << s2;
					ui->ShowDebugMessage(this, ss.str());
				}
			}
//		} else if ((iword & 0x03e0f800) == 0x00008800) {
//			ic->f = instr(fstcr);
//			ic->arg[0] = (size_t) &cpu->cd.m88k.r[s1];
//			ic->arg[1] = cr6;
//			if (s1 != s2)
//				goto bad;
//		} else if ((iword & 0x0000f800) == 0x0000c000) {
//			ic->f = instr(xcr);
//			ic->arg[0] = (size_t) &cpu->cd.m88k.r[d];
//			ic->arg[1] = (size_t) &cpu->cd.m88k.r[s1];
//			ic->arg[2] = cr6;
//			if (s1 != s2)
//				goto bad;
		} else if (ui != NULL) {
			ui->ShowDebugMessage(this, "unimplemented variant of opcode 0x20");
		}
		break;


	case 0x30:	/*  br     */
//	case 0x31:	/*  br.n   */
	case 0x32:	/*  bsr    */
	case 0x33:	/*  bsr.n  */
		{
			void (*f_singleStepping)(CPUDyntransComponent*, struct DyntransIC*) = NULL;
			void (*samepage_function)(CPUDyntransComponent*, struct DyntransIC*) = NULL;

			switch (op26) {
			case 0x30:
				ic->f = NULL; // instr(br);
				samepage_function = instr_branch_samepage;
				break;
	//		case 0x31:
	//			ic->f = instr(br_n);
	//			if (cpu->translation_readahead > 2)
	//				cpu->translation_readahead = 2;
	//			break;
			case 0x32:
				ic->f = instr_bsr;
				samepage_function = instr_bsr_samepage;
				break;
			case 0x33:
				ic->f = instr_bsr_n;
				// TODO samepage_function = instr_bsr_samepage;
				f_singleStepping = instr_bsr_n_functioncalltrace_singlestep;
				break;
			}

			if (singleInstructionLeft && (op26 == 0x31 || op26 == 0x33)) {
				ic->f = f_singleStepping;
				samepage_function = NULL;
			}

			int offset = (m_pc & 0xffc) + d26;

			/*  Prepare both samepage and offset style args.
			    (Only one will be used in the actual instruction.)  */
			ic->arg[0].p = ( m_firstIConPage + (offset >> M88K_INSTR_ALIGNMENT_SHIFT) );
			ic->arg[1].u32 = offset;

			/*  Return offset for bsr and bsr.n (stored in m_r[M88K_RETURN_REG]):  */
			ic->arg[2].u32 = (m_pc & 0xffc) + ((op26 & 1)? 8 : 4);

			if (offset >= 0 && offset <= 0xffc && samepage_function != NULL)
				ic->f = samepage_function;

			if (m_showFunctionTraceCall) {
				if (op26 == 0x32)
					ic->f = instr_bsr_functioncalltrace;
				if (op26 == 0x33) {
					if (singleInstructionLeft)
						ic->f = instr_bsr_n_functioncalltrace_singlestep;
					else
						ic->f = instr_bsr_n_functioncalltrace;
				}
			}
		}
		break;

	case 0x34:	/*  bb0     */
	case 0x35:	/*  bb0.n   */
	case 0x36:	/*  bb1     */
	case 0x37:	/*  bb1.n   */
		{
			void (*samepage_function)(CPUDyntransComponent*, struct DyntransIC*) = NULL;
			void (*singlestep_function)(CPUDyntransComponent*, struct DyntransIC*) = NULL;

			switch (op26) {
			case 0x34:
				ic->f = instr_bb<false,false>;
				samepage_function = instr_bb<false,true>;
				break;
			case 0x35:
				ic->f = instr_bb_n<false>;
				singlestep_function = instr_bb_n_singlestep<false>;
				break;
			case 0x36:
				ic->f = instr_bb<true,false>;
				samepage_function = instr_bb<true,true>;
				break;
			case 0x37:
				ic->f = instr_bb_n<true>;
				singlestep_function = instr_bb_n_singlestep<true>;
				break;
			}
	
			ic->arg[0].p = &m_r[s1];
			ic->arg[1].u32 = (1 << d);
	
			int offset = (m_pc & 0xffc) + d16;
			ic->arg[2].u32 = offset;

			if (singleInstructionLeft && singlestep_function != NULL)
				ic->f = singlestep_function;
			else if (offset >= 0 && offset <= 0xffc && samepage_function != NULL) {
				ic->f = samepage_function;
				ic->arg[2].p = ( m_firstIConPage + (offset >> M88K_INSTR_ALIGNMENT_SHIFT) );
			}
		}
		break;

	case 0x3a:	/*  bcnd    */
	case 0x3b:	/*  bcnd.n  */
		{
			void (*samepage_function)(CPUDyntransComponent*, struct DyntransIC*) = NULL;
			void (*singlestep_f)(CPUDyntransComponent*, struct DyntransIC*) = NULL;

			if (op26 & 1) {
				switch (d) {
				case  1: ic->f = instr_bcnd<true,1, false>; singlestep_f = instr_bcnd<true,1, true>; break;
				case  2: ic->f = instr_bcnd<true,2, false>; singlestep_f = instr_bcnd<true,2, true>; break;
				case  3: ic->f = instr_bcnd<true,3, false>; singlestep_f = instr_bcnd<true,3, true>; break;
				case  7: ic->f = instr_bcnd<true,7, false>; singlestep_f = instr_bcnd<true,7, true>; break;
				case  8: ic->f = instr_bcnd<true,8, false>; singlestep_f = instr_bcnd<true,8, true>; break;
				case 12: ic->f = instr_bcnd<true,12,false>; singlestep_f = instr_bcnd<true,12,true>; break;
				case 13: ic->f = instr_bcnd<true,13,false>; singlestep_f = instr_bcnd<true,13,true>; break;
				case 14: ic->f = instr_bcnd<true,14,false>; singlestep_f = instr_bcnd<true,14,true>; break;
				}
			} else {
				switch (d) {
				case  1: ic->f = instr_bcnd<false,1, false>; break;
				case  2: ic->f = instr_bcnd<false,2, false>; break;
				case  3: ic->f = instr_bcnd<false,3, false>; break;
				case  7: ic->f = instr_bcnd<false,7, false>; break;
				case  8: ic->f = instr_bcnd<false,8, false>; break;
				case 12: ic->f = instr_bcnd<false,12,false>; break;
				case 13: ic->f = instr_bcnd<false,13,false>; break;
				case 14: ic->f = instr_bcnd<false,14,false>; break;
				}
			}

			// TODO: samepage optimization: probably easiest to do using
			// another template bit...
			// samepage_function = m88k_bcnd[64 + d + 32 * (op26 & 1)];

			if (ic->f == NULL) {
				if (ui != NULL) {
					stringstream ss;
					ss.flags(std::ios::hex);
					ss << "unimplemented bcnd condition code d = " << d;
					ui->ShowDebugMessage(this, ss.str());
				}

				break;
			}

			ic->arg[0].p = &m_r[s1];

			int offset = (m_pc & 0xffc) + d16;
			ic->arg[2].u32 = offset;

			if (singleInstructionLeft && singlestep_f != NULL) {
				ic->f = singlestep_f;
			} else if (offset >= 0 && offset <= 0xffc && samepage_function != NULL) {
				ic->f = samepage_function;
				ic->arg[2].p = ( m_firstIConPage + (offset >> M88K_INSTR_ALIGNMENT_SHIFT) );
			}
		}
		break;

	case 0x3c:
		switch (op10) {

//		case 0x20:	/*  clr  */
//		case 0x22:	/*  set  */
		case 0x24:	/*  ext  */
		case 0x26:	/*  extu  */
		case 0x28:	/*  mak  */
			ic->arg[0].p = &m_r[d];
			ic->arg[1].p = &m_r[s1];
			ic->arg[2].u32 = iw & 0x3ff;

			switch (op10) {
//			case 0x20: ic->f = instr(mask_imm);
//				   {
//					int w = ic->arg[2] >> 5;
//					int o = ic->arg[2] & 0x1f;
//					uint32_t x = w == 0? 0xffffffff
//					    : ((uint32_t)1 << w) - 1;
//					x <<= o;
//					ic->arg[2] = ~x;
//				   }
//				   break;
//			case 0x22: ic->f = instr(or_imm);
//				   {
//					int w = ic->arg[2] >> 5;
//					int o = ic->arg[2] & 0x1f;
//					uint32_t x = w == 0? 0xffffffff
//					    : ((uint32_t)1 << w) - 1;
//					x <<= o;
//					ic->arg[2] = x;
//				   }
//				   break;
			case 0x24: ic->f = instr_ext_imm; break;
			case 0x26: ic->f = instr_extu_imm; break;
			case 0x28: ic->f = instr_mak_imm; break;
			}

			if (d == M88K_ZERO_REG)
				ic->f = instr_nop;
			break;

		case 0x34:	/*  tb0  */
		case 0x36:	/*  tb1  */
			ic->arg[0].u32 = 1 << d;	// d is called B5 in the manual
			ic->arg[1].p = &m_r[s1];
			ic->arg[2].u32 = iw & 0x1ff;
			switch (op10) {
			case 0x34: ic->f = instr_tb<false>; break;
			case 0x36: ic->f = instr_tb<true>; break;
			}
			break;

		default:
			if (ui != NULL) {
				stringstream ss;
				ss.flags(std::ios::hex);
				ss << "unimplemented opcode 0x" << op26 << ",0x" << op10;
				ui->ShowDebugMessage(this, ss.str());
			}
		}
		break;

	case 0x3d:
		if ((iw & 0xf000) <= 0x3fff ) {
			// Load, Store, xmem, and lda:
			int op = 0, opsize, user = 0, wt = 0;
			int signedness = 1, scaled = 0;

			switch (iw & 0xf000) {
			case 0x2000: op = 1; /* st */  break;
			case 0x3000: op = 2; /* lda */ break;
			default:     if ((iw & 0xf800) >= 0x0800)
					op = 0; /* ld */
				     else
					op = 3; /* xmem */
			}

			/*  for (most) ld, st, lda:  */
			opsize = (iw >> 10) & 3;

			/*  Turn opsize into x, where size = 1 << x:  */
			opsize = 3 - opsize;

			if (op == 3) {
				/*  xmem:  */
				opsize = -1;
				switch ((iw >> 10) & 3) {
				case 0: opsize = 0; break;
				case 1: opsize = 2; break;
				default:// Weird xmem opsize/type? TODO
					break;
				}
				if (opsize < 0)
					break;
			} else {
				if ((iw & 0xf800) == 0x800) {
					signedness = 0;
					if ((iw & 0xf00) < 0xc00)
						opsize = 1;
					else
						opsize = 0;
				} else {
					if (opsize >= 2 || op == 1)
						signedness = 0;
				}
			}

			if (iw & 0x100)
				user = 1;
			if (iw & 0x80)
				wt = 1;
			if (iw & 0x200)
				scaled = 1;

			if (wt) {
				// wt bit not yet implemented! TODO
				ic->f = NULL;
				break;
			}

			ic->arg[0].p = &m_r[d];
			ic->arg[1].p = &m_r[s1];
			ic->arg[2].p = &m_r[s2];

			if (op == 0 || op == 1) {
				/*  ld or st:  */

				int n = opsize +
					((op == 1)? 4 : 0) +
					(signedness? 8 : 0) +
					(m_isBigEndian? 16 : 0) +
					(scaled? 32 : 0) +
					(user? 64 : 0);

				// <bool store, typename T, bool doubleword, bool regofs, bool scaled, bool signedLoad>
				//       4    ,   0123    ,       3        ,     true   ,      32    ,      8           , user (TODO)
				switch (n) {
				// load = 0
				case 0 + 0 + 0 +  0  +  0 +  0: ic->f = instr_loadstore<false, uint8_t,  false, true, false, false>; break;
				case 0 + 0 + 0 +  0  + 32 +  0: ic->f = instr_loadstore<false, uint8_t,  false, true, true,  false>; break;
				case 0 + 0 + 0 + 16  +  0 +  0: ic->f = instr_loadstore<false, uint8_t,  false, true, false, false>; break;
				case 0 + 0 + 0 + 16  + 32 +  0: ic->f = instr_loadstore<false, uint8_t,  false, true, true,  false>; break;
				case 1 + 0 + 0 +  0  +  0 +  0: ic->f = instr_loadstore<false, uint16_t, false, true, false, false>; break;
				case 1 + 0 + 0 +  0  + 32 +  0: ic->f = instr_loadstore<false, uint16_t, false, true, true,  false>; break;
				case 1 + 0 + 0 + 16  +  0 +  0: ic->f = instr_loadstore<false, uint16_t, false, true, false, false>; break;
				case 1 + 0 + 0 + 16  + 32 +  0: ic->f = instr_loadstore<false, uint16_t, false, true, true,  false>; break;
				case 2 + 0 + 0 +  0  +  0 +  0: ic->f = instr_loadstore<false, uint32_t, false, true, false, false>; break;
				case 2 + 0 + 0 +  0  + 32 +  0: ic->f = instr_loadstore<false, uint32_t, false, true, true,  false>; break;
				case 2 + 0 + 0 + 16  +  0 +  0: ic->f = instr_loadstore<false, uint32_t, false, true, false, false>; break;
				case 2 + 0 + 0 + 16  + 32 +  0: ic->f = instr_loadstore<false, uint32_t, false, true, true,  false>; break;
				case 3 + 0 + 0 +  0  +  0 +  0: ic->f = instr_loadstore<false, uint32_t, true,  true, false, false>; break;
				case 3 + 0 + 0 +  0  + 32 +  0: ic->f = instr_loadstore<false, uint32_t, true,  true, true,  false>; break;
				case 3 + 0 + 0 + 16  +  0 +  0: ic->f = instr_loadstore<false, uint32_t, true,  true, false, false>; break;
				case 3 + 0 + 0 + 16  + 32 +  0: ic->f = instr_loadstore<false, uint32_t, true,  true, true,  false>; break;
				// store = 4
				case 0 + 4 + 0 +  0  +  0 +  0: ic->f = instr_loadstore<true, uint8_t,  false, true, false, false>; break;
				case 0 + 4 + 0 +  0  + 32 +  0: ic->f = instr_loadstore<true, uint8_t,  false, true, true,  false>; break;
				case 0 + 4 + 0 + 16  +  0 +  0: ic->f = instr_loadstore<true, uint8_t,  false, true, false, false>; break;
				case 0 + 4 + 0 + 16  + 32 +  0: ic->f = instr_loadstore<true, uint8_t,  false, true, true,  false>; break;
				case 1 + 4 + 0 +  0  +  0 +  0: ic->f = instr_loadstore<true, uint16_t, false, true, false, false>; break;
				case 1 + 4 + 0 +  0  + 32 +  0: ic->f = instr_loadstore<true, uint16_t, false, true, true,  false>; break;
				case 1 + 4 + 0 + 16  +  0 +  0: ic->f = instr_loadstore<true, uint16_t, false, true, false, false>; break;
				case 1 + 4 + 0 + 16  + 32 +  0: ic->f = instr_loadstore<true, uint16_t, false, true, true,  false>; break;
				case 2 + 4 + 0 +  0  +  0 +  0: ic->f = instr_loadstore<true, uint32_t, false, true, false, false>; break;
				case 2 + 4 + 0 +  0  + 32 +  0: ic->f = instr_loadstore<true, uint32_t, false, true, true,  false>; break;
				case 2 + 4 + 0 + 16  +  0 +  0: ic->f = instr_loadstore<true, uint32_t, false, true, false, false>; break;
				case 2 + 4 + 0 + 16  + 32 +  0: ic->f = instr_loadstore<true, uint32_t, false, true, true,  false>; break;
				case 3 + 4 + 0 +  0  +  0 +  0: ic->f = instr_loadstore<true, uint32_t, true,  true, false, false>; break;
				case 3 + 4 + 0 +  0  + 32 +  0: ic->f = instr_loadstore<true, uint32_t, true,  true, true,  false>; break;
				case 3 + 4 + 0 + 16  +  0 +  0: ic->f = instr_loadstore<true, uint32_t, true,  true, false, false>; break;
				case 3 + 4 + 0 + 16  + 32 +  0: ic->f = instr_loadstore<true, uint32_t, true,  true, true,  false>; break;
				default:
					std::cerr << "TODO generalize! scaled="<<scaled << " user="<<
						user<<" signedness="<<signedness << " opsize=" << opsize << "\n";
				}

				// Loads into the zero register are changed to load into
				// the scratch register.
				// According to the MC88110 manual: special cache operation
				// "(touch, allocate, or flush) may be performed". (TODO)
				if (op == 0 && d == M88K_ZERO_REG && ic->f != NULL)
					ic->arg[0].p = &m_zero_scratch;

				if (opsize == 3 && d == 31) {
					// m88k load/store of register pair r31/r0 is not
					// yet implemented: TODO: figure out how to deal with this.
					ic->f = NULL;
					break;
				}
			} else if (op == 2) {
				/*  lda:  */
				if (scaled) {
					switch (opsize) {
//					case 0: // TODO: 88110 vs 88100 etc. ic->f = instr(addu); break;
					case 1: ic->f = instr_lda<2>; break;
					case 2: ic->f = instr_lda<4>; break;
					case 3: ic->f = instr_lda<8>; break;
					}
				} else {
					// TODO: 88110 vs 88100 etc.
					// ic->f = instr(addu);
				}

				// TODO: Perhaps 88110 loads into 0 are not nops, but cache ops? Look in docs.
				if (d == M88K_ZERO_REG && ic->f != NULL)
					ic->f = instr_nop;
			} else {
				/*  xmem:  */
// TODO
//				ic->f = instr(xmem_slow);
//				ic->arg[0] = iw;
//				if (d == M88K_ZERO_REG)
//					ic->f = instr(nop);
			}
		} else switch ((iw >> 8) & 0xff) {
//		case 0x40:	/*  and    */
//		case 0x44:	/*  and.c  */
		case 0x50:	/*  xor    */
//		case 0x54:	/*  xor.c  */
		case 0x58:	/*  or     */
//		case 0x5c:	/*  or.c   */
		case 0x60:	/*  addu   */
//		case 0x61:	/*  addu.co  */
//		case 0x62:	/*  addu.ci  */
		case 0x64:	/*  subu   */
//		case 0x65:	/*  subu.co  */
//		case 0x66:	/*  subu.ci  */
//		case 0x68:	/*  divu   */
//		case 0x6c:	/*  mul    */
//		case 0x70:	/*  add    */
//		case 0x78:	/*  div    */
		case 0x7c:	/*  cmp    */
//		case 0x80:	/*  clr    */
//		case 0x88:	/*  set    */
		case 0x90:	/*  ext    */
		case 0x98:	/*  extu   */
		case 0xa0:	/*  mak    */
//		case 0xa8:	/*  rot    */
			ic->arg[0].p = &m_r[d];
			ic->arg[1].p = &m_r[s1];
			ic->arg[2].p = &m_r[s2];

			switch ((iw >> 8) & 0xff) {
//			case 0x40: ic->f = instr(and);   break;
//			case 0x44: ic->f = instr(and_c); break;
			case 0x50: ic->f = instr_xor_u32_u32_u32; break;
//			case 0x54: ic->f = instr(xor_c); break;
			case 0x58: ic->f = instr_or_u32_u32_u32; break;
//			case 0x5c: ic->f = instr(or_c);  break;
			case 0x60: ic->f = instr_add_u32_u32_u32; break;
//			case 0x61: ic->f = instr(addu_co); break;
//			case 0x62: ic->f = instr(addu_ci); break;
			case 0x64: ic->f = instr_sub_u32_u32_u32; break;
//			case 0x65: ic->f = instr(subu_co); break;
//			case 0x66: ic->f = instr(subu_ci); break;
//			case 0x68: ic->f = instr(divu);  break;
//			case 0x6c: ic->f = instr(mul);   break;
//			case 0x70: ic->f = instr(add);   break;
//			case 0x78: ic->f = instr(div);   break;
			case 0x7c: ic->f = instr_cmp; break;
//			case 0x80: ic->f = instr(clr);   break;
//			case 0x88: ic->f = instr(set);   break;
			case 0x90: ic->f = instr_ext;   break;
			case 0x98: ic->f = instr_extu;  break;
			case 0xa0: ic->f = instr_mak; break;
//			case 0xa8: ic->f = instr(rot);   break;
			}

			/*
			 * Handle the case when the destination register is r0:
			 *
			 * If there is NO SIDE-EFFECT! (i.e. no carry out, no possibility
			 * of exceptions, etc), then replace the instruction with a nop.
			 * If there is a possible side-effect, we still have to run the
			 * instruction, so replace the destination register with the
			 * scratch register.
			 */
			if (d == M88K_ZERO_REG && ic->f != NULL) {
				int opc = (iw >> 8) & 0xff;
				if (opc != 0x61 /* addu.co */ && opc != 0x63 /* addu.cio */ &&
				    opc != 0x65 /* subu.co */ && opc != 0x67 /* subu.cio */ &&
				    opc != 0x71 /*  add.co */ && opc != 0x73 /*  add.cio */ &&
				    opc != 0x75 /*  sub.co */ && opc != 0x77 /*  sub.cio */ &&
				    opc != 0x68 /*  divu   */ && opc != 0x69 /* divu.d   */ &&
				    opc != 0x6c /*  mul    */ && opc != 0x6d /* mulu.d   */ &&
				    opc != 0x6e /*  muls   */ && opc != 0x78 /*  div     */)
					ic->f = instr_nop;
				else
					ic->arg[0].p = &m_zero_scratch;
			}

			if (ic->f == NULL && ui != NULL) {
				stringstream ss;
				ss.flags(std::ios::hex);
				ss << "unimplemented opcode 0x3d,0x" << ((iw >> 8) & 0xff);
				ui->ShowDebugMessage(this, ss.str());
			}

			break;
		case 0xc0:	/*  jmp    */
		case 0xc4:	/*  jmp.n  */
//		case 0xc8:	/*  jsr    */
//		case 0xcc:	/*  jsr.n  */
			{
				void (*f_ss)(CPUDyntransComponent*, struct DyntransIC*) = NULL;

				switch ((iw >> 8) & 0xff) {
				case 0xc0: ic->f = instr_jmp; break;
				case 0xc4: ic->f = instr_jmp_n; f_ss = instr_jmp_n_functioncalltrace_singlestep; break;
	//			case 0xc8: ic->f = instr(jsr); break;
	//			case 0xcc: ic->f = instr(jsr_n); break;
				}

				ic->arg[1].u32 = (m_pc & 0xffc) + 4;
				ic->arg[2].p = &m_r[s2];

				if (((iw >> 8) & 0x04) == 0x04)
					ic->arg[1].u32 = (m_pc & 0xffc) + 8;

				if (m_showFunctionTraceCall && s2 == M88K_RETURN_REG) {
					if (ic->f == instr_jmp_n) {
						ic->f = instr_jmp_n_functioncalltrace;
						f_ss = instr_jmp_n_functioncalltrace_singlestep;
					}
				}

	//			if (m_showFunctionTraceCall) {
	//				if (ic->f == instr(jsr))
	//					ic->f = instr(jsr_trace);
	//					TODO f_ss
	//				if (ic->f == instr(jsr_n))
	//					ic->f = instr(jsr_n_trace);
	//					TODO f_ss
	//			}

				if (singleInstructionLeft && f_ss != NULL)
					ic->f = f_ss;
			}
			break;
//		case 0xe8:	/*  ff1  */
//		case 0xec:      /*  ff0  */
			// TODO
		case 0xfc:
			switch (iw & 0xff) {
//			case 0x00:	/*  rte  */
//			case 0x01:	/*  illop1  */
//			case 0x02:	/*  illop2  */
//			case 0x03:	/*  illop3  */
//			case (M88K_PROM_INSTR & 0xff):
			case (M88K_FAIL_EARLY_INSTR & 0xff):
				ic->f = instr_fail_early;
				break;
//			case (M88K_FAIL_LATE_INSTR & 0xff):
//				break;
			default:if (ui != NULL) {
					stringstream ss;
					ss.flags(std::ios::hex);
					ss << "unimplemented opcode 0x3d,0xfc,0x" << (iw & 0xff);
					ui->ShowDebugMessage(this, ss.str());
				}
			}
			break;
		default:
			if (ui != NULL) {
				stringstream ss;
				ss.flags(std::ios::hex);
				ss << "unimplemented opcode 0x3d,0x" << ((iw >> 8) & 0xff);
				ui->ShowDebugMessage(this, ss.str());
			}
		}
		break;

	default:
		if (ui != NULL) {
			stringstream ss;
			ss.flags(std::ios::hex);
			ss << "unimplemented opcode 0x" << op26;
			ui->ShowDebugMessage(this, ss.str());
		}
	}
}


DYNTRANS_INSTR(M88K_CPUComponent,ToBeTranslated)
{
	DYNTRANS_INSTR_HEAD(M88K_CPUComponent)

	cpu->DyntransToBeTranslatedBegin(ic);

	uint32_t iword;
	if (cpu->DyntransReadInstruction(iword))
		cpu->Translate(iword, ic);

	if (cpu->m_inDelaySlot && ic->f == NULL)
		ic->f = instr_abort;

	cpu->DyntransToBeTranslatedDone(ic);
}


/*****************************************************************************/


#ifdef WITHUNITTESTS

#include "ComponentFactory.h"

static void Test_M88K_CPUComponent_IsStable()
{
	UnitTest::Assert("the M88K_CPUComponent should be stable",
	    ComponentFactory::HasAttribute("m88k_cpu", "stable"));
}

static void Test_M88K_CPUComponent_Create()
{
	refcount_ptr<Component> cpu =
	    ComponentFactory::CreateComponent("m88k_cpu");
	UnitTest::Assert("component was not created?", !cpu.IsNULL());

	const StateVariable * p = cpu->GetVariable("pc");
	UnitTest::Assert("cpu has no pc state variable?", p != NULL);
	UnitTest::Assert("initial pc", p->ToString(), "0");

	const StateVariable * r31 = cpu->GetVariable("r31");
	UnitTest::Assert("cpu has no r31 state variable?", r31 != NULL);
	UnitTest::Assert("initial r31", r31->ToString(), "0");
}

static void Test_M88K_CPUComponent_Create_with_r31()
{
	refcount_ptr<Component> cpu =
	    ComponentFactory::CreateComponent("m88k_cpu(r31=0x12345678)");
	UnitTest::Assert("component was not created?", !cpu.IsNULL());

	const StateVariable * p = cpu->GetVariable("pc");
	UnitTest::Assert("cpu has no pc state variable?", p != NULL);
	UnitTest::Assert("initial pc", p->ToString(), "0");

	const StateVariable * r31 = cpu->GetVariable("r31");
	UnitTest::Assert("cpu has no r31 state variable?", r31 != NULL);
	UnitTest::Assert("initial r31", r31->ToString(), "0x12345678");

	cpu->SetVariableValue("r31", "0xf00");

	const StateVariable * r31_updated = cpu->GetVariable("r31");
	UnitTest::Assert("could not update r31?", r31_updated->ToString(), "0xf00");

	cpu->Reset();

	const StateVariable * r31_after_reset = cpu->GetVariable("r31");
	UnitTest::Assert("r31 after reset should have been reset", r31_after_reset->ToString(), "0x12345678");
}

static void Test_M88K_CPUComponent_IsCPU()
{
	refcount_ptr<Component> m88k_cpu =
	    ComponentFactory::CreateComponent("m88k_cpu");

	CPUComponent* cpu = m88k_cpu->AsCPUComponent();
	UnitTest::Assert("m88k_cpu is not a CPUComponent?", cpu != NULL);
}

static void Test_M88K_CPUComponent_DefaultModel()
{
	refcount_ptr<Component> cpu =
	    ComponentFactory::CreateComponent("m88k_cpu");

	// Suitable default models would be 88100 and 88110 (the only two
	// implementations there were of the 88K architecture). However,
	// right now (2009-07-27), 88110 emulation isn't implemented yet.
	UnitTest::Assert("wrong default model",
	    cpu->GetVariable("model")->ToString(), "88100");
}

static void Test_M88K_CPUComponent_Disassembly_Basic()
{
	refcount_ptr<Component> m88k_cpu =
	    ComponentFactory::CreateComponent("m88k_cpu");
	CPUComponent* cpu = m88k_cpu->AsCPUComponent();

	vector<string> result;
	size_t len;
	unsigned char instruction[sizeof(uint32_t)];
	// This assumes that the default endianness is BigEndian...
	instruction[0] = 0x63;
	instruction[1] = 0xdf;
	instruction[2] = 0x00;
	instruction[3] = 0x10;

	len = cpu->DisassembleInstruction(0x12345678, sizeof(uint32_t),
	    instruction, result);

	UnitTest::Assert("disassembled instruction was wrong length?", len, 4);
	UnitTest::Assert("disassembly result incomplete?", result.size(), 3);
	UnitTest::Assert("disassembly result[0]", result[0], "63df0010");
	UnitTest::Assert("disassembly result[1]", result[1], "addu");
	UnitTest::Assert("disassembly result[2]", result[2], "r30,r31,0x10");
}

static void Test_M88K_CPUComponent_Execute_Basic()
{
	GXemul gxemul;
	gxemul.GetCommandInterpreter().RunCommand("add testm88k");

	refcount_ptr<Component> cpu = gxemul.GetRootComponent()->LookupPath("root.machine0.mainbus0.cpu0");
	UnitTest::Assert("huh? no cpu?", !cpu.IsNULL());

	AddressDataBus* bus = cpu->AsAddressDataBus();
	UnitTest::Assert("cpu should be addressable", bus != NULL);

	// Place a hardcoded instruction in memory, and try to execute it.
	// addu r30, r31, 0x10
	uint32_t data32 = 0x63df0010;
	bus->AddressSelect(48);
	bus->WriteData(data32, BigEndian);

	bus->AddressSelect(52);
	bus->WriteData(data32, BigEndian);

	cpu->SetVariableValue("pc", "48");
	cpu->SetVariableValue("r30", "1234");
	cpu->SetVariableValue("r31", "5678");

	gxemul.SetRunState(GXemul::Running);
	gxemul.Execute(1);

	UnitTest::Assert("pc should have increased", cpu->GetVariable("pc")->ToInteger(), 52);
	UnitTest::Assert("r30 should have been modified", cpu->GetVariable("r30")->ToInteger(), 5678 + 0x10);
	UnitTest::Assert("r31 should not have been modified", cpu->GetVariable("r31")->ToInteger(), 5678);

	cpu->SetVariableValue("r31", "1111");

	gxemul.SetRunState(GXemul::SingleStepping);
	gxemul.Execute(1);

	UnitTest::Assert("pc should have increased again", cpu->GetVariable("pc")->ToInteger(), 56);
	UnitTest::Assert("r30 should have been modified again", cpu->GetVariable("r30")->ToInteger(), 1111 + 0x10);
}

static void Test_M88K_CPUComponent_Execute_DelayBranchWithValidInstruction()
{
	GXemul gxemul;
	gxemul.GetCommandInterpreter().RunCommand("add testm88k");

	refcount_ptr<Component> cpu = gxemul.GetRootComponent()->LookupPath("root.machine0.mainbus0.cpu0");
	UnitTest::Assert("huh? no cpu?", !cpu.IsNULL());

	AddressDataBus* bus = cpu->AsAddressDataBus();
	UnitTest::Assert("cpu should be addressable", bus != NULL);

	uint32_t data32 = 0xcc000010;	// bsr.n 0x1000 + 10*4, i.e. 0x1040
	bus->AddressSelect(0x1000ULL);
	bus->WriteData(data32, BigEndian);

	data32 = 0x63df0010; // Something valid, addu r30, r31, 0x10
	bus->AddressSelect(0x1004ULL);
	bus->WriteData(data32, BigEndian);

	cpu->SetVariableValue("pc", "0x1000");
	UnitTest::Assert("setting pc failed?", cpu->GetVariable("pc")->ToInteger(), 0x1000);
	UnitTest::Assert("r30 before execute", cpu->GetVariable("r30")->ToInteger(), 0);
	UnitTest::Assert("r31 before execute", cpu->GetVariable("r31")->ToInteger(), 0xff0);

	// This tests that execute 2 steps will execute both the delay branch
	// and the delay slot instruction.
	gxemul.SetRunState(GXemul::Running);
	gxemul.Execute(2);

	UnitTest::Assert("pc should have changed", cpu->GetVariable("pc")->ToInteger(), 0x1040);
	UnitTest::Assert("delay slot after execute", cpu->GetVariable("inDelaySlot")->ToString(), "false");
	UnitTest::Assert("r30 after execute", cpu->GetVariable("r30")->ToInteger(), 0x1000);
	UnitTest::Assert("r31 after execute", cpu->GetVariable("r31")->ToInteger(), 0xff0);
}

static void Test_M88K_CPUComponent_Execute_DelayBranchWithValidInstruction_SingleStepping()
{
	GXemul gxemul;
	gxemul.GetCommandInterpreter().RunCommand("add testm88k");

	refcount_ptr<Component> cpu = gxemul.GetRootComponent()->LookupPath("root.machine0.mainbus0.cpu0");
	UnitTest::Assert("huh? no cpu?", !cpu.IsNULL());

	AddressDataBus* bus = cpu->AsAddressDataBus();
	UnitTest::Assert("cpu should be addressable", bus != NULL);

	uint32_t data32 = 0xcc000010;	// bsr.n 0x1000 + 10*4, i.e. 0x1040
	bus->AddressSelect(0x1000ULL);
	bus->WriteData(data32, BigEndian);

	data32 = 0x63df0010; // Something valid, addu r30, r31, 0x10
	bus->AddressSelect(0x1004ULL);
	bus->WriteData(data32, BigEndian);

	cpu->SetVariableValue("pc", "0x1000");
	UnitTest::Assert("setting pc failed?", cpu->GetVariable("pc")->ToInteger(), 0x1000);
	UnitTest::Assert("r30 before execute", cpu->GetVariable("r30")->ToInteger(), 0);
	UnitTest::Assert("r31 before execute", cpu->GetVariable("r31")->ToInteger(), 0xff0);

	// This tests that execute 2 steps (using single-stepping) will execute both
	// the delay branch and the delay slot instruction.
	gxemul.SetRunState(GXemul::SingleStepping);
	gxemul.Execute(1);

	// Should now be in the delay slot.
	UnitTest::Assert("pc should have changed 1", cpu->GetVariable("pc")->ToInteger(), 0x1004);
	UnitTest::Assert("delay slot after execute 1", cpu->GetVariable("inDelaySlot")->ToString(), "true");
	UnitTest::Assert("r30 after execute 1", cpu->GetVariable("r30")->ToInteger(), 0);
	UnitTest::Assert("r31 after execute 1", cpu->GetVariable("r31")->ToInteger(), 0xff0);

	gxemul.SetRunState(GXemul::SingleStepping);
	gxemul.Execute(1);

	UnitTest::Assert("delay slot after execute 2", cpu->GetVariable("inDelaySlot")->ToString(), "false");
	UnitTest::Assert("pc should have changed 2", cpu->GetVariable("pc")->ToInteger(), 0x1040);
	UnitTest::Assert("r30 after execute 2", cpu->GetVariable("r30")->ToInteger(), 0x1000);
	UnitTest::Assert("r31 after execute 2", cpu->GetVariable("r31")->ToInteger(), 0xff0);
}

static void Test_M88K_CPUComponent_Execute_DelayBranchWithValidInstruction_RunTwoTimes()
{
	GXemul gxemul;
	gxemul.GetCommandInterpreter().RunCommand("add testm88k");

	refcount_ptr<Component> cpu = gxemul.GetRootComponent()->LookupPath("root.machine0.mainbus0.cpu0");
	UnitTest::Assert("huh? no cpu?", !cpu.IsNULL());

	AddressDataBus* bus = cpu->AsAddressDataBus();
	UnitTest::Assert("cpu should be addressable", bus != NULL);

	uint32_t data32 = 0xcc000010;	// bsr.n 0x1000 + 10*4, i.e. 0x1040
	bus->AddressSelect(0x1000ULL);
	bus->WriteData(data32, BigEndian);

	data32 = 0x63df0010; // Something valid, addu r30, r31, 0x10
	bus->AddressSelect(0x1004ULL);
	bus->WriteData(data32, BigEndian);

	cpu->SetVariableValue("pc", "0x1000");
	UnitTest::Assert("setting pc failed?", cpu->GetVariable("pc")->ToInteger(), 0x1000);
	UnitTest::Assert("r30 before execute", cpu->GetVariable("r30")->ToInteger(), 0);
	UnitTest::Assert("r31 before execute", cpu->GetVariable("r31")->ToInteger(), 0xff0);

	// This tests that execute 2 steps (using single-stepping) will execute both
	// the delay branch and the delay slot instruction.
	gxemul.SetRunState(GXemul::Running);
	gxemul.Execute(1);

	// Should now be in the delay slot.
	UnitTest::Assert("pc should have changed 1", cpu->GetVariable("pc")->ToInteger(), 0x1004);
	UnitTest::Assert("delay slot after execute 1", cpu->GetVariable("inDelaySlot")->ToString(), "true");
	UnitTest::Assert("r30 after execute 1", cpu->GetVariable("r30")->ToInteger(), 0);
	UnitTest::Assert("r31 after execute 1", cpu->GetVariable("r31")->ToInteger(), 0xff0);

	gxemul.SetRunState(GXemul::Running);
	gxemul.Execute(1);

	UnitTest::Assert("delay slot after execute 2", cpu->GetVariable("inDelaySlot")->ToString(), "false");
	UnitTest::Assert("pc should have changed 2", cpu->GetVariable("pc")->ToInteger(), 0x1040);
	UnitTest::Assert("r30 after execute 2", cpu->GetVariable("r30")->ToInteger(), 0x1000);
	UnitTest::Assert("r31 after execute 2", cpu->GetVariable("r31")->ToInteger(), 0xff0);
}

static void Test_M88K_CPUComponent_Execute_DelayBranchWithFault()
{
	GXemul gxemul;
	gxemul.GetCommandInterpreter().RunCommand("add testm88k");

	refcount_ptr<Component> cpu = gxemul.GetRootComponent()->LookupPath("root.machine0.mainbus0.cpu0");
	UnitTest::Assert("huh? no cpu?", !cpu.IsNULL());

	AddressDataBus* bus = cpu->AsAddressDataBus();
	UnitTest::Assert("cpu should be addressable", bus != NULL);

	uint32_t data32 = 0xcc000010;	// bsr.n 0x1000 + 10*4, i.e. 0x1040
	bus->AddressSelect(0x1000ULL);
	bus->WriteData(data32, BigEndian);

	data32 = 0xffffffff; // Something invalid
	bus->AddressSelect(0x1004ULL);
	bus->WriteData(data32, BigEndian);

	cpu->SetVariableValue("pc", "0x1000");
	UnitTest::Assert("setting pc failed?", cpu->GetVariable("pc")->ToInteger(), 0x1000);
	UnitTest::Assert("r30 before execute", cpu->GetVariable("r30")->ToInteger(), 0);
	UnitTest::Assert("r31 before execute", cpu->GetVariable("r31")->ToInteger(), 0xff0);

	// This tests that execute 100 steps will only execute 1, if the instruction
	// in the delay slot fails.
	gxemul.SetRunState(GXemul::Running);
	gxemul.Execute(100);

	UnitTest::Assert("pc should have increased one step", cpu->GetVariable("pc")->ToInteger(), 0x1004ULL);
	UnitTest::Assert("should be in delay slot after execution", cpu->GetVariable("inDelaySlot")->ToString(), "true");

	gxemul.SetRunState(GXemul::SingleStepping);
	gxemul.Execute(1);

	UnitTest::Assert("pc should not have increased", cpu->GetVariable("pc")->ToInteger(), 0x1004ULL);
	UnitTest::Assert("should still be in delay slot", cpu->GetVariable("inDelaySlot")->ToString(), "true");
}

static void Test_M88K_CPUComponent_Execute_EarlyAbortDuringRuntime_Singlestep()
{
	GXemul gxemul;
	gxemul.GetCommandInterpreter().RunCommand("add testm88k");

	refcount_ptr<Component> cpu = gxemul.GetRootComponent()->LookupPath("root.machine0.mainbus0.cpu0");
	UnitTest::Assert("huh? no cpu?", !cpu.IsNULL());

	AddressDataBus* bus = cpu->AsAddressDataBus();
	UnitTest::Assert("cpu should be addressable", bus != NULL);

	uint32_t data32 = 0xcc000010;	// bsr.n 0x1000 + 10*4, i.e. 0x1040
	bus->AddressSelect(0x1000ULL);
	bus->WriteData(data32, BigEndian);

	data32 = 0xf400fc93;	// Something which is valid during interpretation,
				// but invalid during runtime (i.e. aborts).
	bus->AddressSelect(0x1004ULL);
	bus->WriteData(data32, BigEndian);

	cpu->SetVariableValue("pc", "0x1000");
	UnitTest::Assert("setting pc failed?", cpu->GetVariable("pc")->ToInteger(), 0x1000);

	// This tests that execute 100 steps will only execute 1, if the
	// instruction in the delay slot aborts early.
	gxemul.SetRunState(GXemul::SingleStepping);
	gxemul.Execute(100);

	UnitTest::Assert("1 step should have executed", cpu->GetVariable("step")->ToInteger(), 1);
	UnitTest::Assert("pc should have increased one step", cpu->GetVariable("pc")->ToInteger(), 0x1004ULL);
	UnitTest::Assert("should be in delay slot after execution", cpu->GetVariable("inDelaySlot")->ToString(), "true");
	UnitTest::Assert("delay target should have been updated", cpu->GetVariable("delaySlotTarget")->ToInteger(), 0x1040);

	gxemul.SetRunState(GXemul::SingleStepping);
	gxemul.Execute(1);

	UnitTest::Assert("no more steps should have executed", cpu->GetVariable("step")->ToInteger(), 1);
	UnitTest::Assert("pc should not have increased", cpu->GetVariable("pc")->ToInteger(), 0x1004ULL);
	UnitTest::Assert("should still be in delay slot", cpu->GetVariable("inDelaySlot")->ToString(), "true");
	UnitTest::Assert("delay target should not have been updated", cpu->GetVariable("delaySlotTarget")->ToInteger(), 0x1040);
}

static void Test_M88K_CPUComponent_Execute_EarlyAbortDuringRuntime_Running()
{
	GXemul gxemul;
	gxemul.GetCommandInterpreter().RunCommand("add testm88k");

	refcount_ptr<Component> cpu = gxemul.GetRootComponent()->LookupPath("root.machine0.mainbus0.cpu0");
	UnitTest::Assert("huh? no cpu?", !cpu.IsNULL());

	AddressDataBus* bus = cpu->AsAddressDataBus();
	UnitTest::Assert("cpu should be addressable", bus != NULL);

	uint32_t data32 = 0xcc000010;	// bsr.n 0x1000 + 10*4, i.e. 0x1040
	bus->AddressSelect(0x1000ULL);
	bus->WriteData(data32, BigEndian);

	data32 = 0xf400fc93;	// Something which is valid during interpretation,
				// but invalid during runtime (i.e. aborts).
	bus->AddressSelect(0x1004ULL);
	bus->WriteData(data32, BigEndian);

	cpu->SetVariableValue("pc", "0x1000");
	UnitTest::Assert("setting pc failed?", cpu->GetVariable("pc")->ToInteger(), 0x1000);

	// This tests that execute 100 steps will only execute 1, if the
	// instruction in the delay slot aborts early.
	gxemul.SetRunState(GXemul::Running);
	gxemul.Execute(100);

	UnitTest::Assert("1 step should have executed", cpu->GetVariable("step")->ToInteger(), 1);
	UnitTest::Assert("pc should have increased one step", cpu->GetVariable("pc")->ToInteger(), 0x1004ULL);
	UnitTest::Assert("should be in delay slot after execution", cpu->GetVariable("inDelaySlot")->ToString(), "true");
	UnitTest::Assert("delay target should have been updated", cpu->GetVariable("delaySlotTarget")->ToInteger(), 0x1040);

	gxemul.SetRunState(GXemul::SingleStepping);
	gxemul.Execute(1);

	UnitTest::Assert("no more steps should have executed", cpu->GetVariable("step")->ToInteger(), 1);
	UnitTest::Assert("pc should not have increased", cpu->GetVariable("pc")->ToInteger(), 0x1004ULL);
	UnitTest::Assert("should still be in delay slot", cpu->GetVariable("inDelaySlot")->ToString(), "true");
	UnitTest::Assert("delay target should not have been updated", cpu->GetVariable("delaySlotTarget")->ToInteger(), 0x1040);
}

UNITTESTS(M88K_CPUComponent)
{
	UNITTEST(Test_M88K_CPUComponent_IsStable);
	UNITTEST(Test_M88K_CPUComponent_Create);
	UNITTEST(Test_M88K_CPUComponent_Create_with_r31);
	UNITTEST(Test_M88K_CPUComponent_IsCPU);
	UNITTEST(Test_M88K_CPUComponent_DefaultModel);

	// Disassembly:
	UNITTEST(Test_M88K_CPUComponent_Disassembly_Basic);

	// Dyntrans execution:
	UNITTEST(Test_M88K_CPUComponent_Execute_Basic);
	UNITTEST(Test_M88K_CPUComponent_Execute_DelayBranchWithValidInstruction);
	UNITTEST(Test_M88K_CPUComponent_Execute_DelayBranchWithValidInstruction_SingleStepping);
	UNITTEST(Test_M88K_CPUComponent_Execute_DelayBranchWithValidInstruction_RunTwoTimes);
	UNITTEST(Test_M88K_CPUComponent_Execute_DelayBranchWithFault);
	UNITTEST(Test_M88K_CPUComponent_Execute_EarlyAbortDuringRuntime_Singlestep);
	UNITTEST(Test_M88K_CPUComponent_Execute_EarlyAbortDuringRuntime_Running);
//	UNITTEST(Test_M88K_CPUComponent_Execute_LateAbortDuringRuntime);
}

#endif

