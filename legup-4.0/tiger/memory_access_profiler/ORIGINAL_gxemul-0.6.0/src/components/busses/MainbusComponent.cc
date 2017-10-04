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

#include "components/MainbusComponent.h"
#include "GXemul.h"


MainbusComponent::MainbusComponent()
	: Component("mainbus", "mainbus")
	, m_memoryMapFailed(false)
	, m_memoryMapValid(false)
	, m_currentAddressDataBus(NULL)
{
}


MainbusComponent::~MainbusComponent()
{
}


refcount_ptr<Component> MainbusComponent::Create(const ComponentCreateArgs& args)
{
	return new MainbusComponent();
}


string MainbusComponent::GetAttribute(const string& attributeName)
{
	if (attributeName == "stable")
		return "yes";

	if (attributeName == "description")
		return "A generic main bus.";

	return Component::GetAttribute(attributeName);
}


void MainbusComponent::FlushCachedStateForComponent()
{
	m_memoryMap.clear();
	m_memoryMapValid = false;
	m_memoryMapFailed = false;

	m_currentAddressDataBus = NULL;
	
	Component::FlushCachedStateForComponent();
}


bool MainbusComponent::PreRunCheckForComponent(GXemul* gxemul)
{
	FlushCachedStateForComponent();
	if (!MakeSureMemoryMapExists(gxemul)) {
		gxemul->GetUI()->ShowDebugMessage(GenerateTreeDump(""));
		return false;
	}

	return true;
}


bool MainbusComponent::MakeSureMemoryMapExists(GXemul* gxemul)
{
	if (m_memoryMapFailed)
		return false;

	if (m_memoryMapValid)
		return true;

	m_memoryMap.clear();

	m_memoryMapValid = true;
	m_memoryMapFailed = false;

	// Build a memory map of all immediate children who implement the
	// AddressDataBus interface:
	Components children = GetChildren();
	for (size_t i=0; i<children.size(); ++i) {
		AddressDataBus* bus = children[i]->AsAddressDataBus();
		if (bus == NULL)
			continue;

		MemoryMapEntry mmEntry;
		mmEntry.addressDataBus = bus;
		mmEntry.addrMul = 1;
		mmEntry.base = 0;

		const StateVariable* varBase =
		    children[i]->GetVariable("memoryMappedBase");
		const StateVariable* varSize =
		    children[i]->GetVariable("memoryMappedSize");
		const StateVariable* varAddrMul =
		    children[i]->GetVariable("memoryMappedAddrMul");

		if (varBase != NULL)
			mmEntry.base = varBase->ToInteger();
		if (varSize != NULL)
			mmEntry.size = varSize->ToInteger();
		if (varAddrMul != NULL)
			mmEntry.addrMul = varAddrMul->ToInteger();

		// No base or size? Then skip this component.
		if (varSize == NULL || varBase == NULL)
			continue;

		// Treat the non-sensical addrMul value 0 as 1.
		if (mmEntry.addrMul == 0)
			mmEntry.addrMul = 1;

		// Empty memory mapped region? Then skip this component.
		if (mmEntry.size == 0)
			continue;

		// Check for overlaps against the already existing mappings.
		//
		// (Note: Current implementation results in O(n^2) time,
		// but if the number of memory-mapped immediate childs are
		// few, then this should not be a big problem.)
		for (size_t j=0; j<m_memoryMap.size(); ++j) {
			if (mmEntry.base+mmEntry.size <= m_memoryMap[j].base)
				continue;

			if (mmEntry.base >= m_memoryMap[j].base +
			    m_memoryMap[j].size)
				continue;
		
			// There is overlap!
			if (gxemul != NULL)
				gxemul->GetUI()->ShowDebugMessage(this,
				    "Error: the base and/or size of " +
				    children[i]->GenerateShortestPossiblePath() +
				    " conflicts with another memory mapped "
				    "component on this bus.\n");

			m_memoryMap.clear();
			m_memoryMapValid = false;
			m_memoryMapFailed = true;
			return false;
		}

		// Finally add the new mapping entry.
		m_memoryMap.push_back(mmEntry);
	}

	return true;
}


AddressDataBus* MainbusComponent::AsAddressDataBus()
{
	return this;
}


