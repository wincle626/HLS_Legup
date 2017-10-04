#ifndef MACHINECOMPONENT_H
#define	MACHINECOMPONENT_H

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

// COMPONENT(machine)


#include "Component.h"

#include "UnitTest.h"


/**
 * \brief A Component representing a machine.
 *
 * A machine is, loosely defined, a box containing e.g.:
 * <ul>
 *	<li>processor(s)
 *	<li>RAM
 *	<li>ROM
 *	<li>interrupt controller
 *	<li>other busses, such as PCI busses (which in turn have things
 *		like network cards and graphics cards on them)
 *	<li>...
 * </ul>
 */
class MachineComponent
	: public Component
	, public UnitTestable
{
public:
	/**
	 * \brief Constructs a MachineComponent.
	 */
	MachineComponent();

	/**
	 * \brief Creates a MachineComponent.
	 */
	static refcount_ptr<Component> Create(const ComponentCreateArgs& args);

	/**
	 * \brief Get attribute information about the MachineComponent class.
	 *
	 * Implementation specifics for the MachineComponent class:
	 * The machine is "stable" and a "machine".
	 *
	 * @param attributeName The attribute name.
	 * @return A string representing the attribute value.
	 */
	static string GetAttribute(const string& attributeName);


	/********************************************************************/

	static void RunUnitTests(int& nSucceeded, int& nFailures);
};


#endif	// MACHINECOMPONENT_H
