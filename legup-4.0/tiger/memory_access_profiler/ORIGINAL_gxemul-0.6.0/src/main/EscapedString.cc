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

#include "EscapedString.h"


EscapedString::EscapedString(const string& str)
	: m_str(str)
{
}


string EscapedString::Generate() const
{
	string result = "\"";

	for (size_t i=0; i<m_str.length(); i++) {
		char ch = m_str[i];

		switch (ch) {
		case '\n':
			result += "\\n";
			break;
		case '\r':
			result += "\\r";
			break;
		case '\t':
			result += "\\t";
			break;
		case '"':
			result += "\\\"";
			break;
		case '\\':
			result += "\\\\";
			break;
		default:
			result += ch;
		}
	}

	result += "\"";

	return result;
}


string EscapedString::Decode(bool& success) const
{
	success = false;

	// Not an escaped string? Then return failure.
	if (m_str.size() < 2 || m_str[0] != '"'
	    || m_str[m_str.size() - 1] != '"')
		return "";

	string result = "";
	size_t i;
	
	for (i = 1; i < m_str.length() - 1; i++) {
		char ch = m_str[i];

		switch (ch) {
		case '\\':
			{
				char ch2 = m_str[++i];
				switch (ch2) {
				case 'n':
					result += '\n';
					break;
				case 'r':
					result += '\r';
					break;
				case 't':
					result += '\t';
					break;
				default:
					result += ch2;
				}
				break;
			}
		default:
			result += ch;
		}
	}

	if (i == m_str.length() - 1)
		success = true;

	return result;
}


/*****************************************************************************/


#ifdef WITHUNITTESTS

static void Test_EscapedString_Generate()
{
	UnitTest::Assert("trivial escape: normal text",
	    EscapedString("hello world 123").Generate() ==
	    "\"hello world 123\"");

	UnitTest::Assert("escape tab",
	    EscapedString("hi\tworld").Generate() ==
	    "\"hi\\tworld\"");

	UnitTest::Assert("escape newline and carriage return",
	    EscapedString("hi\nworld\ragain").Generate() ==
	    "\"hi\\nworld\\ragain\"");

	UnitTest::Assert("escape quotes",
	    EscapedString("hi'123\"456\"789").Generate() ==
	    "\"hi'123\\\"456\\\"789\"");

	UnitTest::Assert("escaped escape char",
	    EscapedString("Hello\\world").Generate() ==
	    "\"Hello\\\\world\"");
}

static void Test_EscapedString_Decode()
{
	bool success = false;

	UnitTest::Assert("trivial escape: normal text",
	    EscapedString("\"hello world 123\"").Decode(success) ==
	    "hello world 123");
	UnitTest::Assert("success failed? (1)", success == true);

	success = false;
	UnitTest::Assert("escape tab",
	    EscapedString("\"hi\\tworld\"").Decode(success) ==
	    "hi\tworld");
	UnitTest::Assert("success failed? (2)", success == true);

	success = false;
	UnitTest::Assert("escape newline and carriage return",
	    EscapedString("\"hi\\nworld\\ragain\"").Decode(success) ==
	    "hi\nworld\ragain");
	UnitTest::Assert("success failed? (3)", success == true);

	success = false;
	UnitTest::Assert("escape quotes",
	    EscapedString("\"hi'123\\\"456\\\"789\"").Decode(success) ==
	    "hi'123\"456\"789");
	UnitTest::Assert("success failed? (4)", success == true);

	success = false;
	UnitTest::Assert("escaped escape char",
	    EscapedString("\"Hello\\\\world\"").Decode(success) ==
	    "Hello\\world");
	UnitTest::Assert("success failed? (5)", success == true);
}

static void Test_EscapedString_Decode_WithoutQuotes()
{
	bool success = true;

	UnitTest::Assert("trivial escape: normal text",
	    EscapedString("hello world 123").Decode(success) ==
	    "");
	UnitTest::Assert("success should have failed", success == false);
}

UNITTESTS(EscapedString)
{
	UNITTEST(Test_EscapedString_Generate);
	UNITTEST(Test_EscapedString_Decode);
	UNITTEST(Test_EscapedString_Decode_WithoutQuotes);
}

#endif