void MainbusComponent::AddressSelect(uint64_t address)
{
	MakeSureMemoryMapExists();

	m_currentAddressDataBus = NULL;

	if (!m_memoryMapValid)
		return;

	// Note: This is a linear O(n) scan of the list of memory-mapped
	// components. For small n, this should be ok.
	//
	// In practice, my hope is that the bulk of all memory access will be
	// using direct-mapped pages anyway, so this should not be that much
	// of a problem.

	for (size_t i=0; i<m_memoryMap.size(); ++i) {
		MemoryMapEntry& mmEntry = m_memoryMap[i];

		// If this memory map entry contains the address we wish to
		// select, then...
		if (address >= mmEntry.base &&
		    address < mmEntry.base + mmEntry.size) {
			// ... tell the corresponding component which address
			// within it we wish to select.
			m_currentAddressDataBus = mmEntry.addressDataBus;
			m_currentAddressDataBus->AddressSelect(
			    (address - mmEntry.base) / mmEntry.addrMul);
			break;
		}
	}
}


bool MainbusComponent::ReadData(uint8_t& data, Endianness endianness)
{
	if (!MakeSureMemoryMapExists())
		return false;

	if (m_currentAddressDataBus != NULL)
		return m_currentAddressDataBus->ReadData(data, endianness);
	else
		return false;
}


bool MainbusComponent::ReadData(uint16_t& data, Endianness endianness)
{
	if (!MakeSureMemoryMapExists())
		return false;

	if (m_currentAddressDataBus != NULL)
		return m_currentAddressDataBus->ReadData(data, endianness);
	else
		return false;
}


bool MainbusComponent::ReadData(uint32_t& data, Endianness endianness)
{
	if (!MakeSureMemoryMapExists())
		return false;

	if (m_currentAddressDataBus != NULL)
		return m_currentAddressDataBus->ReadData(data, endianness);
	else
		return false;
}


bool MainbusComponent::ReadData(uint64_t& data, Endianness endianness)
{
	if (!MakeSureMemoryMapExists())
		return false;

	if (m_currentAddressDataBus != NULL)
		return m_currentAddressDataBus->ReadData(data, endianness);
	else
		return false;
}


bool MainbusComponent::WriteData(const uint8_t& data, Endianness endianness)
{
	if (!MakeSureMemoryMapExists())
		return false;

	if (m_currentAddressDataBus != NULL)
		return m_currentAddressDataBus->WriteData(data, endianness);
	else
		return false;
}


bool MainbusComponent::WriteData(const uint16_t& data, Endianness endianness)
{
	if (!MakeSureMemoryMapExists())
		return false;

	if (m_currentAddressDataBus != NULL)
		return m_currentAddressDataBus->WriteData(data, endianness);
	else
		return false;
}


bool MainbusComponent::WriteData(const uint32_t& data, Endianness endianness)
{
	if (!MakeSureMemoryMapExists())
		return false;

	if (m_currentAddressDataBus != NULL)
		return m_currentAddressDataBus->WriteData(data, endianness);
	else
		return false;
}


bool MainbusComponent::WriteData(const uint64_t& data, Endianness endianness)
{
	if (!MakeSureMemoryMapExists())
		return false;

	if (m_currentAddressDataBus != NULL)
		return m_currentAddressDataBus->WriteData(data, endianness);
	else
		return false;
}


/*****************************************************************************/


#ifdef WITHUNITTESTS

#include "ComponentFactory.h"

static void Test_MainbusComponent_IsStable()
{
	UnitTest::Assert("the MainbusComponent should be stable",
	    ComponentFactory::HasAttribute("mainbus", "stable"));
}

static void Test_MainbusComponent_Creatable()
{
	refcount_ptr<Component> mainbus =
	    ComponentFactory::CreateComponent("mainbus");

	UnitTest::Assert("The MainbusComponent should be "
	    "instanciable", !mainbus.IsNULL());
}

static void Test_MainbusComponent_AddressDataBus()
{
	refcount_ptr<Component> mainbus =
	    ComponentFactory::CreateComponent("mainbus");

	AddressDataBus* bus = mainbus->AsAddressDataBus();
	UnitTest::Assert("The MainbusComponent should implement the "
	    "AddressDataBus interface", bus != NULL);
}

