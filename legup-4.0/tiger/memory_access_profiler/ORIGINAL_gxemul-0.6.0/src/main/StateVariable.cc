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

#include <assert.h>
#include <math.h>

#include "EscapedString.h"
#include "StateVariable.h"


/*****************************************************************************/


// This is basically strtoull(), but it needs to be explicitly implemented
// since some systems lack it. (Also, compiling with GNU C++ in ANSI mode
// does not work with strtoull.)
static uint64_t parse_number(const char* str, bool& error)
{
	int base = 10;
	uint64_t result = 0;
	bool negative = false;

	error = false;

	if (str == NULL)
		return 0;

	while (*str == ' ')
		++str;

	if (*str == '-') {
		negative = true;
		++str;
	}

	while ((*str == 'x' || *str == 'X') || (*str >= '0' && *str <= '9')
	    || (*str >= 'a' && *str <= 'f') || (*str >= 'A' && *str <= 'F')) {
		if (*str == 'x' || *str == 'X') {
			base = 16;
		} else {
			int n = *str - '0';
			if (*str >= 'a' && *str <= 'f')
				n = *str - 'a' + 10;
			if (*str >= 'A' && *str <= 'F')
				n = *str - 'A' + 10;
			result = result * base + n;
		}
		++str;
	}

	if (*str)
		error = true;

	if (negative)
		return -result;
	else
		return result;
}


/*****************************************************************************/


StateVariable::StateVariable()
	: m_type(String)
{
	m_value.pstr = NULL;
}


StateVariable::StateVariable(const string& name, string* ptrToString)
	: m_name(name)
	, m_type(String)
{
	m_value.pstr = ptrToString;
}


StateVariable::StateVariable(const string& name, bool* ptrToVar)
	: m_name(name)
	, m_type(Bool)
{
	m_value.pbool = ptrToVar;
}


StateVariable::StateVariable(const string& name, double* ptrToVar)
	: m_name(name)
	, m_type(Double)
{
	m_value.pdouble = ptrToVar;
}


StateVariable::StateVariable(const string& name, uint8_t* ptrToVar)
	: m_name(name)
	, m_type(UInt8)
{
	m_value.puint8 = ptrToVar;
}


StateVariable::StateVariable(const string& name, uint16_t* ptrToVar)
	: m_name(name)
	, m_type(UInt16)
{
	m_value.puint16 = ptrToVar;
}


StateVariable::StateVariable(const string& name, uint32_t* ptrToVar)
	: m_name(name)
	, m_type(UInt32)
{
	m_value.puint32 = ptrToVar;
}


StateVariable::StateVariable(const string& name, uint64_t* ptrToVar)
	: m_name(name)
	, m_type(UInt64)
{
	m_value.puint64 = ptrToVar;
}


StateVariable::StateVariable(const string& name, int8_t* ptrToVar)
	: m_name(name)
	, m_type(SInt8)
{
	m_value.psint8 = ptrToVar;
}


StateVariable::StateVariable(const string& name, int16_t* ptrToVar)
	: m_name(name)
	, m_type(SInt16)
{
	m_value.psint16 = ptrToVar;
}


StateVariable::StateVariable(const string& name, int32_t* ptrToVar)
	: m_name(name)
	, m_type(SInt32)
{
	m_value.psint32 = ptrToVar;
}


StateVariable::StateVariable(const string& name, int64_t* ptrToVar)
	: m_name(name)
	, m_type(SInt64)
{
	m_value.psint64 = ptrToVar;
}


StateVariable::StateVariable(const string& name, CustomStateVariableHandler* ptrToHandler)
	: m_name(name)
	, m_type(Custom)
{
	m_value.phandler = ptrToHandler;
}


enum StateVariable::Type StateVariable::GetType() const
{
	return m_type;
}


const string& StateVariable::GetName() const
{
	return m_name;
}


