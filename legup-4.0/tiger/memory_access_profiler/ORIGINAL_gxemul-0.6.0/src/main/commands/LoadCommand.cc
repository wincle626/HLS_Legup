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

#include <fstream>
#include <stdlib.h>
#include <string.h>

#include "commands/LoadCommand.h"
#include "FileLoader.h"
#include "GXemul.h"


LoadCommand::LoadCommand()
	: Command("load", "[filename [component-path]]")
{
}


LoadCommand::~LoadCommand()
{
}


static void ShowMsg(GXemul& gxemul, const string& msg)
{
	gxemul.GetUI()->ShowDebugMessage(msg);
}


bool LoadCommand::IsComponentTree(GXemul& gxemul, const string& filename) const
{
	std::ifstream file(filename.c_str());
	if (file.fail())
		return false;

	char buf[256];
	file.read(buf, sizeof(buf));
	if (file.gcount() < 10)
		return false;

	// Saved component trees start with the string "component ".
	return (strncmp(buf, "component ", 10) == 0);
}


bool LoadCommand::LoadComponentTree(GXemul& gxemul, const string&filename,
	refcount_ptr<Component> specifiedComponent) const
{
	const string extension = ".gxemul";
	if (filename.length() < extension.length() || filename.substr(
	    filename.length() - extension.length()) != extension)
		ShowMsg(gxemul, "Warning: the name " + filename +
		    " does not have a .gxemul extension. Continuing anyway.\n");

	refcount_ptr<Component> component;

	// Load from the file
	std::ifstream file(filename.c_str());
	if (file.fail()) {
		ShowMsg(gxemul, "Unable to open " + filename + " for reading.\n");
		return false;
	}

	// Figure out the file's size:
	file.seekg(0, std::ios::end);
	std::streampos fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	// Read the entire file into a string.
	// TODO: This is wasteful, of course. It actually takes twice the
	// size of the file, since the string constructor generates a _copy_.
	// But string takes care of unicode and such (if compiled as ustring).
	vector<char> buf;
	buf.resize((size_t)fileSize + 1);

	memset(&buf[0], 0, fileSize);
	file.read(&buf[0], fileSize);
	if (file.gcount() != fileSize) {
		ShowMsg(gxemul, "Loading from " + filename + " failed; "
		    "could not read all of the file?\n");
		return false;
	}

	string str(&buf[0], fileSize);

	file.close();

	size_t strPos = 0;
	stringstream messages;
	component = Component::Deserialize(messages, str, strPos);

	if (messages.str().length() > 0)
		ShowMsg(gxemul, messages.str());

	if (component.IsNULL()) {
		ShowMsg(gxemul, "Loading from " + filename + " failed; "
		    "no component found?\n");
		return false;
	}

	if (specifiedComponent.IsNULL()) {
		const StateVariable* name = component->GetVariable("name");
		if (name == NULL || name->ToString() != "root")
			gxemul.GetRootComponent()->AddChild(component);
		else
			gxemul.SetRootComponent(component);

		gxemul.SetEmulationFilename(filename);

		ShowMsg(gxemul, filename + " loaded\n");
	} else {
		specifiedComponent->AddChild(component);

		ShowMsg(gxemul, filename + " loaded into " +
		    specifiedComponent->GenerateShortestPossiblePath() + "\n");
	}

	return true;
}

bool LoadCommand::Execute(GXemul& gxemul, const vector<string>& arguments)
{
	string filename = gxemul.GetEmulationFilename();
	string path = "";

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

	// Figure out the component path, if it was specified.
	refcount_ptr<Component> specifiedComponent;
	if (path != "") {
		vector<string> matches = gxemul.GetRootComponent()->
		    FindPathByPartialMatch(path);
		if (matches.size() == 0) {
			ShowMsg(gxemul, path +
			    " is not a path to a known component.\n");
			return false;
		}
		if (matches.size() > 1) {
			ShowMsg(gxemul, path +
			    " matches multiple components:\n");
			for (size_t i=0; i<matches.size(); i++)
				ShowMsg(gxemul, "  " + matches[i] + "\n");
			return false;
		}

		specifiedComponent =
		    gxemul.GetRootComponent()->LookupPath(matches[0]);
		if (specifiedComponent.IsNULL()) {
			ShowMsg(gxemul, "Lookup of " + path + " failed.\n");
			return false;
		}
	}

	// 1. Is it a component tree?
	if (IsComponentTree(gxemul, filename)) {
		if (specifiedComponent.IsNULL())
			gxemul.ClearEmulation();

		return LoadComponentTree(gxemul, filename, specifiedComponent);
	}

	if (specifiedComponent.IsNULL()) {
		ShowMsg(gxemul, "The specified file to load (" + filename +
		    ") is not a configuration tree. If it is a binary to load,"
		    " you need to specify a component path where to load the"
		    " binary.\n");
		return false;
	}

	// 2. Is it a binary (ELF, a.out, ...)?
	FileLoader fileLoader(filename);
	stringstream messages;
	if (fileLoader.Load(specifiedComponent, messages)) {
		gxemul.GetUI()->ShowDebugMessage(specifiedComponent,
		    filename + " loaded\n" + messages.str());
		return true;
	} else {
		gxemul.GetUI()->ShowDebugMessage(specifiedComponent,
		    "FAILED to load " + filename + "\n" + messages.str());
		return false;
	}
}


string LoadCommand::GetShortDescription() const
{
	return "Loads a file.";
}


string LoadCommand::GetLongDescription() const
{
	return 
	    "Loads a file into a location in the component tree. There are two different\n"
	    "uses, which all share the same general syntax but differ in meaning.\n"
	    "\n"
	    "1. Loads an emulation setup (.gxemul) which was previously saved with the\n"
	    "   'save' command. For example, if a machine was previously saved into\n"
	    "   myMachine.gxemul, then\n"
	    "\n"
	    "       load myMachine.gxemul root\n"
	    "\n"
	    "   will add the machine to the current emulation tree, next to any other\n"
	    "   machines.\n"
	    "\n"
	    "       load myMachine.gxemul\n"
	    "\n"
	    "   will instead replace the whole configuration tree with what's in\n"
	    "   myMachine.gxemul. The filename may be omitted, if it is known from an\n"
	    "   earlier save or load command.\n"
	    "\n"
	    "2. Loads a binary (ELF, a.out, ...) into a CPU or data bus. E.g.:\n"
	    "\n"
	    "       load netbsd-RAMDISK cpu0\n"
	    "\n"
	    "   will load a NetBSD kernel into cpu0 (assuming that cpu0 is not ambiguous).\n"
	    "   The reason why files should be loaded into CPUs and not into RAM components\n"
	    "   is that binaries are often linked to virtual addresses, and the CPU does\n"
	    "   virtual to physical address translation.\n"
	    "\n"
	    "See also:  save    (to save an emulation to a file)\n";
}

