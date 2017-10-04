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

#include <iomanip>
#include <assert.h>
#include <sys/mman.h>

#include "components/RAMComponent.h"
#include "GXemul.h"


RAMComponent::RAMComponent(const string& visibleClassName)
	: MemoryMappedComponent("ram", visibleClassName)
	, m_blockSizeShift(22)		// 22 = 4 MB per block
	, m_blockSize(1 << m_blockSizeShift)
	, m_dataHandler(*this)
	, m_writeProtected(false)
	, m_lastDumpAddr(0)
	, m_addressSelect(0)
	, m_selectedHostMemoryBlock(NULL)
	, m_selectedOffsetWithinBlock(0)
{
	AddVariable("writeProtect", &m_writeProtected);
	AddVariable("lastDumpAddr", &m_lastDumpAddr);
	AddCustomVariable("data", &m_dataHandler);
}


RAMComponent::~RAMComponent()
{
	ReleaseAllBlocks();
}


void RAMComponent::ReleaseAllBlocks()
{
	for (size_t i=0; i<m_memoryBlocks.size(); ++i) {
		if (m_memoryBlocks[i] != NULL) {
			munmap(m_memoryBlocks[i], m_blockSize);
			m_memoryBlocks[i] = NULL;
		}
	}
}


refcount_ptr<Component> RAMComponent::Create(const ComponentCreateArgs& args)
{
	return new RAMComponent();
}


string RAMComponent::GetAttribute(const string& attributeName)
{
	if (attributeName == "stable")
		return "yes";

	if (attributeName == "description")
		return "A generic RAM component.";

	return MemoryMappedComponent::GetAttribute(attributeName);
}


void RAMComponent::ResetState()
{
	ReleaseAllBlocks();
}


void RAMComponent::GetMethodNames(vector<string>& names) const
{
	// Add our method names...
	names.push_back("dump");

	// ... and make sure to call the base class implementation:
	Component::GetMethodNames(names);
}


bool RAMComponent::MethodMayBeReexecutedWithoutArgs(const string& methodName) const
{
	if (methodName == "dump")
		return true;

	// ... and make sure to call the base class implementation:
	return Component::MethodMayBeReexecutedWithoutArgs(methodName);
}


void RAMComponent::ExecuteMethod(GXemul* gxemul, const string& methodName,
	const vector<string>& arguments)
{
	if (methodName == "dump") {
		uint64_t vaddr = m_lastDumpAddr;

		if (arguments.size() > 1) {
			gxemul->GetUI()->ShowDebugMessage("syntax: .dump [addr]\n");
			return;
		}

		if (arguments.size() == 1) {
			gxemul->GetUI()->ShowDebugMessage("TODO: parse address expression\n");
			gxemul->GetUI()->ShowDebugMessage("(for now, only hex immediate values are supported!)\n");

			stringstream ss;
			ss << arguments[0];
			ss.flags(std::ios::hex);
			ss >> vaddr;
		}

		const int nRows = 16;
		for (int i=0; i<nRows; i++) {
			const size_t len = 16;
			unsigned char data[len];
			bool readable[len];

			stringstream ss;
			ss.flags(std::ios::hex);

			if (vaddr > 0xffffffff)
				ss << std::setw(16);
			else
				ss << std::setw(8);

			ss << std::setfill('0') << vaddr;

			size_t k;
			for (k=0; k<len; ++k) {
				AddressSelect(vaddr + k);
				readable[k] = ReadData(data[k], BigEndian);
			}
			
			ss << " ";
			
			for (k=0; k<len; ++k) {
				if ((k&3) == 0)
					ss << " ";

				ss << std::setw(2) << std::setfill('0');
				if (readable[k])
					ss << (int)data[k];
				else
					ss << "--";
			}

			ss << "  ";

			for (k=0; k<len; ++k) {
				char s[2];
				s[0] = data[k] >= 32 && data[k] < 127? data[k] : '.';
				s[1] = '\0';
				
				if (readable[k])
					ss << s;
				else
					ss << "-";
			}
			
			ss << "\n";

			gxemul->GetUI()->ShowDebugMessage(ss.str());

			vaddr += len;
		}

		m_lastDumpAddr = vaddr;

		return;
	}

	// Call base...
	Component::ExecuteMethod(gxemul, methodName, arguments);
}


