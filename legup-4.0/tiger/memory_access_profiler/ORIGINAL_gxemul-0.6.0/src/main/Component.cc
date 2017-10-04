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
 *
 *
 *  Note: See DummyComponent.cc for unit tests of the component framework.
 */

#include "assert.h"
#include <fstream>

#include "components/RootComponent.h"
#include "Component.h"
#include "ComponentFactory.h"
#include "EscapedString.h"
#include "GXemul.h"


/*****************************************************************************/


// This is basically strtoull(), but it needs to be explicitly implemented
// since some systems lack it. (Also, compiling with GNU C++ in ANSI mode
// does not work with strtoull.)
static uint64_t parse_number(const char* str, bool& error)
{
	int base = 10;
	uint64_t result = 0;
	bool negative = false;

	error = false;

	if (str == NULL)
		return 0;

	while (*str == ' ')
		++str;

	if (*str == '-') {
		negative = true;
		++str;
	}

	while ((*str == 'x' || *str == 'X') || (*str >= '0' && *str <= '9')
	    || (*str >= 'a' && *str <= 'f') || (*str >= 'A' && *str <= 'F')) {
		if (*str == 'x' || *str == 'X') {
			base = 16;
		} else {
			int n = *str - '0';
			if (*str >= 'a' && *str <= 'f')
				n = *str - 'a' + 10;
			if (*str >= 'A' && *str <= 'F')
				n = *str - 'A' + 10;
			result = result * base + n;
		}
		++str;
	}

	if (*str)
		error = true;

	if (negative)
		return -result;
	else
		return result;
}


/*****************************************************************************/


Component::Component(const string& className, const string& visibleClassName)
	: m_parentComponent(NULL)
	, m_className(className)
	, m_visibleClassName(visibleClassName)
	, m_step(0)
{
	AddVariable("name", &m_name);
	AddVariable("template", &m_template);
	AddVariable("step", &m_step);
}


Component::~Component()
{
}


string Component::GetClassName() const
{
	return m_className;
}


string Component::GetVisibleClassName() const
{
	return m_visibleClassName;
}


string Component::GetAttribute(const string& attributeName)
{
	// The Component base class always returns an empty string
	// for all attribute names. It is up to individual Component
	// implementations to return overrides.

	return "";
}


refcount_ptr<Component> Component::Clone() const
{
	refcount_ptr<Component> clone;

	if (GetClassName() == "root")
		clone = new RootComponent;
	else
		clone = ComponentFactory::CreateComponent(GetClassName());

	if (clone.IsNULL()) {
		std::cerr << "INTERNAL ERROR in Component::Clone(): "
		    "could not clone a '" << GetClassName() << "'\n";
		throw std::exception();
	}

	// Copy the value of each state variable to the clone:
	StateVariableMap::const_iterator varIt = m_stateVariables.begin();
	for ( ; varIt != m_stateVariables.end(); ++varIt) {
		const string& varName = varIt->first;
		const StateVariable& variable = varIt->second;

		StateVariableMap::iterator cloneVarIt =
		    clone->m_stateVariables.find(varName);

		if (cloneVarIt == clone->m_stateVariables.end()) {
			std::cerr << "INTERNAL ERROR in Component::Clone(): "
			    "could not copy variable " << varName << "'s "
			    << " value to the clone: clone is missing"
			    " this variable?\n";
			assert(false);
		} else if (!(cloneVarIt->second).CopyValueFrom(variable)) {
			std::cerr << "INTERNAL ERROR in Component::Clone(): "
			    "could not copy variable " << varName << "'s "
			    << " value to the clone.\n";
			assert(false);
		}
	}

	for (Components::const_iterator it = m_childComponents.begin();
	    it != m_childComponents.end(); ++it) {
		refcount_ptr<Component> child = (*it)->Clone();
		if (child.IsNULL()) {
			std::cerr << "INTERNAL ERROR in Component::Clone(): "
			    "could not clone child of class '" <<
			    (*it)->GetClassName() << "'\n";
			assert(false);
		} else {
			clone->AddChild(child);
		}
	}

	return clone;
}


