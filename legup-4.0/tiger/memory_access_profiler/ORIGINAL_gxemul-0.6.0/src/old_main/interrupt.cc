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
 *  The interrupt subsystem.
 *
 *  Interrupts have a "path", e.g. "machine[0].cpu.5". A device which
 *  wishes to cause this interrupt needs to connect to it.
 *
 *  The possible interrupt paths are registered by CPUs, interrupt controllers,
 *  etc., that have a way of receiving interrupt requests. The physical
 *  version of an interrupt path is usually a "pin" on the CPU, or similar.
 *
 *  Once connected, the interrupt can be asserted or deasserted.
 *
 *  For examples on how it is used, see the various devices in src/devices/.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "interrupt.h"
#include "misc.h"


/*  #define INTERRUPT_DEBUG  */


struct interrupt_handler {
	struct interrupt	templ;
	int			nr_of_exclusive_users;
	int			nr_of_nonexclusive_users;
};


static int nr_of_interrupt_handlers = 0;
static struct interrupt_handler *interrupt_handlers = NULL;


/*
 *  Dummy interrupt assert/deassert for "no interrupt" interrupts:
 */
static void no_interrupt_assert(struct interrupt *i) { }
static void no_interrupt_deassert(struct interrupt *i) { }


/*
 *  interrupt_handler_register():
 *
 *  Add an interrupt handler to the interrupt subsystem. The template
 *  needs to have all members set.
 *
 *  Name is of the form "machine[0].cpu[0].irq[3].isa[14]" etc.
 *
 *  If there already is a handler with this name, the emulator aborts.
 */
void interrupt_handler_register(struct interrupt *templ)
{
	int i;

#ifdef INTERRUPT_DEBUG
	printf("interrupt_handler_register(\"%s\")\n", templ->name);
#endif

	/*  See if the name is already registered:  */
	for (i=0; i<nr_of_interrupt_handlers; i++) {
		if (strcmp(templ->name,
		    interrupt_handlers[i].templ.name) != 0)
			continue;

		fatal("\ninterrupt_handler_register(): An interrupt handler"
		    " using the name '%s' is already registered.\n",
		    templ->name);
		exit(1);
	}

	nr_of_interrupt_handlers ++;
	CHECK_ALLOCATION(interrupt_handlers = 
	    (struct interrupt_handler *) realloc(interrupt_handlers,
	    nr_of_interrupt_handlers * sizeof(struct interrupt_handler)));

	interrupt_handlers[nr_of_interrupt_handlers-1].templ = *templ;
	CHECK_ALLOCATION(interrupt_handlers[nr_of_interrupt_handlers-1].
	    templ.name = strdup(templ->name));
}


/*
 *  interrupt_handler_remove():
 *
 *  Remove an interrupt handler from the interrupt subsystem. If there are
 *  still connected users of this interrupt, then an error message is printed
 *  and the emulator aborts.
 */
void interrupt_handler_remove(const char *name)
{
	int i;

#ifdef INTERRUPT_DEBUG
	printf("interrupt_handler_remove(\"%s\")\n", name);
#endif

	for (i=0; i<nr_of_interrupt_handlers; i++) {
		if (strcmp(name, interrupt_handlers[i].templ.name) != 0)
			continue;

		/*
		 *  Remove handler i, and return:
		 */
		if (interrupt_handlers[i].nr_of_exclusive_users > 0 ||
		    interrupt_handlers[i].nr_of_nonexclusive_users > 0) {
			fatal("interrupt_handler_remove(): Attempt to "
			    "remove interrupt handler '%s' which has %i "
			    "exclusive and %i non-exclusive users. Aborting.\n",
			    name, interrupt_handlers[i].nr_of_exclusive_users,
			    interrupt_handlers[i].nr_of_nonexclusive_users);
			exit(1);
		}

		if (i != nr_of_interrupt_handlers-1)
			memcpy(&interrupt_handlers[i],
			    &interrupt_handlers[i + 1],
			    nr_of_interrupt_handlers - i - 1);

		nr_of_interrupt_handlers --;

		return;
	}

	fatal("interrupt_handler_remove(): '%s' not found? Aborting.\n", name);
	exit(1);
}


