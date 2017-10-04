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

#include "commands/StepCommand.h"
#include "GXemul.h"


StepCommand::StepCommand()
	: Command("step", "[nsteps]")
{
}


StepCommand::~StepCommand()
{
}


bool StepCommand::Execute(GXemul& gxemul, const vector<string>& arguments)
{
	int64_t nsteps = 1;

	if (arguments.size() > 1) {
		gxemul.GetUI()->ShowDebugMessage("step: Too many arguments.\n");
		return false;
	}

	if (arguments.size() > 0) {
		stringstream ss;
		ss << arguments[0];
		ss >> nsteps;
	}

	if (nsteps < 1) {
		gxemul.GetUI()->ShowDebugMessage("nr of steps must be at least 1\n");
		return false;
	}

	gxemul.SetRunState(GXemul::SingleStepping);
	gxemul.SetNrOfSingleStepsInARow(nsteps);

	return true;
}


string StepCommand::GetShortDescription() const
{
	return "Runs one step of the emulation.";
}


string StepCommand::GetLongDescription() const
{
	return "Runs one step (or multiple single-steps) in the emulation.\n"
	    "\n"
	    "See also:  continue      (to continue without single-stepping)\n";
}


/*****************************************************************************/


#ifdef WITHUNITTESTS

static void Test_StepCommand_Affect_RunState()
{
	refcount_ptr<Command> cmd = new StepCommand;
	vector<string> dummyArguments;
	
	GXemul gxemul;

	UnitTest::Assert("the default GXemul instance should be Paused",
	    gxemul.GetRunState() == GXemul::Paused);

	cmd->Execute(gxemul, dummyArguments);

	UnitTest::Assert("runstate should have been changed to SingleStepping",
	    gxemul.GetRunState() == GXemul::SingleStepping);
}

static void Test_StepCommand_GoodArgs()
{
	refcount_ptr<Command> cmd = new StepCommand;
	vector<string> arguments;
	
	GXemul gxemul;
	arguments.push_back("42");

	UnitTest::Assert("should have succeeded; good arguments",
	    cmd->Execute(gxemul, arguments));
}

static void Test_StepCommand_BadArgs_TooMany()
{
	refcount_ptr<Command> cmd = new StepCommand;
	vector<string> arguments;
	
	GXemul gxemul;
	arguments.push_back("42");
	arguments.push_back("43");

	UnitTest::Assert("should not have succeeded; noo many args",
	    !cmd->Execute(gxemul, arguments));
}

static void Test_StepCommand_BadArgs_Zero()
{
	refcount_ptr<Command> cmd = new StepCommand;
	vector<string> arguments;
	
	GXemul gxemul;
	arguments.push_back("0");

	UnitTest::Assert("should not have succeeded; too few steps",
	    !cmd->Execute(gxemul, arguments));
}

static void Test_StepCommand_BadArgs_Negative()
{
	refcount_ptr<Command> cmd = new StepCommand;
	vector<string> arguments;
	
	GXemul gxemul;
	arguments.push_back("-42");

	UnitTest::Assert("should not have succeeded; negative nr of steps",
	    !cmd->Execute(gxemul, arguments));
}

UNITTESTS(StepCommand)
{
	UNITTEST(Test_StepCommand_Affect_RunState);
	UNITTEST(Test_StepCommand_GoodArgs);
	UNITTEST(Test_StepCommand_BadArgs_TooMany);
	UNITTEST(Test_StepCommand_BadArgs_Zero);
	UNITTEST(Test_StepCommand_BadArgs_Negative);
}

#endif
