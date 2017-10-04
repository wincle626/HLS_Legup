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

#include "commands/SaveCommand.h"
#include "GXemul.h"

#include <fstream>


SaveCommand::SaveCommand()
	: Command("save", "[filename [component-path]]")
{
}


SaveCommand::~SaveCommand()
{
}


static void ShowMsg(GXemul& gxemul, const string& msg)
{
	gxemul.GetUI()->ShowDebugMessage(msg);
}


bool SaveCommand::Execute(GXemul& gxemul, const vector<string>& arguments)
{
	string filename = gxemul.GetEmulationFilename();
	string path = "root";

	if (arguments.size() > 2) {
		ShowMsg(gxemul, "Too many arguments.\n");
		return false;
	}

	if (arguments.size() > 0)
		filename = arguments[0];

	if (filename == "") {
		ShowMsg(gxemul, "No filename given.\n");
		return false;
	}

	if (arguments.size() > 1)
		path = arguments[1];

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

	refcount_ptr<Component> component =
	    gxemul.GetRootComponent()->LookupPath(matches[0]);
	if (component.IsNULL()) {
		ShowMsg(gxemul, "Lookup of " + path + " failed.\n");
		return false;
	}

	const string extension = ".gxemul";
	if (filename.length() < extension.length() || filename.substr(
	    filename.length() - extension.length()) != extension)
		ShowMsg(gxemul, "Warning: the name "+filename+" does not have"
		    " a .gxemul extension. Continuing anyway.\n");

	// Write to the file:
	{
		std::fstream outputstream(filename.c_str(),
		    std::ios::out | std::ios::trunc);
		if (outputstream.fail()) {
			ShowMsg(gxemul, "Error: Could not open " + filename +
			    " for writing.\n");
			return false;
		}

		SerializationContext context;
		component->Serialize(outputstream, context);
	}

	// Check that the file exists:
	{
		std::fstream inputstream(filename.c_str(), std::ios::in);
		if (inputstream.fail()) {
			ShowMsg(gxemul, "Error: Could not open " + filename +
			    " for reading after writing to it; saving "
			    " the emulation setup failed!\n");
			return false;
		}
	}

	ShowMsg(gxemul, "Emulation setup saved to " + filename + "\n");
	gxemul.SetEmulationFilename(filename);

	return true;
}


string SaveCommand::GetShortDescription() const
{
	return "Saves the emulation to a file.";
}


string SaveCommand::GetLongDescription() const
{
	return
	    "Saves the entire emulation setup, or a part of it, to a file in the filesystem.\n"
	    "The filename may be omitted, if it is known from an earlier save or load\n"
	    "command. If the component path is omitted, the entire emulation setup, starting\n"
	    "from the 'root' component, is saved.\n"
	    "\n"
	    "The filename extension should usually be .gxemul.\n"
	    "\n"
	    "See also:  load    (to load an emulation setup)\n";
}

