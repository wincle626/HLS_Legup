#ifndef COMPONENTFACTORY_H
#define	COMPONENTFACTORY_H

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

#include "Component.h"
#include "UnitTest.h"


/**
 * \brief A factory which creates Component objects.
 *
 * The main usage of the %ComponentFactory is simply:
 * "Hey, give me an XYZ component."
 * and the %ComponentFactory returns a reference counted pointer to something.
 * This can be a single Component, or it can be something more complex (a
 * %Component with children, etc).
 *
 * This mechanism is also used for templates. If the user wishes to create
 * a "testmips" machine, what actually happens is that CreateComponent() is
 * called with "testmips" as the argument, and it returns an entire tree,
 * which may contain something like:<pre>
 * machine0  [testmips]
 * |-- cpu0
 * |-- ram0
 * \-- framebuffer0
 * </pre>
 *
 * (The example above is semi-bogus, but should illustrate the point of
 * CreateComponent().)
 */
class ComponentFactory
	: public UnitTestable
{
public:
	/**
	 * \brief Creates a component given a short component name.
	 *
	 * componentNameAndOptionalArgs may be e.g.
	 * <tt>"testmips(cpu=R4400,cpus=4)"</tt>.
	 *
	 * @param componentNameAndOptionalArgs The component name, e.g. "dummy",
	 *	optionally followed by arguments in parentheses.
	 * @param gxemul A pointer to a GXemul instance. May be NULL.
	 * @return A reference counted Component pointer. This is set to the
	 *      newly created component on success. On failure it is set to
	 *      NULL.
	 */
	static refcount_ptr<Component> CreateComponent(
		const string& componentNameAndOptionalArgs, GXemul* gxemul = NULL);

	/**
	 * \brief Gets a specific attribute value for a component.
	 *
	 * @param name The name of a component, e.g. "testmips".
	 * @param attributeName The attribute, e.g. "template" or "machine".
	 * @return A string containing the attribute value. This is an
	 *	empty string if the component does not exist, or if the
	 *	attribute was not set for the component.
	 */
	static string GetAttribute(const string& name, const string&
		attributeName);

	/**
	 * \brief Checks if a component has a specific attribute.
	 *
	 * @param name The name of a component, e.g. "testmips".
	 * @param attributeName The attribute, e.g. "template" or "machine".
	 * @return True if the name exists and the attribute is not an
	 *	empty string, false otherwise
	 *	(both if the name is not known, and if it is known but
	 *	the attribute was not set).
	 */
	static bool HasAttribute(const string& name, const string&
		attributeName);

	/**
	 * \brief Returns a vector of all available component names.
	 *
	 * @param onlyTemplates If true, only those component names that
	 *	are templates are returned.
	 * @return A vector of all available component names.
	 */
	static vector<string> GetAllComponentNames(bool onlyTemplates);

	/**
	 * \brief Adds a new component class to the factory at runtime.
	 *
	 * Component classes added using this function are then available
	 * when using e.g. CreateComponent.
	 *
	 * @param name The name of the component class.
	 * @param createFunc A pointer to the component's Create function.
	 * @param getAttributeFunc A pointer to the component's GetAttribute
	 *	function.
	 * @return True if the component class was registered, false
	 *	if the name was already in use.
	 */
	static bool RegisterComponentClass(const string& name,
		refcount_ptr<Component> (*createFunc)(const ComponentCreateArgs& args),
		string (*getAttributeFunc)(const string& attributeName));

	/**
	 * \brief Get override arguments for component creation.
	 */
	static bool GetCreationArgOverrides(ComponentCreationSettings& settings,
		const ComponentCreateArgs& createArgs);


	/********************************************************************/

	static void RunUnitTests(int& nSucceeded, int& nFailures);
};


#endif	// COMPONENTFACTORY_H
