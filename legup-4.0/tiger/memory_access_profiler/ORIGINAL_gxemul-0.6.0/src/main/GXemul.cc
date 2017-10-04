/*
 *  Copyright (C) 2003-2010  Anders Gavare.  All rights reserved.
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
 *
 *
 *  This file contains two things:
 *
 *	1. Doxygen documentation for the general design concepts of the
 *	   emulator. The mainpage documentation written here ends up on
 *	   the "main page" in the generated HTML documentation.
 *
 *	2. The GXemul class implementation.
 */


/*! \mainpage Source code documentation
 *
 * \section intro_sec Introduction
 *
 * This is the automatically generated Doxygen documentation for %GXemul,
 * built from comments throughout the source code.
 *
 * See the <a href="../../index.html">main documentation</a> for more
 * information about this version of %GXemul.
 *
 * See GXemul's home page for more information about %GXemul in general:
 * <a href="http://gxemul.sourceforge.net/">http://gxemul.sourceforge.net/</a>
 *
 * (<b>NOTE:</b> There is a huge portion of code in
 * %GXemul which is legacy code. The documentation you will find on this page
 * is only about the new framework, which is available from release 0.6.0.)
 *
 * The main program creates a GXemul instance, and does one of two things:
 * <ul>
 *	<li>Starts without any template %machine. (<tt>-V</tt>)
 *	<li>Starts with a template %machine, and a list of filenames to load
 *		(usually a kernel binary to boot the emulated %machine).
 * </ul>
 * After letting the %GXemul instance load the files, GXemul::Run() is called.
 * This is the main loop. It doesn't really do much, it simply calls the UI's
 * main loop, i.e. ConsoleUI::MainLoop().
 *
 * Most of the source code in %GXemul centers around a few core concepts.
 * An overview of these concepts are given below. Anyone who wishes to
 * delve into the source code should be familiar with them.
 *
 *
 * \section concepts_sec Core concepts
 *
 * \subsection components_subsec Components
 *
 * The most important core concept in %GXemul is the Component. Examples of
 * components are processors, networks interface cards, video displays, RAM
 * %memory, busses, %interrupt controllers, and all other kinds of devices.
 *
 * Each component has a parent, so the full set of components in an emulation
 * are in fact a tree. A GXemul instance has one such tree. The root
 * component is a special RootComponent, which holds some basic state about
 * the emulation, such as the number of steps executed.
 * It also contains zero or more sub-components.
 *
 * <center><img src="../../model.png"></center>
 *
 * Starting from the root node, each component has a <i>path</i>, e.g.
 * <tt>root.machine1.mainbus0.ram0</tt> for the RAM component in machine1
 * in the example above.
 *
 * The state of each component is stored within that component. The state
 * consists of a number of variables (see StateVariable) such as strings,
 * integers, bools, and other more high-level types such as zero-filled %memory
 * arrays. Such %memory arrays are used e.g. by the RAMComponent to emulate
 * RAM, and can also be used to emulate video framebuffer %memory.
 *
 * Individual components are implemented in <tt>src/components/</tt>, with
 * header files in <tt>src/include/components/</tt>. The <tt>configure</tt>
 * script looks for the string <tt>COMPONENT(name)</tt> in the header files,
 * and automagically adds those to the list of components that will be
 * available at runtime. In addition, <tt>make documentation</tt> also builds
 * HTML pages with lists of available
 * <a href="../../components.html">components</a>, and as a special case,
 * a list of available
 * <a href="../../machines.html">template machines</a> (because they have
 * special meaning to the end-user).
 *
 * \subsection commandinterpreter_subsec Command interpreter
 *
 * A GXemul instance has a CommandInterpreter, which is the part that parses a
 * command line, figures out which Command is to be executed, and executes it.
 * The %CommandInterpreter can be given a complete command line as a string, or
 * it can be given one character (or keypress) at a time. In the later case,
 * the TAB key either completes the word currently being written, or writes
 * out a list of possible completions.
 *
 * The %CommandInterpreter, via the ConsoleUI, is the user interface as seen
 * by the user.
 *
 * \subsection unittest_subsec Unit tests
 *
 * Wherever it makes sense, unit tests should be written to make sure
 * that the code is correct, and stays correct. The UnitTest class contains
 * static helper functions for writing unit tests, such as UnitTest::Assert.
 * To add unit tests to a class, the class should be UnitTestable, and in
 * particular, it should implement UnitTestable::RunUnitTests by using the
 * UNITTESTS(className) macro. Individual test cases are then called, as
 * static non-member functions, using the UNITTEST(testname) macro.
 *
 * Since test cases are non-member functions, they need to create instances
 * of the class they wish to test, and they can only call public member
 * functions on those objects, not private ones. Thus, the unit tests only
 * test the "public API" of all classes. (If the internal API needs to be
 * tested, then a workaround can be to add a ConsistencyCheck member function
 * which is public, but mentioning in the documentation for that function
 * that it is only meant for internal use and debugging.)
 *
 * Unit tests are normally executed by <tt>make test</tt>. This is implicitly
 * done when doing <tt>make install</tt> as well.
 *
 * It is recommended to run the <tt>configure</tt> script with the
 * <tt>--debug</tt> option during development; this enables
 * <a href="http://wyw.dcweb.cn/">Wu Yongwei</a>'s new/debug %memory
 * leak detector (part of
 * <a href="http://sourceforge.net/projects/nvwa/">Stones of NVWA</a>).
 */