refcount_ptr<Component> Component::LightCloneInternal() const
{
	refcount_ptr<Component> clone;

	if (GetClassName() == "root")
		clone = new RootComponent;
	else
		clone = ComponentFactory::CreateComponent(GetClassName());

	if (clone.IsNULL()) {
		std::cerr << "INTERNAL ERROR in Component::LightCloneInternal(): "
		    "could not clone a '" << GetClassName() << "'\n";
		throw std::exception();
	}

	// Copy the value of each state variable to the clone:
	StateVariableMap::const_iterator varIt = m_stateVariables.begin();
	for ( ; varIt != m_stateVariables.end(); ++varIt) {
		const string& varName = varIt->first;
		const StateVariable& variable = varIt->second;

		StateVariableMap::iterator cloneVarIt =
		    clone->m_stateVariables.find(varName);

		if (cloneVarIt == clone->m_stateVariables.end()) {
			std::cerr << "INTERNAL ERROR in Component::LightCloneInternal(): "
			    "could not copy variable " << varName << "'s "
			    << " value to the clone: clone is missing"
			    " this variable?\n";
			throw std::exception();
		}

		// Skip custom variables:
		if (variable.GetType() == StateVariable::Custom)
			continue;

		if (!(cloneVarIt->second).CopyValueFrom(variable)) {
			std::cerr << "INTERNAL ERROR in Component::LightCloneInternal(): "
			    "could not copy variable " << varName << "'s "
			    << " value to the clone.\n";
			throw std::exception();
		}
	}

	for (Components::const_iterator it = m_childComponents.begin();
	    it != m_childComponents.end(); ++it) {
		refcount_ptr<Component> child = (*it)->LightCloneInternal();
		if (child.IsNULL()) {
			std::cerr << "INTERNAL ERROR in Component::LightCloneInternal(): "
			    "could not clone child of class '" <<
			    (*it)->GetClassName() << "'\n";
			throw std::exception();
		} else {
			clone->AddChild(child);
		}
	}

	return clone;
}


const refcount_ptr<Component> Component::LightClone() const
{
	return LightCloneInternal();
}


void Component::DetectChanges(const refcount_ptr<Component>& oldClone,
	ostream& changeMessages) const
{
	// Recursively look for changes in state. Compare the
	// oldClone component with this component first.

	if (oldClone->GetClassName() != GetClassName()) {
		changeMessages << oldClone->GenerateShortestPossiblePath() <<
		    " changed class from " << oldClone->GetClassName() <<
		    " to " << GetClassName() << "?\n";
		return;
	}

	// Compare all state variables:
	StateVariableMap::const_iterator varIt = m_stateVariables.begin();
	for ( ; varIt != m_stateVariables.end(); ++varIt) {
		const string& varName = varIt->first;

		// Don't output "step" changes, because they happen all
		// the time for all executable components.
		if (varName == "step")
			continue;

		const StateVariable& variable = varIt->second;
		const StateVariable* oldVariable = oldClone->GetVariable(varName);

		// NOTE: Custom types have a nonsense tostring, which
		// usually just says "(custom)" or something, so they
		// are quickly compared.

		// TODO: In the future, maybe RAM components and such (i.e.
		// custom data types) can be lazily reference counted or
		// similar, to detect changes in RAM as well.

		string var = variable.ToString();
		string varOld = oldVariable->ToString();

		if (var != varOld)
			changeMessages << "=> " << GenerateShortestPossiblePath() << "."
			    << varName << ": " << varOld << " -> " << var << "\n";
	}

	// Compare all children.
	const Components& oldChildren = oldClone->GetChildren();
	for (size_t i = 0; i < m_childComponents.size(); ++ i) {
		string myName = m_childComponents[i]->GetVariable("name")->ToString();

		// Find the corresponding child component in the oldClone:
		size_t j;
		bool found = false;
		for (j=0; j<oldChildren.size(); ++j)
			if (oldChildren[j]->GetVariable("name")->ToString() == myName) {
				m_childComponents[i]->DetectChanges(oldChildren[j], changeMessages);
				found = true;
				break;
			}

		if (!found)
			changeMessages << m_childComponents[i]->
			    GenerateShortestPossiblePath() << " (appeared)\n";
	}

	// ... and see if any disappeared (i.e. were in the old clone,
	// but not in the current tree):
	for (size_t j=0; j<oldChildren.size(); ++j) {
		string oldName = oldChildren[j]->GetVariable("name")->ToString();

		bool found = false;
		for (size_t k=0; k<m_childComponents.size(); ++k) {
			string newName = m_childComponents[k]->GetVariable("name")->ToString();
			if (newName == oldName) {
				found = true;
				break;
			}
		}

		if (!found)
			changeMessages << oldChildren[j]->
			    GenerateShortestPossiblePath() << " (disappeared)\n";
	}
}