AddressDataBus* RAMComponent::AsAddressDataBus()
{
	return this;
}


void RAMComponent::AddressSelect(uint64_t address)
{
	m_addressSelect = address;

	uint64_t blockNr = address >> m_blockSizeShift;

	if (blockNr+1 > m_memoryBlocks.size())
		m_selectedHostMemoryBlock = NULL;
	else
		m_selectedHostMemoryBlock = m_memoryBlocks[blockNr];

	m_selectedOffsetWithinBlock = address & (m_blockSize-1);
}


void* RAMComponent::AllocateBlock()
{
	void * p = mmap(NULL, m_blockSize, PROT_WRITE | PROT_READ,
	    MAP_ANON | MAP_PRIVATE, -1, 0);

	if (p == MAP_FAILED || p == NULL) {
		std::cerr << "RAMComponent::AllocateBlock: Could not allocate "
		    << m_blockSize << " bytes. Aborting.\n";
		throw std::exception();
	}

	uint64_t blockNr = m_addressSelect >> m_blockSizeShift;

	if (blockNr+1 > m_memoryBlocks.size())
		m_memoryBlocks.resize(blockNr + 1);

	m_memoryBlocks[blockNr] = p;

	return p;
}


bool RAMComponent::ReadData(uint8_t& data, Endianness endianness)
{
	if (m_selectedHostMemoryBlock == NULL)
		data = 0;
	else
		data = (((uint8_t*)m_selectedHostMemoryBlock)
		    [m_selectedOffsetWithinBlock]);

	return true;
}


bool RAMComponent::ReadData(uint16_t& data, Endianness endianness)
{
	assert((m_addressSelect & 1) == 0);

	if (m_selectedHostMemoryBlock == NULL)
		data = 0;
	else
		data = (((uint16_t*)m_selectedHostMemoryBlock)
		    [m_selectedOffsetWithinBlock >> 1]);

	if (endianness == BigEndian)
		data = BE16_TO_HOST(data);
	else
		data = LE16_TO_HOST(data);

	return true;
}


bool RAMComponent::ReadData(uint32_t& data, Endianness endianness)
{
	assert((m_addressSelect & 3) == 0);

	if (m_selectedHostMemoryBlock == NULL)
		data = 0;
	else
		data = (((uint32_t*)m_selectedHostMemoryBlock)
		    [m_selectedOffsetWithinBlock >> 2]);

	if (endianness == BigEndian)
		data = BE32_TO_HOST(data);
	else
		data = LE32_TO_HOST(data);

	return true;
}


bool RAMComponent::ReadData(uint64_t& data, Endianness endianness)
{
	assert((m_addressSelect & 7) == 0);

	if (m_selectedHostMemoryBlock == NULL)
		data = 0;
	else
		data = (((uint64_t*)m_selectedHostMemoryBlock)
		    [m_selectedOffsetWithinBlock >> 3]);

	if (endianness == BigEndian)
		data = BE64_TO_HOST(data);
	else
		data = LE64_TO_HOST(data);

	return true;
}


bool RAMComponent::WriteData(const uint8_t& data, Endianness endianness)
{
	if (m_writeProtected)
		return false;

	if (m_selectedHostMemoryBlock == NULL)
		m_selectedHostMemoryBlock = AllocateBlock();

	(((uint8_t*)m_selectedHostMemoryBlock)
	    [m_selectedOffsetWithinBlock]) = data;

	return true;
}