string StateVariable::GetTypeString() const
{
	switch (m_type) {
	case String:
		return "string";
	case Bool:
		return "bool";
	case Double:
		return "double";
	case UInt8:
		return "uint8";
	case UInt16:
		return "uint16";
	case UInt32:
		return "uint32";
	case UInt64:
		return "uint64";
	case SInt8:
		return "sint8";
	case SInt16:
		return "sint16";
	case SInt32:
		return "sint32";
	case SInt64:
		return "sint64";
	case Custom:
		return "custom";
	default:
		return "unknown";
	}
}


bool StateVariable::CopyValueFrom(const StateVariable& otherVariable)
{
	if (m_type != otherVariable.m_type)
		return false;

	switch (m_type) {
	case String:
		*m_value.pstr = *otherVariable.m_value.pstr;
		break;
	case Bool:
		*m_value.pbool = *otherVariable.m_value.pbool;
		break;
	case Double:
		*m_value.pdouble = *otherVariable.m_value.pdouble;
		break;
	case UInt8:
		*m_value.puint8 = *otherVariable.m_value.puint8;
		break;
	case UInt16:
		*m_value.puint16 = *otherVariable.m_value.puint16;
		break;
	case UInt32:
		*m_value.puint32 = *otherVariable.m_value.puint32;
		break;
	case UInt64:
		*m_value.puint64 = *otherVariable.m_value.puint64;
		break;
	case SInt8:
		*m_value.psint8 = *otherVariable.m_value.psint8;
		break;
	case SInt16:
		*m_value.psint16 = *otherVariable.m_value.psint16;
		break;
	case SInt32:
		*m_value.psint32 = *otherVariable.m_value.psint32;
		break;
	case SInt64:
		*m_value.psint64 = *otherVariable.m_value.psint64;
		break;
	case Custom:
		m_value.phandler->CopyValueFrom(otherVariable.m_value.phandler);
		break;
	default:
		// Unknown type?
		assert(false);
		return false;
	}

	return true;
}


string StateVariable::ToString() const
{
	stringstream sstr;

	switch (m_type) {
	case String:
		return m_value.pstr == NULL? "" : *m_value.pstr;
	case Bool:
		sstr << (*m_value.pbool? "true" : "false");
		return sstr.str();
	case Double:
		sstr << *m_value.pdouble;
		return sstr.str();
	case UInt8:
		sstr << (int) *m_value.puint8;
		return sstr.str();
	case UInt16:
		sstr.flags(std::ios::hex | std::ios::showbase);
		sstr << *m_value.puint16;
		return sstr.str();
	case UInt32:
		sstr.flags(std::ios::hex | std::ios::showbase);
		sstr << *m_value.puint32;
		return sstr.str();
	case UInt64:
		sstr.flags(std::ios::hex | std::ios::showbase);
		sstr << *m_value.puint64;
		return sstr.str();
	case SInt8:
		sstr << (int) *m_value.psint8;
		return sstr.str();
	case SInt16:
		sstr << *m_value.psint16;
		return sstr.str();
	case SInt32:
		sstr << *m_value.psint32;
		return sstr.str();
	case SInt64:
		sstr << *m_value.psint64;
		return sstr.str();
	case Custom:
		return "(custom)";
	}

	// Unimplemented type?
	assert(false);

	return "";
}


uint64_t StateVariable::ToInteger() const
{
	switch (m_type) {
	case String:
		{
			uint64_t tmp;
			stringstream ss;
			ss << *m_value.pstr;
			ss >> tmp;
			return tmp;
		}
	case Bool:
		return (*m_value.pbool)? 1 : 0;
	case Double:
		return (uint64_t) *m_value.pdouble;
	case UInt8:
		return *m_value.puint8;
	case UInt16:
		return *m_value.puint16;
	case UInt32:
		return *m_value.puint32;
	case UInt64:
		return *m_value.puint64;
	case SInt8:
		return *m_value.psint8;
	case SInt16:
		return *m_value.psint16;
	case SInt32:
		return *m_value.psint32;
	case SInt64:
		return *m_value.psint64;
	case Custom:
		return 0;
	}

	// Unimplemented type. Let's abort.
	std::cerr << "StateVariable::ToDouble(): Unimplemented type.\n";
	throw std::exception();
}