void Component::Reset()
{
	ResetState();

	// Recurse:
	for (size_t i = 0; i < m_childComponents.size(); ++ i)
		m_childComponents[i]->Reset();
}


void Component::ResetState()
{
	// Base implementation.
	m_step = 0;
}


bool Component::PreRunCheck(GXemul* gxemul)
{
	bool everythingOk = PreRunCheckForComponent(gxemul);

	// Recurse:
	for (size_t i = 0; i < m_childComponents.size(); ++ i)
		everythingOk &= m_childComponents[i]->PreRunCheck(gxemul);

	return everythingOk;
}


bool Component::PreRunCheckForComponent(GXemul* gxemul)
{
	// Base implementation: Do nothing. Everything is Ok.
	return true;
}


void Component::FlushCachedState()
{
	FlushCachedStateForComponent();

	// Recurse:
	for (size_t i = 0; i < m_childComponents.size(); ++ i)
		m_childComponents[i]->FlushCachedState();
}


void Component::FlushCachedStateForComponent()
{
	// Base implementation: Do nothing.
}


int Component::Execute(GXemul* gxemul, int nrOfCycles)
{
	// Base implementation: Do nothing, but pretend we executed
	// the instructions. Actual components that inherit from this
	// class should of course _do_ something here.

	return nrOfCycles;
}


double Component::GetCurrentFrequency() const
{
	// The base component does not run at any frequency. Only components
	// that actually run something "per cycle" should return values
	// greater than 0.0.

	return 0.0;
}


RootComponent* Component::AsRootComponent()
{
	// Default implementation (the base Component class) is not a RootComponent.
	return NULL;
}


CPUComponent* Component::AsCPUComponent()
{
	// Default implementation (the base Component class) is not a CPU.
	return NULL;
}


AddressDataBus* Component::AsAddressDataBus()
{
	// Default implementation (the base Component class) is not an
	// address data bus.
	return NULL;
}


void Component::SetParent(Component* parentComponent)
{
	m_parentComponent = parentComponent;
}


Component* Component::GetParent()
{
	return m_parentComponent;
}


const Component* Component::GetParent() const
{
	return m_parentComponent;
}


void Component::GetMethodNames(vector<string>& names) const
{
	// The default component has no implemented methods.
}


bool Component::MethodMayBeReexecutedWithoutArgs(const string& methodName) const
{
	// By default, methods are _not_ re-executable without args.
	return false;
}


void Component::ExecuteMethod(GXemul* gxemul,
	const string& methodName,
	const vector<string>& arguments)
{
	std::cerr << "Internal error: someone tried to execute "
	    "method '" << methodName << "' on the Component base"
	    " class. Perhaps you are missing an override?\n";
	throw std::exception();
}


