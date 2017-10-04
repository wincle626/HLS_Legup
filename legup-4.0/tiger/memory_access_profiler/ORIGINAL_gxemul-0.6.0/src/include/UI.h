#ifndef UI_H
#define	UI_H

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

class Component;
class GXemul;


/**
 * \brief Base class for a User Interface.
 */
class UI
	: public ReferenceCountable
{
public:
	class SetIndentationMessageHelper
	{
	public:
		SetIndentationMessageHelper(UI* ui, const string& msg)
			: m_UI(ui)
		{
			m_oldMsg = m_UI->GetIndentationMessage();
			m_UI->SetIndentationMessage(msg);
		}

		~SetIndentationMessageHelper()
		{
			m_UI->SetIndentationMessage(m_oldMsg);
		}

	private:
		UI *	m_UI;
		string	m_oldMsg;
	};

public:
	/**
	 * \brief Constructs a User Interface.
	 *
	 * @param gxemul A pointer to the GXemul owner instance.
	 */
	UI(GXemul* gxemul)
	    : m_gxemul(gxemul)
	{
	}

	virtual ~UI()
	{
	}

	/**
	 * \brief Initializes the UI.
	 */
	virtual void Initialize() = 0;

	/**
	 * \brief Shows a startup banner.
	 */
	virtual void ShowStartupBanner() = 0;

	/**
	 * \brief Updates various UI elements.
	 *
	 * Called whenever:
	 * <ul>
	 *   <li>the undo/redo-applicability changes
	 *   <li>the name of the emulation changes
	 *   <li>the RunState changes
	 * </ul>
	 */
	virtual void UpdateUI() = 0;

	/**
	 * \brief Sets an indentation message, which indents all debug output.
	 *
	 * Note: An UI implementation may change the indentation string. E.g.
	 * from "step X: " to "        ".
	 *
	 * @param msg The indentation message. If the message is an empty
	 *	string, indentation is not used.
	 */
	void SetIndentationMessage(const string& msg)
	{
		m_indentationMsg = msg;
	}

	/**
	 * \brief Gets the indentation message.
	 *
	 * @return The indentation message. If the message is an empty
	 *	string, indentation is not used.
	 */
	string GetIndentationMessage() const
	{
		return m_indentationMsg;
	}

	/**
	 * \brief Shows a debug message.
	 *
	 * @param msg The message to show.
	 */
	virtual void ShowDebugMessage(const string& msg) = 0;

	/**
	 * \brief Shows a debug message for a Component.
	 *
	 * Usually, this involves formatting the debug message using
	 * Component::GenerateShortestPossiblePath().
	 *
	 * If the runstate is GXemul::Running, the debug message
	 * is encapsulated by "[ ]" brackets, making the debug message very
	 * similar to pre-0.6.0 debug output style.
	 *
	 * @param component A pointer to the Component.
	 * @param msg The message to show.
	 */
	virtual void ShowDebugMessage(Component* component, const string& msg) = 0;

	/**
	 * \brief Shows a command being executed.
	 *
	 * @param command The command being executed.
	 */
	virtual void ShowCommandMessage(const string& command) = 0;

	/**
	 * \brief Shows a fatal error message.
	 *
	 * After showing the fatal error message, the application
	 * is expected to terminate. In a GUI implementation, it
	 * is therefore good to wait for the user to acknowledge
	 * the error message before returning.
	 *
	 * @param msg The message to show.
	 */
	virtual void FatalError(const string& msg) = 0;

	/**
	 * \brief Redisplays the interactive command input line.
	 *
	 * This function generally displays a prompt (e.g. "GXemul> ")
	 * followed by the input line, placing the cursor position at
	 * the correct position on the input line.
	 *
	 * @param inputline The entire input line.
	 * @param cursorPosition The current cursor position. 0 is at the
	 *	leftmost position.
	 */
	virtual void RedisplayInputLine(const string& inputline,
	    size_t cursorPosition) = 0;

	/**
	 * \brief Executed by the CommandInterpreter when a line has been
	 * completed (with a newline).
	 *
	 * Usually this clears the current line, and (if possible) moves
	 * down to a new line.
	 */
	virtual void InputLineDone() = 0;

	/**
	 * \brief Runs the UI's main loop.
	 *
	 * The return code should be the exit code that the gxemul binary
	 * should return.
 	 *
	 * @return 0 on success, non-zero on failure.
	 */
	virtual int MainLoop() = 0;

	/**
	 * \brief Shuts down the UI.
	 *
	 * Called from e.g. the "quit" command.
	 */
	virtual void Shutdown() = 0;

protected:
	GXemul*		m_gxemul;
	string		m_indentationMsg;
};


#endif	// UI_H
