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

#include "Command.h"
#include "GXemul.h"


Command::Command(const string& name, const string& argumentFormat)
	: m_name(name)
	, m_argumentFormat(argumentFormat)
{
}


Command::~Command()
{
}


/*****************************************************************************/


#ifdef WITHUNITTESTS

/**
 * \brief A dummy Command, for unit testing purposes
 */
class DummyCommand
	: public Command
{
public:
	DummyCommand(int& valueRef)
		: Command("dummycommand", "[args]")
		, m_value(valueRef)
	{
	}

	~DummyCommand()
	{
	}

	bool Execute(GXemul& gxemul, const vector<string>& arguments)
	{
		m_value ++;
		return true;
	}

	string GetShortDescription() const
	{
		return "A dummy command used for unit testing.";
	}

	string GetLongDescription() const
	{
		return "This is just a dummy command used for unit testing.";
	}

private:
	int&	m_value;
};

static void Test_Command_DummyCommand()
{
	GXemul gxemulDummy;
	vector<string> dummyArgs;
	
	int dummyInt = 0;
	refcount_ptr<Command> cmd = new DummyCommand(dummyInt);

	dummyInt = 42;

	UnitTest::Assert("dummyInt should initially be 42", dummyInt == 42);

	UnitTest::Assert("command should have succeeded",
	    cmd->Execute(gxemulDummy, dummyArgs));
	
	UnitTest::Assert("dummyInt should now be 43", dummyInt == 43);

}

UNITTESTS(Command)
{
	UNITTEST(Test_Command_DummyCommand);
}

#endif