/*****************************************************************************/

#include "ConsoleUI.h"
#include "NullUI.h"

#include "GXemul.h"
#include "components/RootComponent.h"
#include "ComponentFactory.h"
#include "UnitTest.h"

#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <cmath>
#include <fstream>
#include <iostream>


GXemul::GXemul()
	: m_quietMode(false)
	, m_ui(new NullUI(this))
	, m_commandInterpreter(this)
	, m_runState(Paused)
	, m_interrupting(false)
	, m_nrOfSingleStepsLeft(1)
	, m_rootComponent(new RootComponent(this))
	, m_snapshottingEnabled(false)
{
	gettimeofday(&m_lastOutputTime, NULL);
	m_lastOutputStep = 0;

	ClearEmulation();
}


void GXemul::ClearEmulation()
{
	if (GetRunState() == Running)
		SetRunState(Paused);

	m_rootComponent = new RootComponent(this);
	m_emulationFileName = "";

	GetUI()->UpdateUI();
}


bool GXemul::IsTemplateMachine(const string& templateName) const
{
	string nameWithoutArgs = templateName;
	size_t p = nameWithoutArgs.find('(');
	if (p > 0)
		nameWithoutArgs = templateName.substr(0, p);

	if (!ComponentFactory::HasAttribute(nameWithoutArgs, "template"))
		return false;

	if (!ComponentFactory::HasAttribute(nameWithoutArgs, "machine"))
		return false;

	return true;
}


bool GXemul::CreateEmulationFromTemplateMachine(const string& templateName)
{
	if (!IsTemplateMachine(templateName)) {
		std::cerr << templateName << " is not a known template machine name.\n"
		    "Use gxemul -H to get a list of valid machine templates.\n";
		return false;
	}

	refcount_ptr<Component> machine =
	    ComponentFactory::CreateComponent(templateName, this);
	if (machine.IsNULL())
		return false;

	GetRootComponent()->AddChild(machine);
	return true;
}


void GXemul::ListTemplates()
{
	std::cout << "Available template machines:\n\n";

	vector<string> names = ComponentFactory::GetAllComponentNames(true);

	size_t maxNameLen = 0;
	for (size_t i=0; i<names.size(); ++i)
		if (names[i].length() > maxNameLen)
			maxNameLen = names[i].length();

	for (size_t i=0; i<names.size(); ++i) {
		string name = names[i];
		
		std::cout << "  " << name;
		for (size_t j=0; j<maxNameLen - name.length() + 6; ++j)
			std::cout << " ";

		std::cout << ComponentFactory::GetAttribute(
		    name, "description");
		std::cout << "\n";
	}

	std::cout << "\n";
}


void GXemul::DumpMachineAsHTML(const string& machineName)
{
	refcount_ptr<Component> component =
	    ComponentFactory::CreateComponent(machineName);

	if (!component.IsNULL() &&
	    component->GetChildren().size() != 0)
		std::cout << "<pre>" <<
		    component->GenerateTreeDump("", true, "../")
		    << "</pre>";
}