bool RAMComponent::WriteData(const uint16_t& data, Endianness endianness)
{
	assert((m_addressSelect & 1) == 0);

	if (m_writeProtected)
		return false;

	if (m_selectedHostMemoryBlock == NULL)
		m_selectedHostMemoryBlock = AllocateBlock();

	uint16_t d;
	if (endianness == BigEndian)
		d = BE16_TO_HOST(data);
	else
		d = LE16_TO_HOST(data);

	(((uint16_t*)m_selectedHostMemoryBlock)
	    [m_selectedOffsetWithinBlock >> 1]) = d;

	return true;
}


bool RAMComponent::WriteData(const uint32_t& data, Endianness endianness)
{
	assert((m_addressSelect & 3) == 0);

	if (m_writeProtected)
		return false;

	if (m_selectedHostMemoryBlock == NULL)
		m_selectedHostMemoryBlock = AllocateBlock();

	uint32_t d;
	if (endianness == BigEndian)
		d = BE32_TO_HOST(data);
	else
		d = LE32_TO_HOST(data);

	(((uint32_t*)m_selectedHostMemoryBlock)
	    [m_selectedOffsetWithinBlock >> 2]) = d;

	return true;
}


bool RAMComponent::WriteData(const uint64_t& data, Endianness endianness)
{
	assert((m_addressSelect & 7) == 0);

	if (m_writeProtected)
		return false;

	if (m_selectedHostMemoryBlock == NULL)
		m_selectedHostMemoryBlock = AllocateBlock();

	uint64_t d;
	if (endianness == BigEndian)
		d = BE64_TO_HOST(data);
	else
		d = LE64_TO_HOST(data);

	(((uint64_t*)m_selectedHostMemoryBlock)
	    [m_selectedOffsetWithinBlock >> 3]) = d;

	return true;
}


/*****************************************************************************/


#ifdef WITHUNITTESTS

#include "ComponentFactory.h"

static void Test_RAMComponent_IsStable()
{
	UnitTest::Assert("the RAMComponent should be stable",
	    ComponentFactory::HasAttribute("ram", "stable"));
}

static void Test_RAMComponent_AddressDataBus()
{
	refcount_ptr<Component> ram = ComponentFactory::CreateComponent("ram");

	AddressDataBus* bus = ram->AsAddressDataBus();
	UnitTest::Assert("The RAMComponent should implement the "
	    "AddressDataBus interface", bus != NULL);
}

static void Test_RAMComponent_InitiallyZero()
{
	refcount_ptr<Component> ram = ComponentFactory::CreateComponent("ram");
	AddressDataBus* bus = ram->AsAddressDataBus();

	bus->AddressSelect(0);

	// By default, RAM should be zero-filled:
	uint8_t data8 = 42;
	bus->ReadData(data8);
	UnitTest::Assert("A: memory should be zero filled (8)", data8, 0);

	uint16_t data16 = 142;
	bus->ReadData(data16, BigEndian);
	UnitTest::Assert("A: memory should be zero filled (16)", data16, 0);

	uint32_t data32 = 342;
	bus->ReadData(data32, BigEndian);
	UnitTest::Assert("A: memory should be zero filled (32)", data32, 0);

	uint64_t data64 = 942;
	bus->ReadData(data64, BigEndian);
	UnitTest::Assert("A: memory should be zero filled (64)", data64, 0);

	bus->AddressSelect(0x10000);

	data8 = 43;
	bus->ReadData(data8);
	UnitTest::Assert("B: memory should be zero filled (8)", data8, 0);

	data16 = 143;
	bus->ReadData(data16, BigEndian);
	UnitTest::Assert("B: memory should be zero filled (16)", data16, 0);

	data32 = 343;
	bus->ReadData(data32, BigEndian);
	UnitTest::Assert("B: memory should be zero filled (32)", data32, 0);

	data64 = 943;
	bus->ReadData(data64, BigEndian);
	UnitTest::Assert("B: memory should be zero filled (64)", data64, 0);
}