string Component::GenerateTreeDump(const string& branchTemplate,
	bool htmlLinksForClassNames, string prefixForComponentUrls) const
{
	// Basically, this generates a string which looks like:
	//
	//	root
	//	|-- child1
	//	|   |-- child1's child1
	//	|   \-- child1's child2
	//	\-- child2
	//	    \-- child2's child
	//
	// TODO: Comment this better.

	string branch;
	for (size_t pos=0; pos<branchTemplate.length(); pos++) {
		stringchar ch = branchTemplate[pos];
		if (ch == '\\') {
			if (pos < branchTemplate.length() - 4)
				branch += ' ';
			else
				branch += ch;
		} else {
			if (pos == branchTemplate.length() - 3 ||
			    pos == branchTemplate.length() - 2)
				ch = '-';
			branch += ch;
		}
	}

	// Fallback to showing the component's class name in parentheses...
	const string className = GetClassName();
	string name = "(unnamed " + className + ")";

	// ... but prefer the state variable "name" if it is non-empty:
	const StateVariable* value = GetVariable("name");
	if (!value->ToString().empty())
		name = value->ToString();

	string str = branch;

	if (htmlLinksForClassNames) {
		// See if this class name has its own HTML page.
		std::ifstream documentationComponentFile((
		    "doc/components/component_"
		    + className + ".html").c_str());

		if (documentationComponentFile.is_open())
			str += "<a href=\"" + prefixForComponentUrls +
			    "components/component_" +
			    className + ".html\">" + name + "</a>";
		else
			str += name;
	} else {
		str += name;
	}

	// If this component was created by a template, then show the template
	// type in [ ].
	const StateVariable* templateName;
	if ((templateName = GetVariable("template")) != NULL &&
	    !templateName->ToString().empty())
		str += "  [" + templateName->ToString() + "]";

	stringstream ss;

	// If this component has a "model", then show that.
	const StateVariable* model = GetVariable("model");
	if (model != NULL && !model->ToString().empty()) {
		if (!ss.str().empty())
			ss << ", ";
		ss << model->ToString();
	}

	// If this component has a frequency (i.e. it is runnable), then
	// show the frequency:
	double freq = GetCurrentFrequency();
	if (freq != 0.0) {
		if (!ss.str().empty())
			ss << ", ";

		if (freq >= 1e9)
			ss << freq/1e9 << " GHz";
		else if (freq >= 1e6)
			ss << freq/1e6 << " MHz";
		else if (freq >= 1e3)
			ss << freq/1e3 << " kHz";
		else
			ss << freq << " Hz";
	}

	const StateVariable* paused = GetVariable("paused");
	// TODO: ToBool :)
	if (paused != NULL && paused->ToInteger() > 0) {
		if (!ss.str().empty())
			ss << ", ";

		ss << "paused";
	}

	const StateVariable* memoryMappedBase = GetVariable("memoryMappedBase");
	const StateVariable* memoryMappedSize = GetVariable("memoryMappedSize");
	const StateVariable* memoryMappedAddrMul =
	    GetVariable("memoryMappedAddrMul");
	if (memoryMappedBase != NULL && memoryMappedSize != NULL) {
		if (!ss.str().empty())
			ss << ", ";

		uint64_t nBytes = memoryMappedSize->ToInteger();
		if (nBytes >= (1 << 30))
			ss << (nBytes >> 30) << " GB";
		else if (nBytes >= (1 << 20))
			ss << (nBytes >> 20) << " MB";
		else if (nBytes >= (1 << 10))
			ss << (nBytes >> 10) << " KB";
		else if (nBytes != 1)
			ss << nBytes << " bytes";
		else
			ss << nBytes << " byte";

		ss << " at offset ";
		ss.flags(std::ios::hex | std::ios::showbase);
		ss << memoryMappedBase->ToInteger();

		if (memoryMappedAddrMul != NULL &&
		    memoryMappedAddrMul->ToInteger() != 1)
			ss << ", addrmul " << memoryMappedAddrMul->ToInteger();
	}

	if (!ss.str().empty())
		str += "  (" + ss.str() + ")";

	// Show the branch of the tree...
	string result = "  " + str + "\n";

	// ... and recurse to show children, if there are any:
	const Components& children = GetChildren();
	for (size_t i=0, n=children.size(); i<n; ++i) {
		string subBranch = branchTemplate;
		if (i == n-1)
			subBranch += "\\   ";
		else
			subBranch += "|   ";

		result += children[i]->GenerateTreeDump(subBranch,
		    htmlLinksForClassNames, prefixForComponentUrls);
	}

	return result;
}


GXemul* Component::GetRunningGXemulInstance()
{
	Component* root = this;
	while (root->GetParent() != NULL)
		root = root->GetParent();

	RootComponent* rootComponent = root->AsRootComponent();
	if (rootComponent == NULL)
		return NULL;

	return rootComponent->GetOwner();
}


UI* Component::GetUI()
{
	GXemul* gxemul = GetRunningGXemulInstance();
	if (gxemul == NULL)
		return NULL;

	// TODO: Return NULL if the UI has debug messages turned off.

	return gxemul->GetUI();
}


