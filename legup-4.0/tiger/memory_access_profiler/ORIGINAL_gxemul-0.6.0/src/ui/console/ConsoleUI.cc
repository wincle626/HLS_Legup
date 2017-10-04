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

#include <signal.h>
#include <unistd.h>
#include <iostream>

#include "misc.h"
#include "ConsoleUI.h"
#include "GXemul.h"


ConsoleUI::ConsoleUI(GXemul *gxemul)
	: UI(gxemul)
	, m_consoleIsInitialized(false)
{
}


ConsoleUI::~ConsoleUI()
{
	// Restore the terminal mode:
	if (m_consoleIsInitialized)
		tcsetattr(STDIN_FILENO, TCSANOW, &m_oldTermios);
}


// Note: Use of global GXemul pointer!
static GXemul* g_GXemul;
static struct termios g_curTermios;

static void ReshowCurrentCommandBuffer()
{
	if (g_GXemul->GetRunState() == GXemul::Paused) {
		// Reshow the prompt and the current command line:
		g_GXemul->GetCommandInterpreter().ReshowCurrentCommandBuffer();
		std::cout.flush();
	}
}

/**
 * \brief CTRL-C handler which sets the run state to Paused.
 */
extern "C" void ConsoleUI_SIGINT_Handler(int n)
{
	if (g_GXemul->IsInterrupting())
		std::cout << "^C (already attempting to interrupt, please wait)\n";
	else
		std::cout << "^C\n";

	g_GXemul->Interrupt();

	g_GXemul->GetCommandInterpreter().ClearCurrentCommandBuffer();
	ReshowCurrentCommandBuffer();

	signal(SIGINT, ConsoleUI_SIGINT_Handler);
}

/**
 * \brief Restore terminal settings after a CTRL-Z.
 *
 * If the user presses CTRL-Z (to stop the emulator process) and then
 * continues, the termios settings might have been invalidated. This
 * function restores them.
 */
extern "C" void ConsoleUI_SIGCONT_Handler(int n)
{
	tcsetattr(STDIN_FILENO, TCSANOW, &g_curTermios);
	ReshowCurrentCommandBuffer();
	signal(SIGCONT, ConsoleUI_SIGCONT_Handler);
}


void ConsoleUI::Initialize()
{
	if (m_consoleIsInitialized)
		return;

	tcgetattr(STDIN_FILENO, &m_oldTermios);
	m_currentTermios = m_oldTermios;

	// Set the terminal mode:
	m_currentTermios.c_lflag &= ~ICANON;
	m_currentTermios.c_cc[VTIME] = 0;
	m_currentTermios.c_cc[VMIN] = 1;
	m_currentTermios.c_lflag &= ~ECHO;

	tcsetattr(STDIN_FILENO, TCSANOW, &m_currentTermios);

	// Signal handlers for CTRL-C and CTRL-Z.
	// Note: Using a global GXemul instance pointer!
	g_GXemul = m_gxemul;
	g_curTermios = m_currentTermios;
	signal(SIGINT, ConsoleUI_SIGINT_Handler);
	signal(SIGCONT, ConsoleUI_SIGCONT_Handler);

	m_consoleIsInitialized = true;
}


void ConsoleUI::UpdateUI()
{
	// Empty.
}


void ConsoleUI::ShowStartupBanner()
{
	std::cout << GXemul::Version() << "\n";
}


static vector<string> SplitIntoRows(const string &msg, bool addEmptyLines)
{
	// This is slow and hackish, but works.
	vector<string> result;
	string line;

	for (size_t i=0, n=msg.length(); i<n; i++) {
		stringchar ch = msg[i];
		if (ch == '\n') {
			if (line.length() > 0 || addEmptyLines)
				result.push_back(line);
			line = "";
		} else {
			line += ch;
		}
	}

	if (line.length() > 0)
		result.push_back(line);

	return result;
}


void ConsoleUI::ShowDebugMessage(const string& msg)
{
	vector<string> lines = SplitIntoRows(msg, true);

	for (size_t i=0; i<lines.size(); ++i) {
		if (m_gxemul->GetRunState() == GXemul::Running)
			std::cout << "[ " << m_indentationMsg << lines[i] << " ]\n";
		else
			std::cout << m_indentationMsg << lines[i] << "\n";

		// Replace indentation string with spaces after first
		// line of output:
		for (size_t j=m_indentationMsg.length(); j>0; --j)
			m_indentationMsg[j-1] = ' ';
	}

	std::cout.flush();
}


void ConsoleUI::ShowDebugMessage(Component* component, const string& msg)
{
	if (m_gxemul->GetQuietMode())
		return;

	stringstream ss;
	string componentName = component->GenerateShortestPossiblePath();

	vector<string> lines = SplitIntoRows(msg, false);

	// cpu0: blahlonger
	//       blahshort
	size_t i;
	string spaces = "";
	for (i=0; i<componentName.length() + 2; i++)
		spaces += " ";
	
	ss << componentName << ": " << lines[0] << "\n";

	for (i=1; i<lines.size(); ++i)
		ss << spaces << lines[i] << "\n";

	ShowDebugMessage(ss.str());
}


void ConsoleUI::ShowCommandMessage(const string& command)
{
	// Not for ConsoleUI; commands entered by the user are
	// displayed anyway (echoing characters while the command
	// was entered).
}


void ConsoleUI::FatalError(const string& msg)
{
	std::cerr << msg;
	std::cerr.flush();
}


void ConsoleUI::RedisplayInputLine(const string& inputline,
    size_t cursorPosition)
{
	std::cout << "\rGXemul> " << inputline << " \rGXemul> ";

	for (size_t pos = 0; pos < cursorPosition; pos++)
		std::cout << (string() + inputline[pos]);

	std::cout.flush();
}


/**
 * \brief Read a key from stdin, blocking.
 *
 * @return The key read from stdin.
 */
static stringchar ReadKey()
{
	return std::cin.get();
}


void ConsoleUI::ReadAndExecuteCommand()
{
	// Initial dummy addkey, to show the input line with the prompt, etc.:
	m_gxemul->GetCommandInterpreter().AddKey('\0');

	while (!m_gxemul->GetCommandInterpreter().AddKey(ReadKey()))
		;
}


void ConsoleUI::InputLineDone()
{
	std::cout << "\n";
	std::cout.flush();
}


void ConsoleUI::Shutdown() 
{
}


int ConsoleUI::MainLoop()
{
	GXemul::RunState oldRunState = m_gxemul->GetRunState();

	while (m_gxemul->GetRunState() != GXemul::Quitting) {
		GXemul::RunState runState = m_gxemul->GetRunState();

		switch (runState) {

		case GXemul::SingleStepping:
		case GXemul::Running:
			// Switching from Paused state to running? Then
			// we need to:
			// 1) flush old cached state
			// 2) perform pre-run checks.
			if (oldRunState == GXemul::Paused) {
				m_gxemul->GetRootComponent()->FlushCachedState();

				if (!m_gxemul->GetRootComponent()->PreRunCheck(m_gxemul)) {
					FatalError("Pre-run check failed.\n");
					m_gxemul->SetRunState(GXemul::Paused);
					runState = m_gxemul->GetRunState();
					break;
				}
			}

			m_gxemul->Execute();
			break;

		case GXemul::Quitting:
			break;

		case GXemul::Paused:
			// When issuing interactive commands, "anything" can
			// happen to the component tree, and thus any cached
			// state quickly becomes untrustworthy. Let's flush it.
			m_gxemul->GetRootComponent()->FlushCachedState();

			ReadAndExecuteCommand();
			break;
		}

		oldRunState = runState;
	}

	return 0;
}