void GXemul::GenerateHTMLListOfComponents(bool machines)
{
	std::cout <<
		"Available " <<
		(machines? "template machines" : "components") << ":\n"
		"<p><table border=0>\n"
		"<tr>\n"
		" <td><b><u>" <<
		(machines? "Machine&nbsp;name" : "Component&nbsp;name") << ":"
		"</u></b>&nbsp;&nbsp;</td>\n";
	if (machines)
		std::cout << " <td><b><u>Screenshot:</u></b>&nbsp;&nbsp;</td>\n";
	std::cout <<
#ifdef UNSTABLE_DEVEL
		" <td><b><u>Status:</u></b>&nbsp;&nbsp;</td>\n"
#endif
		" <td><b><u>Description:</u></b>&nbsp;&nbsp;</td>\n"
		" <td><b><u>Comments:</u></b>&nbsp;&nbsp;</td>\n"
		" <td><b><u>Contributors:</u></b>&nbsp;&nbsp;</td>\n"
		"</tr>\n";

	bool everyOther = false;
	vector<string> names = ComponentFactory::GetAllComponentNames(false);
	for (size_t i=0; i<names.size(); ++i) {
		const string& componentName = names[i];
		string treeDump;

		refcount_ptr<Component> creatable =
		    ComponentFactory::CreateComponent(componentName);
		if (creatable.IsNULL())
			continue;

		bool isTemplateMachine = !ComponentFactory::GetAttribute(
		    componentName, "machine").empty() &&
		    !ComponentFactory::GetAttribute(
		    componentName, "template").empty();

		if (machines) {
			if (!isTemplateMachine)
				continue;
		} else {
			// Other components:  Don't include template machines.
			if (isTemplateMachine)
			    	continue;
		}

		// Include an ASCII tree dump for template components that
		// have children:
		if (!ComponentFactory::GetAttribute(
		    componentName, "template").empty()) {
			refcount_ptr<Component> component =
			    ComponentFactory::CreateComponent(componentName);

			if (!component.IsNULL() &&
			    component->GetChildren().size() != 0)
				treeDump = "<pre>" +
				    component->GenerateTreeDump("", true)
				    + "</pre>";
		}

		// Some distance between table entries:
		std::cout <<
			"<tr>\n"
			" <td></td>"
			"</tr>\n";

		std::cout <<
			"<tr bgcolor=" <<
			(everyOther? "#f2f2f2" : "#e4e4e4") << ">\n";

		// Include a href link to a "full html page" for a component,
		// if it exists:
		std::ifstream documentationComponentFile((
		    "doc/components/component_"
		    + componentName + ".html").c_str());
		std::ifstream documentationMachineFile((
		    "doc/machines/machine_"
		    + componentName + ".html").c_str());

		if (documentationComponentFile.is_open())
			std::cout <<
				" <td valign=top>"
				"<a href=\"components/component_" <<
				componentName
				<< ".html\"><tt>" << componentName <<
				"</tt></a></td>\n";
		else if (documentationMachineFile.is_open())
			std::cout <<
				" <td valign=top>"
				"<a href=\"machines/machine_" <<
				componentName
				<< ".html\"><tt>" << componentName <<
				"</tt></a></td>\n";
		else
			std::cout <<
				" <td valign=top><tt>" << componentName
				<< "</tt></td>\n";

		if (machines) {
			// Include an img and a href link to a screenshot for a component,
			// if it exists:
			std::ifstream screenshotThumbFile((
			    "doc/machines/machine_"
			    + componentName + "-thumb.png").c_str());
			std::ifstream screenshotLargeFile((
			    "doc/machines/machine_"
			    + componentName + ".png").c_str());

			std::cout << " <td valign=top align=center><tt>";

			if (screenshotLargeFile.is_open())
				std::cout << "<a href=machines/machine_" <<
				    componentName << ".png>";

			if (screenshotThumbFile.is_open())
				std::cout << "<img src=machines/machine_" <<
				    componentName << "-thumb.png>";
			else if (screenshotLargeFile.is_open())
				std::cout << "(screenshot)";

			if (screenshotLargeFile.is_open())
				std::cout << "</a>";

			std::cout << "</tt></td>\n";
		}

		std::cout <<
#ifdef UNSTABLE_DEVEL
			" <td valign=top>" << (ComponentFactory::HasAttribute(
				componentName, "stable")? "stable&nbsp;&nbsp;" :
				"experimental&nbsp;&nbsp;") << "</td>\n"
#endif
			" <td valign=top>" << ComponentFactory::GetAttribute(
				componentName, "description") <<
				treeDump << "</td>\n"
			" <td valign=top>" << ComponentFactory::GetAttribute(
				componentName, "comments") << "</td>\n"
			" <td valign=top>" << ComponentFactory::GetAttribute(
				componentName, "contributors") << "</td>\n"
			"</tr>\n";

		everyOther = !everyOther;
	}

	std::cout << "</table><p>\n";
}


