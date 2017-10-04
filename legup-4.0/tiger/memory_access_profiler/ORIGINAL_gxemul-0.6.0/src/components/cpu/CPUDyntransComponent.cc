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
#include <iomanip>

#include "AddressDataBus.h"
#include "components/CPUDyntransComponent.h"
#include "GXemul.h"


CPUDyntransComponent::CPUDyntransComponent(const string& className, const string& cpuArchitecture)
	: CPUComponent(className, cpuArchitecture)
{
	m_abortIC.f = instr_abort;
}


void CPUDyntransComponent::DyntransInit()
{
	m_nextIC = NULL;
	m_firstIConPage = NULL;

	m_dyntransICshift = GetDyntransICshift();

	m_dyntransICentriesPerPage = m_pageSize >> m_dyntransICshift;
	m_dyntransPageMask = (m_pageSize - 1) - ((1 << m_dyntransICshift) - 1);

	int pageShift = 0;
	while (pageShift < 32 && (1 << pageShift) != m_pageSize)
		pageShift ++;

	if (pageShift >= 32) {
		std::cerr << "Non-power-of-2 page size?\n";
		throw std::exception();
	}

	// 32 MB translation cache (per emulated CPU):
	m_translationCache.Reinit(32 * 1024 * 1024, m_dyntransICentriesPerPage + DYNTRANS_PAGE_NSPECIALENTRIES, pageShift);
}


/*
 * Dynamic translation core
 * ------------------------
 *
 * The core of GXemul's dynamic translation is a simple function call to
 * an entry in an array of pointers. Each call also moves the pointer of the
 * next function call to the next entry in the array. For most simple
 * instruction implementations, the instruction call pointer (m_nextIC) does
 * not have to be modified, because it is assumed that an instruction will
 * change the program counter to the next instruction.
 *
 * Before starting the main loop, the pc is used to look up the correct
 * m_nextIC value, by calling DyntransPCtoPointers().
 *
 * During the loop, the pc value is _not_ necessarily updated for each
 * instruction call. Instead, the low bits of the pc value should be considered
 * meaningless, and the offset of the m_nextIC pointer within the current
 * code page (pointed to by m_firstIConPage) defines the lowest pc bits.
 *
 * After completing the main loop, the pc value is resynched by calling
 * DyntransResyncPC().
 */
int CPUDyntransComponent::Execute(GXemul* gxemul, int nrOfCycles)
{
	DyntransInit();

	DyntransPCtoPointers();

	struct DyntransIC *ic = m_nextIC;
	if (m_nextIC == NULL || m_firstIConPage == NULL) {
		std::cerr << "Internal error: m_nextIC or m_firstIConPage is NULL.\n";
		throw std::exception();
	}

	if (gxemul->GetRunState() == GXemul::SingleStepping) {
		if (nrOfCycles != 1) {
			std::cerr << "Internal error: Single stepping,"
			    " but nrOfCycles = " << nrOfCycles << ".\n";
			throw std::exception();
		}

		stringstream disasm;
		Unassemble(1, false, PCtoInstructionAddress(m_pc), disasm);
		gxemul->GetUI()->ShowDebugMessage(this, disasm.str());

		m_nextIC->f = GetDyntransToBeTranslated();
	}

	/*
	 * The normal instruction execution core: Get the instruction call pointer
	 * (and move the nextIC to the following instruction in advance), then
	 * execute the instruction call by calling its f.
	 */
#define IC	ic = m_nextIC ++; ic->f(this, ic);

	m_nrOfCyclesToExecute = nrOfCycles;
	m_executedCycles = 0;

	// Starting inside a delay slot? Then execute it carefully:
	if (m_inDelaySlot) {
		m_nextIC->f = GetDyntransToBeTranslated();
		m_executedCycles = m_nrOfCyclesToExecute - 1;
		IC
		m_executedCycles -= (m_nrOfCyclesToExecute - 1);
		m_executedCycles ++;

		// Fault in delay slot: return immediately.
		if (m_nextIC->f == instr_abort) {
			m_nextIC->f = GetDyntransToBeTranslated();
			return 0;
		}
	}

	// If possible, do some optimized loops of multiple inlined IC calls...
	const int ICsPerLoop = 60;
	const int maxICcycles = 2;	// TODO: Longer when instr combos are reimplemented
	if (nrOfCycles > ICsPerLoop * maxICcycles) {
		int hazard = nrOfCycles - ICsPerLoop * maxICcycles;

		for (;;) {
			IC IC IC IC IC   IC IC IC IC IC
			IC IC IC IC IC   IC IC IC IC IC
			IC IC IC IC IC   IC IC IC IC IC

			IC IC IC IC IC   IC IC IC IC IC
			IC IC IC IC IC   IC IC IC IC IC
			IC IC IC IC IC   IC IC IC IC IC

			m_executedCycles += ICsPerLoop;
			if (m_executedCycles >= hazard ||
			    m_nextIC->f == instr_abort)
				break;
		}
	}

	// ... then slowly execute the last few instructions.
	// Note: -1, because the last thing we execute may be an instruction
	// with a delay slot (which is automatically executed).
	for (; m_executedCycles<nrOfCycles-1; ) {
		int old = m_executedCycles;
		IC
		m_executedCycles ++;
		if (m_executedCycles == old)
			break;
	}

	// If there's one instruction left (and we're not aborted), then
	// let's execute it:
	if (m_executedCycles<nrOfCycles && m_nextIC->f != instr_abort) {
		m_nextIC->f = GetDyntransToBeTranslated();
		IC
		m_executedCycles ++;
	}

	DyntransResyncPC();

	// If execution aborted, then reset the aborting instruction slot
	// to the to-be-translated function:
	if (m_nextIC->f == instr_abort)
		m_nextIC->f = GetDyntransToBeTranslated();

	return m_executedCycles;
}


