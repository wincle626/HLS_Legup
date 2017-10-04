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

#include "commands/MoveComponentCommand.h"
#include "GXemul.h"


MoveComponentCommand::MoveComponentCommand()
	: Command("move", "path-from path-to")
{
}


MoveComponentCommand::~MoveComponentCommand()
{
}


static void ShowMsg(GXemul& gxemul, const string& msg)
{
	gxemul.GetUI()->ShowDebugMessage(msg);
}


bool MoveComponentCommand::Execute(GXemul& gxemul, const vector<string>& arguments)
{
	if (arguments.size() != 2) {
		ShowMsg(gxemul, "syntax: move path-from path-to\n");
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


	refcount_ptr<Component> whatToMove =
	    gxemul.GetRootComponent()->LookupPath(matchesFrom[0]);
	if (whatToMove.IsNULL()) {
		ShowMsg(gxemul, "Lookup of origin path " + pathFrom + " failed.\n");
		return false;
	}

	refcount_ptr<Component> parent = whatToMove->GetParent();
	if (parent.IsNULL()) {
		ShowMsg(gxemul, "Cannot find the component's parent.\n");
		return false;
	}

	refcount_ptr<Component> whereToAddIt =
	    gxemul.GetRootComponent()->LookupPath(matchesTo[0]);
	if (whereToAddIt.IsNULL()) {
		ShowMsg(gxemul, "Lookup of destination path " + pathTo + " failed.\n");
		return false;
	}

	parent->RemoveChild(whatToMove);
	whereToAddIt->AddChild(whatToMove);

	return true;
}


string MoveComponentCommand::GetShortDescription() const
{
	return "Moves a component within the tree.";
}


string MoveComponentCommand::GetLongDescription() const
{
	return
	    "Moves a component (given a path) from one place in the configuration\n"
	    "tree to another. Example: Let's say a CPU component has by accident\n"
	    "been added outside of the machine's mainbus0 (where it is supposed to\n"
	    "be located). The move command fixes the problem:\n"
	    "\n"
	    "> add testmips\n"
	    "> add mips_cpu \n"
	    "> root\n" 
	    "  root\n"
	    "  |-- machine0  [testmips]\n"
	    "  |   \\-- mainbus0\n"
	    "  |       |-- ram0  (32 MB at offset 0)\n"
	    "  |       |-- rom0  (16 MB at offset 0x1fc00000)\n"
	    "  |       \\-- cpu0  (MIPS, 100 MHz)\n"
	    "  \\-- cpu0  (MIPS, 100 MHz)\n"
	    "> move root.cpu0 mainbus0 \n"
	    "> root\n" 
	    "  root\n"
	    "  \\-- machine0  [testmips]\n"
	    "      \\-- mainbus0\n"
	    "          |-- ram0  (32 MB at offset 0)\n"
	    "          |-- rom0  (16 MB at offset 0x1fc00000)\n"
	    "          |-- cpu0  (MIPS, 100 MHz)\n"
	    "          \\-- cpu1  (MIPS, 100 MHz)\n"
	    "\n"
	    "(Note that the moved cpu was automatically renamed to cpu1, to avoid a\n"
	    "collision.)\n"
	    "\n"
	    "See also:  add     (to add new components)\n"
	    "           root    (to inspect the current emulation setup)\n";
}


/*****************************************************************************/


#ifdef WITHUNITTESTS

static void Test_MoveComponentCommand_Move()
{
	GXemul gxemul;

	gxemul.GetCommandInterpreter().RunCommand("add testmips");
	gxemul.GetCommandInterpreter().RunCommand("add ram");

	refcount_ptr<Component> root = gxemul.GetRootComponent();
	UnitTest::Assert("there should be 2 entries under root",
	    root->GetChildren().size(), 2);

	gxemul.GetCommandInterpreter().RunCommand("move root.ram mainbus");

	// The tree should now look like:
	//   root
	//   \-- machine0  [testmips]
	//       \-- mainbus0
	//           |-- ram0  (32 MB at offset 0)
	//           |-- rom0  (16 MB at offset 0x1fc00000)
	//           |-- fb_videoram0  (15 MB at offset 0x12000000)
	//           |-- cpu0  (MIPS, 100 MHz)
	//           \-- ram1  (0 bytes at offset 0)

	UnitTest::Assert("there should now be 1 entry (machine0) under root",
	    root->GetChildren().size(), 1);

	refcount_ptr<Component> machine0 = root->GetChildren()[0];

	UnitTest::Assert("there should be 1 entry (mainbus0) under machine0",
	    machine0->GetChildren().size(), 1);

	refcount_ptr<Component> mainbus0 = machine0->GetChildren()[0];

	UnitTest::Assert("there should be 5 components on mainbus0",
	    mainbus0->GetChildren().size(), 5);
}

UNITTESTS(MoveComponentCommand)
{
	UNITTEST(Test_MoveComponentCommand_Move);
}

#endif
