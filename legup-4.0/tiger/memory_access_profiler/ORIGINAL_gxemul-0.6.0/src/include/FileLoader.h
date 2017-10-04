#ifndef FILELOADER_H
#define	FILELOADER_H

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

#include "misc.h"

#include "Component.h"
#include "FileLoaderImpl.h"
#include "UnitTest.h"


/**
 * \brief A class used to load binary files into emulated memory.
 *
 * A %FileLoader is given a name of a binary file, e.g. an ELF file, and
 * attempts to load it into emulated memory.
 *
 * Binary files may be loaded to either physical or virtual memory addresses.
 * To load to a physical address, we want to write to a component which
 * is an AddressDataBus directly. To load to virtual addresses, we need to go
 * via a particular CPU, because it is the CPU which does the virtual to
 * physical address translation.
 *
 * The type of the file is normally auto-detected by reading magic sequences
 * at the start of the file.
 */
class FileLoader
	: public UnitTestable
{
public:
	/**
	 * \brief Constructs a %FileLoader object.
	 *
	 * \param filename The name of the file to open.
	 */
	FileLoader(const string& filename);

	/**
	 * \brief Retrieves the filename of this %FileLoader.
	 *
	 * \return The filename.
	 */
	const string& GetFilename() const;

	/**
	 * \brief Attempt to detect the file format of the file.
	 *
	 * \param loader On return from the function, if the file format has
	 *	been detected, this is set to a pointer to a FileLoaderImpl.
	 *	Otherwise (if no format was detected), it is NULL.
	 * \return A string representing the file format.
	 */
	string DetectFileFormat(refcount_ptr<const FileLoaderImpl>& loader) const;

	/**
	 * \brief Loads the file into a CPU or an AddressDataBus.
	 *
	 * Note: The file is usually loaded into a virtual address space.
	 * It is therefore necessary to load it into a CPU, and not directly
	 * into RAM.
	 *
	 * @param component A CPUComponent (or any other
	 *	component which implements the AddressDataBus interface),
	 *	into which the file will be loaded.
	 * @param messages An output stream where debug messages can be put.
	 * @return True if loading succeeded, false otherwise.
	 */
	bool Load(refcount_ptr<Component> component, ostream& messages) const;


	/********************************************************************/

	static void RunUnitTests(int& nSucceeded, int& nFailures);

private:
	// Disallow construction without arguments.
	FileLoader();

private:
	typedef vector< refcount_ptr<const FileLoaderImpl> > FileLoaderImplVector;
	const string		m_filename;
	FileLoaderImplVector	m_fileLoaders;
};


#endif	// FILELOADER_H
