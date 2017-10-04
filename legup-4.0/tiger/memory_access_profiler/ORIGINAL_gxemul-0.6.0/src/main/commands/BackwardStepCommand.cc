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

#include "commands/BackwardStepCommand.h"
#include "GXemul.h"


BackwardStepCommand::BackwardStepCommand()
	: Command("backward-step", "")
{
}


BackwardStepCommand::~BackwardStepCommand()
{
}


bool BackwardStepCommand::Execute(GXemul& gxemul, const vector<string>& arguments)
{
	StateVariable* stepvar = gxemul.GetRootComponent()->GetVariable("step");
	uint64_t step = stepvar->ToInteger();

	if (step == 0) {
		gxemul.GetUI()->ShowDebugMessage("Cannot go back further; "
		    "already at step 0.\n");
		return false;
	}

	stringstream stepss;
	stepss << (step - 1);

	const refcount_ptr<Component> lightClone = gxemul.GetRootComponent()->LightClone();

	if (!gxemul.GetRootComponent()->SetVariableValue("step", stepss.str())) {
		gxemul.GetUI()->ShowDebugMessage("Failed to set root.step.\n");
		return false;
	}

	// Indent all debug output with message header "step X -> step X-1: ":
	stringstream ss;
	ss << "step " << step << " -> " << (step-1) <<  ": ";
	UI::SetIndentationMessageHelper indentationHelper(gxemul.GetUI(), ss.str());

	// Compare the clone of the component tree before changing the step
	// with what we have now.
	stringstream changeMessages;
	gxemul.GetRootComponent()->DetectChanges(lightClone, changeMessages);

	string msg = changeMessages.str();
	if (msg == "")
		msg = "No state change.\n";

	gxemul.GetUI()->ShowDebugMessage(msg);

	return true;
}


string BackwardStepCommand::GetShortDescription() const
{
	return "Runs one step of the emulation backwards.";
}


string BackwardStepCommand::GetLongDescription() const
{
	return
	    "Runs one step of the emulation backwards. This command does the same as\n"
	    "manually decreasing root.step by 1, except that all state changes are also\n"
	    "displayed, e.g.:\n"
	    "\n"
	    "> backward-step\n"
	    "step 3 -> 2: => cpu0.a1: 0 -> 0x2a\n"
	    "             => cpu0.pc: 0xffffffffbfc0004c -> 0xffffffffbfc00048\n"
	    "\n"
	    "This command requires that snapshotting support is enabled (using the\n"
	    "-B command line option).\n";
}


/*****************************************************************************/


#ifdef WITHUNITTESTS

static void Test_BackwardStepCommand_AlreadyAtStep0()
{
	refcount_ptr<Command> cmd = new BackwardStepCommand;
	vector<string> dummyArguments;
	
	GXemul gxemul;

	UnitTest::Assert("root.step should initially be zero", gxemul.GetStep(), 0);

	cmd->Execute(gxemul, dummyArguments);

	UnitTest::Assert("root.step should still be zero", gxemul.GetStep(), 0);
}

static void Test_BackwardStepCommand_NotWhenSnapshotsAreDisabled()
{
	refcount_ptr<Command> cmd = new BackwardStepCommand;
	vector<string> dummyArguments;
	
	GXemul gxemul;

	char filename[] = "test/FileLoader_ELF_MIPS";
	char *filenames[] = { filename };
	gxemul.ParseFilenames("testmips", 1, filenames);
	gxemul.Reset();

	gxemul.SetSnapshottingEnabled(false);

	gxemul.GetCommandInterpreter().RunCommand("step 3");
	gxemul.Execute();

	UnitTest::Assert("root.step should initially be 3", gxemul.GetStep(), 3);

	refcount_ptr<Component> cpu = gxemul.GetRootComponent()->LookupPath("cpu0");
	UnitTest::Assert("3: cpu0.pc", cpu->GetVariable("pc")->ToString(), "0xffffffff80010104");
	UnitTest::Assert("3: cpu0.sp", cpu->GetVariable("sp")->ToString(), "0xffffffffa0007ed0");
	UnitTest::Assert("3: cpu0.v0", cpu->GetVariable("v0")->ToString(), "0xffffffff88880000");
	UnitTest::Assert("3: cpu0.v1", cpu->GetVariable("v1")->ToString(), "0xffffffffcccc0000");

	// This execute should FAIL, because snapshotting is disabled.
	cmd->Execute(gxemul, dummyArguments);
	UnitTest::Assert("root.step should still be 3", gxemul.GetStep(), 3);
}

