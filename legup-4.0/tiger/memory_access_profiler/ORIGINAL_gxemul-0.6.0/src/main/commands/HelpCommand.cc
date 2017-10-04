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

#include "commands/HelpCommand.h"
#include "GXemul.h"


HelpCommand::HelpCommand()
	: Command("help", "[command-name]")
{
}


HelpCommand::~HelpCommand()
{
}


bool HelpCommand::Execute(GXemul& gxemul, const vector<string>& arguments)
{
	const Commands& commands = gxemul.GetCommandInterpreter().GetCommands();
	Commands::const_iterator it;

	if (arguments.size() > 1) {
		gxemul.GetUI()->ShowDebugMessage("usage: help [cmd]\n");
		return false;
	}

	if (arguments.size() == 1) {
		// Special case: root isn't really a command
		if (arguments[0] == "root") {
			gxemul.GetUI()->ShowDebugMessage(
			    "\nroot\n----\n\n"
			    "Shows the component tree which makes up the current emulation configuration.\n"
			    "Technically, 'root' is not a command, but the name of the root node. Typing\n"
			    "the name of any node in the configuration tree will dump the sub-tree starting\n"
			    "at that node, followed by that node's own variables.\n\n");
			return true;
		}
		
		it = commands.find(arguments[0]);
		if (it == commands.end()) {
			gxemul.GetUI()->ShowDebugMessage(
			    "Command \"" + arguments[0] + "\" unknown.\n");
			return false;
		}
		
		// command name + " " + argument format
		// ------------------------------------
		//
		// Long description.

		string longhelp = arguments[0];
		const string& argumentFormat = it->second->GetArgumentFormat();
		if (argumentFormat != "")
			longhelp += " " + argumentFormat;
		int len = longhelp.length();

		longhelp += "\n";
		for (int i=0; i<len; i++)
			longhelp += "-";
		
		longhelp += "\n\n" + it->second->GetLongDescription();
		if (longhelp[longhelp.length() - 1] != '\n')
			longhelp += '\n';
		
		gxemul.GetUI()->ShowDebugMessage("\n" + longhelp + "\n");

		if (it->second->MayBeReexecutedWithoutArgs())
			gxemul.GetUI()->ShowDebugMessage(
			    "This command will be re-executed (without "
			    "arguments), if an empty command\n"
			    "line is issued.\n\n");

		return true;
	}

	// Find the longest name + argument:
	size_t longest = 0;
	for (it = commands.begin(); it != commands.end(); ++ it) {
		const Command& command = *it->second;
		size_t len = command.GetCommandName().length();

		if (command.GetArgumentFormat() != "")
			len += 1 + command.GetArgumentFormat().length();

		if (len > longest)
			longest = len;
	}

	// Show all commands followed by their short descriptions:
	for (it = commands.begin(); it != commands.end(); ++ it) {
		const Command& command = *it->second;

		const string leadingSpaces = "  ";
		const int nLeadingSpaces = 2;

		string str = leadingSpaces + command.GetCommandName();
		if (command.GetArgumentFormat() != "")
			str += " " + command.GetArgumentFormat();

		while (str.length() < longest + nLeadingSpaces)
			str += " ";
		
		str += "  " + command.GetShortDescription();

		gxemul.GetUI()->ShowDebugMessage(str + "\n");
	}

	// ... followed by a short description of other possible commands:
	gxemul.GetUI()->ShowDebugMessage("  ------------------------------"
	    "----------------------------------------------\n");
	gxemul.GetUI()->ShowDebugMessage("  root                    "
	"Shows the entire component tree.\n");
	gxemul.GetUI()->ShowDebugMessage("  <name>                  "
	"Shows the component tree, from the named component.\n");
	gxemul.GetUI()->ShowDebugMessage("  <name>.<method>         "
	    "Executes a method of a named component.\n");
	gxemul.GetUI()->ShowDebugMessage("  <name>.<var>            "
	    "Prints the value of a variable of a named component.\n");
	gxemul.GetUI()->ShowDebugMessage("  <name>.<var> = <value>  "
	    "Assigns a value to a variable of a named component.\n");

	return true;
}


string HelpCommand::GetShortDescription() const
{
	return "Shows a help message.";
}


string HelpCommand::GetLongDescription() const
{
	return "Shows a summary of all available commands, or, "
	    "when given a specific command\n"
	    "name as an argument, shows a detailed description about "
	    "that command.\n";
}