static void Test_RAMComponent_WriteThenRead()
{
	refcount_ptr<Component> ram = ComponentFactory::CreateComponent("ram");
	AddressDataBus* bus = ram->AsAddressDataBus();

	bus->AddressSelect(256);

	uint64_t data64 = ((uint64_t)0x1234567 << 32) | 0x89abcdef;
	bus->WriteData(data64, BigEndian);

	uint64_t data64_b = 0;
	bus->ReadData(data64_b, BigEndian);

	UnitTest::Assert("memory should be same when read", data64_b, data64);

	uint32_t data32_b = 0;
	bus->ReadData(data32_b, BigEndian);

	UnitTest::Assert("32-bit read should give top 32 bits, "
	    "in big endian mode", data32_b, data64 >> 32);

	uint16_t data16_b = 0;
	bus->ReadData(data16_b, BigEndian);

	UnitTest::Assert("16-bit read should give top 16 bits, "
	    "in big endian mode", data16_b, data64 >> 48);

	uint8_t data8_b = 0;
	bus->ReadData(data8_b);

	UnitTest::Assert("8-bit read should give top 8 bits, "
	    "in big endian mode", data8_b, data64 >> 56);
}

static void Test_RAMComponent_WriteThenRead_ReverseEndianness()
{
	refcount_ptr<Component> ram = ComponentFactory::CreateComponent("ram");
	AddressDataBus* bus = ram->AsAddressDataBus();

	bus->AddressSelect(256);

	uint64_t data64 = ((uint64_t)0x1234567 << 32) | 0x89abcdef;
	bus->WriteData(data64, BigEndian);

	bus->AddressSelect(256 + 4);

	uint32_t data32_b = 0;
	bus->ReadData(data32_b, LittleEndian);

	UnitTest::Assert("32-bit read", data32_b, 0xefcdab89);

	uint16_t data16_b = 0;
	bus->ReadData(data16_b, LittleEndian);

	UnitTest::Assert("16-bit read", data16_b, 0xab89);

	uint8_t data8_b = 0;
	bus->ReadData(data8_b);

	UnitTest::Assert("8-bit read", data8_b, 0x89);
}

static void Test_RAMComponent_WriteProtect()
{
	refcount_ptr<Component> ram = ComponentFactory::CreateComponent("ram");
	AddressDataBus* bus = ram->AsAddressDataBus();

	uint32_t data32 = 0x89abcdef;
	bus->AddressSelect(256);
	UnitTest::Assert("non-writeprotected write should succeed",
	    bus->WriteData(data32, BigEndian));

	uint16_t data16_a = 0;
	bus->AddressSelect(256 + 2);
	bus->ReadData(data16_a, BigEndian);
	UnitTest::Assert("16-bit read", data16_a, 0xcdef);

	ram->SetVariableValue("writeProtect", "true");

	data32 = 0x11223344;
	bus->AddressSelect(256);
	UnitTest::Assert("writeprotected write should fail",
	    bus->WriteData(data32, BigEndian) == false);

	data16_a = 0;
	bus->AddressSelect(256 + 2);
	bus->ReadData(data16_a, BigEndian);
	UnitTest::Assert("16-bit read", data16_a, 0xcdef);

	ram->SetVariableValue("writeProtect", "false");

	data32 = 0x12345678;
	bus->AddressSelect(256);
	UnitTest::Assert("write should succeed again",
	    bus->WriteData(data32, BigEndian));

	data16_a = 0;
	bus->AddressSelect(256 + 2);
	bus->ReadData(data16_a, BigEndian);
	UnitTest::Assert("16-bit read", data16_a, 0x5678);
}

