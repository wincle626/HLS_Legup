#ifndef STATEVARIABLE_H
#define	STATEVARIABLE_H

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
 *
 *
 *  Note/TODO: This class could perhaps be rewritten in a nicer way by
 *             using C++ templates.
 */

#include "misc.h"

#include "SerializationContext.h"
#include "UnitTest.h"


class StateVariable;
typedef map<string,StateVariable> StateVariableMap;


class CustomStateVariableHandler
{
public:
	CustomStateVariableHandler()
	{
	}

	virtual ~CustomStateVariableHandler()
	{
	}

	virtual void Serialize(ostream& ss) const = 0;
	virtual bool Deserialize(const string& value) = 0;
	virtual void CopyValueFrom(CustomStateVariableHandler* other) = 0;
};


/**
 * \brief StateVariables make up the persistent state of Component objects.
 *
 * A %StateVariable has a name, and a value.
 */
class StateVariable
	: public UnitTestable
{
public:
	/**
	 * \brief An enumeration of the possible types of a %StateVariable.
	 */
	enum Type {
		String = 0,
		Bool,
		Double,
		UInt8,
		UInt16,
		UInt32,
		UInt64,
		SInt8,
		SInt16,
		SInt32,
		SInt64,
		Custom
	};

public:
	/**
	 * \brief Default constructor.
	 */
	StateVariable();

	/**
	 * \brief Constructor for a String StateVariable.
	 *
	 * @param name The variable's name.
	 * @param ptrToString A pointer to the string which this
	 *	variable refers to.
	 */
	StateVariable(const string& name, string* ptrToString);

	/**
	 * \brief Constructor for a Bool StateVariable.
	 *
	 * @param name The variable's name.
	 * @param ptrToVar A pointer to the variable.
	 */
	StateVariable(const string& name, bool* ptrToVar);

	/**
	 * \brief Constructor for a Double StateVariable.
	 *
	 * @param name The variable's name.
	 * @param ptrToVar A pointer to the variable.
	 */
	StateVariable(const string& name, double* ptrToVar);

	/**
	 * \brief Constructor for a UInt8 StateVariable.
	 *
	 * @param name The variable's name.
	 * @param ptrToVar A pointer to the variable.
	 */
	StateVariable(const string& name, uint8_t* ptrToVar);

	/**
	 * \brief Constructor for a UInt16 StateVariable.
	 *
	 * @param name The variable's name.
	 * @param ptrToVar A pointer to the variable.
	 */
	StateVariable(const string& name, uint16_t* ptrToVar);

	/**
	 * \brief Constructor for a UInt32 StateVariable.
	 *
	 * @param name The variable's name.
	 * @param ptrToVar A pointer to the variable.
	 */
	StateVariable(const string& name, uint32_t* ptrToVar);

	/**
	 * \brief Constructor for a UInt64 StateVariable.
	 *
	 * @param name The variable's name.
	 * @param ptrToVar A pointer to the variable.
	 */
	StateVariable(const string& name, uint64_t* ptrToVar);

	/**
	 * \brief Constructor for a SInt8 StateVariable.
	 *
	 * @param name The variable's name.
	 * @param ptrToVar A pointer to the variable.
	 */
	StateVariable(const string& name, int8_t* ptrToVar);

	/**
	 * \brief Constructor for a SInt16 StateVariable.
	 *
	 * @param name The variable's name.
	 * @param ptrToVar A pointer to the variable.
	 */
	StateVariable(const string& name, int16_t* ptrToVar);

	/**
	 * \brief Constructor for a SInt32 StateVariable.
	 *
	 * @param name The variable's name.
	 * @param ptrToVar A pointer to the variable.
	 */
	StateVariable(const string& name, int32_t* ptrToVar);

	/**
	 * \brief Constructor for a SInt64 StateVariable.
	 *
	 * @param name The variable's name.
	 * @param ptrToVar A pointer to the variable.
	 */
	StateVariable(const string& name, int64_t* ptrToVar);

	/**
	 * \brief Constructor for a custom StateVariable.
	 *
	 * @param name The variable's name.
	 * @param ptrToHandler A pointer to the custom handler.
	 */
	StateVariable(const string& name, CustomStateVariableHandler* ptrToHandler);

	/**
	 * \brief Gets the name of the variable.
	 *
	 * @return The name of the variable.
	 */
	const string& GetName() const;

	/**
	 * \brief Gets the type of the variable.
	 *
	 * @return The type of the variable.
	 */
	enum Type GetType() const;

	/**
	 * \brief Serializes the variable value into a string.
	 *
	 * Strings are serialized as quoted strings; most other types without
	 * quoting.
	 *
	 * @param ss A stream where the variable value will be appended.
	 */
	void SerializeValue(ostream& ss) const;

	/**
	 * \brief Serializes the variable into a string.
	 *
	 * @param ss A stream where the variable (type, name, and value)
	 *	will be appended.
	 * @param context Serialization context (tab indentation, etc).
	 */
	void Serialize(ostream& ss, SerializationContext& context) const;

	/**
	 * \brief Copy the value from another variable into this variable.
	 *
	 * @param otherVariable The variable to copy from.
	 * @return True if the value was copied, false if copying was not
	 *	possible (i.e. the types differed).
	 */
	bool CopyValueFrom(const StateVariable& otherVariable);

	/**
	 * \brief Returns the variable as a readable string.
	 *
	 * @return A string, representing the variable's value.
	 */
	string ToString() const;

	/**
	 * \brief Returns the variable as an unsignedinteger value.
	 *
	 * @return A uint64_t, representing the variable's value.
	 */
	uint64_t ToInteger() const;

	/**
	 * \brief Returns the variable as a double value.
	 *
	 * @return A double, representing the variable's value.
	 */
	double ToDouble() const;

	/**
	 * \brief Set the variable's value, using a string expression.
	 *
	 * Note that the expression may be a single value (e.g. <tt>42</tt>),
	 * or something more complex (e.g. <tt>123 + cpu0.pc * 4</tt>).
	 *
	 * When setting string values, the expression should be a C-style
	 * escaped string, e.g.: <tt>"This is a string with a \" inside
	 * it."</tt>
	 *
	 * @param expression The new value.
	 * @return True if the value was set, false if e.g. there was a
	 *	parse error.
	 */
	bool SetValue(const string& expression);

	/**
	 * \brief Set the variable's value to an integer.
	 *
	 * @param value The new value.
	 * @return True if the value was set, false if e.g. there was a
	 *	type error.
	 */
	bool SetValue(uint64_t value);


	/********************************************************************/

	static void RunUnitTests(int& nSucceeded, int& nFailures);

private:
	/**
	 * \brief Evaluates an expression, and returns it as a single value.
	 *
	 * @param expression The expression to evaluate.
	 * @param success Set to true if the expression could be evaluated,
	 *	false otherwise.
	 * @return A string representation of the evaluated expression.
	 *	(Only valid if <tt>success</tt> was set to true.)
	 */
	string EvaluateExpression(const string& expression,
		bool &success) const;

	/**
	 * \brief Returns the type of the variable, as a string.
	 *
	 * @return A string representation of the variable's type.
	 */
	string GetTypeString() const;

	/**
	 * \brief Returns a string representation of the variable's value.
	 *
	 * @return A string representation of the variable's value.
	 */
	string ValueToString() const;

private:
	string			m_name;
	enum Type		m_type;
	
	union {
		string*		pstr;
		bool*		pbool;
		double*		pdouble;
		uint8_t*	puint8;
		uint16_t*	puint16;
		uint32_t*	puint32;
		uint64_t*	puint64;
		int8_t*		psint8;
		int16_t*	psint16;
		int32_t*	psint32;
		int64_t*	psint64;
		CustomStateVariableHandler *phandler;
	} m_value;
};


#endif	// STATEVARIABLE_H
