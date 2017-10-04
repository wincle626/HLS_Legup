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

#include "components/MVME187Machine.h"
#include "ComponentFactory.h"


/*
 *  This is for experiments with OpenBSD/mvme88k.
 *
 *  MVME187 according to http://mcg.motorola.com/us/products/docs/pdf/187igd.pdf
 *  ("MVME187 RISC Single Board Computer Installation Guide"):
 *
 *	88100 MPU, two MC88200 or MC88204 CMMUs (one for data cache and
 *		one for instruction cache).
 *	82596CA LAN Ethernet
 *	53C710 SCSI
 *	CD2401 SCC SERIAL IO
 *	PRINTER PORT
 *	MK48T08 BBRAM & CLOCK
 *	EPROM
 *	VME bus
 *
 *  ... and more details from OpenBSD/mvme88k sources:
 *
 *	0xff800000 .. 0xffbfffff = BUG PROM
 *	0xffe00000 .. 0xffe1ffff = BUG SRAM
 *	0xfff00000               = PCCTWO
 *	  0xfff40000             = VME bus
 *	  0xfff43000             = MEMC040 (Memory controller)
 *	  0xfff45000             = CD2401 SCC SERIAL IO (cl0)
 *	  0xfff46000             = 82596 Ethernet (ie0)
 *	  0xfff47000             = 53C710 SCSI (osiop0)
 *	  0xfffc0000             = MK48T08 (nvram0)
 *
 *  Note: It may turn out to be easier to support Lance ethernet (via VME)
 *  than to support the 82596 ethernet controller.
 */


refcount_ptr<Component> MVME187Machine::Create(const ComponentCreateArgs& args)
{
	// Defaults:
	ComponentCreationSettings settings;
	settings["ram"] = "0x2000000";

	if (!ComponentFactory::GetCreationArgOverrides(settings, args))
		return NULL;

	refcount_ptr<Component> machine =
	    ComponentFactory::CreateComponent("machine");
	if (machine.IsNULL())
		return NULL;

	machine->SetVariableValue("template", "\"MVME187\"");

	refcount_ptr<Component> mainbus =
	    ComponentFactory::CreateComponent("mainbus");
	if (mainbus.IsNULL())
		return NULL;

	machine->AddChild(mainbus);

	refcount_ptr<Component> ram = ComponentFactory::CreateComponent("ram");
	if (ram.IsNULL())
		return NULL;

	ram->SetVariableValue("memoryMappedSize", settings["ram"]);
	mainbus->AddChild(ram);

	refcount_ptr<Component> rom = ComponentFactory::CreateComponent("ram");
	if (rom.IsNULL())
		return NULL;

	rom->SetVariableValue("name", "\"rom0\"");
	rom->SetVariableValue("memoryMappedBase", "0xff800000");
	rom->SetVariableValue("memoryMappedSize", "0x400000");
	mainbus->AddChild(rom);

	refcount_ptr<Component> cpu =
	    ComponentFactory::CreateComponent("m88k_cpu(model=88100)");

	if (cpu.IsNULL())
		return NULL;

	mainbus->AddChild(cpu);

	return machine;
}


string MVME187Machine::GetAttribute(const string& attributeName)
{
	if (attributeName == "template")
		return "yes";

	if (attributeName == "machine")
		return "yes";

	// if (attributeName == "stable")
	//	return "yes";

	if (attributeName == "description")
		return "MVME187 machine.";

	if (attributeName == "comments")
		return "For experiments with <a href=\"http://www.openbsd.org/mvme88k.html\">OpenBSD/mvme88k</a>.";

	return "";
}