void CPUDyntransComponent::DyntransClearICPage(struct DyntransIC* icpage)
{
	// Fill the page with "to be translated" entries, which when executed
	// will read the instruction from memory, attempt to translate it, and
	// then execute it.
	void (*f)(CPUDyntransComponent*, DyntransIC*) = GetDyntransToBeTranslated();

	for (int i=0; i<m_dyntransICentriesPerPage; ++i)
		icpage[i].f = f;

	// ... and set the entries after the last instruction slot to
	// special "end of page" handlers.
	icpage[m_dyntransICentriesPerPage + 0].f = CPUDyntransComponent::instr_endOfPage;
	icpage[m_dyntransICentriesPerPage + 1].f = CPUDyntransComponent::instr_endOfPage2;
}


struct DyntransIC *CPUDyntransComponent::DyntransGetICPage(uint64_t addr)
{
	bool clear = false;
	struct DyntransIC *icpage = m_translationCache.GetICPage(
	    addr, m_showFunctionTraceCall, clear);

	if (clear) {
		// This is either
		//
		// a) a completely new page (the address was not in the cache),
		// or
		// b) a page which was translated before, but with different
		//    settings (e.g. m_showFunctionTraceCall).
		//
		// So let's fill the page with suitable to-be-translated
		// function pointers.
		DyntransClearICPage(icpage);
	}

	return icpage;
}


void CPUDyntransComponent::DyntransPCtoPointers()
{
	if (m_nextIC != NULL && m_nextIC->f == instr_abort) {
		// Already aborted, let's not update m_nextIC.
		std::cerr << "TODO: Already aborted, let's not update m_nextIC."
		    " Is this correct behavior?\n";
		return;
	}

	m_firstIConPage = DyntransGetICPage(m_pc);

	assert(m_firstIConPage != NULL);

	// Here, m_firstIConPage points to a valid page. Calculate m_nextIC from
	// the low bits of m_pc:
	int offsetWithinPage = (m_pc & m_dyntransPageMask) >> m_dyntransICshift;
	m_nextIC = m_firstIConPage + offsetWithinPage;
}