static void Test_MainbusComponent_Simple()
{
	refcount_ptr<Component> mainbus =
	    ComponentFactory::CreateComponent("mainbus");
	refcount_ptr<Component> ram0 =
	    ComponentFactory::CreateComponent("ram");

	mainbus->AddChild(ram0);
	ram0->SetVariableValue("memoryMappedSize", "0x100000");
	ram0->SetVariableValue("memoryMappedBase", "0");

	AddressDataBus* bus = mainbus->AsAddressDataBus();

	uint8_t dataByte = 42;
	bus->AddressSelect(128);
	bus->WriteData(dataByte);
	bus->AddressSelect(129);
	dataByte = 100;
	bus->WriteData(dataByte);
	bus->AddressSelect(128);
	bus->ReadData(dataByte);
	UnitTest::Assert("memory wasn't written to correctly?", dataByte, 42);
	bus->AddressSelect(129);
	bus->ReadData(dataByte);
	UnitTest::Assert("memory wasn't written to correctly?", dataByte, 100);
}

static void Test_MainbusComponent_Remapping()
{
	refcount_ptr<Component> mainbus =
	    ComponentFactory::CreateComponent("mainbus");
	refcount_ptr<Component> ram0 =
	    ComponentFactory::CreateComponent("ram");

	mainbus->AddChild(ram0);
	ram0->SetVariableValue("memoryMappedSize", "0x10000");
	ram0->SetVariableValue("memoryMappedBase", "0x1000");

	AddressDataBus* bus = mainbus->AsAddressDataBus();

	uint8_t dataByte = 123;
	bus->AddressSelect(0x1030);	// offset 0x30 of ram0
	bus->WriteData(dataByte);
	dataByte = 18;
	bus->AddressSelect(0x1050);	// offset 0x50 of ram0
	bus->WriteData(dataByte);

	// Set a new base for ram0, but do _NOT_ flush cached state yet!
	// (This is to assert that cached state is actually cached.)
	ram0->SetVariableValue("memoryMappedBase", "0x1020");

	uint8_t dataByte2 = 99;
	bus->AddressSelect(0x1050);	// offset 0x30 of ram0
	bus->ReadData(dataByte2);
	UnitTest::Assert("remapping should NOT have taken place yet, "
	    "cached state should still be in effect!", dataByte2, 18);

	// Now, flush the state and make sure the new mapping takes effect:
	mainbus->FlushCachedState();

	dataByte2 = 99;
	bus->AddressSelect(0x1050);	// offset 0x30 of ram0
	bus->ReadData(dataByte2);
	UnitTest::Assert("remapping failed?", dataByte2, 123);
}

static void Test_MainbusComponent_Multiple_NonOverlapping()
{
	refcount_ptr<Component> mainbus =
	    ComponentFactory::CreateComponent("mainbus");
	refcount_ptr<Component> ram0 =
	    ComponentFactory::CreateComponent("ram");
	refcount_ptr<Component> ram1 =
	    ComponentFactory::CreateComponent("ram");
	refcount_ptr<Component> ram2 =
	    ComponentFactory::CreateComponent("ram");

	mainbus->AddChild(ram0);
	mainbus->AddChild(ram1);
	mainbus->AddChild(ram2);
	ram0->SetVariableValue("memoryMappedSize", "0x100");
	ram0->SetVariableValue("memoryMappedBase", "0x000");
	ram1->SetVariableValue("memoryMappedSize", "0x100");
	ram1->SetVariableValue("memoryMappedBase", "0x100");
	ram2->SetVariableValue("memoryMappedSize", "0x100");
	ram2->SetVariableValue("memoryMappedBase", "0x200");

	AddressDataBus* bus = mainbus->AsAddressDataBus();

	for (size_t i = 0; i < 0x300; i += sizeof(uint16_t)) {
		uint16_t data = (uint8_t) i;
		bus->AddressSelect(i);
		bus->WriteData(data, LittleEndian);
	}

	for (size_t i = 0; i < 0x300; i += sizeof(uint16_t)) {
		uint16_t data;
		bus->AddressSelect(i);
		bus->ReadData(data, BigEndian);
		UnitTest::Assert("memory wasn't written to correctly?",
		   data, (uint16_t) (i << 8));
	}
}

