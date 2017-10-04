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

#include "commands/CopyComponentCommand.h"
#include "GXemul.h"


CopyComponentCommand::CopyComponentCommand()
	: Command("copy", "path-from path-to")
{
}


CopyComponentCommand::~CopyComponentCommand()
{
}


static void ShowMsg(GXemul& gxemul, const string& msg)
{
	gxemul.GetUI()->ShowDebugMessage(msg);
}


bool CopyComponentCommand::Execute(GXemul& gxemul, const vector<string>& arguments)
{
	if (arguments.size() != 2) {
		ShowMsg(gxemul, "syntax: copy path-from path-to\n");
		return false;
	}

	string pathFrom = arguments[0];
	string pathTo = arguments[1];

	vector<string> matchesFrom = gxemul.GetRootComponent()->
	    FindPathByPartialMatch(pathFrom);
	if (matchesFrom.size() == 0) {
		ShowMsg(gxemul, pathFrom + " is not a path to a known component.\n");
		return false;
	}
	if (matchesFrom.size() > 1) {
		ShowMsg(gxemul, pathFrom + " matches multiple components:\n");
		for (size_t i=0; i<matchesFrom.size(); i++)
			ShowMsg(gxemul, "  " + matchesFrom[i] + "\n");
		return false;
	}

	vector<string> matchesTo = gxemul.GetRootComponent()->
	    FindPathByPartialMatch(pathTo);
	if (matchesTo.size() == 0) {
		ShowMsg(gxemul, pathTo + " is not a path to a known component.\n");
		return false;
	}
	if (matchesTo.size() > 1) {
		ShowMsg(gxemul, pathTo + " matches multiple components:\n");
		for (size_t i=0; i<matchesTo.size(); i++)
			ShowMsg(gxemul, "  " + matchesTo[i] + "\n");
		return false;
	}


	refcount_ptr<Component> whatToCopy =
	    gxemul.GetRootComponent()->LookupPath(matchesFrom[0]);
	if (whatToCopy.IsNULL()) {
		ShowMsg(gxemul, "Lookup of origin path " + pathFrom + " failed.\n");
		return false;
	}

	refcount_ptr<Component> clone = whatToCopy->Clone();
	if (clone.IsNULL()) {
		ShowMsg(gxemul, "Failed to clone " + pathFrom + ".\n");
		return false;
	}

	refcount_ptr<Component> whereToAddIt =
	    gxemul.GetRootComponent()->LookupPath(matchesTo[0]);
	if (whereToAddIt.IsNULL()) {
		ShowMsg(gxemul, "Lookup of destination path " + pathTo + " failed.\n");
		return false;
	}

	whereToAddIt->AddChild(clone);

	return true;
}


string CopyComponentCommand::GetShortDescription() const
{
	return "Copies (clones) a component.";
}


string CopyComponentCommand::GetLongDescription() const
{
	return
	    "Copies a component (given a path) from one place in the configuration\n"
	    "tree to another. The following example duplicates a CPU component:\n"
	    "\n"
	    "> root\n" 
	    "  root\n"
	    "  \\-- machine0  [testmips]\n"
	    "      \\-- mainbus0\n"
	    "          |-- ram0  (32 MB at offset 0)\n"
	    "          |-- rom0  (16 MB at offset 0x1fc00000)\n"
	    "          \\-- cpu0  (MIPS, 100 MHz)\n"
	    "> copy cpu0 mainbus\n" 
	    "> root\n" 
	    "  root\n"
	    "  \\-- machine0  [testmips]\n"
	    "      \\-- mainbus0\n"
	    "          |-- ram0  (32 MB at offset 0)\n"
	    "          |-- rom0  (16 MB at offset 0x1fc00000)\n"
	    "          |-- cpu0  (MIPS, 100 MHz)\n"
	    "          \\-- cpu1  (MIPS, 100 MHz)\n"
	    "\n"
	    "(Note that the cloned CPU was automatically renamed to cpu1, to avoid a\n"
	    "collision.)\n"
	    "\n"
	    "See also:  add       (to add new components)\n"
	    "           move      (to move components within the tree)\n"
	    "           remove    (to remove components)\n"
	    "           root      (to inspect the current emulation setup)\n";
}


/*****************************************************************************/


#ifdef WITHUNITTESTS

static void Test_CopyComponentCommand_Copy()
{
	GXemul gxemul;

	gxemul.GetCommandInterpreter().RunCommand("add testmips");

	refcount_ptr<Component> root = gxemul.GetRootComponent();
	UnitTest::Assert("there should initially be 1 entry under root",
	    root->GetChildren().size(), 1);

	gxemul.GetCommandInterpreter().RunCommand("copy machine0 root");

	// The tree should now look like:
	//   root
	//   |-- machine0  [testmips]
	//   |   \-- mainbus0
	//   |       |-- ram0  (32 MB at offset 0)
	//   |       |-- rom0  (16 MB at offset 0x1fc00000)
	//   |       \-- cpu0  (MIPS, 100 MHz)
	//   \-- machine1  [testmips]
	//       \-- mainbus0
	//           |-- ram0  (32 MB at offset 0)
	//           |-- rom0  (16 MB at offset 0x1fc00000)
	//           \-- cpu0  (MIPS, 100 MHz)

	UnitTest::Assert("there should now be 2 entries (machine0 and machine1) under root",
	    root->GetChildren().size(), 2);

	refcount_ptr<Component> machine0 = root->GetChildren()[0];
	refcount_ptr<Component> machine1 = root->GetChildren()[1];

	UnitTest::Assert("name 0 mismatch", machine0->GetVariable("name")->ToString(), "machine0");
	UnitTest::Assert("name 1 mismatch", machine1->GetVariable("name")->ToString(), "machine1");
}

UNITTESTS(CopyComponentCommand)
{
	UNITTEST(Test_CopyComponentCommand_Copy);
}

#endif
