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

#include "SymbolRegistry.h"


SymbolRegistry::SymbolRegistry()
{
}


void SymbolRegistry::Clear()
{
	m_map.clear();
}


void SymbolRegistry::AddSymbol(const string& symbol, uint64_t vaddr)
{
	m_map[vaddr] = symbol;
}


string SymbolRegistry::LookupAddress(uint64_t vaddr, bool allowOffset) const
{
	// Must be an exact match?
	if (!allowOffset) {
		SymbolMap::const_iterator a = m_map.find(vaddr);
		return a == m_map.end()? "" : a->second;
	}

	// Try to find the symbol given the address, or at least some
	// symbol close to it:
	SymbolMap::const_iterator a = m_map.lower_bound(vaddr);
	if (a == m_map.end()) {
		if (m_map.empty())
			return "";

		a = m_map.end();
	}

	// Exact match? Then just return the symbol.
	if (a != m_map.end() && vaddr == a->first)
		return a->second;

	if (a == m_map.begin())
		return "";

	// Return the symbol _before_ the address, plus an offset.
	--a;

	stringstream ss;
	ss.flags(std::ios::hex);
	ss << a->second << "+0x" << (vaddr - a->first);
	return ss.str();
}


/*****************************************************************************/


#ifdef WITHUNITTESTS

static void Test_SymbolRegistry_Basic()
{
	SymbolRegistry registry;

	registry.AddSymbol("symA", 0x1000);
	registry.AddSymbol("symB", 0x1020);
	registry.AddSymbol("symC", 0x1060);

	UnitTest::Assert("lookup failed 1?",
	    registry.LookupAddress(0x1020, true), "symB");

	UnitTest::Assert("lookup failed 2?",
	    registry.LookupAddress(0x1020, false), "symB");

	registry.Clear();

	UnitTest::Assert("clear failed?",
	    registry.LookupAddress(0x1020, true), "");
}

static void Test_SymbolRegistry_BeforeFirst()
{
	SymbolRegistry registry;

	registry.AddSymbol("symA", 0x1000);
	registry.AddSymbol("symB", 0x1020);

	UnitTest::Assert("lookup should have failed 1",
	    registry.LookupAddress(0xffc, true), "");

	UnitTest::Assert("lookup should have failed 2",
	    registry.LookupAddress(0xffc, false), "");
}

static void Test_SymbolRegistry_WithOffset()
{
	SymbolRegistry registry;

	registry.AddSymbol("symA", 0x1000);
	registry.AddSymbol("symB", 0x1020);

	UnitTest::Assert("lookup failed?",
	    registry.LookupAddress(0x1006, true), "symA+0x6");

	UnitTest::Assert("lookup should have failed",
	    registry.LookupAddress(0x1006, false), "");
}

static void Test_SymbolRegistry_AfterLast()
{
	SymbolRegistry registry;

	registry.AddSymbol("symA", 0x1000);
	registry.AddSymbol("symB", 0x1020);

	UnitTest::Assert("lookup failed?",
	    registry.LookupAddress(0x1090, true), "symB+0x70");

	UnitTest::Assert("lookup should have failed",
	    registry.LookupAddress(0x1090, false), "");
}

UNITTESTS(SymbolRegistry)
{
	UNITTEST(Test_SymbolRegistry_Basic);
	UNITTEST(Test_SymbolRegistry_BeforeFirst);
	UNITTEST(Test_SymbolRegistry_WithOffset);
	UNITTEST(Test_SymbolRegistry_AfterLast);
}

#endif
