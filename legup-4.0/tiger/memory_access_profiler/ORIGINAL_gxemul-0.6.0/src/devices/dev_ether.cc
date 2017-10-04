/*
 *  Copyright (C) 2005-2009  Anders Gavare.  All rights reserved.
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
 *  COMMENT: A simple ethernet controller, for the test machines
 *
 *  Basic "ethernet" network device. This is a simple test device which can
 *  be used to send and receive packets to/from a simulated ethernet network.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "device.h"
#include "emul.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"
#include "net.h"

#include "testmachine/dev_ether.h"


#define	DEV_ETHER_TICK_SHIFT	14

struct ether_data {
	unsigned char		buf[DEV_ETHER_BUFFER_SIZE];
	unsigned char		mac[6];

	int			status;
	int			packet_len;

	struct interrupt	irq;
};


DEVICE_TICK(ether)
{  
	struct ether_data *d = (struct ether_data *) extra;
	int r = 0;

	d->status &= ~DEV_ETHER_STATUS_MORE_PACKETS_AVAILABLE;
	if (cpu->machine->emul->net != NULL)
		r = net_ethernet_rx_avail(cpu->machine->emul->net, d);
	if (r)
		d->status |= DEV_ETHER_STATUS_MORE_PACKETS_AVAILABLE;

	if (d->status)
		INTERRUPT_ASSERT(d->irq);
	else
		INTERRUPT_DEASSERT(d->irq);
}


DEVICE_ACCESS(ether_buf)
{
	struct ether_data *d = (struct ether_data *) extra;

	if (writeflag == MEM_WRITE)
		memcpy(d->buf + relative_addr, data, len);
	else
		memcpy(data, d->buf + relative_addr, len);

	return 1;
}


DEVICE_ACCESS(ether)
{
	struct ether_data *d = (struct ether_data *) extra;
	uint64_t idata = 0, odata = 0;
	unsigned char *incoming_ptr;
	int incoming_len;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	/*  Note: relative_addr + DEV_ETHER_BUFFER_SIZE to get the same
	    offsets as in the header file:  */

	switch (relative_addr + DEV_ETHER_BUFFER_SIZE) {

	case DEV_ETHER_STATUS:
		if (writeflag == MEM_READ) {
			odata = d->status;
			d->status = 0;
			INTERRUPT_DEASSERT(d->irq);
		} else
			fatal("[ ether: WARNING: write to status ]\n");
		break;

	case DEV_ETHER_PACKETLENGTH:
		if (writeflag == MEM_READ)
			odata = d->packet_len;
		else {
			if ((int64_t)idata < 0) {
				fatal("[ ether: ERROR: packet len too"
				    " short (%i bytes) ]\n", (int)idata);
				idata = (uint64_t) -1;
			}
			if (idata > DEV_ETHER_BUFFER_SIZE) {
				fatal("[ ether: ERROR: packet len too"
				    " large (%i bytes) ]\n", (int)idata);
				idata = DEV_ETHER_BUFFER_SIZE;
			}
			d->packet_len = idata;
		}
		break;

	case DEV_ETHER_COMMAND:
		if (writeflag == MEM_READ) {
			fatal("[ ether: WARNING: read from command ]\n");
			break;
		}

		/*  Write:  */
		switch (idata) {

		case DEV_ETHER_COMMAND_RX:	/*  Receive:  */
			if (cpu->machine->emul->net == NULL)
				fatal("[ ether: RECEIVE but no net? ]\n");
			else {
				d->status &= ~DEV_ETHER_STATUS_PACKET_RECEIVED;
				if (net_ethernet_rx(cpu->machine->emul->net,
				    d, &incoming_ptr, &incoming_len)) {
					d->status |=
					    DEV_ETHER_STATUS_PACKET_RECEIVED;
					if (incoming_len>DEV_ETHER_BUFFER_SIZE)
						incoming_len =
						    DEV_ETHER_BUFFER_SIZE;
					memcpy(d->buf, incoming_ptr,
					    incoming_len);
					free(incoming_ptr);
					d->packet_len = incoming_len;
				}
			}
			dev_ether_tick(cpu, d);
			break;

		case DEV_ETHER_COMMAND_TX:	/*  Send  */
			if (cpu->machine->emul->net == NULL)
				fatal("[ ether: SEND but no net? ]\n");
			else
				net_ethernet_tx(cpu->machine->emul->net,
				    d, d->buf, d->packet_len);
			d->status &= ~DEV_ETHER_STATUS_PACKET_RECEIVED;
			dev_ether_tick(cpu, d);
			break;

		default:fatal("[ ether: UNIMPLEMENTED command 0x"
			    "%02x ]\n", idata);
			cpu->running = 0;
		}
		break;

	case DEV_ETHER_MAC:
		if (writeflag == MEM_READ) {
			fatal("[ ether: read of MAC is not allowed! ]\n");
		} else {
			// Write out the MAC address to the address given.
			cpu->memory_rw(cpu, cpu->mem, idata, d->mac,
			    6, MEM_WRITE, CACHE_NONE);
		}
		break;

	default:if (writeflag == MEM_WRITE) {
			fatal("[ ether: unimplemented write to "
			    "offset 0x%x: data=0x%x ]\n", (int)
			    relative_addr, (int)idata);
		} else {
			fatal("[ ether: unimplemented read from "
			    "offset 0x%x ]\n", (int)relative_addr);
		}
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVINIT(ether)
{
	struct ether_data *d;
	size_t nlen;
	char *n1, *n2;
	char tmp[50];

	CHECK_ALLOCATION(d = (struct ether_data *) malloc(sizeof(struct ether_data)));
	memset(d, 0, sizeof(struct ether_data));

	nlen = strlen(devinit->name) + 80;
	CHECK_ALLOCATION(n1 = (char *) malloc(nlen));
	CHECK_ALLOCATION(n2 = (char *) malloc(nlen));

	INTERRUPT_CONNECT(devinit->interrupt_path, d->irq);

	net_generate_unique_mac(devinit->machine, d->mac);
	snprintf(tmp, sizeof(tmp), "%02x:%02x:%02x:%02x:%02x:%02x",
	    d->mac[0], d->mac[1], d->mac[2], d->mac[3], d->mac[4], d->mac[5]);

	snprintf(n1, nlen, "%s [%s]", devinit->name, tmp);
	snprintf(n2, nlen, "%s [%s, control]", devinit->name, tmp);

	memory_device_register(devinit->machine->memory, n1, devinit->addr,
	    DEV_ETHER_BUFFER_SIZE, dev_ether_buf_access, (void *)d,
	    DM_DYNTRANS_OK | DM_DYNTRANS_WRITE_OK |
	    DM_READS_HAVE_NO_SIDE_EFFECTS, d->buf);
	memory_device_register(devinit->machine->memory, n2,
	    devinit->addr + DEV_ETHER_BUFFER_SIZE,
	    DEV_ETHER_LENGTH-DEV_ETHER_BUFFER_SIZE, dev_ether_access, (void *)d,
	    DM_DEFAULT, NULL);

	net_add_nic(devinit->machine->emul->net, d, d->mac);

	machine_add_tickfunction(devinit->machine,
	    dev_ether_tick, d, DEV_ETHER_TICK_SHIFT);

	return 1;
}

