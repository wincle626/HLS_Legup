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

#include "components/SGI_IP30_Machine.h"
#include "ComponentFactory.h"
#include "GXemul.h"


refcount_ptr<Component> SGI_IP30_Machine::Create(const ComponentCreateArgs& args)
{
	// Defaults:
	ComponentCreationSettings settings;
	settings["cpu"] = "R10000";	// or R12000 or R14000
	settings["ram"] = "0x8000000";
	settings["ncpus"] = "1";

	if (!ComponentFactory::GetCreationArgOverrides(settings, args))
		return NULL;

	refcount_ptr<Component> machine =
	    ComponentFactory::CreateComponent("machine");
	if (machine.IsNULL())
		return NULL;

	machine->SetVariableValue("template", "\"sgi_ip30\"");

	refcount_ptr<Component> mainbus =
	    ComponentFactory::CreateComponent("mainbus");
	if (mainbus.IsNULL())
		return NULL;

	machine->AddChild(mainbus);

	refcount_ptr<Component> ram = ComponentFactory::CreateComponent("ram");
	if (ram.IsNULL())
		return NULL;

	ram->SetVariableValue("memoryMappedBase", "0x20000000");
	ram->SetVariableValue("memoryMappedSize", settings["ram"]);
	mainbus->AddChild(ram);

	refcount_ptr<Component> rom = ComponentFactory::CreateComponent("ram");
	if (rom.IsNULL())
		return NULL;

	stringstream tmpss2;
	tmpss2 << 4 * 1048576;		// TODO: Actual ROM size
	rom->SetVariableValue("name", "\"rom0\"");
	rom->SetVariableValue("memoryMappedBase", "0x1fc00000");
	rom->SetVariableValue("memoryMappedSize", tmpss2.str());
	rom->SetVariableValue("writeProtect", "true");
	mainbus->AddChild(rom);

	int ncpus;
	stringstream tmpss3;
	tmpss3 << settings["ncpus"];
	tmpss3 >> ncpus;
	if (ncpus < 1) {
		if (args.gxemul != NULL)
			args.gxemul->GetUI()->ShowDebugMessage("nr of cpus must be more than 0.");
		return NULL;
	}

	for (int i=0; i<ncpus; ++i) {
		refcount_ptr<Component> cpu =
		    ComponentFactory::CreateComponent("mips_cpu(model=" + settings["cpu"] + ")");
		if (cpu.IsNULL())
			return NULL;

		if (i > 0)
			cpu->SetVariableValue("paused", "true");

		mainbus->AddChild(cpu);
	}

	return machine;
}


string SGI_IP30_Machine::GetAttribute(const string& attributeName)
{
	if (attributeName == "template")
		return "yes";

	if (attributeName == "machine")
		return "yes";

	// if (attributeName == "stable")
	//	return "yes";

	if (attributeName == "comments")
		return "For experiments with <a href=\"http://www.netbsd.org/ports/sgimips/\">NetBSD/sgimips</a>, and possibly"
		    " also Linux for SGI Octane in the future.";

	if (attributeName == "description")
		return "SGI IP30 (Octane) machine.";

	return "";
}

