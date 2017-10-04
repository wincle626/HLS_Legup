#ifndef COMMANDINTERPRETER_H
#define	COMMANDINTERPRETER_H

/*
 *  Copyright (C) 2007-2010  Anders Gavare.  All rights reserved.
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

#include "Command.h"
#include "UnitTest.h"


class GXemul;


/**
 * \brief An interactive command interpreter, which run Commands.
 *
 * A command interpreter can execute commands in the form of complete strings,
 * or it can be given one character (keypress) at a time to build up a command.
 * When given individual keypresses, the command interpreter echoes back
 * what is to be printed. It also supports TAB completion.
 */
class CommandInterpreter
	: public UnitTestable
{
public:
	/**
	 * \brief Constructs a %CommandInterpreter.
	 *
	 * @param owner the GXemul instance that owns the %CommandInterpreter
	 */
	CommandInterpreter(GXemul* owner);

	/**
	 * \brief Adds a character (keypress) to the current command buffer.
	 *
	 * Most normal keys are added at the end of the buffer. Some exceptions
	 * are:
	 * <ul>
	 *	<li>nul char: does not change the input line buffer,
	 *		but forces it to be visually redrawn/updated
	 *	<li>backspace: removes the last character (if any)
	 *	<li>tab: attempts TAB completion of the last word
	 *	<li>newline or cr: calls RunCommand, and then clears
	 *		the current buffer
	 *	<li>escape: special handling
	 * </ul>
	 *
	 * @param key the character/key to add
	 * @return true if this was a complete command line (i.e. the key
	 *	was a newline), false otherwise
	 */
	bool AddKey(stringchar key);

	/**
	 * \brief Clears the current command buffer.
	 */
	void ClearCurrentCommandBuffer();

	/**
	 * \brief Re-displays the current command buffer.
	 *
	 * Useful e.g. when running in text console mode, and the user has
	 * pressed CTRL-Z. When returning to %GXemul, the current command
	 * buffer can be showed again by calling this function.
	 */
	void ReshowCurrentCommandBuffer();

	/**
	 * \brief Runs a command, given as a string.
	 *
	 * The return value from the function is true if the command was run.
	 * However, the command may run and fail, and that's what pSuccess is
	 * for.
	 *
	 * @param command the command to run
	 * @param pSuccess a pointer to a bool, which (if pSuccess is non-NULL)
	 *	will be set to whether the command succeeded or not.
	 * @return true if the command was run, false if the command was
	 *	not known
	 */
	bool RunCommand(const string& command, bool* pSuccess = NULL);

	/**
	 * \brief Retrieves the current command buffer.
	 *
	 * @return a string representing the current command buffer
	 */
	const string& GetCurrentCommandBuffer() const;

	/**
	 * \brief Adds a new Command to the command interpreter.
	 *
	 * @param command A reference counter pointer to the Command.
	 */
	void AddCommand(refcount_ptr<Command> command);

	/**
	 * \brief Gets a collection of all commands.
	 *
	 * @return A const reference to the collection of commands.
	 */
	const Commands& GetCommands() const;

	/**
	 * \brief Adds a command line to the command history.
	 *
	 * If the command is empty, or the same as the last command in the
	 * command history, it is ignored.
	 *
	 * The command history buffer only holds a small fixed number of
	 * entries.
	 *
	 * @param command The command line to add to the command history.
	 * @return The next insert position in the command history. Useful
	 *	for unit testing purposes; should otherwise be ignored.
	 */
	int AddLineToCommandHistory(const string& command);

	/**
	 * \brief Retrieves a line from the command history.
	 *
	 * @param nStepsBack The number of steps back into the history. 0
	 *	means return an empty line, 1 means the last history line,
	 *	2 means the second last, and so on.
	 * @return The line from the history, or an empty string if
	 *	nStepsBack was zero.
	 */
	string GetHistoryLine(int nStepsBack) const;

private:
	/**
	 * \brief Internal helper which clears a line by outputting spaces.
	 */
	void ClearCurrentInputLineVisually();

	/**
	 * \brief Completes the word at the current position in the input line.
	 *
	 * @param commandString A reference to the string to
	 *	tab-complete.
	 * @param cursorPosition A refernce to a size_t, which
	 *	indicates the current cursor position within the string.
	 * @param visibleShowAvailable True if available words should be
	 *	echoed back via the UI.
	 * @return True if there was a single match, false otherwise.
	 */
	bool TabComplete(string& commandString, size_t& cursorPosition,
		bool visibleShowAvailable = false);

	/**
	 * \brief Tab-completes; takes optional method or state variable
	 *	name into account.
	 *
	 * Strings such as "cpu." and "cpu.u" should e.g. be expanded to
	 * "root.machine0.mainbus0.cpu." and
	 * "root.machine0.mainbus0.cpu.unassemble", respectively.
	 *
	 * @param commandString A reference to the string to
	 *	tab-complete.
	 * @param cursorPosition A refernce to a size_t, which
	 *	indicates the current cursor position within the string.
	 * @param visibleShowAvailable True if available words should be
	 *	echoed back via the UI.
	 * @return True if there was a single match, false otherwise.
	 */
	bool TabCompleteWithSubname(string& commandString,
		size_t& cursorPosition, bool visibleShowAvailable = false);

	/**
	 * \brief Runs a method on a Component.
	 *
	 * Note: The componentPathAndMethod argument may contain an optional
	 * ".method" suffix. The part before the method may need to be
	 * tab-completed.
	 *
	 * Some examples of componentPathAndMethod:
	 * <ul>
	 *	<li>cpu
	 *	<li>cpu.u
	 *	<li>cpu.unassemble
	 *	<li>root.machine0.mainbus0.cpu
	 *	<li>root.machine0.mainbus0.cpu0
	 *	<li>root.machine0.mainbus0.cpu.u
	 *	<li>root.machine0.mainbus0.cpu0.unassemble
	 * </ul>
	 *
	 * If the method name is missing, a default method will be executed.
	 *
	 * @param componentPathAndMethod The path to the component, plus a
	 *	possible ".method" suffix.
	 * @param arguments A vector of string arguments.
	 * @return True if a component method was executed, false otherwise.
	 */
	bool RunComponentMethod(const string& componentPathAndMethod,
		const vector<string>& arguments);

	/**
	 * \brief Prints a list of available words (for tab completion).
	 *
	 * @param words A vector of all available words.
	 */
	void ShowAvailableWords(const vector<string>& words);

	void VariableAssignment(const string& componentPath,
	    const string& variableName, const string& expression);


	/********************************************************************/
public:
	static void RunUnitTests(int& nSucceeded, int& nFailures);


private:
	// Pointer to the owning GXemul instance:
	GXemul*			m_GXemul;

	// A collection of all available commands that can be executed:
	Commands		m_commands;

	// The current input line:
	string			m_currentCommandString;
	size_t			m_currentCommandCursorPosition;
	bool			m_inEscapeSequence;
	string			m_escapeSequence;
	int			m_historyEntryToCopyFrom;
	
	// Command history (usually accessed by cursor up/down keys):
	vector<string>		m_commandHistory;
	size_t			m_commandHistoryInsertPosition;
	size_t			m_commandHistoryMaxSize;

	// If non-empty, a command which may be reexecuted if an empty
	// command line is given:
	string			m_mayBeReexecuted;
};


#endif	// COMMANDINTERPRETER_H