double StateVariable::ToDouble() const
{
	switch (m_type) {
	case String:
		{
			double tmp;
			stringstream ss;
			ss << *m_value.pstr;
			ss >> tmp;
			return tmp;
		}
	case Bool:
		return (*m_value.pbool)? 1.0 : 0.0;
	case Double:
		return *m_value.pdouble;
	case UInt8:
		return *m_value.puint8;
	case UInt16:
		return *m_value.puint16;
	case UInt32:
		return *m_value.puint32;
	case UInt64:
		return *m_value.puint64;
	case SInt8:
		return *m_value.psint8;
	case SInt16:
		return *m_value.psint16;
	case SInt32:
		return *m_value.psint32;
	case SInt64:
		return *m_value.psint64;
	case Custom:
		return 0.0;
	}

	// Unimplemented type. Let's abort.
	std::cerr << "StateVariable::ToDouble(): Unimplemented type.\n";
	throw std::exception();
}


void StateVariable::SerializeValue(ostream& ss) const
{
	switch (m_type) {

	case String:
		{
			EscapedString escaped(ToString());
			ss << escaped.Generate();
		}
		break;

	case Custom:
		m_value.phandler->Serialize(ss);
		break;

	default:
		ss << ToString();
	}
}


void StateVariable::Serialize(ostream& ss, SerializationContext& context) const
{
	ss << context.Tabs() << GetTypeString() << " " << m_name + " ";
	SerializeValue(ss);
	ss << "\n";
}


string StateVariable::EvaluateExpression(const string& expression,
	bool& success) const
{
	success = false;

	string result = expression;
	
	// Remove leading and trailing spaces:
	while (result.size() > 0 && result[0] == ' ')
		result.erase((size_t) 0);
	while (result.size() > 0 && result[result.size() - 1] == ' ')
		result.erase(result.size()-1);

// TODO
success = true;
return result;

	return "";
}


bool StateVariable::SetValue(const string& expression)
{
	// Nothing to assign to?
	if (m_value.pstr == NULL)
		return false;

	// Reduce the expression to a single value.
	bool success = false;
	string value = EvaluateExpression(expression, success);
	if (!success)
		return false;

	switch (m_type) {

	case String:
		{
			bool success = false;
			string newStr = EscapedString(value).Decode(success);
			if (success)
				*m_value.pstr = newStr;
			else
				return false;
		}
		return true;

	case Bool:
		{
			if (value == "true")
				*m_value.pbool = true;
			else if (value == "false")
				*m_value.pbool = false;
			else
				return false;
		}
		return true;

	case Double:
		{
			double doubleTmp;
			stringstream sstr;
			sstr << value;
			sstr >> doubleTmp;
			if (isnan(doubleTmp) || isinf(doubleTmp))
				return false;
			*m_value.pdouble = doubleTmp;
		}
		return true;

	case UInt8:
		{
			bool error = true;
			uint64_t tmp64 = parse_number(value.c_str(), error);
			uint8_t tmp = tmp64;
			if (tmp == tmp64 && !error)
				*m_value.puint8 = tmp;
			else
				return false;
		}
		return true;

	case UInt16:
		{
			bool error = true;
			uint64_t tmp64 = parse_number(value.c_str(), error);
			uint16_t tmp = tmp64;
			if (tmp == tmp64 && !error)
				*m_value.puint16 = tmp;
			else
				return false;
		}
		return true;
		
	case UInt32:
		{
			bool error = true;
			uint64_t tmp64 = parse_number(value.c_str(), error);
			uint32_t tmp = tmp64;
			if (tmp == tmp64 && !error)
				*m_value.puint32 = tmp;
			else
				return false;
		}
		return true;

	case UInt64:
		{
			bool error = true;
			uint64_t tmp64 = parse_number(value.c_str(), error);
			if (!error)
				*m_value.puint64 = tmp64;
			else
				return false;
		}
		return true;

	case SInt8:
		{
			bool error = true;
			int64_t tmp64 = parse_number(value.c_str(), error);
			int8_t tmp = tmp64;
			if (tmp == tmp64 && !error)
				*m_value.psint8 = tmp;
			else
				return false;
		}
		return true;

	case SInt16:
		{
			bool error = true;
			int64_t tmp64 = parse_number(value.c_str(), error);
			int16_t tmp = tmp64;
			if (tmp == tmp64 && !error)
				*m_value.psint16 = tmp;
			else
				return false;
		}
		return true;
		
	case SInt32:
		{
			bool error = true;
			int64_t tmp64 = parse_number(value.c_str(), error);
			int32_t tmp = tmp64;
			if (tmp == tmp64 && !error)
				*m_value.psint32 = tmp;
			else
				return false;
		}
		return true;

	case SInt64:
		{
			bool error = true;
			int64_t tmp64 = parse_number(value.c_str(), error);
			if (!error)
				*m_value.psint64 = tmp64;
			else
				return false;
		}
		return true;

	case Custom:
		return m_value.phandler->Deserialize(value);

	default:
		// Unimplemented type. Let's abort.
		std::cerr << "StateVariable::SetValue: Unimplemented type.\n";
		throw std::exception();
	}
}


