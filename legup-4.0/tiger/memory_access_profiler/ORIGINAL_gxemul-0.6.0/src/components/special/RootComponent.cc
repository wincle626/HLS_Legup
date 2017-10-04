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

#include "components/RootComponent.h"
#include "ComponentFactory.h"
#include "GXemul.h"


RootComponent::RootComponent(GXemul* owner)
	: Component("root", "root")
	, m_gxemul(owner)
	, m_accuracy("cycle")
{
	SetVariableValue("name", "\"root\"");

	AddVariable("accuracy", &m_accuracy);
}


bool RootComponent::PreRunCheckForComponent(GXemul* gxemul)
{
	if (m_accuracy != "cycle" && m_accuracy != "sloppy") {
		gxemul->GetUI()->ShowDebugMessage(this, "accuracy must be \"cycle\" or \"sloppy\".\n");		
		return false;
	}

	return true;
}


void RootComponent::SetOwner(GXemul* owner)
{
	m_gxemul = owner;
}


bool RootComponent::CheckVariableWrite(StateVariable& var, const string& oldValue)
{
	UI* ui = GetUI();
	const string& name = var.GetName();

	if (name == "accuracy") {
		if (var.ToString() != "cycle" && var.ToString() != "sloppy") {
			if (ui != NULL)
				ui->ShowDebugMessage(this, "accuracy must be \"cycle\" or \"sloppy\".\n");

			return false;
		}
	}

	return Component::CheckVariableWrite(var, oldValue);
}


/*****************************************************************************/


#ifdef WITHUNITTESTS

static void Test_RootComponent_CreateComponent()
{
	refcount_ptr<Component> component;

	component = ComponentFactory::CreateComponent("root");
	UnitTest::Assert("creating a root component with CreateComponent "
	    "should NOT be possible", component.IsNULL() == true);
}

static void Test_RootComponent_InitialVariables()
{
	refcount_ptr<Component> component = new RootComponent;

	StateVariable* name = component->GetVariable("name");
	StateVariable* step = component->GetVariable("step");
	StateVariable* accuracy = component->GetVariable("accuracy");

	UnitTest::Assert("name should be root", name->ToString(), "root");
	UnitTest::Assert("step should be 0", step->ToInteger(), 0);
	UnitTest::Assert("accuracy should be cycle", accuracy->ToString(), "cycle");
}

static void Test_RootComponent_AccuracyValues()
{
	GXemul dummyGXemul;

	refcount_ptr<Component> component = new RootComponent;
	StateVariable* accuracy = component->GetVariable("accuracy");

	accuracy->SetValue("\"sloppy\"");
	UnitTest::Assert("sloppy should be ok", component->PreRunCheck(&dummyGXemul));

	accuracy->SetValue("\"nonsense\"");
	UnitTest::Assert("nonsense should not be ok", !component->PreRunCheck(&dummyGXemul));

	accuracy->SetValue("\"cycle\"");
	UnitTest::Assert("cycle should be ok", component->PreRunCheck(&dummyGXemul));
}

UNITTESTS(RootComponent)
{
	UNITTEST(Test_RootComponent_CreateComponent);
	UNITTEST(Test_RootComponent_InitialVariables);
	UNITTEST(Test_RootComponent_AccuracyValues);

	// TODO: Test owner
}

#endif