static void Test_RAMComponent_ClearOnReset()
{
	refcount_ptr<Component> ram = ComponentFactory::CreateComponent("ram");
	AddressDataBus* bus = ram->AsAddressDataBus();

	uint32_t data32 = 0x89abcdef;
	bus->AddressSelect(256);
	bus->WriteData(data32, BigEndian);

	uint16_t data16_a = 0;
	bus->AddressSelect(256 + 2);
	bus->ReadData(data16_a, BigEndian);
	UnitTest::Assert("16-bit read", data16_a, 0xcdef);

	ram->Reset();

	uint16_t data16_b = 0;
	bus->AddressSelect(256 + 2);
	bus->ReadData(data16_b, BigEndian);
	UnitTest::Assert("reset should have cleared RAM", data16_b, 0x0000);
}

static void Test_RAMComponent_Clone()
{
	refcount_ptr<Component> ram = ComponentFactory::CreateComponent("ram");
	AddressDataBus* bus = ram->AsAddressDataBus();

	uint32_t data32 = 0x89abcdef;
	bus->AddressSelect(4);
	bus->WriteData(data32, BigEndian);

	uint16_t data16_a = 0x1234;
	bus->AddressSelect(10);
	bus->WriteData(data16_a, LittleEndian);

	UnitTest::Assert("serialization/deserialization failed?", ram->CheckConsistency());

	refcount_ptr<Component> clone = ram->Clone();
	bus = clone->AsAddressDataBus();

	data32 = 0x22222222;
	bus->AddressSelect(4);
	bus->ReadData(data32, LittleEndian);
	UnitTest::Assert("32-bit read", data32, 0xefcdab89);

	data16_a = 0xffff;
	bus->AddressSelect(10);
	bus->ReadData(data16_a, BigEndian);
	UnitTest::Assert("16-bit read", data16_a, 0x3412);
}

static void Test_RAMComponent_ManualSerialization()
{
	refcount_ptr<Component> ram = ComponentFactory::CreateComponent("ram");
	AddressDataBus* bus = ram->AsAddressDataBus();

	uint32_t data32 = 0x89abcde5;
	bus->AddressSelect(4);
	bus->WriteData(data32, BigEndian);

	uint16_t data16_a = 0x1235;
	bus->AddressSelect(10);
	bus->WriteData(data16_a, LittleEndian);

	SerializationContext context;
	stringstream ss;
	ram->Serialize(ss, context);

	string result = ss.str();
	size_t pos = 0;
	stringstream messages;
	refcount_ptr<Component> ram2 = Component::Deserialize(messages, result, pos);
	bus = ram2->AsAddressDataBus();

	data32 = 0x22222222;
	bus->AddressSelect(4);
	bus->ReadData(data32, LittleEndian);
	UnitTest::Assert("32-bit read", data32, 0xe5cdab89);

	data16_a = 0xffff;
	bus->AddressSelect(10);
	bus->ReadData(data16_a, BigEndian);
	UnitTest::Assert("16-bit read", data16_a, 0x3512);
}

static void Test_RAMComponent_Methods_Reexecutableness()
{
	refcount_ptr<Component> ram = ComponentFactory::CreateComponent("ram");

	UnitTest::Assert("dump method SHOULD be re-executable"
	    " without args", ram->MethodMayBeReexecutedWithoutArgs("dump") == true);

	UnitTest::Assert("nonexistant method should NOT be re-executable"
	    " without args", ram->MethodMayBeReexecutedWithoutArgs("nonexistant") == false);
}

UNITTESTS(RAMComponent)
{
	UNITTEST(Test_RAMComponent_IsStable);
	UNITTEST(Test_RAMComponent_AddressDataBus);
	UNITTEST(Test_RAMComponent_InitiallyZero);
	UNITTEST(Test_RAMComponent_WriteThenRead);
	UNITTEST(Test_RAMComponent_WriteThenRead_ReverseEndianness);
	UNITTEST(Test_RAMComponent_WriteProtect);
	UNITTEST(Test_RAMComponent_ClearOnReset);
	UNITTEST(Test_RAMComponent_Clone);
	UNITTEST(Test_RAMComponent_ManualSerialization);
	UNITTEST(Test_RAMComponent_Methods_Reexecutableness);
}

#endif

