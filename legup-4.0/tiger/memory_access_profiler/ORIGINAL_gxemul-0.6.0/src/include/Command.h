#ifndef COMMAND_H
#define	COMMAND_H

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

#include "misc.h"

#include "UnitTest.h"

class GXemul;


/**
 * \brief A %Command is a named function, executed by the CommandInterpreter.
 *
 * To implement a new %Command, simply create the corresponding .h file
 * in src/include/commands/ and a .cc file in src/main/commands/ (e.g. by
 * copying from a similar command), and add the name of the command's .o
 * file to src/main/commands/Makefile.skel. (The configure script takes care
 * of adding all commands to the CommandInterpreter, so there is no need
 * to manually "register" the command anywhere.)
 */
class Command
	: public ReferenceCountable
	, public UnitTestable
{
public:
	/**
	 * \brief Constructs a %Command.
	 *
	 * @param name The command's name. This should be a unique lower-case
	 *	string, consisting only of letters a-z.
	 * @param argumentFormat A string describing the command's arguments.
	 *	May be empty, if the command takes no arguments.
	 */
	Command(const string& name, const string& argumentFormat);

	virtual ~Command() = 0;

	/**
	 * \brief Gets the name of the command.
	 *
	 * @return The name of the command.
	 */
	const string& GetCommandName() const
	{
		return m_name;
	}

	/**
	 * \brief Gets the argument format for the command.
	 *
	 * @return The argument format for the command.
	 */
	const string& GetArgumentFormat() const
	{
		return m_argumentFormat;
	}

	/**
	 * \brief Executes the command on a given GXemul instance.
	 *
	 * @param gxemul A reference to the GXemul instance.
	 * @param arguments A vector of zero or more string arguments.
	 * @return true if the command succeeded, false if it failed.
	 */
	virtual bool Execute(GXemul& gxemul,
		const vector<string>& arguments) = 0;

	/**
	 * \brief Returns whether the command can be quickly re-executed using
	 * an empty command line.
	 *
	 * Typical examples are: step, cpu disassembly, and memory dump,
	 * where re-executing the command without arguments is very natural.
	 *
	 * @return true if the command may be re-executed without arguments,
	 * false otherwise.
	 */
	virtual bool MayBeReexecutedWithoutArgs() const
	{
		return false;
	}

	/**
	 * \brief Returns a short (one-line) description of the command.
	 *
	 * @return A short description of the command.
	 */
	virtual string GetShortDescription() const = 0;

	/**
	 * \brief Returns a long description/help message for the command.
	 *
	 * @return A long description/help message for the command.
	 */
	virtual string GetLongDescription() const = 0;


	/********************************************************************/

	static void RunUnitTests(int& nSucceeded, int& nFailures);

private:
	string		m_name;
	string		m_argumentFormat;
};


typedef map< string,refcount_ptr<Command> > Commands;


#endif	// COMMAND_H
