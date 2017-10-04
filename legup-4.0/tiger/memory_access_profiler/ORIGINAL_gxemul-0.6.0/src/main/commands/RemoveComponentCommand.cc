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

#include "commands/RemoveComponentCommand.h"
#include "GXemul.h"


RemoveComponentCommand::RemoveComponentCommand()
	: Command("remove", "path")
{
}


RemoveComponentCommand::~RemoveComponentCommand()
{
}


static void ShowMsg(GXemul& gxemul, const string& msg)
{
	gxemul.GetUI()->ShowDebugMessage(msg);
}


bool RemoveComponentCommand::Execute(GXemul& gxemul, const vector<string>& arguments)
{
	if (arguments.size() < 1) {
		ShowMsg(gxemul, "No path given.\n");
		return false;
	}

	if (arguments.size() > 1) {
		ShowMsg(gxemul, "Too many arguments.\n");
		return false;
	}

	string path = arguments[0];

	vector<string> matches = gxemul.GetRootComponent()->
	    FindPathByPartialMatch(path);
	if (matches.size() == 0) {
		ShowMsg(gxemul, path+" is not a path to a known component.\n");
		return false;
	}
	if (matches.size() > 1) {
		ShowMsg(gxemul, path+" matches multiple components:\n");
		for (size_t i=0; i<matches.size(); i++)
			ShowMsg(gxemul, "  " + matches[i] + "\n");
		return false;
	}

	refcount_ptr<Component> whatToRemove =
	    gxemul.GetRootComponent()->LookupPath(matches[0]);
	if (whatToRemove.IsNULL()) {
		ShowMsg(gxemul, "Lookup of " + path + " failed.\n");
		return false;
	}

	refcount_ptr<Component> parent = whatToRemove->GetParent();
	if (parent.IsNULL()) {
		ShowMsg(gxemul, "Cannot remove the root component.\n");
		return false;
	}

	parent->RemoveChild(whatToRemove);

	return true;
}


string RemoveComponentCommand::GetShortDescription() const
{
	return "Removes a component from the emulation.";
}


string RemoveComponentCommand::GetLongDescription() const
{
	return
	    "Removes a component (given a path) from the current emulation setup.\n"
	    "The following example shows how to remove cpu1 from a particular machine:\n"
	    "\n"
	    "> root\n"
	"  root\n"
	"  \\-- machine0  [testmips]\n"
	"      \\-- mainbus0\n"
	"          |-- ram0  (32 MB at offset 0)\n"
	"          |-- rom0  (16 MB at offset 0x1fc00000)\n"
	"          |-- cpu0  (MIPS, 100 MHz)\n"
	"          \\-- cpu1  (MIPS, 100 MHz)\n"
	"> remove cpu1 \n"
	"> root\n" 
	"  root\n"
	"  \\-- machine0  [testmips]\n"
	"      \\-- mainbus0\n"
	"          |-- ram0  (32 MB at offset 0)\n"
	"          |-- rom0  (16 MB at offset 0x1fc00000)\n"
	"          \\-- cpu0  (MIPS, 100 MHz)\n"
	"\n"
	    "See also:  add     (to add new components)\n"
	    "           root    (to inspect the current emulation setup)\n";
}

