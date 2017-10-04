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

#include "commands/ResetCommand.h"
#include "GXemul.h"


ResetCommand::ResetCommand()
	: Command("reset", "")
{
}


ResetCommand::~ResetCommand()
{
}


bool ResetCommand::Execute(GXemul& gxemul, const vector<string>& arguments)
{
	gxemul.SetRunState(GXemul::Paused);
	gxemul.Reset();

	return true;
}


string ResetCommand::GetShortDescription() const
{
	return "Resets the current emulation.";
}


string ResetCommand::GetLongDescription() const
{
	return "Resets the emulation, by:\n\n"
	    "  1) setting the current run state to Paused,\n"
	    "  2) clearing the state of all components\n"
	    "  3) reloading any binaries loaded on the command"
	    	" line (on-reset commands)\n";
}


/*****************************************************************************/


#ifdef WITHUNITTESTS

static void Test_ResetCommand_RunState()
{
	refcount_ptr<Command> cmd = new ResetCommand;
	vector<string> dummyArguments;
	
	GXemul gxemul;

	UnitTest::Assert("the default GXemul instance should be Paused",
	    gxemul.GetRunState() == GXemul::Paused);

	gxemul.SetRunState(GXemul::Running);

	cmd->Execute(gxemul, dummyArguments);

	UnitTest::Assert("runstate should now be Paused",
	    gxemul.GetRunState() == GXemul::Paused);
}

UNITTESTS(ResetCommand)
{
	UNITTEST(Test_ResetCommand_RunState);
}

#endif