void Component::AddChild(refcount_ptr<Component> childComponent,
	size_t insertPosition)
{
	if (insertPosition == (size_t) -1) {
		insertPosition = m_childComponents.size();
		m_childComponents.push_back(childComponent);
	} else {
		m_childComponents.insert(
		    m_childComponents.begin() + insertPosition,
		    childComponent);
	}

	// A component cannot have two parents.	
	assert(childComponent->GetParent() == NULL);

	childComponent->SetParent(this);

	// Make sure that the child's "name" state variable is unique among
	// all children of this component. (Yes, this is O(n^2) and it may
	// need to be rewritten to cope with _lots_ of components.)
	size_t postfix = 0;
	bool collision = false;
	do {
		const StateVariable* name =
		    childComponent->GetVariable("name");
		if (name->ToString().empty() || collision) {
			// Set the default name:
			//	visibleclassname + postfix number
			stringstream ss;
			ss << childComponent->GetVisibleClassName() << postfix;
			EscapedString escaped(ss.str());
			childComponent->SetVariableValue("name",
			    escaped.Generate());

			name = childComponent->GetVariable("name");
		}

		collision = false;
		for (size_t i=0; i<m_childComponents.size(); ++i) {
			if (i == insertPosition)
				continue;

			// Collision?
			const StateVariable* otherName =
			    m_childComponents[i]->GetVariable("name");
			if (otherName != NULL) {
				if (name->ToString() == otherName->ToString()) {
					collision = true;
					++ postfix;
					break;
				}
			} else {
				// Hm. All components should have a "name".
				assert(false);
			}
		}
	} while (collision);
}


size_t Component::RemoveChild(Component* childToRemove)
{
	size_t index = 0;
	for (Components::iterator it = m_childComponents.begin();
	     it != m_childComponents.end(); ++it, ++index) {
		if (childToRemove == (*it)) {
			childToRemove->SetParent(NULL);
			m_childComponents.erase(it);
			return index;
		}
	}
	
	// Child not found? Should not happen.
	assert(false);

	return (size_t) -1;
}


Components& Component::GetChildren()
{
	return m_childComponents;
}


const Components& Component::GetChildren() const
{
	return m_childComponents;
}


string Component::GeneratePath() const
{
	string path = GetVariable("name")->ToString();
	if (path.empty())
		path = "(" + GetClassName() + ")";

	if (m_parentComponent != NULL)
		path = m_parentComponent->GeneratePath() + "." + path;

	return path;
}


static vector<string> SplitPathStringIntoVector(const string &path)
{
	// Split the path into a vector. This is slow and hackish, but works.
	vector<string> pathStrings;
	string word;

	for (size_t i=0, n=path.length(); i<n; i++) {
		stringchar ch = path[i];
		if (ch == '.') {
			pathStrings.push_back(word);
			word = "";
		} else {
			word += ch;
		}
	}

	pathStrings.push_back(word);

	return pathStrings;
}


string Component::GenerateShortestPossiblePath() const
{
	string myPath = GeneratePath();
	vector<string> allComponentPaths;

	const Component* root = this;
	while (root->GetParent() != NULL)
		root = root->GetParent();

	root->AddAllComponentPaths(allComponentPaths);

	// Include as few as possible sub-parts of myPath (starting from the
	// end), to uniqely identify the component.
	vector<string> myPathParts = SplitPathStringIntoVector(myPath);

	for (size_t n=1; n<=myPathParts.size(); ++n) {
		string attempt = "";
		for (size_t i=myPathParts.size()-n; i<myPathParts.size(); i++) {
			if (attempt.length() > 0)
				attempt += ".";
			attempt += myPathParts[i];
		}

		// std::cerr << "attempt = " << attempt << "\n";
		// attempt = ram0
		// attempt = mainbus0.ram0
		// attempt = machine0.mainbus0.ram0
		// etc.

		// See if this substring is unique in allComponentPaths.
		int nHits = 0;
		string dotAttempt = "." + attempt;
		size_t dotAttemptLength = dotAttempt.length();
		for (size_t j=0; j<allComponentPaths.size(); ++j) {
			const string& s = allComponentPaths[j];
			if (s.length() < attempt.length())
				continue;
				
			if (s == attempt)
				nHits ++;
			else if (s.length() > dotAttemptLength &&
			    s.substr(s.length() - dotAttemptLength, dotAttemptLength) == dotAttempt)
				nHits ++;
		}

		// Unique? Then we found a good short path.
		if (nHits == 1)
			return attempt;
			
		// Otherwise continue.
	}

	// Worst case: return full path.
	return myPath;
}