static void Test_BackwardStepCommand_Basic()
{
	refcount_ptr<Command> cmd = new BackwardStepCommand;
	vector<string> dummyArguments;
	
	GXemul gxemul;

	char filename[] = "test/FileLoader_ELF_MIPS";
	char *filenames[] = { filename };
	gxemul.ParseFilenames("testmips", 1, filenames);
	gxemul.Reset();

	gxemul.SetSnapshottingEnabled(true);

	gxemul.GetCommandInterpreter().RunCommand("step 3");
	gxemul.Execute();

	UnitTest::Assert("root.step should initially be 3", gxemul.GetStep(), 3);

	refcount_ptr<Component> cpu = gxemul.GetRootComponent()->LookupPath("cpu0");
	UnitTest::Assert("3: cpu0.pc", cpu->GetVariable("pc")->ToString(), "0xffffffff80010104");
	UnitTest::Assert("3: cpu0.sp", cpu->GetVariable("sp")->ToString(), "0xffffffffa0007ed0");
	UnitTest::Assert("3: cpu0.v0", cpu->GetVariable("v0")->ToString(), "0xffffffff88880000");
	UnitTest::Assert("3: cpu0.v1", cpu->GetVariable("v1")->ToString(), "0xffffffffcccc0000");

	cmd->Execute(gxemul, dummyArguments);
	UnitTest::Assert("root.step should be 2", gxemul.GetStep(), 2);
	cpu = gxemul.GetRootComponent()->LookupPath("cpu0");
	UnitTest::Assert("2: cpu0.pc", cpu->GetVariable("pc")->ToString(), "0xffffffff80010100");
	UnitTest::Assert("2: cpu0.sp", cpu->GetVariable("sp")->ToString(), "0xffffffffa0007ed0");
	UnitTest::Assert("2: cpu0.v0", cpu->GetVariable("v0")->ToString(), "0");
	UnitTest::Assert("2: cpu0.v1", cpu->GetVariable("v1")->ToString(), "0xffffffffcccc0000");

	cmd->Execute(gxemul, dummyArguments);
	UnitTest::Assert("root.step should be 1", gxemul.GetStep(), 1);
	cpu = gxemul.GetRootComponent()->LookupPath("cpu0");
	UnitTest::Assert("1: cpu0.pc", cpu->GetVariable("pc")->ToString(), "0xffffffff800100fc");
	UnitTest::Assert("1: cpu0.sp", cpu->GetVariable("sp")->ToString(), "0xffffffffa0007ed0");
	UnitTest::Assert("1: cpu0.v0", cpu->GetVariable("v0")->ToString(), "0");
	UnitTest::Assert("1: cpu0.v1", cpu->GetVariable("v1")->ToString(), "0");

	cmd->Execute(gxemul, dummyArguments);
	UnitTest::Assert("root.step should be 0", gxemul.GetStep(), 0);
	cpu = gxemul.GetRootComponent()->LookupPath("cpu0");
	UnitTest::Assert("0: cpu0.pc", cpu->GetVariable("pc")->ToString(), "0xffffffff800100f8");
	UnitTest::Assert("0: cpu0.sp", cpu->GetVariable("sp")->ToString(), "0xffffffffa0007f00");
	UnitTest::Assert("0: cpu0.v0", cpu->GetVariable("v0")->ToString(), "0");
	UnitTest::Assert("0: cpu0.v1", cpu->GetVariable("v1")->ToString(), "0");

	cmd->Execute(gxemul, dummyArguments);
	UnitTest::Assert("root.step should still be 0", gxemul.GetStep(), 0);
	cpu = gxemul.GetRootComponent()->LookupPath("cpu0");
	UnitTest::Assert("X: cpu0.pc", cpu->GetVariable("pc")->ToString(), "0xffffffff800100f8");
	UnitTest::Assert("X: cpu0.sp", cpu->GetVariable("sp")->ToString(), "0xffffffffa0007f00");
	UnitTest::Assert("X: cpu0.v0", cpu->GetVariable("v0")->ToString(), "0");
	UnitTest::Assert("X: cpu0.v1", cpu->GetVariable("v1")->ToString(), "0");
}

