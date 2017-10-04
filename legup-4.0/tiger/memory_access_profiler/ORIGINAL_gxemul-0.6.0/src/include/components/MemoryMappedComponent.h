#ifndef MEMORYMAPPEDCOMPONENT_H
#define	MEMORYMAPPEDCOMPONENT_H

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

// Note: not included in the component registry. This is simply a base-class.


#include "Component.h"

#include "UnitTest.h"


/**
 * \brief A base-class for memory-mapped components.
 *
 * A memory-mapped component is a Component which is meant to be mapped
 * into a bus' address space. It has a <b>base offset</b> and a <b>size</b>,
 * which the bus sees. The bus then forwards only those read/write requests
 * to the memory-mapped component that are within that range.
 *
 * There is also an <b>address multiplicator</b>, to handle the common
 * case when a device has, say, N 8-bit registers, but is accessed using
 * memory-mapped addresses that are seen as N 32-bit registers. In this
 * example, the address multiplicator would be 4.
 */
class MemoryMappedComponent
	: public Component
{
public:
	/**
	 * \brief Constructs a MemoryMappedComponent.
	 */
	MemoryMappedComponent(const string& className, const string& visibleClassName);

private:
	// Variables common to all memory mapped components:
	uint64_t	m_memoryMappedBase;
	uint64_t	m_memoryMappedSize;
	uint64_t	m_memoryMappedAddrMul;
};


#endif	// MEMORYMAPPEDCOMPONENT_H
