#ifndef	SCOPEDTEMPORARYVALUE_H
#define	SCOPEDTEMPORARYVALUE_H

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

/**
 * \brief Scoped temporary variable template.
 *
 * Usage:<pre>
 * SomeType var = A;
 * {
 *	ScopedTemporaryValue<SomeType> holder(var);
 *	var = B;
 *	...
 * }
 * // var will here be reset to A.
 * </pre>
 * or
 * <pre>
 * SomeType var = A;
 * {
 *	ScopedTemporaryValue<SomeType> holder(var, B);
 *	// Here,  var is B.
 *	...
 * }
 * // var will here be reset to A.
 * </pre>
 *
 * Implementation note: This is just a schoolbook-style implementation
 * of a class which holds a variable, and then restores the original value
 * when going out of scope.
 */
template <class T>
class ScopedTemporaryValue
{
private:
	// Prevent construction without reference.
	ScopedTemporaryValue();

public:
	/**
	 * \brief Constructor, which reads the old value from T, but
	 * does not change it.
	 *
	 * @param var The variable.
 	 */
	ScopedTemporaryValue(T& var)
		: m_var(var)
	{
		m_origValue = m_var;
	}

	/**
	 * \brief Constructor, which reads the old value from T, and sets
	 * it to a new (temporary) value.
	 *
	 * @param var The variable.
	 * @param newValue The new (temporary) value.
 	 */
	ScopedTemporaryValue(T& var, T newValue)
		: m_var(var)
	{
		m_origValue = m_var;
		m_var = newValue;
	}

	/**
	 * \brief Destructor, which restores the original value.
	 */
	~ScopedTemporaryValue()
	{
		m_var = m_origValue;
	}

private:
	T&		m_var;
	T		m_origValue;
};


#endif	// SCOPEDTEMPORARYVALUE_H