const refcount_ptr<Component> Component::LookupPath(string path) const
{
	// Trim whitespace
	while (path.length() > 0 && path[path.length() - 1] == ' ')
		path = path.substr(0, path.length() - 1);

	refcount_ptr<Component> component = LookupPath(SplitPathStringIntoVector(path), 0);
	
	if (component.IsNULL()) {
		// Maybe it was a path starting from somewhere other than the
		// root. That is, if we find "." + path in the list of all
		// components' full names exactly 1 time, then we return that.
		vector<string> allComponentPaths;
		AddAllComponentPaths(allComponentPaths);

		const Component* root = this;
		while (root->GetParent() != NULL)
			root = root->GetParent();

		int nMatches = 0;
		string strToFind = "." + path;
		for (size_t i=0, n=allComponentPaths.size(); i<n; i++) {
			if (allComponentPaths[i].length() > strToFind.length()) {
				string subs = allComponentPaths[i].substr(
				    allComponentPaths[i].length() - strToFind.length());
				if (strToFind == subs) {
					nMatches ++;
					component = root->LookupPath(allComponentPaths[i]);
				}
			}
		}

		if (nMatches != 1)
			return NULL;
	}
	
	return component;
}


const refcount_ptr<Component> Component::LookupPath(const vector<string>& path,
	size_t index) const
{
	refcount_ptr<Component> component;

	if (index > path.size()) {
		// Huh? Lookup of empty path? Should not usually happen.
		assert(false);
		return component;
	}

	StateVariableMap::const_iterator it = m_stateVariables.find("name");
	if (it == m_stateVariables.end()) {
		// Failure (return NULL) if we don't have a name.
		return component;
	}

	string nameOfThisComponent = (it->second).ToString();
	bool match = (path[index] == nameOfThisComponent);

	// No match? Or was it the last part of the path? Then return.
	if (!match || index == path.size() - 1) {
		// (Successfully, if there was a match.)
		if (match)
			return const_cast<Component*>(this);
		return component;
	}

	// If there are still parts left to check, look among all the children:
	const string& pathPartToLookup = path[index+1];
	for (size_t i=0, n=m_childComponents.size(); i<n; i++) {
		const StateVariable* childName =
		    m_childComponents[i]->GetVariable("name");
		if (childName != NULL) {
			if (childName->ToString() == pathPartToLookup) {
				component = m_childComponents[i]->
				    LookupPath(path, index+1);
				break;
			}
		}
	}

	return component;
}


void Component::AddAllComponentPaths(vector<string>& allComponentPaths) const
{
	// Add the component itself first:
	allComponentPaths.push_back(GeneratePath());

	// Then all children:
	for (size_t i=0, n=m_childComponents.size(); i<n; i++)
		m_childComponents[i]->AddAllComponentPaths(allComponentPaths);
}


static bool PartialMatch(const string& partialPath, const string& path)
{
	if (partialPath.empty())
		return true;

	const size_t partialPathLength = partialPath.length();
	const size_t pathLength = path.length();
	size_t pathPos = 0;

	do {
		// Partial path too long? Then abort immediately.
		if (partialPathLength + pathPos > pathLength)
			break;

		// A substring match? Then we might have found it.
		if (path.substr(pathPos, partialPathLength) == partialPath) {
			// If the path has no tail (".subcomponent"), then
			// we found it:
			if (path.find('.', pathPos + partialPathLength) ==
			    string::npos)
				return true;
		}

		// Find next place in path to test:
		do {
			pathPos ++;
		} while (pathPos < pathLength && path[pathPos] != '.');

		if (pathPos < pathLength)
			pathPos ++;

	} while (pathPos < pathLength);

	return false;
}


vector<string> Component::FindPathByPartialMatch(
	const string& partialPath, bool shortestPossible) const
{
	vector<string> allComponentPaths;
	vector<string> matches;

	AddAllComponentPaths(allComponentPaths);

	for (size_t i=0, n=allComponentPaths.size(); i<n; i++)
		if (PartialMatch(partialPath, allComponentPaths[i])) {
			string match = allComponentPaths[i];

			if (shortestPossible) {
				const Component* root = this;
				while (root->GetParent() != NULL)
					root = root->GetParent();

				refcount_ptr<Component> component =
				    root->LookupPath(match);
				match = component->GenerateShortestPossiblePath();
			}

			matches.push_back(match);
		}

	return matches;
}


void Component::GetVariableNames(vector<string>& names) const
{
	for (StateVariableMap::const_iterator it = m_stateVariables.begin();
	    it != m_stateVariables.end(); ++it)
		names.push_back(it->first);
}


StateVariable* Component::GetVariable(const string& name)
{
	StateVariableMap::iterator it = m_stateVariables.find(name);
	if (it == m_stateVariables.end())
		return NULL;
	else
		return &(it->second);
}


