#ifndef CPUCOMPONENT_H
#define	CPUCOMPONENT_H

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

// Note: Not included in the component registry.


#include "AddressDataBus.h"
#include "Component.h"
#include "SymbolRegistry.h"
#include "UnitTest.h"


/**
 * \brief A base-class for processors Component implementations.
 */
class CPUComponent
	: public Component
	, public AddressDataBus
	, public UnitTestable
{
public:
	/**
	 * \brief Constructs a CPUComponent.
	 *
	 * @param className The class name for the component.
	 * @param cpuKind The CPU kind, e.g. "MIPS R4400" for a
	 *	MIPS R4400 processor.
	 */
	CPUComponent(const string& className, const string& cpuKind);

	/**
	 * \brief Gets a reference to the CPU's symbol registry.
	 *
	 * @return A reference to the symbol registry.
	 */
	SymbolRegistry& GetSymbolRegistry()
	{
		return m_symbolRegistry;
	}
	const SymbolRegistry& GetSymbolRegistry() const
	{
		return m_symbolRegistry;
	}

	virtual void ResetState();

	virtual double GetCurrentFrequency() const;

	virtual CPUComponent* AsCPUComponent();

        virtual void GetMethodNames(vector<string>& names) const;

	virtual bool MethodMayBeReexecutedWithoutArgs(const string& methodName) const;

	virtual void ExecuteMethod(GXemul* gxemul,
		const string& methodName,
		const vector<string>& arguments);

	virtual AddressDataBus* AsAddressDataBus();

	/* Implementation of AddressDataBus: */
	virtual void AddressSelect(uint64_t address);
	virtual bool ReadData(uint8_t& data, Endianness endianness);
	virtual bool ReadData(uint16_t& data, Endianness endianness);
	virtual bool ReadData(uint32_t& data, Endianness endianness);
	virtual bool ReadData(uint64_t& data, Endianness endianness);
	virtual bool WriteData(const uint8_t& data, Endianness endianness);
	virtual bool WriteData(const uint16_t& data, Endianness endianness);
	virtual bool WriteData(const uint32_t& data, Endianness endianness);
	virtual bool WriteData(const uint64_t& data, Endianness endianness);

	/**
	 * \brief Disassembles an instruction into readable strings.
	 *
	 * @param vaddr The virtual address of the program counter.
	 * @param maxLen The number of bytes in the instruction buffer.
	 * @param instruction A pointer to a buffer containing the instruction.
	 * @param result A vector where the implementation will add:
	 *	<ol>
	 *		<li>machine code bytes in a standard notation
	 *		<li>instruction mnemonic
	 *		<li>instruction arguments
	 *		<li>instruction comments
	 *	</ol>
	 *	All of the fields above are optional, but they have to be
	 *	specified in the same order for a particular CPU implementation,
	 *	so that the fields of the vector can be listed in a tabular
	 *	format.
	 * @return The number of bytes that the instruction occupied.
	 */	
	virtual size_t DisassembleInstruction(uint64_t vaddr, size_t maxLen,
		unsigned char *instruction, vector<string>& result) = 0;


	/********************************************************************/

	static void RunUnitTests(int& nSucceeded, int& nFailures);

protected:
	virtual void FlushCachedStateForComponent();
	virtual bool PreRunCheckForComponent(GXemul* gxemul);
	virtual void ShowRegisters(GXemul* gxemul, const vector<string>& arguments) const;

	uint64_t Unassemble(int nRows, bool indicatePC, uint64_t vaddr, ostream& output);

	/**
	 * \brief Virtual to physical address translation (MMU).
	 *
	 * This function should be overridden in each CPU implementation.
	 *
	 * @param vaddr The virtual address to translate.
	 * @param paddr The return value; physical address.
	 * @param writable This is set to true or false by the function,
	 *	depending on if the memory at the virtual address was
	 *	writable or not.
	 * @return True if the translation succeeded, false if there was a
	 *	translation error.
	 */
	virtual bool VirtualToPhysical(uint64_t vaddr, uint64_t& paddr,
					bool& writable) = 0;

	/**
	 * \brief Format a virtual address as a displayable string.
	 *
	 * This function may be overridden in each CPU implementation.
	 * The default implementation just uses the stringstream << operator.
	 *
	 * @param vaddr The virtual address to translate.
	 * @return A string rendering of the virtual address, e.g. "0x00100f00"
	 */
	virtual string VirtualAddressAsString(uint64_t vaddr)
	{
		stringstream ss;
		ss.flags(std::ios::hex | std::ios::showbase);
		ss << vaddr;
		return ss.str();
	}

	/**
	 * \brief Convert PC value to instuction address.
	 *
	 * Usually, this does not need to be overridden. However, some
	 * architectures use e.g. the lowest bit of the PC register to indicate
	 * a different encoding mode (MIPS16), but the instruction is still
	 * aligned as if the lowest bit was 0.
	 */
	virtual uint64_t PCtoInstructionAddress(uint64_t pc)
	{
		return pc;
	}

	// CPUComponent:
	bool FunctionTraceCall();
	bool FunctionTraceReturn();

	// Overridden by each CPU architecture:
	virtual int FunctionTraceArgumentCount() { return 0; }
	virtual int64_t FunctionTraceArgument(int n) { return 0; }
	virtual bool FunctionTraceReturnImpl(int64_t& retval) { return false; }

private:
	bool LookupAddressDataBus(GXemul* gxemul = NULL);

protected:
	/*
	 * Variables common to all (or most) kinds of CPUs:
	 */

	// Framework frequency/runability:
	double			m_frequency;
	bool			m_paused;

	// Architecture fundamentals:
	string			m_cpuArchitecture;
	int			m_pageSize;

	// Program counter:
	uint64_t		m_pc;

	// Memory dump and disassembly:
	uint64_t		m_lastDumpAddr;
	uint64_t		m_lastUnassembleVaddr;
	bool			m_hasUsedUnassemble;

	// Endianness:
	bool			m_isBigEndian;

	// Function call trace:
	bool			m_showFunctionTraceCall;
	bool			m_showFunctionTraceReturn;
	int32_t			m_functionCallTraceDepth;
	int64_t			m_nrOfTracedFunctionCalls;

	// Delay slot related:
	bool			m_inDelaySlot;
	uint64_t		m_delaySlotTarget;


	/*
	 * Cached/volatile state:
	 */
	AddressDataBus *	m_addressDataBus;
	uint64_t		m_addressSelect;
	bool			m_exceptionOrAbortInDelaySlot;

private:
	SymbolRegistry		m_symbolRegistry;
};


#endif	// CPUCOMPONENT_H
