#ifndef SYMBOLREGISTRY_H
#define	SYMBOLREGISTRY_H

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

#include "misc.h"

#include "UnitTest.h"


/**
 * \brief A registry for loaded symbols.
 */
class SymbolRegistry
	: public UnitTestable
{
public:
	/**
	 * \brief Constructs a SymbolRegistry.
	 */
	SymbolRegistry();

	/**
	 * \brief Clears the registry.
	 */
	void Clear();

	/**
	 * \brief Adds a symbol to the registry.
	 *
	 * @param symbol The symbol name.
	 * @param vaddr The virtual address.
	 */
	void AddSymbol(const string& symbol, uint64_t vaddr);

	/**
	 * \brief Looks up an address.
	 *
	 * The returned address may be of the format "symbol+offset", where
	 * offset is a hexadecimal number, if allowOffset is true.
	 *
	 * @param vaddr The virtual address.
	 * @param allowOffset If false, the address must be found exactly.
	 *	If true, a symbol which almost matches the address, plus an
	 *	offset, may be returned.
	 * @return A string representing the address, or an empty string
	 *	if no suitable match was found.
	 */
	string LookupAddress(uint64_t vaddr, bool allowOffset) const;


	/********************************************************************/

	static void RunUnitTests(int& nSucceeded, int& nFailures);

private:
	typedef map<uint64_t,string> SymbolMap;
	SymbolMap	m_map;
};


#endif	// SYMBOLREGISTRY_H
