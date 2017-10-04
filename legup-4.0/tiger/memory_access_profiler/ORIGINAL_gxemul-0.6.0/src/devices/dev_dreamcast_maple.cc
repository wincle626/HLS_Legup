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
 *  COMMENT: Dreamcast Maple bus controller
 *
 *  The Maple bus has 4 ports (A-D), and each port has up to 6 possible "units"
 *  (where unit 0 is the main unit). Communication is done using DMA; each
 *  DMA transfer sends one or more requests, and for each of these requests
 *  a response is generated (or a timeout if there was no device at the
 *  specific port).
 *
 *  See Marcus Comstedt's page (http://mc.pp.se/dc/maplebus.html) for more
 *  details about the DMA request/responses.
 *
 *
 *  TODO:
 *	Unit numbers / IDs for real Maple devices.
 *	The Controller (up/down/left/right, buttons, etc).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "console.h"
#include "cpu.h"
#include "device.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"

#include "thirdparty/dreamcast_maple.h"
#include "thirdparty/dreamcast_sysasicvar.h"


#define debug fatal

#define	N_MAPLE_PORTS		4

#define	MAX_CHARS		8192
#define	MAX_CONTROLLER_DATA	4096

#define	MAPLE_TICK_SHIFT	17

struct maple_device {
	struct maple_devinfo devinfo;
};

struct dreamcast_maple_data {
	/*  Registers:  */
	uint32_t	dmaaddr;
	int		enable;
	int		timeout;

	/*  Attached devices:  */
	struct maple_device *device[N_MAPLE_PORTS];

	/*  For keyboard/controller input:  */
	int		console_handle;
	uint8_t		char_queue[MAX_CHARS];
	uint16_t	controller_queue[MAX_CONTROLLER_DATA];
	int		char_queue_head, char_queue_tail;
	int		controller_queue_head, controller_queue_tail;
};


/*
 *  Maple devices:
 *
 *  TODO: Figure out strings and numbers of _real_ Maple devices.
 */
#if 0
static struct maple_device maple_device_controller = {
	{
		BE32_TO_HOST(MAPLE_FUNC(MAPLE_FN_CONTROLLER)),	/*  di_func  */
		{ 0,0,0 },			/*  di_function_data[3]  */
		0,				/*  di_area_code  */
		0,				/*  di_connector_direction  */
		"Dreamcast Controller",		/*  di_product_name  */
		"di_product_license",		/*  di_product_license  */
		LE16_TO_HOST(100),		/*  di_standby_power  */
		LE16_TO_HOST(100)		/*  di_max_power  */
	}
};
#endif
static struct maple_device maple_device_keyboard = {
	{
		BE32_TO_HOST(MAPLE_FUNC(MAPLE_FN_KEYBOARD)),/*  di_func  */
		{ LE32_TO_HOST(2),0,0 },	/*  di_function_data[3]  */
		0,				/*  di_area_code  */
		0,				/*  di_connector_direction  */
		"Keyboard",			/*  di_product_name  */
		"di_product_license",		/*  di_product_license  */
		LE16_TO_HOST(100),		/*  di_standby_power  */
		LE16_TO_HOST(100)		/*  di_max_power  */
	}
};
#if 0
static struct maple_device maple_device_mouse = {
	{
		BE32_TO_HOST(MAPLE_FUNC(MAPLE_FN_MOUSE)),/*  di_func  */
		{ 0,0,0 },			/*  di_function_data[3]  */
		0,				/*  di_area_code  */
		0,				/*  di_connector_direction  */
		"Dreamcast Mouse",		/*  di_product_name  */
		"di_product_license",		/*  di_product_license  */
		LE16_TO_HOST(100),		/*  di_standby_power  */
		LE16_TO_HOST(100)		/*  di_max_power  */
	}
};
#endif