void CPUDyntransComponent::DyntransResyncPC()
{
	// Special case during aborts:
	if (m_nextIC == &m_abortIC) {
		// The situation which caused m_nextIC to be set to an abort
		// IC must have synched PC just before that. So we don't need
		// to do anything here.
		return;
	}

	ptrdiff_t instructionIndex = m_nextIC - m_firstIConPage;

	// On a page with e.g. 1024 instruction slots, instructionIndex is usually
	// between 0 and 1023. This means that the PC points to within this
	// page.
	//
	// We synchronize the PC by clearing out the bits within the IC page,
	// and then adding the offset to the instruction.
	if (instructionIndex >= 0 && instructionIndex < m_dyntransICentriesPerPage) {
		m_pc &= ~m_dyntransPageMask;
		m_pc += (instructionIndex << m_dyntransICshift);
		return;
	}

	// However, the instruction index may point outside the IC page.
	// This happens when synching the PC just after the last instruction
	// on a page has been executed. This means that we set the PC to
	// the start of the next page.
	if (instructionIndex == m_dyntransICentriesPerPage) {
		m_pc &= ~m_dyntransPageMask;
		m_pc += (m_dyntransPageMask + (1 << m_dyntransICshift));
		return;
	}

	if (instructionIndex == m_dyntransICentriesPerPage + 1) {
		std::cerr << "TODO: DyntransResyncPC: Second end-of-page slot.\n";
		// This may happen for delay-slot architectures.
		throw std::exception();
	}

	std::cerr << "TODO: DyntransResyncPC: next ic outside of page?!\n";
	throw std::exception();
}


void CPUDyntransComponent::DyntransToBeTranslatedBegin(struct DyntransIC* ic)
{
	// Resynchronize the PC to the instruction currently being translated.
	// (m_nextIC should already have been increased, to point to the _next_
	// instruction slot.)
	m_nextIC = ic;
	DyntransResyncPC();

	// TODO: Check for m_pc breakpoints etc.

	// First, let's assume that the translation will fail.
	ic->f = NULL;
}


bool CPUDyntransComponent::DyntransReadInstruction(uint16_t& iword)
{
	// TODO: Fast lookup.

	AddressSelect(PCtoInstructionAddress(m_pc));
	bool readable = ReadData(iword, m_isBigEndian? BigEndian : LittleEndian);

	if (!readable) {
		UI* ui = GetUI();
		if (ui != NULL) {
			stringstream ss;
			ss.flags(std::ios::hex);
			ss << "instruction at 0x" << PCtoInstructionAddress(m_pc)
			    << " could not be read!";
			ui->ShowDebugMessage(this, ss.str());
		}

		return false;
	}

	return true;
}


bool CPUDyntransComponent::DyntransReadInstruction(uint32_t& iword)
{
	// TODO: Fast lookup.

	AddressSelect(PCtoInstructionAddress(m_pc));
	bool readable = ReadData(iword, m_isBigEndian? BigEndian : LittleEndian);

	if (!readable) {
		UI* ui = GetUI();
		if (ui != NULL) {
			stringstream ss;
			ss.flags(std::ios::hex);
			ss << "instruction at 0x" << PCtoInstructionAddress(m_pc)
			    << " could not be read!";
			ui->ShowDebugMessage(this, ss.str());
		}
		return false;
	}

	return true;
}


void CPUDyntransComponent::DyntransToBeTranslatedDone(struct DyntransIC* ic)
{
	bool abort = false;

	if (ic->f == NULL || ic->f == instr_abort) {
		abort = true;

		// Instruction translation failed. If we were running in
		// quiet mode, then simply dropping into the GXemul> prompt
		// with no good explanation would be bad, so we always turn
		// off quiet mode on Aborts:
		GetRunningGXemulInstance()->SetQuietMode(false);

		UI* ui = GetUI();
		if (ui != NULL) {
			bool isSingleStepping = GetRunningGXemulInstance()->GetRunState() == GXemul::SingleStepping;

			stringstream ss;
			ss.flags(std::ios::hex);
			ss << "instruction translation failed";

			// If we were single-stepping, then the instruction
			// disassembly has already been displayed. If we were
			// running in continuous mode, then we need to display
			// it now:
			if (!isSingleStepping) {
				ss << " at";

				string symbol = GetSymbolRegistry().LookupAddress(
				    PCtoInstructionAddress(m_pc), true);
				if (symbol != "")
					ss << " " << symbol;

				ss << ":\n";

				Unassemble(1, false, PCtoInstructionAddress(m_pc), ss);
			}

			ui->ShowDebugMessage(this, ss.str());
		}

		if (ic->f == NULL)
			ic->f = instr_abort;
	}

	// Finally, execute the translated instruction.
	bool ds = m_inDelaySlot;
	bool dsExceptionOrAbort = m_exceptionOrAbortInDelaySlot;
	bool singleInstructionLeft = m_executedCycles == m_nrOfCyclesToExecute - 1;

	m_nextIC = ic + 1;
	ic->f(this, ic);

	if (m_nextIC == &m_abortIC)
		abort = true;

	if (singleInstructionLeft && !abort) {
		// If this instruction was in the delay slot of another instruction,
		// and we are running a single instruction, then manually branch to
		// the branch target:
		if (ds && !dsExceptionOrAbort) {
			m_pc = m_delaySlotTarget;
			DyntransPCtoPointers();
			m_inDelaySlot = false;
			m_exceptionOrAbortInDelaySlot = false;
		}

		// Don't leave any "single instruction left" instructions
		// in any of the slots:
		ic->f = GetDyntransToBeTranslated();
	}
}