bool GXemul::ParseFilenames(string templateMachine, int filenameCount, char *filenames[])
{
	bool optionsEnoughToStartRunning = false;

	if (templateMachine != "") {
		if (CreateEmulationFromTemplateMachine(templateMachine)) {
			// A template is now being used.
		} else {
			std::cerr << "Failed to create configuration from "
			    "template: " << templateMachine << "\n" <<
			    "Aborting." << "\n";
			return false;
		}
	}

	//  1. If a machine template has been selected, then treat the following
	//     arguments as files to load. (Legacy compatibility with previous
	//     versions of GXemul.)
	//
	//  2. Otherwise, treat the argument as a configuration file.

	if (filenameCount > 0) {
		if (templateMachine != "") {
			// Machine template.
			while (filenameCount > 0) {
				// Hm. Why the full path to cpu0 here? Well,
				// a typical use case I imagine is that people
				// start with a machine template and one or more
				// files on the command line, and then add
				// another machine afterwards. Then the load/
				// reset commands will still work, if the path
				// is full-length instead of short.
				stringstream cmd;
				cmd << "load " << filenames[0]
				    << " root.machine0.mainbus0.cpu0";
				m_onResetCommands.push_back(cmd.str());

				filenameCount --;
				filenames ++;
			}

			optionsEnoughToStartRunning = true;
		} else {
			// Config file.
			if (filenameCount == 1) {
				string configfileName = filenames[0];
				optionsEnoughToStartRunning = true;

				string cmd = "load " + configfileName;
				m_onResetCommands.push_back(cmd);
			} else {
				std::cerr << "More than one configfile name "
				    "supplied on the command line?" << "\n" <<
				    "Aborting." << "\n";
				return false;
			}
		}
	}

	if (optionsEnoughToStartRunning) {
		return true;
	} else {
		if (templateMachine != "") {
			if (GetRunState() == Paused)
				return true;

			std::cerr << 
			    "No binary specified. Usually when starting up an emulation based on a template\n"
			    "machine, you need to supply one or more binaries. This could be an operating\n"
			    "system kernel, a ROM image, or something similar.\n"
			    "\n"
			    "You can also use the -V option to start in paused mode, and load binaries\n"
			    "interactively.\n"
			    "\n"
			    "(Run  gxemul -h  for more help on command line options.)\n";
			return false;
		}
		
		PrintUsage();
		return false;
	}
}


string GXemul::Version()
{
	stringstream ss;
	ss << "GXemul "
#ifdef VERSION
	    << VERSION
#else
	    << "(unknown version)"
#endif
	    << "      "COPYRIGHT_MSG"\n"SECONDARY_MSG;

	return ss.str();
}


void GXemul::PrintUsage() const
{
	std::cout << Version() << "\n";

	std::cout << "Insufficient command line arguments given to"
	    " start an emulation. You have\n"
	    "the following alternatives:\n" <<
	    "\n" <<
	    "  1. Run  gxemul  with the machine selection option "
	    "(-e), which creates\n"
	    "     a default emulation from a template machine.\n\n"
	    "  2. Run  gxemul  with a configuration file (.gxemul).\n"
	    "     This is useful for more complicated setups.\n\n"
	    "  3. Run  gxemul -V  with no other options, which causes"
	    " gxemul to be started\n"
	    "     with no emulation loaded at all.\n\n" <<
	    "\n" <<
	    "Run  gxemul -h  for help on command line options.\n\n";
}


void GXemul::InitUI()
{
	// Default to the console UI:
	m_ui = new ConsoleUI(this);

	// Once a GUI has been implemented, this is the
	// place to call its constructor. TODO

	GetUI()->Initialize();
}


