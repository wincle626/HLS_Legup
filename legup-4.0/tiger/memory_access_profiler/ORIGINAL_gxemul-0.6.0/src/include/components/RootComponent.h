#ifndef ROOTCOMPONENT_H
#define	ROOTCOMPONENT_H

/*
 *  Copyright (C) 2009-2010  Anders Gavare.  All rights reserved.
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

/*
 * Note: This is a special component, which cannot be created interactively
 * by the user.
 */

#include "Component.h"

#include "UnitTest.h"

class GXemul;


/**
 * \brief A Component which is the default root node in the configuration.
 *
 * This Component is mostly a dummy component, but it holds the 'step'
 * count for the entire emulation (inherited from Component), and the following
 * settings:
 *
 * <ul>
 *	<li>accuracy ("cycle" or "sloppy")
 * </ul>
 *
 * NOTE: A RootComponent is not registered in the component registry, and
 * can thus not be created interactively by the user at runtime.
 */
class RootComponent
	: public Component
	, public UnitTestable
{
public:
	/**
	 * \brief Constructs a RootComponent.
	 */
	RootComponent(GXemul* owner = NULL);

	virtual RootComponent* AsRootComponent()
	{
		return this;
	}

	virtual bool PreRunCheckForComponent(GXemul* gxemul);

	GXemul* GetOwner()
	{
		return m_gxemul;
	}

	void SetOwner(GXemul* owner);


	/********************************************************************/
public:
	static void RunUnitTests(int& nSucceeded, int& nFailures);

protected:
	virtual bool CheckVariableWrite(StateVariable& var, const string& oldValue);

private:
	// Pointer to owner (may be NULL):
	GXemul*		m_gxemul;

	// Model:
	string		m_accuracy;
};


#endif	// ROOTCOMPONENT_H