/*
 *  interrupt_handler_lookup():
 *
 *  Scans the list of registered interrupt handlers for a given name. If the
 *  name is found, the template is filled with valid data, and 1 is returned.
 *  If the name is not found, 0 is returned.
 */
int interrupt_handler_lookup(const char *name, struct interrupt *templ)
{
	int i;

#ifdef INTERRUPT_DEBUG
	printf("interrupt_handler_lookup(\"%s\")\n", name);
#endif

	if (name[0] == '\0') {
		/*  No interrupt:  */
		memset(templ, 0, sizeof(struct interrupt));
		templ->interrupt_assert = no_interrupt_assert;
		templ->interrupt_deassert = no_interrupt_deassert;
	}

	for (i=0; i<nr_of_interrupt_handlers; i++) {
		if (strcmp(name, interrupt_handlers[i].templ.name) != 0)
			continue;

		*templ = interrupt_handlers[i].templ;
		return 1;
	}

	/*  The 'if' is an ugly hack to prevent a Compaq CC warning.  */
	if (i >= nr_of_interrupt_handlers) {
		printf("interrupt_handler_lookup(\"%s\") failed. "
		    "Aborting.\n", name);
		abort();
	}

	return 0;
}


/*
 *  interrupt_connect():
 *
 *  Increases the exclusive or nonexclusive nr or users of an interrupt.
 */
void interrupt_connect(struct interrupt *in, int exclusive)
{
	int i;

#ifdef INTERRUPT_DEBUG
	printf("interrupt_connect(\"%s\")\n", in->name);
#endif

	if (in->name == NULL || in->name[0] == '\0')
		return;

	for (i=0; i<nr_of_interrupt_handlers; i++) {
		if (strcmp(in->name, interrupt_handlers[i].templ.name) != 0)
			continue;

		if (exclusive) {
			interrupt_handlers[i].nr_of_exclusive_users ++;
			if (interrupt_handlers[i].nr_of_exclusive_users > 1) {
				fatal("Fatal error in interrupt_connect(): "
				    "more than 1 exclusive user. Dumping "
				    "core for backtrace.\n");
				abort();
			}
		} else {
			interrupt_handlers[i].nr_of_nonexclusive_users ++;
		}

		return;
	}

	fatal("Internal error in interrupt_connect(): name '%s' not "
	    "found? Dumping core for debugging.\n", in->name);
	abort();
}


/*
 *  interrupt_disconnect():
 *
 *  Decreases the exclusive or nonexclusive nr or users of an interrupt.
 */
void interrupt_disconnect(struct interrupt *in, int exclusive)
{
	int i;

	if (in->name == NULL || in->name[0] == '\0')
		return;

	for (i=0; i<nr_of_interrupt_handlers; i++) {
		if (strcmp(in->name, interrupt_handlers[i].templ.name) != 0)
			continue;

		if (exclusive) {
			interrupt_handlers[i].nr_of_exclusive_users --;
			if (interrupt_handlers[i].nr_of_exclusive_users < 0) {
				fatal("Fatal error in interrupt_disconnect():"
				    "nr of exclusive users < 0?\n");
				exit(1);
			}
		} else {
			interrupt_handlers[i].nr_of_nonexclusive_users --;
			if (interrupt_handlers[i].nr_of_nonexclusive_users<0) {
				fatal("Fatal error in interrupt_disconnect():"
				    "nr of non-exclusive users < 0?\n");
				exit(1);
			}
		}

		return;
	}

	fatal("Internal error in interrupt_disconnect(): name '%s' not "
	    "found?\n", in->name);
	exit(1);
}