bool StateVariable::SetValue(uint64_t value)
{
	// Nothing to assign to?
	if (m_value.pstr == NULL)
		return false;

	switch (m_type) {

	case String:
		{
			stringstream ss;
			ss << value;
			*m_value.pstr = ss.str();
		}
		return true;

	case Bool:
		*m_value.pbool = value != 0;
		return true;

	case Double:
		*m_value.pdouble = value;
		return true;

	case UInt8:
		*m_value.puint8 = value;
		return true;

	case UInt16:
		*m_value.puint16 = value;
		return true;
		
	case UInt32:
		*m_value.puint32 = value;
		return true;

	case UInt64:
		*m_value.puint64 = value;
		return true;

	case SInt8:
		*m_value.psint8 = value;
		return true;

	case SInt16:
		*m_value.psint16 = value;
		return true;
		
	case SInt32:
		*m_value.psint32 = value;
		return true;

	case SInt64:
		*m_value.psint64 = value;
		return true;

	case Custom:
		return false;

	default:
		// Unimplemented type. Let's abort.
		std::cerr << "StateVariable::SetValue: Unimplemented type.\n";
		throw std::exception();
	}
}


/*****************************************************************************/


#ifdef WITHUNITTESTS

static void Test_StateVariable_String_Construct()
{
	string myString = "hi";

	StateVariable var("hello", &myString);

	UnitTest::Assert("name should be hello",
	    var.GetName(), "hello");
	UnitTest::Assert("type should be String",
	    var.GetType() == StateVariable::String);
	UnitTest::Assert("value should be hi",
	    var.ToString(), "hi");
}

static void Test_StateVariable_String_SetValue()
{
	string myString = "hi";

	StateVariable var("hello", &myString);

	UnitTest::Assert("setting string value without quotes should not work",
	    var.SetValue("value2") == false);
	UnitTest::Assert("setting string value with quotes should work",
	    var.SetValue("\"value2\"") == true);

	UnitTest::Assert("type should still be String",
	    var.GetType() == StateVariable::String);
	UnitTest::Assert("value should now be value2",
	    var.ToString(), "value2");
	UnitTest::Assert("myString should have been updated",
	    myString, "value2");
}

