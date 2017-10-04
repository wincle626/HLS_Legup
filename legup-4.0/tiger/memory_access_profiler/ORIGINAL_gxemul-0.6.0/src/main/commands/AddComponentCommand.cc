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

#include "commands/AddComponentCommand.h"
#include "ComponentFactory.h"
#include "GXemul.h"


AddComponentCommand::AddComponentCommand()
	: Command("add", "component-name [path]")
{
}


AddComponentCommand::~AddComponentCommand()
{
}


static void ShowMsg(GXemul& gxemul, const string& msg)
{
	gxemul.GetUI()->ShowDebugMessage(msg);
}


bool AddComponentCommand::Execute(GXemul& gxemul, const vector<string>& arguments)
{
	if (arguments.size() < 1) {
		ShowMsg(gxemul, "No component-name given.\n");
		return false;
	}

	if (arguments.size() > 2) {
		ShowMsg(gxemul, "Too many arguments.\n");
		return false;
	}

	string componentName = arguments[0];

	refcount_ptr<Component> componentToAdd =
	    ComponentFactory::CreateComponent(componentName, &gxemul);

	if (componentToAdd.IsNULL()) {
		ShowMsg(gxemul, componentName + ": unknown component,"
		    " or invalid arguments given.\n");
		return false;
	}

	string path = "root";
	if (arguments.size() == 2)
		path = arguments[1];

	vector<string> matches = gxemul.GetRootComponent()->
	    FindPathByPartialMatch(path);
	if (matches.size() == 0) {
		ShowMsg(gxemul, path +
		    ": not a path to a known component.\n");
		return false;
	}
	if (matches.size() > 1) {
		ShowMsg(gxemul, path + " matches multiple components:\n");
		for (size_t i=0; i<matches.size(); i++)
			ShowMsg(gxemul, "  " + matches[i] + "\n");
		return false;
	}

	refcount_ptr<Component> whereToAddIt =
	    gxemul.GetRootComponent()->LookupPath(matches[0]);
	if (whereToAddIt.IsNULL()) {
		ShowMsg(gxemul, path + ": lookup of path failed.\n");
		return false;
	}

	whereToAddIt->AddChild(componentToAdd);

	return true;
}


string AddComponentCommand::GetShortDescription() const
{
	return "Adds a component to the emulation.";
}


string AddComponentCommand::GetLongDescription() const
{
	return
	    "Adds a component (given by the component-name) to the current emulation\n"
	    "setup. If path is omitted, the component is added at the root of the\n"
	    "component tree. For example:\n"
	    "\n"
	    "> add testmips                                  <-- this adds machine0\n"
	    "> root\n" 
	    "  root\n"
	    "  \\-- machine0  [testmips]\n"
	    "      \\-- mainbus0\n"
	    "          |-- ram0  (32 MB at offset 0)\n"
	    "          |-- rom0  (16 MB at offset 0x1fc00000)\n"
	    "          \\-- cpu0  (5KE, 100 MHz)\n"
	    "...\n"
	    "> add mips_cpu(model=R4400) mainbus0            <-- note that arguments\n"
	    "> root                                              can be given during\n"
	    "  root                                              component creation\n"
	    "  \\-- machine0  [testmips]\n"
	    "      \\-- mainbus0\n"
	    "          |-- ram0  (32 MB at offset 0)\n"
	    "          |-- rom0  (16 MB at offset 0x1fc00000)\n"
	    "          |-- cpu0  (5KE, 100 MHz)\n"
	    "          \\-- cpu1  (R4400, 100 MHz)\n"
	    "\n"
	    "See also:  copy             (to copy/clone a component in the tree)\n"
	    "           list-components  (to get a list of available component types)\n"
	    "           remove           (to remove a component from the emulation)\n"
	    "           root             (to inspect the current emulation setup)\n";
}