int GXemul::Run()
{
	// Not really running yet:
	RunState savedRunState = GetRunState();
	SetRunState(Paused);
	
	if (!GetQuietMode()) {
		GetUI()->ShowStartupBanner();

		// Dump (a suitable part of) the configuration tree at startup.
		const Component* component = GetRootComponent();
		if (component->GetChildren().size() > 0) {
			while (true) {
				int nChildren = component->GetChildren().size();
				if (nChildren == 0 || nChildren > 1)
					break;

				component = component->GetChildren()[0];
			}

			GetUI()->ShowDebugMessage(component->GenerateTreeDump("") + "\n");
		}
	}

	if (!Reset()) {
		GetUI()->ShowDebugMessage("Aborting.\n");
		return 1;
	}

	if (!GetQuietMode()) {
		// A separator line, if we start emulating directly without dropping
		// into the interactive debugger. (To mimic pre-0.6.0 appearance.)
		if (savedRunState == Running)
			GetUI()->ShowDebugMessage("--------------------------------"
			    "-----------------------------------------------\n\n");
	}

	SetRunState(savedRunState);


	try {
		GetUI()->MainLoop();
	} catch (std::exception& ex) {
		stringstream ss;
		ss << "\n### FATAL ERROR ###\n\n" << ex.what() << "\n\n" <<
		    "If you are able to reproduce this crash, "
		    "please send detailed repro-steps to\n"
		    "the author, to the gxemul-devel mailing list, or"
		    " ask in #GXemul on the\n"
		    "FreeNode IRC network.\n";

		GetUI()->FatalError(ss.str());

		return 1;
	}

	return 0;
}


const string& GXemul::GetEmulationFilename() const
{
	return m_emulationFileName;
}


void GXemul::SetEmulationFilename(const string& filename)
{
	m_emulationFileName = filename;

	GetUI()->UpdateUI();
}


CommandInterpreter& GXemul::GetCommandInterpreter()
{
	return m_commandInterpreter;
}


uint64_t GXemul::GetStep() const
{
	const StateVariable* step = GetRootComponent()->GetVariable("step");
	if (step == NULL) {
		std::cerr << "root component has no 'step' variable? aborting.\n";
		throw std::exception();
	}

	return step->ToInteger();
}


void GXemul::SetStep(uint64_t step)
{
	StateVariable* stepVariable = GetRootComponent()->GetVariable("step");
	if (stepVariable == NULL) {
		std::cerr << "root component has no 'step' variable? aborting.\n";
		throw std::exception();
	}

	stepVariable->SetValue(step);
}


UI* GXemul::GetUI()
{
	return m_ui;
}


refcount_ptr<Component> GXemul::GetRootComponent()
{
	return m_rootComponent;
}


const refcount_ptr<Component> GXemul::GetRootComponent() const
{
	return m_rootComponent;
}


void GXemul::SetRootComponent(refcount_ptr<Component> newRootComponent)
{
	if (newRootComponent.IsNULL()) {
		std::cerr << "GXemul::SetRootComponent: NULL\n";
		throw std::exception();
	}

	RootComponent* rootComponent = newRootComponent->AsRootComponent();
	if (rootComponent == NULL) {
		std::cerr << "GXemul::SetRootComponent: not a RootComponent\n";
		throw std::exception();
	}

	rootComponent->SetOwner(this);

	m_rootComponent = newRootComponent;

	GetUI()->UpdateUI();
}


bool GXemul::Reset()
{
	// 1. Reset all components in the tree.
	GetRootComponent()->Reset();

	// 2. Run "on reset" commands. (These are usually commands to load
	//    binaries into CPUs.)
	vector<string>::const_iterator it = m_onResetCommands.begin();
	for (; it != m_onResetCommands.end(); ++it) {
		string cmd = *it;
		bool success = false;

		GetCommandInterpreter().RunCommand(cmd, &success);

		if (!GetQuietMode())
			GetUI()->ShowDebugMessage("\n");

		if (!success) {
			GetUI()->ShowDebugMessage("Failing on-reset command:\n"
			    "    " + cmd + "\n");
			return false;
		}
	}

	return true;
}


void GXemul::Interrupt()
{
	switch (GetRunState()) {
	case SingleStepping:
	case Running:
		m_interrupting = true;
		break;
	default:
		m_interrupting = false;
	}
}