static void Test_StateVariable_String_CopyValueFrom()
{
	string myString1 = "hi";
	string myString2 = "something";

	StateVariable var1("hello", &myString1);
	StateVariable var2("world", &myString2);

	UnitTest::Assert("value should initially be hi",
	    var1.ToString(), "hi");

	var1.CopyValueFrom(var2);

	UnitTest::Assert("name should still be hello",
	    var1.GetName(), "hello");
	UnitTest::Assert("type should still be String",
	    var1.GetType() == StateVariable::String);
	UnitTest::Assert("value should be changed to something",
	    var1.ToString(), "something");
	UnitTest::Assert("myString1 should have been updated",
	    myString1, "something");
}

static void Test_StateVariable_String_Serialize()
{
	string hi = "value world";
	StateVariable var("hello", &hi);

	SerializationContext dummyContext;
	stringstream ss;

	var.Serialize(ss, dummyContext);
	UnitTest::Assert("variable serialization mismatch?",
	    ss.str(), "string hello \"value world\"\n");
}

static void Test_StateVariable_String_Serialize_WithEscapes()
{
	string s = "a\\b\tc\nd\re\bf\"g'h";
	StateVariable var("hello", &s);

	SerializationContext dummyContext;
	stringstream ss;
	
	var.Serialize(ss, dummyContext);
	UnitTest::Assert("variable serialization mismatch?",
	    ss.str(),
	    "string hello " + EscapedString(s).Generate() + "\n");
}

static void Test_StateVariable_Bool_Construct()
{
	bool myBool = true;

	StateVariable var("hello", &myBool);

	UnitTest::Assert("name should be hello",
	    var.GetName(), "hello");
	UnitTest::Assert("type should be Bool",
	    var.GetType() == StateVariable::Bool);
	UnitTest::Assert("value should be true",
	    var.ToString(), "true");
}

static void Test_StateVariable_Bool_SetValue()
{
	bool myBool = true;

	StateVariable var("hello", &myBool);

	UnitTest::Assert("changing to false should be possible",
	    var.SetValue("false") == true);

	UnitTest::Assert("type should still be Bool",
	    var.GetType() == StateVariable::Bool);
	UnitTest::Assert("value should now be changed",
	    var.ToString(), "false");
	UnitTest::Assert("myBool should have been updated",
	    myBool == false);

	UnitTest::Assert("changing to true should be possible",
	    var.SetValue("true") == true);

	UnitTest::Assert("value should now be changed again",
	    var.ToString(), "true");
	UnitTest::Assert("myBool should have been updated again",
	    myBool == true);

	UnitTest::Assert("changing to non-bool value should not be possible",
	    var.SetValue("hello") == false);

	UnitTest::Assert("value should not be changed",
	    var.ToString(), "true");
}

static void Test_StateVariable_Bool_CopyValueFrom()
{
	bool myBool1 = false;
	bool myBool2 = true;

	StateVariable var1("hello", &myBool1);
	StateVariable var2("world", &myBool2);

	UnitTest::Assert("copying from bool to bool should be possible",
	    var1.CopyValueFrom(var2) == true);

	UnitTest::Assert("name should still be hello",
	    var1.GetName(), "hello");
	UnitTest::Assert("type should still be Bool",
	    var1.GetType() == StateVariable::Bool);
	UnitTest::Assert("value should be changed to true",
	    var1.ToString(), "true");
	UnitTest::Assert("myBool1 should have been updated",
	    myBool1 == true);

	string myString = "hm";
	StateVariable var3("test", &myString);

	UnitTest::Assert("copying from string to bool should not be possible",
	    var1.CopyValueFrom(var3) == false);
}

static void Test_StateVariable_Bool_Serialize()
{
	bool myBool = true;
	StateVariable var("hello", &myBool);

	SerializationContext dummyContext;
	stringstream ss;
	
	var.Serialize(ss, dummyContext);
	UnitTest::Assert("variable serialization mismatch (1)",
	    ss.str(), "bool hello true\n");
	
	myBool = false;
	stringstream ss2;
	var.Serialize(ss2, dummyContext);

	UnitTest::Assert("variable serialization mismatch (2)",
	    ss2.str(), "bool hello false\n");
}

