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

#include "Checksum.h"


Checksum::Checksum()
	: m_value(0)
{
}


uint64_t Checksum::Value() const
{
	return m_value;
}


void Checksum::Add(uint64_t x)
{
	m_value += x * (((uint64_t)0xc151429 << 32) | 0x517851bf);
	m_value ^= (((uint64_t)0x9183bfec << 32) | 0x01921947);
}


void Checksum::Add(const string& str)
{
	size_t n = str.length();
	Add(10000 + n);
	
	for (size_t i=0; i<n; ++i)
		Add(str[i]);
}


bool Checksum::operator == (const Checksum& other) const
{
	return m_value == other.m_value;
}


bool Checksum::operator != (const Checksum& other) const
{
	return m_value != other.m_value;
}


/*****************************************************************************/


#ifdef WITHUNITTESTS

static void Test_Checksum_DefaultValue()
{
	Checksum checksum;

	UnitTest::Assert("value should initially be zero",
	    checksum.Value() == 0);
}

static void Test_Checksum_Add()
{
	Checksum checksum;

	checksum.Add(123);
	uint64_t v1 = checksum.Value();
	
	checksum.Add(123);
	uint64_t v2 = checksum.Value();

	UnitTest::Assert("v1 should be non-zero", v1 != 0);
	UnitTest::Assert("v2 should be non-zero", v2 != 0);
	UnitTest::Assert("v1 and v2 should differ", v1 != v2);
}

static void Test_Checksum_SameChecksum()
{
	Checksum checksumA;
	Checksum checksumB;

	checksumA.Add("testing");
	checksumB.Add("testing");

	UnitTest::Assert("checksums should be the same",
	    checksumA.Value() == checksumB.Value());
}

static void Test_Checksum_OrderIsSignificant_Numeric()
{
	Checksum checksumA;
	Checksum checksumB;

	checksumA.Add(123);
	checksumA.Add(234);
	uint64_t valueA = checksumA.Value();
	
	checksumB.Add(234);
	checksumB.Add(123);
	uint64_t valueB = checksumB.Value();

	UnitTest::Assert("valueA and valueB should differ", valueA != valueB);
}

static void Test_Checksum_OrderIsSignificant_String()
{
	Checksum checksumA;
	Checksum checksumB;

	checksumA.Add("123");
	checksumA.Add("234");
	uint64_t valueA = checksumA.Value();
	
	checksumB.Add("234");
	checksumB.Add("123");
	uint64_t valueB = checksumB.Value();

	UnitTest::Assert("valueA and valueB should differ", valueA != valueB);
}

static void Test_Checksum_OrderIsSignificant_String_1()
{
	Checksum checksumA;
	Checksum checksumB;

	checksumA.Add("abcdef");
	uint64_t valueA = checksumA.Value();
	
	checksumB.Add("abdcef");
	uint64_t valueB = checksumB.Value();

	UnitTest::Assert("valueA and valueB should differ", valueA != valueB);
}

static void Test_Checksum_OrderIsSignificant_String_2()
{
	Checksum checksumA;
	Checksum checksumB;

	checksumA.Add("hello ");
	checksumA.Add("world");
	uint64_t valueA = checksumA.Value();
	
	checksumB.Add("hello");
	checksumB.Add(" world");
	uint64_t valueB = checksumB.Value();

	UnitTest::Assert("valueA and valueB should differ", valueA != valueB);
}

static void Test_Checksum_OrderIsSignificant_String_3()
{
	Checksum checksumA;
	Checksum checksumB;

	checksumA.Add("string x \"modified\"\n");
	checksumA.Add("string y \"value 2\"\n");
	checksumA.Add("string x \"value\\nhello\"\n");
	uint64_t valueA = checksumA.Value();

	checksumB.Add("string x \"value 1\"\n");
	checksumB.Add("string y \"value 2\"\n");
	checksumB.Add("string x \"value\\nhello\"\n");
	uint64_t valueB = checksumB.Value();

	UnitTest::Assert("valueA and valueB should differ", valueA != valueB);
}

static void Test_Checksum_String_Concatenation()
{
	Checksum checksumA;
	Checksum checksumB;

	checksumA.Add("hello");
	checksumA.Add(" world");
	uint64_t valueA = checksumA.Value();
	
	checksumB.Add("hello world");
	uint64_t valueB = checksumB.Value();

	UnitTest::Assert("valueA and valueB should differ", valueA != valueB);
}

UNITTESTS(Checksum)
{
	UNITTEST(Test_Checksum_DefaultValue);
	UNITTEST(Test_Checksum_Add);
	UNITTEST(Test_Checksum_SameChecksum);
	UNITTEST(Test_Checksum_OrderIsSignificant_Numeric);
	UNITTEST(Test_Checksum_OrderIsSignificant_String);
	UNITTEST(Test_Checksum_OrderIsSignificant_String_1);
	UNITTEST(Test_Checksum_OrderIsSignificant_String_2);
	UNITTEST(Test_Checksum_OrderIsSignificant_String_3);
	UNITTEST(Test_Checksum_String_Concatenation);
}

#endif