void GXemul::SetRunState(RunState newState)
{
	m_runState = newState;

	GetUI()->UpdateUI();
}


GXemul::RunState GXemul::GetRunState() const
{
	return m_runState;
}


string GXemul::GetRunStateAsString() const
{
	switch (m_runState) {
	case Paused:
		return "Paused";
	case SingleStepping:
		return "Single-stepping";
	case Running:
		return "Running";
	case Quitting:
		return "Quitting";
	}

	return "Unknown RunState";
}


bool GXemul::GetSnapshottingEnabled() const
{
	return m_snapshottingEnabled;
}


void GXemul::SetSnapshottingEnabled(bool enabled)
{
	if (enabled)
		GetUI()->ShowDebugMessage("(Enabling "
		    "snapshotting/reverse execution support.)\n");

	m_snapshottingEnabled = enabled;
}


bool GXemul::GetQuietMode() const
{
	return m_quietMode;
}


void GXemul::SetQuietMode(bool quietMode)
{
	m_quietMode = quietMode;
}


void GXemul::SetNrOfSingleStepsInARow(uint64_t steps)
{
	if (steps < 1)
		steps = 1;

	m_nrOfSingleStepsLeft = steps;
}


bool GXemul::ModifyStep(int64_t oldStep, int64_t newStep)
{
	if (!GetSnapshottingEnabled())
		return false;

	if (oldStep == newStep)
		return true;

	if (newStep < oldStep) {
		// Run in reverse, by running forward from the most suitable
		// snapshot.

		// TODO: Multiple snapshots!
		refcount_ptr<Component> newRoot = m_snapshot->Clone();
		SetRootComponent(newRoot);

		// GetStep will now return the step count for the new root.
		int64_t nrOfStepsToRunFromSnapshot = newStep - GetStep();

		RunState oldRunState = GetRunState();
		SetRunState(Running);

		Execute(nrOfStepsToRunFromSnapshot);

		SetRunState(oldRunState);
	} else {
		// Run forward, by setting a step breakpoint.
		GetUI()->ShowDebugMessage("TODO: run forward by setting a step breakpoint!\n");
		return false;
	}

	return true;
}


void GXemul::TakeSnapshot()
{
	// TODO: Multiple snapshots!

	if (m_snapshot.IsNULL()) {
		stringstream ss;
		ss << "(snapshot at step " << GetStep() << ")\n";
		GetUI()->ShowDebugMessage(ss.str());

		m_snapshot = GetRootComponent()->Clone();
	}
}


struct ComponentAndFrequency
{
	refcount_ptr<Component>	component;
	double			frequency;
	StateVariable*		step;

	uint64_t		nextTimeToExecute;
};


// Gathers a list of components and their frequencies. (Only components that
// have a variable named "frequency" are executable.)
static void GetComponentsAndFrequencies(refcount_ptr<Component> component,
	vector<ComponentAndFrequency>& componentsAndFrequencies)
{
	const StateVariable* paused = component->GetVariable("paused");
	const StateVariable* freq = component->GetVariable("frequency");
	StateVariable* step = component->GetVariable("step");
	if (freq != NULL && step != NULL &&
	    (paused == NULL || paused->ToInteger() == 0)) {
		struct ComponentAndFrequency caf;
		memset(&caf, 0, sizeof(caf));

		caf.component = component;
		caf.frequency = freq->ToDouble();
		caf.step      = step;

		componentsAndFrequencies.push_back(caf);
	}
	
	Components children = component->GetChildren();
	for (size_t i=0; i<children.size(); ++i)
		GetComponentsAndFrequencies(children[i], componentsAndFrequencies);
}