static void Test_MainbusComponent_Simple_With_AddrMul()
{
	refcount_ptr<Component> mainbus =
	    ComponentFactory::CreateComponent("mainbus");
	refcount_ptr<Component> ram0 =
	    ComponentFactory::CreateComponent("ram");

	mainbus->AddChild(ram0);
	ram0->SetVariableValue("memoryMappedSize", "0x1000");
	ram0->SetVariableValue("memoryMappedBase", "0x80");
	ram0->SetVariableValue("memoryMappedAddrMul", "5");

	AddressDataBus* bus = mainbus->AsAddressDataBus();

	uint8_t dataByte = 42;
	bus->AddressSelect(128);
	bus->WriteData(dataByte);
	bus->AddressSelect(133);
	dataByte = 100;
	bus->WriteData(dataByte);

	bus->AddressSelect(128);
	bus->ReadData(dataByte);
	UnitTest::Assert("memory wasn't written to correctly?", dataByte, 42);
	bus->AddressSelect(133);
	bus->ReadData(dataByte);
	UnitTest::Assert("memory wasn't written to correctly?", dataByte, 100);

	ram0->SetVariableValue("memoryMappedAddrMul", "2");
	mainbus->FlushCachedState();

	bus->AddressSelect(128);
	bus->ReadData(dataByte);
	UnitTest::Assert("addr mul strangeness?", dataByte, 42);
	bus->AddressSelect(130);
	bus->ReadData(dataByte);
	UnitTest::Assert("offset 130 should have the same value as "
	    "offset 133 had", dataByte, 100);
	bus->AddressSelect(133);
	bus->ReadData(dataByte);
	UnitTest::Assert("offset 133 should NOT have any value "
	    "written to it yet!", dataByte, 0);

	ram0->SetVariableValue("memoryMappedAddrMul", "1");
	mainbus->FlushCachedState();

	bus->AddressSelect(128);
	bus->ReadData(dataByte);
	UnitTest::Assert("addr mul strangeness [2]", dataByte, 42);
	bus->AddressSelect(129);
	bus->ReadData(dataByte);
	UnitTest::Assert("offset 129 mismatch [2]", dataByte, 100);
	bus->AddressSelect(133);
	bus->ReadData(dataByte);
	UnitTest::Assert("offset 133 should NOT have any value "
	    "written to it yet! [2]", dataByte, 0);

	// AddrMul "0" should be same as "1", because the user might think that
	// address multiplication is "turned off" by setting it to 0.
	ram0->SetVariableValue("memoryMappedAddrMul", "0");
	mainbus->FlushCachedState();

	bus->AddressSelect(128);
	bus->ReadData(dataByte);
	UnitTest::Assert("addr mul strangeness [3]", dataByte, 42);
	bus->AddressSelect(129);
	bus->ReadData(dataByte);
	UnitTest::Assert("offset 129 mismatch [3]", dataByte, 100);
	bus->AddressSelect(133);
	bus->ReadData(dataByte);
	UnitTest::Assert("offset 133 should NOT have any value "
	    "written to it yet! [3]", dataByte, 0);
}

static void Test_MainbusComponent_PreRunCheck()
{
	GXemul gxemul;

	gxemul.GetCommandInterpreter().RunCommand("add testmips");

	UnitTest::Assert("preruncheck should initially succeed for testmips",
	    gxemul.GetRootComponent()->PreRunCheck(&gxemul) == true);

	// Adding a second RAM component should succeed, since the initial size
	// is 0, and does not (yet) conflict.
	gxemul.GetCommandInterpreter().RunCommand("add ram mainbus0");
	UnitTest::Assert("preruncheck should still succeed",
	    gxemul.GetRootComponent()->PreRunCheck(&gxemul) == true);

	// By changing the size to 42, the new ram1 component will overlap
	// the ram0 component (at least partially).
	gxemul.GetCommandInterpreter().RunCommand("ram1.memoryMappedSize = 42");
	UnitTest::Assert("preruncheck should now fail",
	    gxemul.GetRootComponent()->PreRunCheck(&gxemul) == false);
}

UNITTESTS(MainbusComponent)
{
	// Construction, etc.:
	UNITTEST(Test_MainbusComponent_IsStable);
	UNITTEST(Test_MainbusComponent_Creatable);
	UNITTEST(Test_MainbusComponent_AddressDataBus);

	// Memory mapping, ranges, overlaps, addrmul, etc.:
	UNITTEST(Test_MainbusComponent_Simple);
	UNITTEST(Test_MainbusComponent_Remapping);
	UNITTEST(Test_MainbusComponent_Multiple_NonOverlapping);
	UNITTEST(Test_MainbusComponent_Simple_With_AddrMul);

	// TODO: Write outside of mapped space
	// TODO: Write PARTIALLY outside of mapped space!!! e.g. 64-bit
	//	into a 3-byte memory area???

	UNITTEST(Test_MainbusComponent_PreRunCheck);
}

#endif