DEVICE_TICK(maple)
{
	struct dreamcast_maple_data *d = (struct dreamcast_maple_data *) extra;
	int key;

	while ((key = console_readchar(d->console_handle)) >= 0) {
		/*  Add to the keyboard queue:  */
		d->char_queue[d->char_queue_head] = key;
		d->char_queue_head = (d->char_queue_head + 1) % MAX_CHARS;
		if (d->char_queue_head == d->char_queue_tail)
			fatal("[ dreamcast_maple: KEYBOARD QUEUE OVERRUN! ]\n");
	}

#if 0
	/*
	 *  NOTE/TODO:
	 *
	 *  Implement the controller in a reasonable way!
	 */

	control_bits = 0;
	switch (key) {
	case 'a':
	case 'A':
		control_bits = 0x0004;
		break;
	case 'b':
	case 'B':
		control_bits = 0x0002;
		break;
	case 'c':
	case 'C':
		control_bits = 0x0001;
		break;
	case 'x':
	case 'X':
		control_bits = 0x0400;
		break;
	case 'y':
	case 'Y':
		control_bits = 0x0200;
		break;
	case 'z':
	case 'Z':
		control_bits = 0x0100;
		break;
	case 's':
	case 'S':	/*  Start  */
		control_bits = 0x0008;
		break;
	case '8':	/*  up  */
		control_bits = 0x0010;
		break;
	case '2':	/*  down  */
	case 'k':
		control_bits = 0x0020;
		break;
	case '4':	/*  left  */
	case 'u':
		control_bits = 0x0040;
		break;
	case '6':	/*  right  */
	case 'o':
		control_bits = 0x0080;
		break;
	}

	if (control_bits != 0) {
		/*  Add to the controller queue:  */
		d->controller_queue[d->controller_queue_head] = control_bits;
		d->controller_queue_head =
		    (d->controller_queue_head + 1) % MAX_CONTROLLER_DATA;
		if (d->controller_queue_head == d->controller_queue_tail)
			fatal("[ dreamcast_maple: CONTROLLER QUEUE "
			    "OVERRUN! ]\n");
	}
#endif
}


static int get_key(struct dreamcast_maple_data *d)
{
	int key = d->char_queue[d->char_queue_tail];
	if (d->char_queue_head == d->char_queue_tail)
		return -1;

	d->char_queue_tail = (d->char_queue_tail + 1) % MAX_CHARS;
	return key;
}


static int get_controller(struct dreamcast_maple_data *d)
{
	int c = d->controller_queue[d->controller_queue_tail];
	if (d->controller_queue_head == d->controller_queue_tail)
		return 0;

	d->controller_queue_tail = (d->controller_queue_tail + 1)
	    % MAX_CONTROLLER_DATA;
	return c;
}


/*
 *  maple_getcond_controller_response():
 *
 *  Generate a controller response. Based on info from Marcus
 *  Comstedt's page: http://mc.pp.se/dc/controller.html
 */
static void maple_getcond_controller_response(struct dreamcast_maple_data *d,
	struct cpu *cpu, int port, uint32_t receive_addr)
{
	uint8_t buf[8];
	uint32_t response_code, transfer_code;
	int c;

	transfer_code = (MAPLE_RESPONSE_DATATRF << 24) |
	    (((port << 6) | 0x20) << 16) |
	    ((port << 6) << 8) |
	    3  /*  Transfer length in 32-bit words  */;
	transfer_code = BE32_TO_HOST(transfer_code);
	cpu->memory_rw(cpu, cpu->mem, receive_addr, (unsigned char *) (void *) &transfer_code,
	    4, MEM_WRITE, NO_EXCEPTIONS | PHYSICAL);
	receive_addr += 4;

	response_code = BE32_TO_HOST(MAPLE_FUNC(MAPLE_FN_CONTROLLER));
	cpu->memory_rw(cpu, cpu->mem, receive_addr, (unsigned char *) (void *) &response_code,
	    4, MEM_WRITE, NO_EXCEPTIONS | PHYSICAL);
	receive_addr += 4;

