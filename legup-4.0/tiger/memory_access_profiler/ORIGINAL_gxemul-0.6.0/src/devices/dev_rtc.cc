/*
 *  Copyright (C) 2006-2009  Anders Gavare.  All rights reserved.
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
 *  COMMENT: A generic Real-Time Clock device, for the test machines
 *
 *  It can be used to retrieve the current system time, and to cause periodic
 *  interrupts.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "cpu.h"
#include "device.h"
#include "emul.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"
#include "timer.h"

#include "testmachine/dev_rtc.h"


#define	DEV_RTC_TICK_SHIFT	14

struct rtc_data {
	struct interrupt	irq;
	int			pending_interrupts;

	int			hz;
	struct timer		*timer;

	struct timeval		cur_time;	
};


/*
 *  timer_tick():
 *
 *  This function is called d->hz times per second.
 */
static void timer_tick(struct timer *t, void *extra)
{
        struct rtc_data *d = (struct rtc_data *) extra;
        d->pending_interrupts ++;
}


DEVICE_TICK(rtc)
{  
	struct rtc_data *d = (struct rtc_data *) extra;

	if (d->pending_interrupts > 0)
		INTERRUPT_ASSERT(d->irq);
	else
		INTERRUPT_DEASSERT(d->irq);
}


DEVICE_ACCESS(rtc)
{
	struct rtc_data *d = (struct rtc_data *) extra;
	uint64_t idata = 0, odata = 0;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	switch (relative_addr) {

	case DEV_RTC_TRIGGER_READ:
		gettimeofday(&d->cur_time, NULL);
		break;

	case DEV_RTC_SEC:
		odata = d->cur_time.tv_sec;
		break;

	case DEV_RTC_USEC:
		odata = d->cur_time.tv_usec;
		break;

	case DEV_RTC_HZ:
		if (writeflag == MEM_READ) {
			odata = d->hz;
		} else {
			d->hz = idata;

			if (d->hz == 0) {
				/*  Remove the timer, if any:  */
				if (d->timer != NULL)
					timer_remove(d->timer);

				d->timer = NULL;
				d->pending_interrupts = 0;
			} else {
				/*  Add a timer, or update the existing one:  */
				if (d->timer == NULL)
					d->timer = timer_add(d->hz,
					    timer_tick, d);
				else
					timer_update_frequency(d->timer, d->hz);
			}
		}
		break;

	case DEV_RTC_INTERRUPT_ACK:
		if (d->pending_interrupts > 0)
			d->pending_interrupts --;

		INTERRUPT_DEASSERT(d->irq);

		/*  TODO: Reassert the interrupt here, if
		    d->pending_interrupts is still above zero?  */

		break;

	default:if (writeflag == MEM_WRITE) {
			fatal("[ rtc: unimplemented write to "
			    "offset 0x%x: data=0x%x ]\n", (int)
			    relative_addr, (int)idata);
		} else {
			fatal("[ rtc: unimplemented read from "
			    "offset 0x%x ]\n", (int)relative_addr);
		}
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVINIT(rtc)
{
	struct rtc_data *d;

	CHECK_ALLOCATION(d = (struct rtc_data *) malloc(sizeof(struct rtc_data)));
	memset(d, 0, sizeof(struct rtc_data));

	INTERRUPT_CONNECT(devinit->interrupt_path, d->irq);

	memory_device_register(devinit->machine->memory, devinit->name,
	    devinit->addr, DEV_RTC_LENGTH, dev_rtc_access, (void *)d,
	    DM_DEFAULT, NULL);

	machine_add_tickfunction(devinit->machine,
	    dev_rtc_tick, d, DEV_RTC_TICK_SHIFT);

	return 1;
}