static void Test_StateVariable_Numeric_Construct()
{
	double   varDouble = -12.345;
	uint8_t  varUInt8  = 223;
	uint16_t varUInt16 = 55000;
	uint32_t varUInt32 = 3000000001UL;
	uint64_t varUInt64 = ((uint64_t) 0xfedc0102 << 32) | 0x03040506;
	int8_t   varSInt8  = -120;
	int16_t  varSInt16 = -22000;
	int32_t  varSInt32 = -1000000001;
	int64_t  varSInt64 = ((uint64_t) 0xfedc0102 << 32) | 0x03040506;

	StateVariable vdouble("vdouble", &varDouble);
	StateVariable vuint8 ("vuint8",  &varUInt8);
	StateVariable vuint16("vuint16", &varUInt16);
	StateVariable vuint32("vuint32", &varUInt32);
	StateVariable vuint64("vuint64", &varUInt64);
	StateVariable vsint8 ("vsint8",  &varSInt8);
	StateVariable vsint16("vsint16", &varSInt16);
	StateVariable vsint32("vsint32", &varSInt32);
	StateVariable vsint64("vsint64", &varSInt64);

	// Types
	UnitTest::Assert("Double", vdouble.GetType() == StateVariable::Double);
	UnitTest::Assert("UInt8",  vuint8.GetType()  == StateVariable::UInt8);
	UnitTest::Assert("UInt16", vuint16.GetType() == StateVariable::UInt16);
	UnitTest::Assert("UInt32", vuint32.GetType() == StateVariable::UInt32);
	UnitTest::Assert("UInt64", vuint64.GetType() == StateVariable::UInt64);
	UnitTest::Assert("SInt8",  vsint8.GetType()  == StateVariable::SInt8);
	UnitTest::Assert("SInt16", vsint16.GetType() == StateVariable::SInt16);
	UnitTest::Assert("SInt32", vsint32.GetType() == StateVariable::SInt32);
	UnitTest::Assert("SInt64", vsint64.GetType() == StateVariable::SInt64);

	// Values
	UnitTest::Assert("value Double", vdouble.ToString(), "-12.345");
	UnitTest::Assert("value UInt8",  vuint8.ToString(),  "223");
	UnitTest::Assert("value UInt16", vuint16.ToString(), "0xd6d8");
	UnitTest::Assert("value UInt32", vuint32.ToString(), "0xb2d05e01");
	UnitTest::Assert("value UInt64", vuint64.ToString(),
	    "0xfedc010203040506");
	UnitTest::Assert("value SInt8",  vsint8.ToString(),  "-120");
	UnitTest::Assert("value SInt16", vsint16.ToString(), "-22000");
	UnitTest::Assert("value SInt32", vsint32.ToString(), "-1000000001");
	UnitTest::Assert("value SInt64", vsint64.ToString(),
	    "-82189585047354106");
}

