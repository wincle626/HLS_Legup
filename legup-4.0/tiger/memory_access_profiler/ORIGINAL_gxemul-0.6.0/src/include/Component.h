#ifndef COMPONENT_H
#define	COMPONENT_H

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

#include "Checksum.h"
#include "SerializationContext.h"
#include "StateVariable.h"


class AddressDataBus;
class Component;
class CPUComponent;
class GXemul;
class RootComponent;
class UI;

typedef vector< refcount_ptr<Component> > Components;
typedef map< string,string > ComponentCreationSettings;

struct ComponentCreateArgs
{
	GXemul*				gxemul;
	ComponentCreationSettings	componentCreationSettings;
};


/**
 * \brief A %Component is a node in the configuration tree that
 *	makes up an emulation setup.
 *
 * The %Component is the core concept in %GXemul. All devices,
 * CPUs, networks, and so on are components.
 */
class Component
	: public ReferenceCountable
{
protected:
	/**
	 * \brief Base constructor for a %Component.
	 *
	 * See also: the Create() function.
	 *
	 * @param className	The name of the component class.
	 *			It should be a short, descriptive name.
	 *			For e.g. a PCI bus class, it can be "pcibus".
	 *			For a MIPS CPU, it can be "mips_cpu".
	 * @param visibleClassName	The visible name of the component class.
	 *			It should be a short, descriptive name.
	 *			For e.g. a PCI bus class, it can be "pcibus".
	 *			For a MIPS CPU, it can be "cpu".
	 */
	Component(const string& className, const string& visibleClassName);

public:
	virtual ~Component() = 0;

	/**
	 * \brief Gets the class name of the component.
	 *
	 * @return the class name of the component, e.g. "pcibus" for a PCI
	 *	bus component class, or "mips_cpu" for a MIPS CPU.
	 */
	string GetClassName() const;

	/**
	 * \brief Gets the visible class name of the component.
	 *
	 * @return the class name of the component, e.g. "pcibus" for a PCI
	 *	bus component class, or "cpu" for a MIPS CPU.
	 */
	string GetVisibleClassName() const;

	/**
	 * \brief Creates a Component.
	 *
	 * The reason for having this helper, instead of simply calling the
	 * constructor of a specific %Component, is to make templates work
	 * in a reasonable and straight-forward manner. (Note: This concept
	 * of templates is unrelated to the C++ concept of templates. It just
	 * happens to be the same word.)
	 *
	 * E.g. DummyComponent::Create() returns a new DummyComponent, but
	 * TestMIPSMachine::Create() returns a new MachineComponent
	 * (with some pre-defined child components), not a TestMIPS component.
	 */
//	static refcount_ptr<Component> Create(const ComponentCreateArgs& args);

	/**
	 * \brief Get attribute information about a Component.
	 *
	 * @param attributeName The attribute name.
	 * @return A string representing the attribute value. The base
	 *	implementation returns an empty string; it is up to individual
	 *	Components to return other values.
	 */
	static string GetAttribute(const string& attributeName);

	/**
	 * \brief Clones the component and all its children.
	 *
	 * The new copy is a complete copy; modifying either the copy or the
	 * original will not affect the other.
	 *
	 * @return A reference counted pointer to the clone.
	 */
	refcount_ptr<Component> Clone() const;

	/**
	 * \brief Makes a light clone of the component and all its children.
	 *
	 * Modifying the original component will not affect the component
	 * returned by this function.
	 *
	 * The new copy is <i>not</i> a complete copy. Expensive data
	 * structures are not copied, only "light" data structures. For
	 * example, RAM memory cell contents is not cloned.
	 *
	 * The main purpose of this function is to allow e.g. a single
	 * stepper to take a light clone, run one step, and the compare
	 * the current component tree with the light clone, to see what
	 * was modified in the step.
	 *
	 * @return A const reference counted pointer to the light clone.
	 */
	const refcount_ptr<Component> LightClone() const;

	/**
	 * \brief Compare an older clone to the current tree, to find changes.
	 *
	 * Any changes found are written as messages to changeMessages.
	 * Typical use of this function is to input a "light clone" which
	 * is taken before running a single execution step, to find out
	 * any side effects of that execution step.
	 *
	 * @param oldClone The original component tree to compare to.
	 * @param changeMessages An output stream where to send messages.
	 */
	void DetectChanges(const refcount_ptr<Component>& oldClone,
		ostream& changeMessages) const;

	/**
	 * \brief Generates an ASCII tree dump of a component tree.
	 *
	 * @param branchTemplate Used for recursion. Start with an empty
	 *	string ("").
	 * @param htmlLinksForClassNames Used to generate HTML links
	 *	by 'make documentation'.
	 * @param prefixForComponentUrls Placed before
	 *	component/xyz.html in html links to components.
	 * @return An ASCII string containing the tree.
	 */
	string GenerateTreeDump(const string& branchTemplate,
		bool htmlLinksForClassNames = false,
		string prefixForComponentUrls = "") const;

	/**
	 * \brief Resets the state of this component and all its children.
	 */
	void Reset();

	/**
	 * \brief Checks the state of this component and all its children,
	 *	before starting execution.
	 *
	 * @return true if the component and all its children are ready to run,
	 *	false otherwise.
	 */
	bool PreRunCheck(GXemul* gxemul);

	/**
	 * \brief Resets the cached state of this component and all its
	 * children.
	 *
	 * For performance reasons, while an emulation is running, it is usually
	 * a good idea to cache e.g. pointers to various things. However,
	 * after pausing an emulation, and continuing again, these pointers
	 * may not be valid. FlushCachedState() should be called before
	 * Run() is called.
	 */
	void FlushCachedState();

	/**
	 * \brief Execute one or more cycles.
	 *
	 * Note 1: A component must attempt to execute exactly the number of
	 * specified cycles.
	 *
	 * Note 2: If e.g. a breakpoint is reached, or some fatal error occurs
	 * (such as an unimplemented feature in the emulation of the component
	 * is triggered), then it is ok to return fewer than the requested
	 * number of cycles. Execution will then stop (or the breakpoint will
	 * be handled, etc).
	 *
	 * Note 3: The framework will only call this function if the component
	 * has a StateVariable named "frequency". Components that do not have
	 * any frequency do not execute anything periodically by themselves.
	 *
	 * @param gxemul A pointer to the GXemul instance.
	 * @param nrOfCycles	The requested number of cycles to run.
	 * @return	The number of cycles actually executed.
	 */
	virtual int Execute(GXemul* gxemul, int nrOfCycles);

	/**
	 * \brief Returns the current frequency (in Hz) that the component
	 *	runs at.
	 *
	 * @return	The component's frequency in Hz.
	 */
	virtual double GetCurrentFrequency() const;

	/**
	 * \brief Returns the component's RootComponent interface.
	 *
	 * @return A pointer to the component as a %RootComponent, or
	 * NULL if the component isn't a %RootComponent.
	 */
	virtual RootComponent* AsRootComponent();

	/**
	 * \brief Returns the component's CPUComponent interface.
	 *
	 * @return A pointer to the component as a %CPUComponent, or
	 * NULL if the component isn't a CPU.
	 */
	virtual CPUComponent* AsCPUComponent();

	/**
	 * \brief Returns the component's AddressDataBus interface, if any.
	 *
	 * @return	A pointer to an AddressDataBus, or NULL if the
	 *	component does not support that interface.
	 */
	virtual AddressDataBus* AsAddressDataBus();

	/**
	 * \brief Sets the parent component of this component.
	 *
	 * Note: Not reference counted.
	 *
	 * @param parentComponent	a pointer to the parent component.
	 */
	void SetParent(Component* parentComponent);

	/**
	 * \brief Gets this component's parent component, if any.
	 *
	 * Note: Not reference counted.
	 *
	 * Returns NULL if there is no parent component.
	 *
	 * @return the pointer to the parent component, or NULL
	 */
	Component* GetParent();
	const Component* GetParent() const;

	/**
	 * \brief Retrieves a component's implemented method names.
	 *
	 * Note that a component's implementation should call its
	 * base class' GetMethodNames(vector<string>&) too. However, when
	 * methods are executed, the most specific implementation (i.e.
	 * not the base class) will get a chance first to execute the method.
	 *
	 * @param names A vector of strings, where method names
	 *	should be added.
	 */
	virtual void GetMethodNames(vector<string>& names) const;

	/**
	 * \brief Returns whether a method name may be re-executed without args.
	 *
	 * Typical examples may be a RAMComponent which has a "dump" method,
	 * or a CPUComponent which has "dump" and "unassemble" methods.
	 *
	 * Note that a component's implementation should call its
	 * base class' MethodMayBeReexecutedWithoutArgs(cosnt string&) too.
	 *
	 * @param methodName The name of the method.
	 * @return true if the method may be re-executed without arguments,
	 *	false otherwise.
	 */
	virtual bool MethodMayBeReexecutedWithoutArgs(const string& methodName) const;

	/**
	 * \brief Executes a method on the component.
	 *
	 * Note 1: The method name must be one of those returned
	 *	by GetMethodNames(vector<string>&), either in the class itself
	 *	or by one of the base implementations.
	 *
	 * Note 2: The base class' member function should <i>only</i> be called
	 *	if this class does not handle the method.
	 *
	 * @param gxemul A reference to the GXemul instance.
	 * @param methodName The name of the method.
	 * @param arguments A vector of arguments to the method.
	 */
	virtual void ExecuteMethod(GXemul* gxemul,
		const string& methodName,
		const vector<string>& arguments);

	/**
	 * \brief Generates a string representation of the path to the
	 *	%Component.
	 *
	 * The path consists of "name.name...name.name" where the last name
	 * is the name of this component, and the preceding names are names
	 * of parents.
	 *
	 * If a component does not have a "name" state variable, then the
	 * class name in parentheses is used instead (which may make the path
	 * non-unique).
	 *
	 * @return A path to the component, as a string.
	 */
	string GeneratePath() const;

	/**
	 * \brief Generates a short string representation of the path to the
	 *	%Component.
	 *
	 * This function generates a string which is the shortest possible
	 * sub-path required to uniqely identify a component in the tree.
	 * (Actually, not really the shortest. For example if a component is
	 * called root.machine0.mainbus0.ram0, then the shortest string may
	 * be "ra". This function will return "ram0".)
	 *
	 * If there are components a.b.x, a.b.y, and a.c.x, then the shortest
	 * possible paths for the three components are "b.x", "y", and "c.x"
	 * respectively.
	 *
	 * @return A string which is part of the path of the component.
	 */
	string GenerateShortestPossiblePath() const;

	/**
	 * \brief Looks up a path from this %Component,
	 *	and returns a pointer to the found %Component, if any
	 *
	 * The first name in the path should match the name of this component.
	 * If so, this function moves on to the next part of the path (if there
	 * are more parts) and looks up a child with that name, and so on.
	 *
	 * Alternatively, the path may be a partial match.
	 *
	 * @param path The path to the component, consisting of names with
	 *	"." (period) between names.
	 * @return A reference counted pointer to the looked up component,
	 *	which is set to NULL if the path was not found.
	 */
	const refcount_ptr<Component> LookupPath(string path) const;

	/**
	 * \brief Finds complete component paths, given a partial path.
	 *
	 * E.g. if the following components are available:<pre>
	 *	root.machine0.isabus0
	 *	root.machine1.pcibus0
	 *	root.machine1.pcibus1
	 *	root.machine2.pcibus0
 	 *	root.machine3.otherpci
	 * </pre>
	 *
	 * then a search for "pci" would return<pre>
	 *	root.machine1.pcibus0
	 *	root.machine1.pcibus1
	 *	root.machine2.pcibus0
	 * </pre>
	 * (note: not otherpci)
	 *
	 * A search for machine1 would return<pre>
	 *	root.machine1
	 * </pre>
	 *
	 * Multiple words separated by "." should also be possible, so
	 * "machine2.pcibus" will find "root.machine2.pcibus0", but
	 * "machine.pcibus" will <i>not</i> match anything.
	 *
	 * Searching for an empty string ("") returns a vector of <i>all</i>
	 * component paths.
	 *
	 * @param partialPath The partial path to complete.
	 * @param shortestPossible Return shortest possible paths, instead of
	 *	full paths.
	 * @return A vector of possible complete paths.
	 */
	vector<string> FindPathByPartialMatch(const string& partialPath,
		bool shortestPossible = false) const;

	/**
	 * \brief Adds a reference to a child component.
	 *
	 * @param childComponent  A reference counted pointer to the child
	 *	component to add.
	 * @param insertPosition  If specified, this is the position in the
	 *	vector of child components where the child will be inserted.
	 *	If not specified (or -1), then the child will be added to
	 *	the end of the vector.
	 */
	void AddChild(refcount_ptr<Component> childComponent,
		size_t insertPosition = (size_t) -1);

	/**
	 * \brief Removes a reference to a child component.
	 *
	 * @param childToRemove  A pointer to the child to remove.
	 * @return  A zero-based index, which is the position in the vector
	 *	of children where the child was. (Needed for e.g. Undo
	 *	functionality.)
	 */
	size_t RemoveChild(Component* childToRemove);

	/**
	 * \brief Gets pointers to child components.
	 *
	 * @return Reference counted pointers to child components.
	 */
	Components& GetChildren();

	/**
	 * \brief Gets pointers to child components, as a const reference.
	 *
	 * @return Reference counted pointers to child components.
	 */
	const Components& GetChildren() const;

	/**
	 * \brief Retrieves a component's state variable names.
	 *
	 * @param names A vector of strings, where method names
	 *	should be added.
	 */
	void GetVariableNames(vector<string>& names) const;

	/**
	 * \brief Gets a pointer to a state variable.
	 *
	 * NOTE: The returned pointer should be used immediately after the
	 *	call. It will in general not be valid after e.g. an
	 *	AddVariable call.
	 *
	 * @param name The variable name.
	 * @return A pointer to the variable, it the name was
	 *	known; NULL otherwise.
	 */
	StateVariable* GetVariable(const string& name);

	/**
	 * \brief Gets a pointer to a state variable.
	 *
	 * NOTE: The returned pointer should be used immediately after the
	 *	call. It will in general not be valid after e.g. an
	 *	AddVariable call.
	 *
	 * @param name The variable name.
	 * @return A pointer to the variable, it the name was
	 *	known; NULL otherwise.
	 */
	const StateVariable* GetVariable(const string& name) const;

	/**
	 * \brief Sets a variable to a new value.
	 *
	 * @param name The variable name.
	 * @param expression The new value.
	 * @return True if the value was set, false otherwise. (E.g. if
	 *	the name was not known, or if there was a parse error
	 *	when parsing the value expression.)
	 */
	bool SetVariableValue(const string& name, const string& expression);

	/**
	 * \brief Serializes the %Component into a string stream.
	 *
	 * @param ss An ostream which the %Component will be serialized to.
	 * @param context A serialization context (used for TAB indentation).
	 */
	void Serialize(ostream& ss, SerializationContext& context) const;

	/**
	 * \brief Deserializes a string into a component tree.
	 *
	 * @param messages A stream where errors/warnings may be reported.
	 * @param str The string to deserialize.
	 * @param pos Initial deserialization position in the string; should
	 *	be 0 when invoked manually. (Used for recursion, to avoid
	 *	copying.)
	 * @return If deserialization was successful, the
	 *	reference counted pointer will point to a component tree;
	 *	on error, it will be set to NULL
	 */
	static refcount_ptr<Component> Deserialize(ostream& messages,
	    const string& str, size_t& pos);

	/**
	 * \brief Checks consistency by serializing and deserializing the
	 *	component (including all its child components), and comparing
	 *	the checksum of the original tree with the deserialized tree.
	 *
	 * @return true if the serialization/deserialization was correct,
	 *	false if there was some inconsistency
	 */
	bool CheckConsistency() const;

	/**
	 * \brief Adds this component's state, including children, to
	 *	a checksum.
	 *
	 * @param checksum The checksum to add to.
	 */
	void AddChecksum(Checksum& checksum) const;

protected:
	/**
	 * \brief Adds a state variable of type T to the %Component.
	 *
	 * This function is only meant to be called from the component's
	 * constructor.
	 *
	 * @param name The variable name.
	 * @param variablePointer A pointer to the variable that the name should
	 *	be connected to.
	 * @return True if the state variable was added, false if the name
	 *	was already in use.
	 */
	template<class T>
	bool AddVariable(const string& name, T* variablePointer)
	{
		StateVariableMap::iterator it = m_stateVariables.find(name);
		if (it != m_stateVariables.end())
			return false;

		m_stateVariables[name] = StateVariable(name, variablePointer);
		return true;
	}

	/**
	 * \brief Adds a custom state variable to the %Component.
	 *
	 * This function is only meant to be called from the component's
	 * constructor.
	 *
	 * @param name The variable name.
	 * @param variableHandler A pointer to the handler that knows how
	 *	to serialize/deserialize the variable.
	 * @return True if the state variable was added, false if the name
	 *	was already in use.
	 */
	bool AddCustomVariable(const string& name, CustomStateVariableHandler* variableHandler)
	{
		StateVariableMap::iterator it = m_stateVariables.find(name);
		if (it != m_stateVariables.end())
			return false;

		m_stateVariables[name] = StateVariable(name, variableHandler);
		return true;
	}

	/**
	 * \brief Checks whether a write to a variable is OK.
	 *
	 * This function is called <i>after</i> the variable has been written.
	 * By returning false, the component indicates that the value which
	 * was written is invalid, and the write will be undone.
	 *
	 * An implementation should first check variables defined for the
	 * implementation class, and then call its base class' function.
	 *
	 * The implementation in the base Component class handles the variables
	 * that are defined for all components, and returns true for anything
	 * else.
	 *
	 * @param var The variable to check.
	 * @param oldValue The serialized previous value.
	 * @return true if the write was ok, false otherwise.
	 */
	virtual bool CheckVariableWrite(StateVariable& var, const string& oldValue);

	/**
	 * \brief Resets the state variables of this component.
	 *
	 * Note 1: This function is not recursive, so children should not be
	 * traversed.
	 *
	 * Note 2: After a component's state variables have been reset, the
	 * base class' ResetState() function should also be called.
	 *
	 * The implementation of this function ususally takes the form of a
	 * number of assignment of values to member variables, and then
	 * the call to the base class' ResetState() function.
	 */
	virtual void ResetState();

	/**
	 * \brief Checks the state of this component, before starting execution.
	 *
	 * Note 1: This function is not recursive, so children should not be
	 * traversed.
	 *
	 * Note 2: After a component's state variables have been checked, the
	 * base class' PreRunCheckForComponent(GXemul*) function should also be
	 * called.
	 *
	 * The implementation of this function may choose to print warning
	 * messages but still return true, or it can print error messages and
	 * return false.
	 *
	 * Typical examples of pre-run-check failures are:
	 * <ul>
	 *	<li>collision of MemoryMappedComponent on an AddressDataBus
	 *	<li>a CPUComponent that can not reach any kind of AddressDataBus
	 * </ul>
	 *
	 * @return true if the component is ready to run, false otherwise.
	 */
	virtual bool PreRunCheckForComponent(GXemul* gxemul);

	/**
	 * \brief Resets the cached state of this component.
	 *
	 * Note 1: This function is not recursive, so children should not be
	 * traversed.
	 *
	 * Note 2: After a component's state variables have been reset, the
	 * base class' FlushCachedStateForComponent() function should also be
	 * called.
	 *
	 * The implementation of this function ususally takes the form of
	 * setting a number of pointers to NULL, and then
	 * the call to the base class' FlushCachedStateForComponent() function.
	 */
	virtual void FlushCachedStateForComponent();

	/**
	 * \brief Returns a reference to the current GXemul instance.
	 *
	 * @return NULL if there was no currently running instance, otherwise
	 * a pointer to the currently running GXemul instance.
	 */
	GXemul* GetRunningGXemulInstance();

	/**
	 * \brief Gets an UI reference for outputting debug messages during
	 * runtime.
	 *
	 * @return NULL if debug messages are turned off, or if there is no
	 * owning GXemul instance. Otherwise, a pointer to an UI.
	 */
	 UI* GetUI();

private:
	/**
	 * \brief Looks up a path from this %Component,
	 *	and returns a pointer to the found %Component, if any
	 *
	 * The first name in the path should match the name of this component.
	 * If so, this function moves on to the next part of the path (if there
	 * are more parts) and looks up a child with that name, and so on.
	 *
	 * @param path The path to the component, split into separate names.
	 *	This vector should contain at least 1 element.
	 * @param index The index into the path vector, where the current
	 *	lookup should start. (This is to avoid copying.)
	 * @return A reference counted pointer to the looked up component,
	 *	which is set to NULL if the path was not found.
	 */
	const refcount_ptr<Component> LookupPath(const vector<string>& path,
		size_t index) const;

	/**
	 * \brief Internal helper, which gathers a list of all component paths.
	 *
	 * @param allComponentPaths A vector which will be filled with all
	 *	components' paths.
	 */
	void AddAllComponentPaths(vector<string>& allComponentPaths) const;

	/**
	 * \brief Internal helper for LightClone.
	 *
	 * See LightClone() for details.
	 */
	refcount_ptr<Component> LightCloneInternal() const;

	/**
	 * \brief Disallow creation of %Component objects using the
	 * default constructor.
	 */
	Component();

private:
	Component*		m_parentComponent;
	Components		m_childComponents;
	string			m_className;
	string			m_visibleClassName;
	StateVariableMap	m_stateVariables;

	// The following are this component's variables. Components that
	// inherit from this class add their own instance variables.
	string		m_name;		// This Component's instance name.
	string		m_template;	// Set if this Component
					// was based on a template.
	uint64_t	m_step;		// Nr of executed steps.
};


#endif	// COMPONENT_H