/*****************************************************************************/


/*
 * A do-nothing instruction. (It still counts as a cylce, though.)
 */
DYNTRANS_INSTR(CPUDyntransComponent,nop)
{
}


/*
 * A break-out-of-dyntrans function. Setting ic->f to this function will
 * cause dyntrans execution to be aborted. The cycle counter will _not_
 * count this as executed cycles.
 */
DYNTRANS_INSTR(CPUDyntransComponent,abort)
{
	// Cycle reduction:
	-- cpubase->m_executedCycles;

	// Are we in a delay slot?
	if (cpubase->m_inDelaySlot)
		cpubase->m_exceptionOrAbortInDelaySlot = true;

	cpubase->m_nextIC = ic;
}


DYNTRANS_INSTR(CPUDyntransComponent,endOfPage)
{
	std::cerr << "TODO: endOfPage\n";
	throw std::exception();
}


DYNTRANS_INSTR(CPUDyntransComponent,endOfPage2)
{
	std::cerr << "TODO: endOfPage2\n";
	throw std::exception();
}


/*
 * arg 0: pointer to the new IC
 *
 * Branches within a dyntrans page.
 */
DYNTRANS_INSTR(CPUDyntransComponent,branch_samepage)
{
	cpubase->m_nextIC = (struct DyntransIC *) ic->arg[0].p;
}


/*
 * arg 0: 64-bit register
 * arg 1: 32-bit signed immediate
 *
 * Sets the register at arg 0 to the immediate value in arg 1.
 */
DYNTRANS_INSTR(CPUDyntransComponent,set_u64_imms32)
{
	REG64(ic->arg[0]) = (int32_t) ic->arg[1].u32;
}


/*
 * arg 0: 64-bit register
 * arg 1: 64-bit register
 *
 * Moves (copies) the contents of arg 1 to arg 0.
 */
DYNTRANS_INSTR(CPUDyntransComponent,mov_u64_u64)
{
	REG64(ic->arg[0]) = REG64(ic->arg[1]);
}


/*
 * arg 0: 32-bit register
 * arg 1: 32-bit register
 * arg 2: 32-bit unsigned immediate
 *
 * Adds the unsigned immediate to arg 1, and stores the result in arg 0.
 */
DYNTRANS_INSTR(CPUDyntransComponent,add_u32_u32_immu32)
{
	REG32(ic->arg[0]) = REG32(ic->arg[1]) + (uint32_t)ic->arg[2].u32;
}


/*
 * arg 0: 32-bit register
 * arg 1: 32-bit register
 * arg 2: 32-bit register
 *
 * Adds arg 1 and arg 2, and stores the result in arg 0.
 */
DYNTRANS_INSTR(CPUDyntransComponent,add_u32_u32_u32)
{
	REG32(ic->arg[0]) = REG32(ic->arg[1]) + REG32(ic->arg[2]);
}


/*
 * arg 0: 64-bit register
 * arg 1: 64-bit register
 * arg 2: 32-bit signed immediate
 *
 * Adds the signed immediate to arg 1, and stores the result in arg 0, truncated
 * to a signed 32-bit value.
 */
DYNTRANS_INSTR(CPUDyntransComponent,add_u64_u64_imms32_truncS32)
{
	REG64(ic->arg[0]) = (int32_t) (REG64(ic->arg[1]) + (int32_t)ic->arg[2].u32);
}


/*
 * arg 0: 64-bit register
 * arg 1: 64-bit register
 * arg 2: 64-bit register
 *
 * Adds the the registers in arg 1 and arg 2, and stores the result in arg 0
 * (truncated to a signed 32-bit value).
 */
