#ifndef CONSOLEUI_H
#define	CONSOLEUI_H

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

#include <termios.h>

#include "UI.h"


/**
 * \brief Text-terminal based User Interface.
 */
class ConsoleUI
	: public UI
{
public:
	/**
	 * \brief Constructs a text console %UI instance.
	 *
	 * @param gxemul Pointer to the owning GXemul instance.
	 */
	ConsoleUI(GXemul *gxemul);

	virtual ~ConsoleUI();

	/**
	 * \brief Initializes the terminal for blocking, non-echo I/O.
	 */
	virtual void Initialize();

	/**
	 * \brief Updates UI items. Not used for %ConsoleUI.
	 */
	virtual void UpdateUI();

	/**
	 * \brief Prints the text console startup banner.
	 */
	virtual void ShowStartupBanner();

	/**
	 * \brief Shows a debug message, by printing it to stdout.
	 *
	 * @param msg The message to show.
	 */
	virtual void ShowDebugMessage(const string& msg);
	
	/**
	 * \brief Shows a debug message for a Component, by printing it to
	 * stdout.
	 *
	 * See UI::ShowDebugMessage(Component*,const string&) for
	 * a longer comment.
	 *
	 * @param component A pointer to the Component.
	 * @param msg The message to show.
	 */
	virtual void ShowDebugMessage(Component* component, const string& msg);

	/**
	 * \brief Does nothing for the %ConsoleUI.
	 *
	 * @param command The command being executed.
	 */
	virtual void ShowCommandMessage(const string& command);

	/**
	 * \brief Shows a fatal error message, by printing it to stderr.
	 *
	 * @param msg The error message to show.
	 */
	virtual void FatalError(const string& msg);

	/**
	 * \brief Redisplays the interactive command input line.
	 *
	 * For the %ConsoleUI, this function displays a prompt ("GXemul> ")
	 * followed by the input line, placing the cursor position at
	 * the correct position on the input line.
	 *
	 * @param inputline The entire input line.
	 * @param cursorPosition The current cursor position. 0 is at the
	 *	leftmost position.
	 */
	virtual void RedisplayInputLine(const string& inputline,
	    size_t cursorPosition);

	/**
	 * \brief Executed by the CommandInterpreter when a line has been
	 * completed (with a newline).
	 *
	 * For the %ConsoleUI, this simply outputs a newline character.
	 */
	virtual void InputLineDone();

	/**
	 * \brief Runs the text console main loop.
	 *
	 * As long as the RunState is not Quitting:
	 * <ul>
	 *	<li>If an emulation is Running, the main loop lets the
	 *		emulation run, until Ctrl-C is pressed.
	 *	<li>Otherwise, if the RunState is Paused, the main loop shows
	 *		a prompt and lets the user input commands.
	 * </ul>
	 */
	virtual int MainLoop();

	virtual void Shutdown();

private:
	/**
	 * \brief Shows the %GXemul&gt; prompt, reads characters from stdin
	 *	until a newline char, and executes the resulting command.
	 *
	 * This function is blocking.
	 */
	void ReadAndExecuteCommand();

private:
	bool		m_consoleIsInitialized;
	struct termios	m_oldTermios;
	struct termios	m_currentTermios;
};


#endif	// CONSOLEUI_H