	/*  NOTE: Inverse of the buttons pressed!  */
	c = ~get_controller(d);

	/*
	 *  buf[0..1] = little endian button bitfield
	 *  buf[2] = right analogue trigger (0-255)
	 *  buf[3] = left analogue trigger (0-255)
	 *  buf[4] = analogue joystick X (0-255)
	 *  buf[5] = analogue joystick Y (0-255)
	 *  buf[6] = second analogue joystick X (0-255)
	 *  buf[7] = second analogue joystick Y (0-255)
	 */
	memset(buf, 0, 8);
	buf[0] = c & 0xff;
	buf[1] = c >> 8;
	buf[4] = buf[5] = buf[6] = buf[7] = 0;

	cpu->memory_rw(cpu, cpu->mem, receive_addr, (unsigned char *) (void *) &buf, 8,
	    MEM_WRITE, NO_EXCEPTIONS | PHYSICAL);
}


/*
 *  maple_getcond_keyboard_response():
 *
 *  Generate a keyboard key-press response. Based on info from Marcus
 *  Comstedt's page: http://mc.pp.se/dc/kbd.html
 */
static void maple_getcond_keyboard_response(struct dreamcast_maple_data *d,
	struct cpu *cpu, int port, uint32_t receive_addr)
{
	int key;
	uint8_t buf[8];
	uint32_t response_code, transfer_code;

	transfer_code = (MAPLE_RESPONSE_DATATRF << 24) |
	    (((port << 6) | 0x20) << 16) |
	    ((port << 6) << 8) |
	    3  /*  Transfer length in 32-bit words  */;
	transfer_code = BE32_TO_HOST(transfer_code);
	cpu->memory_rw(cpu, cpu->mem, receive_addr, (unsigned char *) (void *) &transfer_code,
	    4, MEM_WRITE, NO_EXCEPTIONS | PHYSICAL);
	receive_addr += 4;

	response_code = BE32_TO_HOST(MAPLE_FUNC(MAPLE_FN_KEYBOARD));
	cpu->memory_rw(cpu, cpu->mem, receive_addr, (unsigned char *) (void *) &response_code,
	    4, MEM_WRITE, NO_EXCEPTIONS | PHYSICAL);
	receive_addr += 4;

	key = get_key(d);

	/*
	 *  buf[0] = shift keys (1 = left ctrl, 2 = shift, 0x10 = right ctrl)
	 *  buf[1] = led state
	 *  buf[2] = key
	 */
	memset(buf, 0, 8);

	if (key >= 'a' && key <= 'z')	buf[2] = 4 + key - 'a';
	if (key >= 'A' && key <= 'Z')	buf[0] = 2, buf[2] = 4 + key - 'A';
	if (key >= 1 && key <= 26)	buf[0] = 1, buf[2] = 4 + key - 1;
	if (key >= '1' && key <= '9')	buf[2] = 0x1e + key - '1';
	if (key == '!')			buf[0] = 2, buf[2] = 0x1e;
	if (key == '"')			buf[0] = 2, buf[2] = 0x1f;
	if (key == '#')			buf[0] = 2, buf[2] = 0x20;
	if (key == '$')			buf[0] = 2, buf[2] = 0x21;
	if (key == '%')			buf[0] = 2, buf[2] = 0x22;
	if (key == '^')			buf[0] = 2, buf[2] = 0x23;
	if (key == '&')			buf[0] = 2, buf[2] = 0x24;
	if (key == '*')			buf[0] = 2, buf[2] = 0x25;
	if (key == '(')			buf[0] = 2, buf[2] = 0x26;
	if (key == '@')			buf[0] = 2, buf[2] = 0x1f;
	if (key == '\n' || key == '\r')	buf[0] = 0, buf[2] = 0x28;
	if (key == ')')			buf[0] = 2, buf[2] = 0x27;
	if (key == '\b')		buf[0] = 0, buf[2] = 0x2a;
	if (key == '\t')		buf[0] = 0, buf[2] = 0x2b;
	if (key == ' ')			buf[0] = 0, buf[2] = 0x2c;
	if (key == '0')			buf[2] = 0x27;
	if (key == 27)			buf[2] = 0x29;
	if (key == '-')			buf[2] = 0x2d;
	if (key == '=')			buf[2] = 0x2e;
	if (key == '[')			buf[2] = 0x2f;
	if (key == '\\')		buf[2] = 0x31;
	if (key == '|')			buf[2] = 0x31, buf[0] = 2;
	if (key == ']')			buf[2] = 0x32;
	if (key == ';')			buf[2] = 0x33;
	if (key == ':')			buf[2] = 0x34;
	if (key == ',')			buf[2] = 0x36;
	if (key == '.')			buf[2] = 0x37;
	if (key == '/')			buf[2] = 0x38;
	if (key == '<')			buf[2] = 0x36, buf[0] = 2;
	if (key == '>')			buf[2] = 0x37, buf[0] = 2;
	if (key == '?')			buf[2] = 0x38, buf[0] = 2;
	if (key == '+')			buf[2] = 0x57;