const StateVariable* Component::GetVariable(const string& name) const
{
	StateVariableMap::const_iterator it = m_stateVariables.find(name);
	if (it == m_stateVariables.end())
		return NULL;
	else
		return &(it->second);
}


bool Component::CheckVariableWrite(StateVariable& var, const string& oldValue)
{
	GXemul* gxemul = GetRunningGXemulInstance();
	UI* ui = GetUI();

	if (gxemul != NULL) {
		const string& name = var.GetName();

		if (name == "step") {
			// If we are the root component, then writing to step
			// has special meaning:
			if (GetParent() == NULL) {
				bool error = false;
				int64_t oldStep = parse_number(oldValue.c_str(), error);
				int64_t newStep = var.ToInteger();

				// 0. Value is the same as before. Simply return.
				if (newStep == oldStep)
					return true;

				// 1. The new value is too low (less than 0).
				if (newStep < 0) {
					if (ui != NULL)
						ui->ShowDebugMessage("root.step can"
						    " not be set to lower than zero.\n");
					return false;
				}

				// 2. The value is lower; run backwards if possible.
				if (newStep < oldStep) {
					if (!gxemul->GetSnapshottingEnabled()) {
						if (ui != NULL)
							ui->ShowDebugMessage("root.step can"
							    " not be decreased; snapshotting"
							    " was not enabled prior to\nstarting"
							    " the emulation. (-B command line"
							    " option.)\n");
						return false;
					}

					return gxemul->ModifyStep(oldStep, newStep);
				}

				// 3. The value is higher; run forwards.
				return gxemul->ModifyStep(oldStep, newStep);
			} else {
				// We are not the root component. Direct (interactive)
				// writes to the step variable is not allowed.
				if (ui != NULL)
					ui->ShowDebugMessage("The step variable of "
					    "this component cannot be set manually.\n");
			}

			return false;
		}
	}

	return true;
}


bool Component::SetVariableValue(const string& name, const string& expression)
{
	UI* ui = GetUI();

	StateVariableMap::iterator it = m_stateVariables.find(name);
	if (it == m_stateVariables.end()) {
		if (ui != NULL)
			ui->ShowDebugMessage((string) name + ": no such variable\n");
		return false;
	}

	StateVariable& var = it->second;
	
	stringstream oldValue;
	var.SerializeValue(oldValue);

	bool success = var.SetValue(expression);
	if (!success) {
		if (ui != NULL)
			ui->ShowDebugMessage((string) name + ": expression could"
			    " not be assigned; type mismatch?\n");
		return false;
	}

	stringstream newValue;
	var.SerializeValue(newValue);

	if (oldValue.str() != newValue.str()) {
		success = CheckVariableWrite(var, oldValue.str());
		if (!success) {
			// Revert to the previous:
			var.SetValue(oldValue.str());
			return false;
		}
	}

	return true;
}


void Component::Serialize(ostream& ss, SerializationContext& context) const
{
	SerializationContext subContext = context.Indented();
	string tabs = context.Tabs();

	ss << tabs << "component " << m_className << "\n" << tabs << "{\n";
	    
	for (StateVariableMap::const_iterator it = m_stateVariables.begin();
	    it != m_stateVariables.end(); ++it)
		(it->second).Serialize(ss, subContext);

	for (size_t i = 0, n = m_childComponents.size(); i < n; ++ i)
		m_childComponents[i]->Serialize(ss, subContext);

	ss << tabs << "}\n";
}


static bool GetNextToken(const string& str, size_t& pos, string& token)
{
	token = "";

	size_t len = str.length();
	
	// Skip initial whitespace:
	while (pos < len &&
	    (str[pos] == ' '  || str[pos] == '\t' ||
	     str[pos] == '\r' || str[pos] == '\n'))
		++ pos;

	if (pos >= len)
		return false;

	// Get the token, until end-of-string or until whitespace is found:
	bool quoted = false;
	do {
		char ch = str[pos];

		if (!quoted) {
			if (ch == '"')
				quoted = true;
			if (ch == ' '  || ch == '\t' ||
			    ch == '\r' || ch == '\n')
				break;
			token += ch;
		} else {
			if (ch == '"')
				quoted = false;
			token += ch;
			if (ch == '\\' && pos < len-1)
				token += str[++pos];
		}

		++ pos;
	} while (pos < len);

	return true;
}