// Reset resets the component tree, but does not load back the binary!
static void Test_BackwardStepCommand_ManualAddAndLoad()
{
	refcount_ptr<Command> cmd = new BackwardStepCommand;
	vector<string> dummyArguments;
	
	GXemul gxemul;

	gxemul.GetCommandInterpreter().RunCommand("add testmips");
	gxemul.GetCommandInterpreter().RunCommand("load test/FileLoader_ELF_MIPS cpu0");
	gxemul.SetSnapshottingEnabled(true);
	gxemul.GetCommandInterpreter().RunCommand("step 3");
	gxemul.Execute();

	UnitTest::Assert("root.step should initially be 3", gxemul.GetStep(), 3);

	refcount_ptr<Component> cpu = gxemul.GetRootComponent()->LookupPath("cpu0");
	UnitTest::Assert("3: cpu0.pc", cpu->GetVariable("pc")->ToString(), "0xffffffff80010104");
	UnitTest::Assert("3: cpu0.sp", cpu->GetVariable("sp")->ToString(), "0xffffffffa0007ed0");
	UnitTest::Assert("3: cpu0.v0", cpu->GetVariable("v0")->ToString(), "0xffffffff88880000");
	UnitTest::Assert("3: cpu0.v1", cpu->GetVariable("v1")->ToString(), "0xffffffffcccc0000");

	cmd->Execute(gxemul, dummyArguments);
	UnitTest::Assert("root.step should be 2", gxemul.GetStep(), 2);
	cpu = gxemul.GetRootComponent()->LookupPath("cpu0");
	UnitTest::Assert("2: cpu0.pc", cpu->GetVariable("pc")->ToString(), "0xffffffff80010100");
	UnitTest::Assert("2: cpu0.sp", cpu->GetVariable("sp")->ToString(), "0xffffffffa0007ed0");
	UnitTest::Assert("2: cpu0.v0", cpu->GetVariable("v0")->ToString(), "0");
	UnitTest::Assert("2: cpu0.v1", cpu->GetVariable("v1")->ToString(), "0xffffffffcccc0000");

	cmd->Execute(gxemul, dummyArguments);
	UnitTest::Assert("root.step should be 1", gxemul.GetStep(), 1);
	cpu = gxemul.GetRootComponent()->LookupPath("cpu0");
	UnitTest::Assert("1: cpu0.pc", cpu->GetVariable("pc")->ToString(), "0xffffffff800100fc");
	UnitTest::Assert("1: cpu0.sp", cpu->GetVariable("sp")->ToString(), "0xffffffffa0007ed0");
	UnitTest::Assert("1: cpu0.v0", cpu->GetVariable("v0")->ToString(), "0");
	UnitTest::Assert("1: cpu0.v1", cpu->GetVariable("v1")->ToString(), "0");

	cmd->Execute(gxemul, dummyArguments);
	UnitTest::Assert("root.step should be 0", gxemul.GetStep(), 0);
	cpu = gxemul.GetRootComponent()->LookupPath("cpu0");
	UnitTest::Assert("0: cpu0.pc", cpu->GetVariable("pc")->ToString(), "0xffffffff800100f8");
	UnitTest::Assert("0: cpu0.sp", cpu->GetVariable("sp")->ToString(), "0xffffffffa0007f00");
	UnitTest::Assert("0: cpu0.v0", cpu->GetVariable("v0")->ToString(), "0");
	UnitTest::Assert("0: cpu0.v1", cpu->GetVariable("v1")->ToString(), "0");

	cmd->Execute(gxemul, dummyArguments);
	UnitTest::Assert("root.step should still be 0", gxemul.GetStep(), 0);
	cpu = gxemul.GetRootComponent()->LookupPath("cpu0");
	UnitTest::Assert("X: cpu0.pc", cpu->GetVariable("pc")->ToString(), "0xffffffff800100f8");
	UnitTest::Assert("X: cpu0.sp", cpu->GetVariable("sp")->ToString(), "0xffffffffa0007f00");
	UnitTest::Assert("X: cpu0.v0", cpu->GetVariable("v0")->ToString(), "0");
	UnitTest::Assert("X: cpu0.v1", cpu->GetVariable("v1")->ToString(), "0");
}

UNITTESTS(BackwardStepCommand)
{
	UNITTEST(Test_BackwardStepCommand_AlreadyAtStep0);
	UNITTEST(Test_BackwardStepCommand_NotWhenSnapshotsAreDisabled);
	UNITTEST(Test_BackwardStepCommand_Basic);
	UNITTEST(Test_BackwardStepCommand_ManualAddAndLoad);
}

#endif