	cpu->memory_rw(cpu, cpu->mem, receive_addr, (unsigned char *) (void *) &buf, 8,
	    MEM_WRITE, NO_EXCEPTIONS | PHYSICAL);
}


/*
 *  maple_do_dma_xfer():
 *
 *  Perform a DMA transfer. enable should be 1, and dmaaddr should point to
 *  the memory to transfer.
 */
void maple_do_dma_xfer(struct cpu *cpu, struct dreamcast_maple_data *d)
{
	uint32_t addr = d->dmaaddr;

	if (!d->enable) {
		fatal("[ maple_do_dma_xfer: not enabled? ]\n");
		return;
	}

	/*  debug("[ dreamcast_maple: DMA transfer, dmaaddr = "
	    "0x%08"PRIx32" ]\n", addr);  */

	/*
	 *  DMA transfers must be 32-byte aligned, according to Marcus
	 *   Comstedt's Maple demo program.
	 */
	if (addr & 0x1f) {
		fatal("[ dreamcast_maple: dmaaddr 0x%08"PRIx32" is NOT"
		    " 32-byte aligned; aborting ]\n", addr);
		return;
	}

	/*
	 *  Handle one or more requests/responses:
	 *
	 *  (This is "reverse engineered" from Comstedt's maple demo program;
	 *  it might not be good enough to emulate how the Maple is being
	 *  used by other programs.)
	 */
	for (;;) {
		uint32_t receive_addr, response_code, cond;
		int datalen, port, last_message, cmd, to, from, datalen_cmd;
		int unit;
		uint8_t buf[8];

		/*  Read the message' two control words:  */
		cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *) (void *) &buf, 8, MEM_READ,
		    NO_EXCEPTIONS | PHYSICAL);
		addr += 8;

		datalen = buf[0] * sizeof(uint32_t);
		if (buf[1] & 2) {
			fatal("[ dreamcast_maple: TODO: GUN bit. ]\n");
			/*  TODO: Set some bits in A05F80C4 to indicate
			    which raster position a lightgun is pointing
			    at!  */
			exit(1);
		}
		port = buf[2];
		last_message = buf[3] & 0x80;
		receive_addr = buf[4] + (buf[5] << 8) + (buf[6] << 16)
		    + (buf[7] << 24);

		if (receive_addr & 0xe000001f)
			fatal("[ dreamcast_maple: WARNING! receive address 0x"
			    "%08"PRIx32" isn't valid! ]\n", receive_addr);

		/*  Read the command word for this message:  */
		cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *) (void *) &buf, 4, MEM_READ,
		    NO_EXCEPTIONS | PHYSICAL);
		addr += 4;

		cmd = buf[0];
		to = buf[1];
		from = buf[2];
		datalen_cmd = buf[3];

		/*  Decode the unit number:  */
		unit = 0;
		switch (to & 0x3f) {
		case 0x00:
		case 0x20: unit = 0; break;
		case 0x01: unit = 1; break;
		case 0x02: unit = 2; break;
		case 0x04: unit = 3; break;
		case 0x08: unit = 4; break;
		case 0x10: unit = 5; break;
		default: fatal("[ dreamcast_maple: ERROR! multiple "
			    "units? Not yet implemented. to = 0x%02x ]\n", to);
			exit(1);
		}

		/*  debug("[ dreamcast_maple: cmd=0x%02x, port=%c, unit=%i"
		    ", datalen=%i words ]\n", cmd, port+'A', unit,
		    datalen_cmd);  */

		/*
		 *  Handle the command:
		 */
		switch (cmd) {

		case MAPLE_COMMAND_DEVINFO:
			if (d->device[port] == NULL || unit != 0) {
				/*  No device present: Timeout.  */
				/*  debug("[ dreamcast_maple: response="
				    "timeout ]\n");  */
				response_code = (uint32_t) -1;
				response_code = LE32_TO_HOST(response_code);
				cpu->memory_rw(cpu, cpu->mem, receive_addr,
				    (unsigned char *) (void *) &response_code, 4, MEM_WRITE,
				    NO_EXCEPTIONS | PHYSICAL);
			} else {
				/*  Device present:  */
				unsigned int i;
				struct maple_devinfo *di =
				    &d->device[port]->devinfo;
				/*  debug("[ dreamcast_maple: response="
				    "\"%s\" ]\n", di->di_product_name);  */
				response_code = MAPLE_RESPONSE_DEVINFO |
				    (((port << 6) | 0x20) << 8) |
				    ((port << 6) << 16) |
				    ((sizeof(struct maple_devinfo) /
				    sizeof(uint32_t)) << 24);
				response_code = LE32_TO_HOST(response_code);
				cpu->memory_rw(cpu, cpu->mem, receive_addr,
				    (unsigned char *) (void *) &response_code, 4, MEM_WRITE,
				    NO_EXCEPTIONS | PHYSICAL);
				for (i=0; i<sizeof(struct maple_devinfo); i++)
					cpu->memory_rw(cpu, cpu->mem,
					    receive_addr + 4 + i, (unsigned
					    char *) di + i, 1, MEM_WRITE,
					    NO_EXCEPTIONS | PHYSICAL);
			}
			break;

		case MAPLE_COMMAND_GETCOND:
			cpu->memory_rw(cpu, cpu->mem, addr, (unsigned char *) (void *) &buf, 4,
			    MEM_READ, NO_EXCEPTIONS | PHYSICAL);
			cond = buf[3] + (buf[2] << 8) + (buf[1] << 16)
			    + (buf[0] << 24);
			if (cond & MAPLE_FUNC(MAPLE_FN_CONTROLLER)) {
				maple_getcond_controller_response(
				    d, cpu, port, receive_addr);
			} else if (cond & MAPLE_FUNC(MAPLE_FN_KEYBOARD)) {
				maple_getcond_keyboard_response(
				    d, cpu, port, receive_addr);
			} else {
				fatal("[ dreamcast_maple: WARNING: GETCOND: "
				    "UNIMPLEMENTED 0x%08"PRIx32" ]\n", cond);
			}
			break;

		case MAPLE_COMMAND_BWRITE:
			fatal("[ dreamcast_maple: BWRITE: TODO ]\n");
			break;

		default:fatal("[ dreamcast_maple: command %i: TODO ]\n", cmd);
			exit(1);
		}

		addr += datalen_cmd * 4;

		/*  Last request? Then stop.  */
		if (last_message)
			break;
	}

	/*  Assert the SYSASIC_EVENT_MAPLE_DMADONE event:  */
	SYSASIC_TRIGGER_EVENT(SYSASIC_EVENT_MAPLE_DMADONE);
}


