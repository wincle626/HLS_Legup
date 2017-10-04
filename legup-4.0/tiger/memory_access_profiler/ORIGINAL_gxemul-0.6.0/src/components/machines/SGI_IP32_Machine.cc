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

#include "components/SGI_IP32_Machine.h"
#include "ComponentFactory.h"
#include "GXemul.h"


refcount_ptr<Component> SGI_IP32_Machine::Create(const ComponentCreateArgs& args)
{
	// Defaults:
	ComponentCreationSettings settings;
	settings["cpu"] = "R5000";	// R5000, RM7000, R10000, R12000
	settings["ram"] = "0x8000000";

	if (!ComponentFactory::GetCreationArgOverrides(settings, args))
		return NULL;

	refcount_ptr<Component> machine =
	    ComponentFactory::CreateComponent("machine");
	if (machine.IsNULL())
		return NULL;

	machine->SetVariableValue("template", "\"sgi_ip32\"");

	refcount_ptr<Component> mainbus =
	    ComponentFactory::CreateComponent("mainbus");
	if (mainbus.IsNULL())
		return NULL;

	machine->AddChild(mainbus);

	refcount_ptr<Component> ram = ComponentFactory::CreateComponent("ram");
	if (ram.IsNULL())
		return NULL;

	ram->SetVariableValue("memoryMappedBase", "0");
	ram->SetVariableValue("memoryMappedSize", settings["ram"]);
	mainbus->AddChild(ram);

	refcount_ptr<Component> prom = ComponentFactory::CreateComponent("ram");
	if (prom.IsNULL())
		return NULL;

	prom->SetVariableValue("name", "\"prom0\"");
	prom->SetVariableValue("memoryMappedBase", "0x1fc00000");
	prom->SetVariableValue("memoryMappedSize", "0x80000");
	// The prom (or at least part of it) is flashable/writeable.
	mainbus->AddChild(prom);

	refcount_ptr<Component> cpu = ComponentFactory::CreateComponent(
	    "mips_cpu(model=" + settings["cpu"] + ")");
	if (cpu.IsNULL())
		return NULL;

	mainbus->AddChild(cpu);

	return machine;
}


string SGI_IP32_Machine::GetAttribute(const string& attributeName)
{
	if (attributeName == "template")
		return "yes";

	if (attributeName == "machine")
		return "yes";

	// if (attributeName == "stable")
	//	return "yes";

	if (attributeName == "comments")
		return "For experiments with <a href=\"http://www.netbsd.org/ports/sgimips/\">NetBSD/sgimips</a>,"
		    " <a href=\"http://www.openbsd.org/sgi.html\">OpenBSD/sgi</a>, "
		    "Linux for O2, and possibly also <a href=\"http://en.wikipedia.org/wiki/SGI_O2\">SGI O2</a> PROMs and/or"
		    " <a href=\"http://en.wikipedia.org/wiki/IRIX\">IRIX</a> in the future.";

	if (attributeName == "description")
		return "SGI IP32 (O2) machine.";

	return "";
}

