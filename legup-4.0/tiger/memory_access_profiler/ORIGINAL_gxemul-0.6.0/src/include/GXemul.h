#ifndef GXEMUL_H
#define	GXEMUL_H

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

#include <sys/time.h>

#include "CommandInterpreter.h"
#include "Component.h"
#include "UI.h"


/**
 * \brief The main emulator class.
 *
 * A %GXemul instance basically has the following member variables:
 *
 * <ol>
 *	<li>a tree of components, which make up the full
 *		state of the current emulation setup
 *	<li>a UI, which runs the main [interactive] loop
 *	<li>a CommandInterpreter
 *	<li>a RunState
 *	<li>a list of "on reset commands", which are executed on Reset().
 * </ol>
 */
class GXemul
{
public:
	enum RunState
	{
		Paused,
		SingleStepping,			// Single-step execution
		Running,			// Continuous execution
		Quitting
	};

public:
	/**
	 * \brief Creates a GXemul instance.
	 */
	GXemul();

	/**
	 * \brief Parses command line arguments (file names).
	 *
	 * @param templateMachine The template machine to use.
	 * @param filenameCount for parsing command line options.
	 * @param filenames for parsing command line options.
	 * @return true if options were parsable, false if there was
	 *		some error.
	 */
	bool ParseFilenames(string templateMachine, int filenameCount, char *filenames[]);

	/**
	 * \brief Discards the current emulation, and starts anew with just
	 *	an empty root component.
	 */
	void ClearEmulation();

	/**
	 * \brief Initializes the %UI.
	 */
	void InitUI();

	/**
	 * \brief Runs GXemul's main loop.
	 *
	 * @return Zero on success, non-zero on error.
	 */
	int Run();

	/**
	 * \brief Gets the current emulation setup's filename.
	 *
	 * @return The name of the file that is used for the current emulation
	 *	setup. If no filename is defined yet, this is an empty string.
	 */
	const string& GetEmulationFilename() const;

	/**
	 * \brief Sets the current emulation setup's filename.
	 *
	 * @param filename This is the name of the file that is used
	 *	for the current emulation setup.
	 */
	void SetEmulationFilename(const string& filename);

	/**
	 * \brief Gets a reference to the CommandInterpreter.
	 *
	 * @return A reference to the %GXemul instance' CommandInterpreter.
	 */
	CommandInterpreter& GetCommandInterpreter();

	/**
	 * \brief Gets a pointer to the %GXemul instance' active UI.
	 *
	 * Note: Never NULL. The UI may be the NullUI, or another UI (such
	 * as the ConsoleUI).
	 *
	 * @return A pointer to the UI in use.
	 */
	UI* GetUI();

	/**
	 * \brief Gets a pointer to the root configuration component.
	 *
	 * @return A pointer to the root component. If no configuration tree
	 *	is loaded, then this is at least an empty dummy component.
	 *	(The return value is never NULL.)
	 */
	refcount_ptr<Component> GetRootComponent();
	const refcount_ptr<Component> GetRootComponent() const;

	/**
	 * \brief Sets the root component, discarding the previous one.
	 *
	 * This function should not be used to set the root component
	 * to NULL. Use ClearEmulation() instead.
	 *
	 * @param newRootComponent A reference counted pointer to the new
	 *	root component. It may not be a NULL pointer.
	 */
	void SetRootComponent(refcount_ptr<Component> newRootComponent);

	/**
	 * \brief Resets the emulation.
	 *
	 * This function recursively resets all components in the tree, and
	 * then executes the "on reset" commands (usually commands to load
	 * files into CPUs).
	 *
	 * @return false if any of the reset commands failed.
	 */
	bool Reset();

	/**
	 * \brief Interrupts emulation.
	 *
	 * Only meaningful if RunState is Running or SingleStepping.
	 */
	void Interrupt();

	/**
	 * \brief Returns whether or not the current emulation is being
	 * interrupted.
	 *
	 * Only meaningful if RunState is Running or SingleStepping.
	 */
	bool IsInterrupting() const
	{
		return m_interrupting;
	}

	/**
	 * \brief Sets the RunState.
	 *
	 * @param newState The new RunState.
	 */
	void SetRunState(RunState newState);