DYNTRANS_INSTR(CPUDyntransComponent,add_u64_u64_u64_truncS32)
{
	REG64(ic->arg[0]) = (int32_t) (REG64(ic->arg[1]) + REG64(ic->arg[2]));
}


/*
 * arg 0: 64-bit register
 * arg 1: 64-bit register
 * arg 2: 32-bit signed immediate
 *
 * Adds the signed immediate to arg 1, and stores the result in arg 0.
 */
DYNTRANS_INSTR(CPUDyntransComponent,add_u64_u64_imms32)
{
	REG64(ic->arg[0]) = REG64(ic->arg[1]) + (int64_t)(int32_t)ic->arg[2].u32;
}


/*
 * arg 0: 32-bit register
 * arg 1: 32-bit register
 * arg 2: 32-bit unsigned immediate
 *
 * Subtracts the unsigned immediate from arg 1, and stores the result in arg 0.
 */
DYNTRANS_INSTR(CPUDyntransComponent,sub_u32_u32_immu32)
{
	REG32(ic->arg[0]) = REG32(ic->arg[1]) - (uint32_t)ic->arg[2].u32;
}


/*
 * arg 0: 32-bit register
 * arg 1: 32-bit register
 * arg 2: 32-bit register
 *
 * Subtracts arg 2 from arg 1, and stores the result in arg 0.
 */
DYNTRANS_INSTR(CPUDyntransComponent,sub_u32_u32_u32)
{
	REG32(ic->arg[0]) = REG32(ic->arg[1]) - REG32(ic->arg[2]);
}


/*
 * arg 0: 64-bit register
 * arg 1: 64-bit register
 * arg 2: 64-bit register
 *
 * Subtracts arg2 from arg1, and stores the result in arg0
 * (truncated to a signed 32-bit value).
 */
DYNTRANS_INSTR(CPUDyntransComponent,sub_u64_u64_u64_truncS32)
{
	REG64(ic->arg[0]) = (int32_t) (REG64(ic->arg[1]) - REG64(ic->arg[2]));
}


/*
 * arg 0: 32-bit register
 * arg 1: 32-bit register
 * arg 2: 32-bit unsigned immediate
 *
 * ANDs the 32-bit immediate into arg 1, storing the result in arg 0.
 */
DYNTRANS_INSTR(CPUDyntransComponent,and_u32_u32_immu32)
{
	REG32(ic->arg[0]) = REG32(ic->arg[1]) & ic->arg[2].u32;
}


/*
 * arg 0: 64-bit register
 * arg 1: 64-bit register
 * arg 2: 32-bit unsigned immediate
 *
 * ANDs the 32-bit immediate into arg 1, storing the result in arg 0.
 *
 * Note: No sign truncation is performed, i.e. if arg 1 is 0xffffffff80001234
 * and arg 2 is 0x80001200, then arg 0 becomes 0x0000000080001200 (note: the
 * upper bits are not sign-extended from bit 31).
 */
DYNTRANS_INSTR(CPUDyntransComponent,and_u64_u64_immu32)
{
	REG64(ic->arg[0]) = REG64(ic->arg[1]) & (uint32_t)ic->arg[2].u32;
}


/*
 * arg 0: 32-bit register
 * arg 1: 32-bit register
 * arg 2: 32-bit unsigned immediate
 *
 * ORs arg 1 and arg 2 together, and stores the result in arg 0.
 */
DYNTRANS_INSTR(CPUDyntransComponent,or_u32_u32_immu32)
{
	REG32(ic->arg[0]) = REG32(ic->arg[1]) | ic->arg[2].u32;
}


/*
 * arg 0: 32-bit register
 * arg 1: 32-bit register
 * arg 2: 32-bit register
 *
 * ORs arg 1 and arg 2 together, and stores the result in arg 0.
 */
DYNTRANS_INSTR(CPUDyntransComponent,or_u32_u32_u32)
{
	REG32(ic->arg[0]) = REG32(ic->arg[1]) | REG32(ic->arg[2]);
}