static void Test_StateVariable_Numeric_SetValue()
{
	double   varDouble = -12.345;
	uint8_t  varUInt8  = 223;
	uint16_t varUInt16 = 55000;
	uint32_t varUInt32 = 3000000001UL;
	uint64_t varUInt64 = ((uint64_t) 0xfedc0102 << 32) | 0x03040506;
	int8_t   varSInt8  = -120;
	int16_t  varSInt16 = -22000;
	int32_t  varSInt32 = -1000000001;
	int64_t  varSInt64 = ((uint64_t) 0xfedc0102 << 32) | 0x03040506;

	StateVariable vdouble("vdouble", &varDouble);
	StateVariable vuint8 ("vuint8",  &varUInt8);
	StateVariable vuint16("vuint16", &varUInt16);
	StateVariable vuint32("vuint32", &varUInt32);
	StateVariable vuint64("vuint64", &varUInt64);
	StateVariable vsint8 ("vsint8",  &varSInt8);
	StateVariable vsint16("vsint16", &varSInt16);
	StateVariable vsint32("vsint32", &varSInt32);
	StateVariable vsint64("vsint64", &varSInt64);

	UnitTest::Assert("changing to 'hello' should not be possible",
	    vuint8.SetValue("hello") == false);

	// Double
	UnitTest::Assert("changing to 100 should be possible",
	    vdouble.SetValue("100") == true);
	UnitTest::Assert("varDouble should have been updated",
	    varDouble == 100);
	UnitTest::Assert("changing to -210.42 should be possible",
	    vdouble.SetValue("-210.42") == true);
	UnitTest::Assert("varDouble should not have been updated",
	    varDouble == -210.42);
	UnitTest::Assert("changing to 1e-100 should be possible (2)",
	    vdouble.SetValue("1e-100") == true);
	UnitTest::Assert("varDouble should have been updated (2)",
	    varDouble == 1e-100);

	// UInt8
	UnitTest::Assert("changing to 100 should be possible",
	    vuint8.SetValue("100") == true);
	UnitTest::Assert("varUInt8 should have been updated",
	    varUInt8, 100);
	UnitTest::Assert("changing to 0x2f should be possible",
	    vuint8.SetValue("0x2f") == true);
	UnitTest::Assert("varUInt8 should have been updated to 0x2f",
	    varUInt8, 0x2f);
	UnitTest::Assert("changing to 300 should not be possible",
	    vuint8.SetValue("300") == false);
	UnitTest::Assert("varUInt8 should not have been updated",
	    varUInt8, 0x2f);
	UnitTest::Assert("changing to -110 should not be possible",
	    vuint8.SetValue("-110") == false);
	UnitTest::Assert("varUInt8 should not have been updated",
	    varUInt8, 0x2f);

	// SInt8
	UnitTest::Assert("changing to 100 should be possible",
	    vsint8.SetValue("100") == true);
	UnitTest::Assert("varSInt8 should have been updated",
	    varSInt8, 100);
	UnitTest::Assert("changing to 200 should not be possible",
	    vsint8.SetValue("200") == false);
	UnitTest::Assert("varSInt8 should not have been updated",
	    varSInt8, 100);
	UnitTest::Assert("changing to -210 should not be possible",
	    vsint8.SetValue("-210") == false);
	UnitTest::Assert("varSInt8 should not have been updated",
	    varSInt8, 100);
	UnitTest::Assert("changing to -110 should be possible",
	    vsint8.SetValue("-110") == true);
	UnitTest::Assert("varSInt8 should have been updated",
	    varSInt8, (uint64_t) -110);
	UnitTest::Assert("changing to -0x1a should be possible",
	    vsint8.SetValue("-0x1a") == true);
	UnitTest::Assert("varSInt8 should have been updated",
	    varSInt8, (uint64_t) -0x1a);

	// Tests for other numeric types: TODO
}

UNITTESTS(StateVariable)
{
	// String tests
	UNITTEST(Test_StateVariable_String_Construct);
	UNITTEST(Test_StateVariable_String_SetValue);
	UNITTEST(Test_StateVariable_String_CopyValueFrom);
	UNITTEST(Test_StateVariable_String_Serialize);
	UNITTEST(Test_StateVariable_String_Serialize_WithEscapes);

	// Bool tests
	UNITTEST(Test_StateVariable_Bool_Construct);
	UNITTEST(Test_StateVariable_Bool_SetValue);
	UNITTEST(Test_StateVariable_Bool_CopyValueFrom);
	UNITTEST(Test_StateVariable_Bool_Serialize);

	// Numeric tests
	UNITTEST(Test_StateVariable_Numeric_Construct);
	UNITTEST(Test_StateVariable_Numeric_SetValue);
	//UNITTEST(Test_StateVariable_Numeric_CopyValueFrom);
	//UNITTEST(Test_StateVariable_Numeric_Serialize);

	// TODO: ToInteger tests.

	// TODO: Custom tests.
}

#endif