DEVICE_ACCESS(dreamcast_maple)
{
	struct dreamcast_maple_data *d = (struct dreamcast_maple_data *) extra;
	uint64_t idata = 0, odata = 0;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	switch (relative_addr) {

	case 0x04:	/*  MAPLE_DMAADDR  */
		if (writeflag == MEM_WRITE) {
			d->dmaaddr = idata;
			/*  debug("[ dreamcast_maple: dmaaddr set to 0x%08x"
			    " ]\n", d->dmaaddr);  */
		} else {
			fatal("[ dreamcast_maple: TODO: read from dmaaddr ]\n");
			odata = d->dmaaddr;
		}
		break;

	case 0x10:	/*  MAPLE_RESET2  */
		if (writeflag == MEM_WRITE && idata != 0)
			fatal("[ dreamcast_maple: UNIMPLEMENTED reset2 value"
			    " 0x%08x ]\n", (int)idata);
		break;

	case 0x14:	/*  MAPLE_ENABLE  */
		if (writeflag == MEM_WRITE)
			d->enable = idata;
		else
			odata = d->enable;
		break;

	case 0x18:	/*  MAPLE_STATE  */
		if (writeflag == MEM_WRITE) {
			switch (idata) {
			case 0:	break;
			case 1:	maple_do_dma_xfer(cpu, d);
				break;
			default:fatal("[ dreamcast_maple: UNIMPLEMENTED "
				    "state value %i ]\n", (int)idata);
			}
		} else {
			/*  Always return 0 to indicate DMA xfer complete.  */
			odata = 0;
		}
		break;

	case 0x80:	/*  MAPLE_SPEED  */
		if (writeflag == MEM_WRITE) {
			d->timeout = (idata >> 16) & 0xffff;
			/*  TODO: Bits 8..9 are "speed", but only the value
			    0 (indicating 2 Mbit/s) should be used.  */
			debug("[ dreamcast_maple: timeout set to %i ]\n",
			    d->timeout);
		} else {
			odata = d->timeout << 16;
		}
		break;

	case 0x8c:	/*  MAPLE_RESET  */
		if (writeflag == MEM_WRITE) {
			if (idata != 0x6155404f)
				fatal("[ dreamcast_maple: UNIMPLEMENTED reset "
				    "value 0x%08x ]\n", (int)idata);
			d->enable = 0;
		}
		break;

	default:if (writeflag == MEM_READ) {
			fatal("[ dreamcast_maple: UNIMPLEMENTED read from "
			    "addr 0x%x ]\n", (int)relative_addr);
		} else {
			fatal("[ dreamcast_maple: UNIMPLEMENTED write to addr "
			    "0x%x: 0x%x ]\n", (int)relative_addr, (int)idata);
		}
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVINIT(dreamcast_maple)
{
	struct dreamcast_maple_data *d;

	CHECK_ALLOCATION(d = (struct dreamcast_maple_data *) malloc(sizeof(struct dreamcast_maple_data)));
	memset(d, 0, sizeof(struct dreamcast_maple_data));

	memory_device_register(devinit->machine->memory, devinit->name,
	    0x5f6c00, 0x100, dev_dreamcast_maple_access, d, DM_DEFAULT, NULL);

	/*  Devices connected to port A..D:  */
	d->device[0] = NULL;	/*  &maple_device_controller;  */
	d->device[1] = NULL;
	d->device[2] = &maple_device_keyboard;
	d->device[3] = NULL;	/*  TODO:  &maple_device_mouse;  */

	d->console_handle = console_start_slave_inputonly(
	    devinit->machine, "maple", 1);
	devinit->machine->main_console_handle = d->console_handle;

	machine_add_tickfunction(devinit->machine, dev_maple_tick, d,
	    MAPLE_TICK_SHIFT);

	return 1;
}

