#ifndef NULLUI_H
#define	NULLUI_H

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

#include "UI.h"


/**
 * \brief Dummy UI, which does not do anything.
 */
class NullUI
	: public UI
{
public:
	/**
	 * \brief Constructs a %NullUI.
	 *
	 * @param gxemul Pointer to the owning GXemul instance.
	 */
	NullUI(GXemul *gxemul);

	virtual ~NullUI();

	/**
	 * \brief Does nothing, for the dummy UI.
	 */
	virtual void Initialize();

	/**
	 * \brief Does nothing, for the dummy UI.
	 */
	virtual void UpdateUI();

	/**
	 * \brief Does nothing, for the dummy UI.
	 */
	virtual void ShowStartupBanner();

	/**
	 * \brief Does nothing, for the dummy UI.
	 *
	 * @param msg The message to show. (Ignored.)
	 */
	virtual void ShowDebugMessage(const string& msg);

	/**
	 * \brief Does nothing, for the dummy UI.
	 *
	 * @param component A pointer to the Component. (Ignored.)
	 * @param msg The message to show. (Ignored.)
	 */
	virtual void ShowDebugMessage(Component* component, const string& msg);

	/**
	 * \brief Does nothing, for the dummy UI.
	 *
	 * @param command The command to show. (Ignored.)
	 */
	virtual void ShowCommandMessage(const string& command);

	/**
	 * \brief Does nothing, for the dummy UI.
	 *
	 * @param msg The error message to show. (Ignored.)
	 */
	virtual void FatalError(const string& msg);

	/**
	 * \brief Does nothing, for the dummy UI.
	 *
	 * @param inputline The entire input line. (Ignored.)
	 * @param cursorPosition The current cursor position. 0 is at the
	 *	leftmost position. (Ignored.)
	 */
	virtual void RedisplayInputLine(const string& inputline,
	    size_t cursorPosition);

	/**
	 * \brief Executed by the CommandInterpreter when a line has been
	 * completed (with a newline).
	 *
	 * For the %NullUI, this is ignored.
	 */
	virtual void InputLineDone();

	/**
	 * \brief Runs the main loop. Ignored by the NullUI.
	 *
	 * The NullUI returns 0 immediately.
	 */
	virtual int MainLoop();

	virtual void Shutdown();
};


#endif	// NULLUI_H