void GXemul::Execute(const int longestTotalRun)
{
	vector<ComponentAndFrequency> componentsAndFrequencies;
	GetComponentsAndFrequencies(GetRootComponent(), componentsAndFrequencies);

	if (componentsAndFrequencies.size() == 0) {
		GetUI()->ShowDebugMessage("No executable components"
		    " found in the configuration.\n");
		SetRunState(Paused);
		return;
	}

	// Take an initial snapshot at step 0, if snapshotting is enabled:
	if (m_snapshottingEnabled && GetStep() == 0)
		TakeSnapshot();

	// Find the fastest component:
	double fastestFrequency = componentsAndFrequencies[0].frequency;
	size_t fastestComponentIndex = 0;
	for (size_t i=0; i<componentsAndFrequencies.size(); ++i)
		if (componentsAndFrequencies[i].frequency > fastestFrequency) {
			fastestFrequency = componentsAndFrequencies[i].frequency;
			fastestComponentIndex = i;
		}

	bool printEmptyLineBetweenSteps = false;

	switch (GetRunState()) {

	case SingleStepping:
		if (m_nrOfSingleStepsLeft == 0)
			m_nrOfSingleStepsLeft = 1;

		// Note that setting run state to something else, OR
		// decreasing nr of single steps left to 0, will break the loop.
		while (!m_interrupting && m_nrOfSingleStepsLeft > 0 && GetRunState() == SingleStepping) {
			uint64_t step = GetStep();

			if (printEmptyLineBetweenSteps)
				GetUI()->ShowDebugMessage("\n");
			else
				printEmptyLineBetweenSteps = true;

			stringstream ss;
			ss << "step " << step << ": ";

			// Indent all debug output with message header "step X: ":
			UI::SetIndentationMessageHelper indentationHelper(GetUI(), ss.str());

			++ step;

			// Component X, using frequency fX, should have executed
			// nstepsX = steps * fX / fastestFrequency  nr of steps.
			for (size_t k=0; k<componentsAndFrequencies.size(); ++k) {
				uint64_t nsteps = (k == fastestComponentIndex ? step
				    : (uint64_t) (step * componentsAndFrequencies[k].frequency / fastestFrequency));

				uint64_t stepsExecutedSoFar = componentsAndFrequencies[k].step->ToInteger();

				if (stepsExecutedSoFar > nsteps) {
					std::cerr << "Internal error: " <<
					    componentsAndFrequencies[k].component->GetVariable("name")->ToString() <<
					    " has executed " << stepsExecutedSoFar << " steps, goal is " << nsteps << ".\n";
					throw std::exception();
				}

				if (stepsExecutedSoFar < nsteps) {
					++ stepsExecutedSoFar;

					const refcount_ptr<Component> lightClone =
					    GetRootComponent()->LightClone();

					// Execute one step...
					int n = componentsAndFrequencies[k].component->Execute(this, 1);
					if (n != 1) {
						GetUI()->ShowDebugMessage("Single-stepping aborted.\n");
						SetRunState(Paused);
						return;
					}
					
					// ... and write back the number of executed steps:
					componentsAndFrequencies[k].step->SetValue(stepsExecutedSoFar);

					// Now, let's compare the clone of the component tree
					// before execution with what we have now.
					stringstream changeMessages;
					GetRootComponent()->DetectChanges(lightClone, changeMessages);
					string msg = changeMessages.str();
					if (msg.length() > 0)
						GetUI()->ShowDebugMessage(msg);
				}
			}

			SetStep(step);
			-- m_nrOfSingleStepsLeft;
		}

		// Done. Let's pause again.
		SetRunState(Paused);
		m_nrOfSingleStepsLeft = 0;
		break;

	case Running:
		{
			uint64_t step = GetStep();
			uint64_t startingStep = step;

			// TODO: sloppy vs cycle accuracy.
			if (GetRootComponent()->GetVariable("accuracy")->ToString() != "cycle") {
				std::cerr << "GXemul::Execute(): TODO: Only "
				    "root.accuracy=\"cycle\" is currently supported\n";
				SetRunState(Paused);
				return;
			}

			// The following code is for cycle accurate emulation:

			while (step < startingStep + longestTotalRun) {
				if (m_interrupting || GetRunState() != Running)
					break;

				int toExecute = -1;
				
				if (componentsAndFrequencies.size() == 1) {
					toExecute = longestTotalRun;
					componentsAndFrequencies[0].nextTimeToExecute = step;
				} else {
					// First, calculate the next time step when each
					// component k will execute.
					//
					// For n = 0,1,2,3, ...
					// n * fastestFrequency / componentsAndFrequencies[k].frequency
					// are the steps at which the component executes
					// (when rounded UP! i.e. executing at step 4.2 means that it
					// did not execute at step 4, but will at step 5).
				
					for (size_t k=0; k<componentsAndFrequencies.size(); ++k) {
						double q = (k == fastestComponentIndex ? 1.0
						    : fastestFrequency / componentsAndFrequencies[k].frequency);

						double c = (componentsAndFrequencies[k].step->ToInteger()+1) * q;
						componentsAndFrequencies[k].nextTimeToExecute = (uint64_t) ceil(c) - 1;
					}

					// std::cerr << "step " << step << " debug:\n";
					for (size_t k=0; k<componentsAndFrequencies.size(); ++k) {
						// std::cerr << "  next step for component " <<
						//    componentsAndFrequencies[k].component->GetVariable("name")->ToString()
						//    << ": " << componentsAndFrequencies[k].nextTimeToExecute << "\n";

						int diff = componentsAndFrequencies[k].nextTimeToExecute -
						    componentsAndFrequencies[fastestComponentIndex].nextTimeToExecute;
						if (k != fastestComponentIndex) {
							if (toExecute == -1 || diff < toExecute)
								toExecute = diff;
						}
					}

					if (toExecute < 1)
						toExecute = 1;
				}

				if (step + toExecute > startingStep + longestTotalRun)
					toExecute = startingStep + longestTotalRun - step;

				// std::cerr << "  toExecute = " << toExecute << "\n";

				// Run the components.
				// If multiple components are to run at the same time (i.e.
				// same nextTimeToExecute), toExecute will be exactly 1.
				int maxExecuted = 0;
				bool abort = false;
				for (size_t k=0; k<componentsAndFrequencies.size(); ++k) {
					if (step != componentsAndFrequencies[k].nextTimeToExecute)
						continue;

					// Execute the calculated number of steps...
					int n = componentsAndFrequencies[k].component->Execute(this, toExecute);

					// ... and write back the number of executed steps:
					uint64_t stepsExecutedSoFar = n +
					    componentsAndFrequencies[k].step->ToInteger();
					componentsAndFrequencies[k].step->SetValue(stepsExecutedSoFar);

					if (k == fastestComponentIndex)
						maxExecuted = n;

					if (n != toExecute) {
						abort = true;

						if (n > toExecute) {
							std::cerr << "Internal error: " << n <<
							    " steps executed, toExecute = " << toExecute << "\n";
							throw std::exception();
						}

						stringstream ss;
						ss << "only " << n << " steps of " << toExecute << " executed.";
						GetUI()->ShowDebugMessage(componentsAndFrequencies[k].component, ss.str());
					}
				}

				if (abort) {
					GetUI()->ShowDebugMessage("Continuous execution aborted.\n");
					SetRunState(Paused);
				}

				if (maxExecuted == 0 && GetRunState() == Running) {
					std::cerr << "maxExecuted=0. internal error\n";
					throw std::exception();
				}

				step += maxExecuted;
				SetStep(step);
			}

			// Output nr of steps (and speed) every second:
			struct timeval tvend;
			gettimeofday(&tvend, NULL);

			double secondsSinceLastOutput =
			    ((double)tvend.tv_sec + tvend.tv_usec / 1000000.0)
			    - ((double)m_lastOutputTime.tv_sec + m_lastOutputTime.tv_usec / 1000000.0);

			if (secondsSinceLastOutput > 1.0 && (step - m_lastOutputStep) > 10000) {
				m_lastOutputTime = tvend;

				int64_t stepsPerSecond = (int64_t)
				    ( (double)(step - m_lastOutputStep) / secondsSinceLastOutput );
				m_lastOutputStep = step;

				stringstream ss;
				ss << step << " steps";
				if (stepsPerSecond > 0)
					ss << " (" << stepsPerSecond << " steps/second)";

				GetUI()->ShowDebugMessage(ss.str());
			}
		}
		break;

	default:
		std::cerr << "GXemul::Execute() called without being in a"
		    " running state. Internal error?\n";
		throw std::exception();
	}

	if (m_interrupting) {
		m_interrupting = false;
		SetRunState(Paused);
	}
}


/*****************************************************************************/

#ifdef WITHUNITTESTS

static void Test_Construction()
{
	GXemul gxemul;
}

UNITTESTS(GXemul)
{
	UNITTEST(Test_Construction);

	// Note: Most execution tests are in DummyComponent.cc, because they
	// test component behavior. But they also test GXemul::Execute etc.
}


#endif

