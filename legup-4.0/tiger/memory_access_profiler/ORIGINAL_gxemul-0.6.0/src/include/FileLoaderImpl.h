#ifndef FILELOADERIMPL_H
#define	FILELOADERIMPL_H

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

#include "Component.h"
#include "UnitTest.h"


/**
 * \brief A file loader.
 */
class FileLoaderImpl
	: public ReferenceCountable
{
public:
	/**
	 * \brief Constructs a file loader.
	 *
	 * @param filename The filename to load.
	 */
	FileLoaderImpl(const string& filename)
		: m_filename(filename)
	{
	}

	virtual ~FileLoaderImpl()
	{
	}

	/**
	 * \brief Attempt to detect file type.
	 *
	 * @param buf A buffer containing the header of the file.
	 * @param buflen The length of the buffer.
	 * @param matchness Set to a value between 0.0 and 1.0, indicating the
	 *	match certainty.
	 * @return A file type description, if there was a match; otherwise
	 * an empty string.
	 */
	virtual string DetectFileType(unsigned char *buf, size_t buflen, float& matchness) const = 0;

	/**
	 * \brief Loads the file into a Component.
	 *
	 * @param component The AddressDataBus component to load the file
	 *	into. (This is usually a CPUComponent.)
	 * @param messages An output stream where debug messages can be put.
	 * @return True if loading succeeded, false otherwise.
	 */
	virtual bool LoadIntoComponent(refcount_ptr<Component> component, ostream& messages) const = 0;

protected:
	const string& Filename() const
	{
		return m_filename;
	}

private:
	const string	m_filename;
};

#endif	// FILELOADERIMPL_H
