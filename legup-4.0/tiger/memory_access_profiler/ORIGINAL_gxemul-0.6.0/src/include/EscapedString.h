#ifndef ESCAPEDSTRING_H
#define	ESCAPEDSTRING_H

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
 * \brief A helper class for escaping strings using C-style escapes.
 *
 * TODO: It is ugly to use this for conversions in both directions! Should
 *	be fixed some day.
 */
class EscapedString
	: public UnitTestable
{
public:
	/**
	 * \brief Constructs an EscapedString helper.
	 *
	 * @param str	A string, either escaped or not escaped.
	 */
	EscapedString(const string& str);

	/**
	 * \brief Generates an escaped string, from the original string.
	 *
	 * @return	an escaped string
	 */
	string Generate() const;

	/**
	 * \brief Decodes an escaped string, from the original string.
	 *
	 * The original string should be a C-style escaped string, with
	 * or without surrounding quote (") characters.
	 *
	 * @param success Set to true if decoding was successful, false
	 *	if there was an error.
	 * @return A decoded (unescaped) string. (Only valid if
	 *	<tt>success</tt> was set to true.)
	 */
	string Decode(bool& success) const;


	/********************************************************************/

	static void RunUnitTests(int& nSucceeded, int& nFailures);

private:
	string		m_str;
};


#endif	// ESCAPEDSTRING_H