	/**
	 * \brief Gets the current RunState.
	 *
	 * @return The current RunState.
	 */
	RunState GetRunState() const;

	/**
	 * \brief Gets the current RunState as a string.
	 *
	 * @return The current RunState, formatted as a string.
	 */
	string GetRunStateAsString() const;

	/**
	 * \brief Gets the current step of the emulation.
	 *
	 * @return The nr of steps that the emulation has been executing,
	 *	since the start.
	 */
	uint64_t GetStep() const;

	/**
	 * \brief Checks whether snapshots are currently enabled or not.
	 *
	 * @return True if running in quiet mode, false for normal operation.
	 */
	bool GetSnapshottingEnabled() const;

	/**
	 * \brief Sets whether or not to use snapshots.
	 *
	 * @param enabled true to enable snapshotting, false to disable it.
	 */
	void SetSnapshottingEnabled(bool enabled);

	/**
	 * \brief Gets the current quiet mode setting.
	 *
	 * @return True if running in quiet mode, false for normal operation.
	 */
	bool GetQuietMode() const;

	/**
	 * \brief Sets whether or not to run in quiet mode.
	 *
	 * @param quietMode true to run in quiet mode, false otherwise.
	 */
	void SetQuietMode(bool quietMode);

	/**
	 * \brief Sets the nr of single-steps to perform in a row.
	 *
	 * @param steps The number of steps, at least 1.
	 */
	void SetNrOfSingleStepsInARow(uint64_t steps);

	/**
	 * \brief Change step either forwards or backwards.
	 *
	 * @param oldStep The old step count.
	 * @param newStep The new step count.
	 * @return True if changing step worked, false if there was a failure.
	 */
	bool ModifyStep(int64_t oldStep, int64_t newStep);

	/**
	 * \brief Run the emulation for "a while".
	 *
	 * When single-stepping, this function will:
	 * <ul>
	 *	<li>execute one step,
	 *	<li>set the run state to Paused,
	 *	<li>and then return.
	 * </ul>
	 *
	 * When not single-stepping, components will execute multiple steps at
	 * once, if possible. In the most common case (no breakpoints or other
	 * special cases), when this function returns, the run state will not
	 * have been affected.
	 *
	 * @param longestTotalRun Maximum number of steps to execute.
	 */
	void Execute(const int longestTotalRun = 100000);

	/**
	 * \brief Dump a list to stdout with all available machine templates.
	 */
	static void ListTemplates();

	/**
	 * \brief Returns the GXemul version string.
	 *
	 * @return A string describing the GXemul version.
	 */
	static string Version();

	bool IsTemplateMachine(const string& templateName) const;
	static void DumpMachineAsHTML(const string& machineName);
	static void GenerateHTMLListOfComponents(bool machines);

private:
	/**
	 * \brief Creates an emulation setup from a template machine name.
	 *
	 * @param templateName The name of the template machine.
	 * @return True if the emulation was created, false otherwise.
	 */
	bool CreateEmulationFromTemplateMachine(const string& templateName);

	/**
	 * \brief Prints help message to std::cout.
	 */
	void PrintUsage() const;

	/**
	 * \brief Sets the current step of the emulation.
	 *
	 * @param step The number of steps.
	 */
	void SetStep(uint64_t step);

	/**
	 * \brief Takes a snapshot of the full emulation state.
	 */
	void TakeSnapshot();


	/********************************************************************/
public:
	static void RunUnitTests(int& nSucceeded, int& nFailures);

private:
	// Base:
	bool			m_quietMode;
	refcount_ptr<UI>	m_ui;
	CommandInterpreter	m_commandInterpreter;
	vector<string>		m_onResetCommands;

	// Runtime:
	RunState		m_runState;
	bool			m_interrupting;
	uint64_t		m_nrOfSingleStepsLeft;

	// Performance measurement:
	struct timeval		m_lastOutputTime;
	uint64_t		m_lastOutputStep;

	// Model:
	string			m_emulationFileName;
	refcount_ptr<Component>	m_rootComponent;

	// Snapshotting:   TODO: Multiple snapshots!
	bool			m_snapshottingEnabled;
	refcount_ptr<Component>	m_snapshot;
};

#endif	// GXEMUL_H
