#ifndef ADDRESSDATABUS_H
#define	ADDRESSDATABUS_H

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


/**
 * \brief An interface for implementing components that read/write data via an
 *	address bus.
 *
 * Any component which allows data to be read or written by using an address
 * as an index/offset should implement this interface.
 *
 * A typical example of a Component which implements the functions
 * of this class is the RAMComponent.
 */
class AddressDataBus
{
public:
	/**
	 * \brief Constructs an AddressDataBus instance.
	 */
	AddressDataBus() { }

	virtual ~AddressDataBus() { }

	/**
	 * \brief Place an address on the bus.
	 *
	 * \param address The address to select.
	 */
	virtual void AddressSelect(uint64_t address) = 0;

	/**
	 * \brief Reads 8-bit data from the currently selected address.
	 *
	 * \param data A reference to a variable which will receive the data.
	 * \param endianness Selects the endianness of the operation. Ignored
	 *	for 8-bit reads and writes.
	 * \return True if the access was successful, false otherwise (e.g.
	 *	because of a timeout).
	 */
	virtual bool ReadData(uint8_t& data, Endianness endianness = BigEndian) = 0;

	/**
	 * \brief Reads 16-bit data from the currently selected address.
	 *
	 * \param data A reference to a variable which will receive the data.
	 * \param endianness Selects the endianness of the operation.
	 * \return True if the access was successful, false otherwise (e.g.
	 *	because of a timeout).
	 */
	virtual bool ReadData(uint16_t& data, Endianness endianness) = 0;

	/**
	 * \brief Reads 32-bit data from the currently selected address.
	 *
	 * \param data A reference to a variable which will receive the data.
	 * \param endianness Selects the endianness of the operation.
	 * \return True if the access was successful, false otherwise (e.g.
	 *	because of a timeout).
	 */
	virtual bool ReadData(uint32_t& data, Endianness endianness) = 0;

	/**
	 * \brief Reads 64-bit data from the currently selected address.
	 *
	 * \param data A reference to a variable which will receive the data.
	 * \param endianness Selects the endianness of the operation.
	 * \return True if the access was successful, false otherwise (e.g.
	 *	because of a timeout).
	 */
	virtual bool ReadData(uint64_t& data, Endianness endianness) = 0;

	/**
	 * \brief Writes 8-bit data to the currently selected address.
	 *
	 * \param data A reference to a variable which contains the data.
	 * \param endianness Selects the endianness of the operation. Ignored
	 *	for 8-bit reads and writes.
	 * \return True if the access was successful, false otherwise (e.g.
	 *	because of a timeout).
	 */
	virtual bool WriteData(const uint8_t& data, Endianness endianness = BigEndian) = 0;

	/**
	 * \brief Writes 16-bit data to the currently selected address.
	 *
	 * \param data A reference to a variable which contains the data.
	 * \param endianness Selects the endianness of the operation.
	 * \return True if the access was successful, false otherwise (e.g.
	 *	because of a timeout).
	 */
	virtual bool WriteData(const uint16_t& data, Endianness endianness) = 0;

	/**
	 * \brief Writes 32-bit data to the currently selected address.
	 *
	 * \param data A reference to a variable which contains the data.
	 * \param endianness Selects the endianness of the operation.
	 * \return True if the access was successful, false otherwise (e.g.
	 *	because of a timeout).
	 */
	virtual bool WriteData(const uint32_t& data, Endianness endianness) = 0;

	/**
	 * \brief Writes 64-bit data to the currently selected address.
	 *
	 * \param data A reference to a variable which contains the data.
	 * \param endianness Selects the endianness of the operation.
	 * \return True if the access was successful, false otherwise (e.g.
	 *	because of a timeout).
	 */
	virtual bool WriteData(const uint64_t& data, Endianness endianness) = 0;
};


#endif	// ADDRESSDATABUS_H