/*
 * arg 0: 64-bit register
 * arg 1: 64-bit register
 * arg 2: 32-bit unsigned immediate
 *
 * ORs the 32-bit immediate into arg 1, storing the result in arg 0.
 *
 * Note: No sign truncation is performed, i.e. if arg 1 is 0x0000000000001234
 * and arg 2 is 0x80001200, then arg 0 becomes 0x0000000080001234 (note: the
 * upper bits are not sign extended from bit 31).
 */
DYNTRANS_INSTR(CPUDyntransComponent,or_u64_u64_immu32)
{
	REG64(ic->arg[0]) = REG64(ic->arg[1]) | (uint32_t)ic->arg[2].u32;
}


/*
 * arg 0: 32-bit register
 * arg 1: 32-bit register
 * arg 2: 32-bit unsigned immediate
 *
 * XORs arg 1 and arg 2, and stores the result in arg 0.
 */
DYNTRANS_INSTR(CPUDyntransComponent,xor_u32_u32_immu32)
{
	REG32(ic->arg[0]) = REG32(ic->arg[1]) ^ ic->arg[2].u32;
}


/*
 * arg 0: 32-bit register
 * arg 1: 32-bit register
 * arg 2: 32-bit register
 *
 * XORs arg 1 and arg 2, and stores the result in arg 0.
 */
DYNTRANS_INSTR(CPUDyntransComponent,xor_u32_u32_u32)
{
	REG32(ic->arg[0]) = REG32(ic->arg[1]) ^ REG32(ic->arg[2]);
}


/*
 * arg 0: 64-bit register
 * arg 1: 64-bit register
 * arg 2: 32-bit unsigned immediate
 *
 * XORs the 32-bit immediate into arg 1, storing the result in arg 0.
 *
 * Note: No sign truncation is performed, i.e. if arg 1 is 0xffffffff80001234
 * and arg 2 is 0x80001200, then arg 0 becomes 0xffffffff00000034 (note: the
 * upper bits are not sign-extended from bit 31).
 */
DYNTRANS_INSTR(CPUDyntransComponent,xor_u64_u64_immu32)
{
	REG64(ic->arg[0]) = REG64(ic->arg[1]) ^ (uint32_t)ic->arg[2].u32;
}


/*
 * arg 0: 64-bit register
 * arg 1: 64-bit register
 * arg 2: 64-bit register
 *
 * XORs the arg 1 and arg 2, storing the result in arg 0.
 *
 * Note: No sign truncation is performed, i.e. if arg 1 is 0xffffffff80001234
 * and arg 2 is 0x80001200, then arg 0 becomes 0xffffffff00000034 (note: the
 * upper bits are not sign-extended from bit 31).
 */
DYNTRANS_INSTR(CPUDyntransComponent,xor_u64_u64_u64)
{
	REG64(ic->arg[0]) = REG64(ic->arg[1]) ^ REG64(ic->arg[2]);
}


/*
 * arg 0: 64-bit register
 * arg 1: 64-bit register
 * arg 2: 5-bit immediate
 *
 * Left-shifts arg 1 the number of steps indicated by the immediate, storing
 * the result in arg 0 truncated to a signed 32-bit value.
 */
DYNTRANS_INSTR(CPUDyntransComponent,shift_left_u64_u64_imm5_truncS32)
{
	REG64(ic->arg[0]) = (int32_t)(REG64(ic->arg[1]) << (ic->arg[2].u32 & 0x1f));
}


/*
 * arg 0: 64-bit register
 * arg 1: 64-bit register
 * arg 2: 5-bit immediate
 *
 * Right-shifts arg 1 (truncated into an unsigned 32-bit) the number of steps
 * indicated by the immediate, storing the result in arg 0 truncated to a 
 * signed 32-bit value.
 */
DYNTRANS_INSTR(CPUDyntransComponent,shift_right_u64_u64asu32_imm5_truncS32)
{
	REG64(ic->arg[0]) = (int32_t)(((uint32_t)REG64(ic->arg[1])) >> (ic->arg[2].u32 & 0x1f));
}


/*****************************************************************************/


#ifdef WITHUNITTESTS

#include "ComponentFactory.h"

static void Test_CPUDyntransComponent_Dyntrans_PreReq()
{
	UnitTest::Assert("nr of dyntrans args too few", N_DYNTRANS_IC_ARGS >= 3);
}

UNITTESTS(CPUDyntransComponent)
{
	UNITTEST(Test_CPUDyntransComponent_Dyntrans_PreReq);
}

#endif

