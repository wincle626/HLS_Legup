#ifndef CHECKSUM_H
#define	CHECKSUM_H

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

#include "UnitTest.h"


/**
 * \brief A checksum accumulator.
 *
 * The main purpose of this class is as a helper in unit tests, where
 * objects such as trees are hard to compare to each other. A checksum
 * of the first tree can then be compared with a checksum of the second
 * tree.
 *
 * Note: This is not scientifically correct in any way. It is just something
 * I made up, for unit testing purposes. (2007-12-27)
 */
class Checksum
	: public UnitTestable
{
public:
	/**
	 * \brief Constructs a zeroed checksum.
	 */
	Checksum();

	/**
	 * \brief Retrieves the value of the checksum, as a uint64_t.
	 *
	 * @return the checksum value
	 */
	uint64_t Value() const;

	/**
	 * \brief Add a uint64_t to the checksum.
	 *
	 * @param x Value to add.
	 */
	void Add(uint64_t x);

	/**
	 * \brief Add a string to the checksum.
	 *
	 * @param str The string to add.
	 */
	void Add(const string& str);

	/**
	 * \brief Compares one %Checksum to another for equality.
	 *
	 * @param other  The %Checksum to compare to.
	 * @return true if the checksums match, false otherwise.
	 */
	bool operator == (const Checksum& other) const;

	/**
	 * \brief Compares one %Checksum to another for inequality.
	 *
	 * @param other  The %Checksum to compare to.
	 * @return false if the checksums match, true otherwise.
	 */
	bool operator != (const Checksum& other) const;


	/********************************************************************/

	static void RunUnitTests(int& nSucceeded, int& nFailures);

private:
	uint64_t	m_value;
};


#endif	// CHECKSUM_H