refcount_ptr<Component> Component::Deserialize(ostream& messages, const string& str, size_t& pos)
{
	refcount_ptr<Component> deserializedTree = NULL;
	string token;

	if (!GetNextToken(str, pos, token) || token != "component") {
		messages << "Expecting \"component\".\n";
		return deserializedTree;
	}

	string className;
	if (!GetNextToken(str, pos, className)) {
		messages << "Expecting a class name.\n";
		return deserializedTree;
	}

	if (!GetNextToken(str, pos, token) || token != "{") {
		messages << "Expecting {.\n";
		return deserializedTree;
	}

	// root is a special case (cannot be created by the factory). All other
	// class types should be possible to create using the factory.
	if (className == "root") {
		deserializedTree = new RootComponent;
	} else {
		deserializedTree = ComponentFactory::CreateComponent(className);
		if (deserializedTree.IsNULL()) {
			messages << "Could not create a '" << className << "' component.\n";
			return deserializedTree;
		}
	}

	while (pos < str.length()) {
		size_t savedPos = pos;

		// Either 1)   }     (end of current component)
		// or     2)   component name { ...
		// or     3)   variableType name "value"

		if (!GetNextToken(str, pos, token)) {
			// Failure.
			messages << "Failure. (0)\n";
			deserializedTree = NULL;
			break;
		}

		// Case 1:
		if (token == "}")
			break;

		string name;
		if (!GetNextToken(str, pos, name)) {
			// Failure.
			messages << "Failure. (1)\n";
			deserializedTree = NULL;
			break;
		}

		if (token == "component") {
			// Case 2:
			refcount_ptr<Component> child =
			    Component::Deserialize(messages, str, savedPos);
			if (child.IsNULL()) {
				// Failure.
				messages << "Failure. (2)\n";
				deserializedTree = NULL;
				break;
			}
			
			deserializedTree->AddChild(child);
			pos = savedPos;
		} else {
			// Case 3:
			string varType = token;
			string varValue;
			if (!GetNextToken(str, pos, varValue)) {
				// Failure.
				messages << "Failure. (3)\n";
				deserializedTree = NULL;
				break;
			}

			if (!deserializedTree->SetVariableValue(name,
			    varValue)) {
				messages << "Warning: variable '" << name <<
				    "' for component class " << className <<
				    " could not be deserialized; skipping.\n";
			}
		}
	}

	return deserializedTree;
}


bool Component::CheckConsistency() const
{
	// Serialize
	SerializationContext context;
	stringstream ss;
	Serialize(ss, context);

	string result = ss.str();

	Checksum checksumOriginal;
	AddChecksum(checksumOriginal);

	// Deserialize
	size_t pos = 0;
	stringstream messages;
	refcount_ptr<Component> tmpDeserializedTree = Deserialize(messages, result, pos);
	if (tmpDeserializedTree.IsNULL())
		return false;

	Checksum checksumDeserialized;
	tmpDeserializedTree->AddChecksum(checksumDeserialized);

	// ... and compare the checksums:
	return checksumOriginal == checksumDeserialized;
}


void Component::AddChecksum(Checksum& checksum) const
{
	// Some random stuff is added between normal fields to the checksum.
	// This is to make it harder to get the same checksum for two different
	// objects, that just have some fields swapped, or such. (Yes, I know,
	// that is not a very scientific explanation :) but it will have to do.)

	checksum.Add(((uint64_t) 0x12491725 << 32) | 0xabcef011);
	checksum.Add(m_className);
	
	SerializationContext dummyContext;

	// Add all state variables.
	for (StateVariableMap::const_iterator it = m_stateVariables.begin();
	    it != m_stateVariables.end();
	    ++ it) {
		checksum.Add(((uint64_t) 0x019fb879 << 32) | 0x25addae1);

		stringstream ss;
		(it->second).Serialize(ss, dummyContext);
		checksum.Add(ss.str());
	}
	
	// Add all child components.
	for (size_t i = 0; i < m_childComponents.size(); ++ i) {
		checksum.Add((((uint64_t) 0xf98a7c7c << 32) | 0x109f0000)
		    + i * 0x98127417);
		m_childComponents[i]->AddChecksum(checksum);
	}

	checksum.Add(((uint64_t) 0x90a10224 << 32) | 0x97defa7a);
}


