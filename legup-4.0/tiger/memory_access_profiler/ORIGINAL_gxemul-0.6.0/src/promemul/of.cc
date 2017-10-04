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
 *  COMMENT: OpenFirmware emulation
 *
 *  NOTE: This module is/was a quick hack, with the purpose of getting
 *        NetBSD/macppc to boot. If anything else boots using this hackish
 *        implementation of OpenFirmware, then that is a bonus.
 *
 ******************************************************************************
 *
 *  NOTE: OpenFirmware is used on quite a variety of different hardware archs,
 *        at least POWER/PowerPC, ARM, and SPARC, so the code in this module
 *        must always remain architecture agnostic.
 *
 *  NOTE: Some things, e.g. 32-bit integers as returned by the "getprop"
 *        service, are always fixed to big-endian. (According to the standard.)
 *
 *  TODO: o) 64-bit OpenFirmware?
 *        o) More devices...
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define OF_C

#include "console.h"
#include "cpu.h"
#include "device.h"
#include "devices.h"
#include "diskimage.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"
#include "of.h"


/*  #define debug fatal  */

extern int quiet_mode;
extern int verbose;


/*
 *  readstr():
 *
 *  Helper function to read a string from emulated memory.
 */
static void readstr(struct cpu *cpu, uint64_t addr, char *strbuf,
	int bufsize)
{
	int i;
	for (i=0; i<bufsize; i++) {
		unsigned char ch;
		cpu->memory_rw(cpu, cpu->mem, addr + i,
		    &ch, sizeof(ch), MEM_READ, CACHE_DATA | NO_EXCEPTIONS);
		strbuf[i] = '\0';
		if (ch >= 1 && ch < 32)
			ch = 0;
		strbuf[i] = ch;
		if (strbuf[i] == '\0')
			break;
	}

	strbuf[bufsize - 1] = '\0';
}


/*
 *  of_store_32bit_in_host():
 *
 *  Store big-endian. OpenFirmware properties returned by getprop etc are
 *  always big-endian, even on little-endian machines.
 */
static void of_store_32bit_in_host(unsigned char *d, uint32_t x)
{
	d[0] = x >> 24; d[1] = x >> 16;
	d[2] = x >>  8; d[3] = x;
}


/*
 *  find_device_handle():
 *
 *  name may consist of multiple names, separaed with slashes.
 */
static int find_device_handle(struct of_data *ofd, const char *name)
{
	int handle = 1, cur_parent = 1;

	if (name[0] == 0)
		return 0;

	for (;;) {
		struct of_device *od = ofd->of_devices;
		char tmp[200];
		int i;

		while (name[0] == '/')
			name++;
		if (name[0] == '\0')
			break;
		snprintf(tmp, sizeof(tmp), "%s", name);
		i = 0;
		while (tmp[i] != '\0' && tmp[i] != '/')
			i++;
		tmp[i] = '\0';

		OF_FIND(od, strcmp(od->name, tmp) == 0 &&
		    od->parent == cur_parent);
		if (od == NULL)
			return -1;

		handle = cur_parent = od->handle;
		name += strlen(tmp);
	}

	return handle;
}


/*****************************************************************************/


OF_SERVICE(call_method_2_2)
{
	fatal("[ of: call_method_2_2('%s'): TODO ]\n", arg[0]);
	return -1;
}


OF_SERVICE(call_method_3_4)
{
	fatal("[ of: call_method_3_4('%s'): TODO ]\n", arg[0]);
	return -1;
}


OF_SERVICE(call_method_5_2)
{
	if (strcmp(arg[0], "set-colors") == 0) {
		/*  Used by OpenBSD/macppc:  */
		struct vfb_data *v = cpu->machine->md.of_data->vfb_data;
		int color = OF_GET_ARG(3);
		uint64_t ptr = OF_GET_ARG(4);
		unsigned char rgb[3];
		cpu->memory_rw(cpu, cpu->mem, ptr, rgb, 3, MEM_READ,
		    CACHE_DATA | NO_EXCEPTIONS);
		if (v != NULL) {
			memcpy(v->rgb_palette + 3 * color, rgb, 3);
			v->update_x1 = v->update_y1 = 0;
			v->update_x2 = v->xsize - 1;
			v->update_y2 = v->ysize - 1;
		}
	} else {
		fatal("[ of: call_method_5_2('%s'): TODO ]\n", arg[0]);
		return -1;
	}
	return 0;
}


OF_SERVICE(call_method_6_1)
{
	fatal("[ of: call_method_6_1('%s'): TODO ]\n", arg[0]);
	return -1;
}


OF_SERVICE(call_method_6_2)
{
	fatal("[ of: call_method_6_2('%s'): TODO ]\n", arg[0]);
	return -1;
}


OF_SERVICE(child)
{
	struct of_device *od = cpu->machine->md.of_data->of_devices;
	int handle = OF_GET_ARG(0);
	OF_FIND(od, od->parent == handle);
	store_32bit_word(cpu, base + retofs, od == NULL? 0 : od->handle);
	return 0;
}


OF_SERVICE(claim)
{
	// Arguments:  virtualaddr, size, alignment
	// Returns:    pointer to claimed memory

	// TODO: This is just a dummy.
	fatal("[ of: claim(0x%x,0x%x,0x%x): TODO ]\n",
	    OF_GET_ARG(0), OF_GET_ARG(1), OF_GET_ARG(2));

	store_32bit_word(cpu, base + retofs, OF_GET_ARG(0));
	return 0;
}


OF_SERVICE(exit)
{
	cpu->running = 0;
	return 0;
}


OF_SERVICE(finddevice)
{
	int h = find_device_handle(cpu->machine->md.of_data, arg[0]);
	store_32bit_word(cpu, base + retofs, h);
	return h>0? 0 : -1;
}


OF_SERVICE(getprop)
{
	struct of_device *od = cpu->machine->md.of_data->of_devices;
	struct of_device_property *pr;
	int handle = OF_GET_ARG(0), i, len_returned = 0;
	uint64_t buf = OF_GET_ARG(2);
	uint64_t max = OF_GET_ARG(3);

	OF_FIND(od, od->handle == handle);
	if (od == NULL) {
		fatal("[ of: WARNING: getprop handle=%i; no such handle ]\n",
		    handle);
		return -1;
	}

	pr = od->properties;
	OF_FIND(pr, strcmp(pr->name, arg[1]) == 0);
	if (pr == NULL) {
		fatal("[ of: WARNING: getprop: no property '%s' at handle"
		    " %i (device '%s') ]\n", arg[1], handle, od->name);
		return -1;
	}

	if (pr->data == NULL) {
		fatal("[ of: WARNING: property '%s' of '%s' has no data! ]\n",
		    arg[1], od->name);
		goto ret;
	}

	/*  Return the property into emulated RAM:  */
	len_returned = pr->len <= max? pr->len : max;

	for (i=0; i<len_returned; i++) {
		if (!cpu->memory_rw(cpu, cpu->mem, buf + i, pr->data + i,
		    1, MEM_WRITE, CACHE_DATA | NO_EXCEPTIONS)) {
			fatal("[ of: getprop memory_rw() error ]\n");
			exit(1);
		}
	}

ret:
	store_32bit_word(cpu, base + retofs, len_returned);
	return 0;
}


OF_SERVICE(getproplen)
{
	struct of_device *od = cpu->machine->md.of_data->of_devices;
	struct of_device_property *pr;
	int handle = OF_GET_ARG(0);

	OF_FIND(od, od->handle == handle);
	if (od == NULL) {
		fatal("[ of: TODO: getproplen handle=%i; no such handle ]\n",
		    handle);
		return -1;
	}

	pr = od->properties;
	OF_FIND(pr, strcmp(pr->name, arg[1]) == 0);
	if (pr == NULL) {
		fatal("[ of: TODO: getproplen: no property '%s' at handle"
		    " %i (device '%s') ]\n", arg[1], handle, od->name);
		return -1;
	}

	store_32bit_word(cpu, base + retofs, pr->len);
	return 0;
}


OF_SERVICE(instance_to_package)
{
	int handle = OF_GET_ARG(0);
	/*  TODO: actually do something here? :-)  */
	store_32bit_word(cpu, base + retofs, handle);
	return 0;
}


OF_SERVICE(interpret_1)
{
	if (strcmp(arg[0], "#lines 2 - to line#") == 0) {
	} else {
		fatal("[ of: interpret_1('%s'): TODO ]\n", arg[0]);
		return -1;
	}
	return 0;
}


OF_SERVICE(interpret_2)
{
	store_32bit_word(cpu, base + retofs, 0);	/*  ?  TODO  */
	if (strcmp(arg[0], "#columns") == 0) {
		store_32bit_word(cpu, base + retofs + 4, 80);
	} else if (strcmp(arg[0], "#lines") == 0) {
		store_32bit_word(cpu, base + retofs + 4, 40);
	} else if (strcmp(arg[0], "char-height") == 0) {
		store_32bit_word(cpu, base + retofs + 4, 15);
	} else if (strcmp(arg[0], "char-width") == 0) {
		store_32bit_word(cpu, base + retofs + 4, 10);
	} else if (strcmp(arg[0], "line#") == 0) {
		store_32bit_word(cpu, base + retofs + 4, 0);
	} else if (strcmp(arg[0], "font-adr") == 0) {
		store_32bit_word(cpu, base + retofs + 4, 0);
	} else {
		fatal("[ of: interpret_2('%s'): TODO ]\n", arg[0]);
		return -1;
	}
	return 0;
}


OF_SERVICE(package_to_path)
{
	fatal("[ of: package-to-path: TODO ]\n");
	return -1;
}


OF_SERVICE(parent)
{
	struct of_device *od = cpu->machine->md.of_data->of_devices;
	int handle = OF_GET_ARG(0);
	OF_FIND(od, od->handle == handle);
	store_32bit_word(cpu, base + retofs, od == NULL? 0 : od->parent);
	return 0;
}


OF_SERVICE(peer)
{
	struct of_device *od = cpu->machine->md.of_data->of_devices;
	int handle = OF_GET_ARG(0), parent = 0, peer = 0, seen_self = 1;

	if (handle == 0) {
		/*  Return the handle of the root node (1):  */
		store_32bit_word(cpu, base + retofs, 1);
		return 0;
	}

	OF_FIND(od, od->handle == handle);
	if (od == NULL) {
		fatal("[ of: TODO: peer(): can't find handle %i ]\n", handle);
		exit(1);
	}
	parent = od->parent;
	seen_self = 0;

	od = cpu->machine->md.of_data->of_devices;

	while (od != NULL) {
		if (od->parent == parent) {
			if (seen_self) {
				peer = od->handle;
				break;
			}
			if (od->handle == handle)
				seen_self = 1;
		}
		od = od->next;
	}
	store_32bit_word(cpu, base + retofs, peer);
	return 0;
}


OF_SERVICE(read)
{
	/*  int handle = OF_GET_ARG(0);  */
	uint64_t ptr = OF_GET_ARG(1);
	/*  int len = OF_GET_ARG(2);  */
	int c;
	unsigned char ch;

	/*  TODO: check handle! This just reads chars from the console!  */
	/*  TODO: This is blocking!  */

	c = console_readchar(cpu->machine->main_console_handle);
	ch = c;
	if (!cpu->memory_rw(cpu, cpu->mem, ptr, &ch, 1, MEM_WRITE,
	    CACHE_DATA | NO_EXCEPTIONS)) {
		fatal("[ of: TODO: read: memory_rw() error ]\n");
		exit(1);
	}

	store_32bit_word(cpu, base + retofs, c == -1? 0 : 1);
	return c == -1? -1 : 0;
}


OF_SERVICE(write)
{
	/*  int handle = OF_GET_ARG(0);  */
	uint64_t ptr = OF_GET_ARG(1);
	int n_written = 0, i, len = OF_GET_ARG(2);

	/*  TODO: check handle! This just dumps the data to the console!  */

	for (i=0; i<len; i++) {
		unsigned char ch;
		if (!cpu->memory_rw(cpu, cpu->mem, ptr + i, &ch,
		    1, MEM_READ, CACHE_DATA | NO_EXCEPTIONS)) {
			fatal("[ of: TODO: write: memory_rw() error ]\n");
			exit(1);
		}
		if (ch != 7)
			console_putchar(cpu->machine->main_console_handle, ch);
		n_written ++;
	}

	store_32bit_word(cpu, base + retofs, n_written);
	return 0;
}


/*****************************************************************************/


/*
 *  of_get_unused_device_handle():
 *
 *  Returns an unused device handle number (1 or higher).
 */
static int of_get_unused_device_handle(struct of_data *of_data)
{
	int max_handle = 0;
	struct of_device *od = of_data->of_devices;

	while (od != NULL) {
		if (od->handle > max_handle)
			max_handle = od->handle;
		od = od->next;
	}

	return max_handle + 1;
}


/*
 *  of_add_device():
 *
 *  Adds a device.
 */
static struct of_device *of_add_device(struct of_data *of_data, const char *name,
	const char *parentname)
{
	struct of_device *od;

	CHECK_ALLOCATION(od = (struct of_device *) malloc(sizeof(struct of_device)));
	memset(od, 0, sizeof(struct of_device));

	CHECK_ALLOCATION(od->name = strdup(name));

	od->handle = of_get_unused_device_handle(of_data);
	od->parent = find_device_handle(of_data, parentname);
	if (od->parent < 0) {
		fatal("of_add_device(): adding '%s' to parent '%s' failed: "
		    "parent not found!\n", name, parentname);
		exit(1);
	}

	od->next = of_data->of_devices;
	of_data->of_devices = od;

	return od;
}


/*
 *  of_add_prop():
 *
 *  Adds a property to a device.
 */
static void of_add_prop(struct of_data *of_data, const char *devname,
	const char *propname, unsigned char *data, uint32_t len, int flags)
{
	struct of_device_property *pr;
	struct of_device *od = of_data->of_devices;
	int h = find_device_handle(of_data, devname);

	CHECK_ALLOCATION(pr = (struct of_device_property *) malloc(sizeof(struct of_device_property)));
	memset(pr, 0, sizeof(struct of_device_property));

	OF_FIND(od, od->handle == h);
	if (od == NULL) {
		fatal("of_add_prop(): device '%s' not registered\n", devname);
		exit(1);
	}

	CHECK_ALLOCATION(pr->name = strdup(propname));
	pr->data = data;
	pr->len = len;
	pr->flags = flags;

	pr->next = od->properties;
	od->properties = pr;
}


/*
 *  of_add_service():
 *
 *  Adds a service.
 */
static void of_add_service(struct of_data *of_data, const char *name,
	int (*f)(OF_SERVICE_ARGS), int n_args, int n_ret_args)
{
	struct of_service *os;

	CHECK_ALLOCATION(os = (struct of_service *) malloc(sizeof(struct of_service)));
	memset(os, 0, sizeof(struct of_service));

	CHECK_ALLOCATION(os->name = strdup(name));

	os->f = f;
	os->n_args = n_args;
	os->n_ret_args = n_ret_args;

	os->next = of_data->of_services;
	of_data->of_services = os;
}


/*
 *  of_dump_devices():
 *
 *  Debug dump helper.
 */
static void of_dump_devices(struct of_data *ofd, int parent)
{
	int iadd = DEBUG_INDENTATION;
	struct of_device *od = ofd->of_devices;

	while (od != NULL) {
		struct of_device_property *pr = od->properties;
		if (od->parent != parent) {
			od = od->next;
			continue;
		}
		debug("\"%s\"\n", od->name, od->handle);
		debug_indentation(iadd);
		while (pr != NULL) {
			debug("(%s: ", pr->name);
			if (pr->flags == OF_PROP_STRING)
				debug("\"%s\"", pr->data);
			else
				debug("%i bytes", pr->len);
			debug(")\n");
			pr = pr->next;
		}
		of_dump_devices(ofd, od->handle);
		debug_indentation(-iadd);
		od = od->next;
	}
}


/*
 *  of_dump_all():
 *
 *  Debug dump.
 */
static void of_dump_all(struct of_data *ofd)
{
	int iadd = DEBUG_INDENTATION;
	struct of_service *os;

	debug("openfirmware debug dump:\n");
	debug_indentation(iadd);

	/*  Devices:  */
	of_dump_devices(ofd, 0);

	/*  Services:  */
	os = ofd->of_services;
	while (os != NULL) {
		debug("service '%s'", os->name);
		if (os->n_ret_args > 0 || os->n_args > 0) {
			debug(" (");
			if (os->n_args > 0) {
				debug("%i arg%s", os->n_args,
				    os->n_args > 1? "s" : "");
				if (os->n_ret_args > 0)
					debug(", ");
			}
			if (os->n_ret_args > 0)
				debug("%i return value%s", os->n_ret_args,
				    os->n_ret_args > 1? "s" : "");
			debug(")");
		}
		debug("\n");
		os = os->next;
	}

	debug_indentation(-iadd);
}


/*
 *  of_add_prop_int32():
 *
 *  Helper function.
 */
static void of_add_prop_int32(struct of_data *ofd,
	const char *devname, const char *propname, uint32_t x)
{
	unsigned char *p;

	CHECK_ALLOCATION(p = (unsigned char *) malloc(sizeof(int32_t)));

	of_store_32bit_in_host(p, x);
	of_add_prop(ofd, devname, propname, p, sizeof(int32_t),
	    OF_PROP_INT);
}


/*
 *  of_add_prop_str():
 *
 *  Helper function.
 */
static void of_add_prop_str(struct machine *machine, struct of_data *ofd,
	const char *devname, const char *propname, const char *data)
{
	char *p;

	CHECK_ALLOCATION(p = strdup(data));

	of_add_prop(ofd, devname, propname, (unsigned char *)p, strlen(p) + 1,
	    OF_PROP_STRING);
}


/*
 *  of_emul_init_isa():
 */
void of_emul_init_isa(struct machine *machine)
{
	struct of_data *ofd = machine->md.of_data;
	unsigned char *isa_ranges;

	of_add_device(ofd, "isa", "/");

	CHECK_ALLOCATION(isa_ranges = (unsigned char *) malloc(32));
	memset(isa_ranges, 0, 32);

	/*  2 *: isa_phys_hi, isa_phys_lo, parent_phys_start, size  */
	/*  MEM space:  */
	of_store_32bit_in_host(isa_ranges + 0, 0);
	of_store_32bit_in_host(isa_ranges + 4, 0xc0000000);
	/*  I/O space: low bit if isa_phys_hi set  */
	of_store_32bit_in_host(isa_ranges + 16, 1);
	of_store_32bit_in_host(isa_ranges + 20, 0xd0000000);

	of_add_prop(ofd, "/isa", "ranges", isa_ranges, 32, 0);
}


/*
 *  of_emul_init_adb():
 */
void of_emul_init_adb(struct machine *machine)
{
	struct of_data *ofd = machine->md.of_data;
	unsigned char *adb_interrupts, *adb_reg;

	CHECK_ALLOCATION(adb_interrupts = (unsigned char *) malloc(4 * sizeof(uint32_t)));
	CHECK_ALLOCATION(adb_reg = (unsigned char *) malloc(8 * sizeof(uint32_t)));

	of_add_device(ofd, "adb", "/bandit/gc");
	of_add_prop_str(machine, ofd, "/bandit/gc/adb", "name", "via-cuda");
	of_store_32bit_in_host(adb_interrupts + 0, 25);
	of_store_32bit_in_host(adb_interrupts + 4, 0);
	of_store_32bit_in_host(adb_interrupts + 8, 0);
	of_store_32bit_in_host(adb_interrupts + 12, 0);
	of_add_prop(ofd, "/bandit/gc/adb", "interrupts", adb_interrupts,
	    4*sizeof(uint32_t), 0);
	of_store_32bit_in_host(adb_reg + 0, 0x16000);
	of_store_32bit_in_host(adb_reg + 4, 0x2000);
	of_store_32bit_in_host(adb_reg + 8, 0);
	of_store_32bit_in_host(adb_reg + 12, 0);
	of_store_32bit_in_host(adb_reg + 16, 0);
	of_store_32bit_in_host(adb_reg + 20, 0);
	of_store_32bit_in_host(adb_reg + 24, 0);
	of_store_32bit_in_host(adb_reg + 28, 0);
	of_add_prop(ofd, "/bandit/gc/adb", "reg", adb_reg,
	    8*sizeof(uint32_t), 0);
}


/*
 *  of_emul_init_zs():
 */
void of_emul_init_zs(struct machine *machine)
{
	struct of_data *ofd = machine->md.of_data;
	unsigned char *zs_interrupts, *zs_reg;

	CHECK_ALLOCATION(zs_reg = (unsigned char *) malloc(6 * sizeof(uint32_t)));

	/*  The controller:  */
	of_add_device(ofd, "zs", "/bandit/gc");
	of_add_prop_str(machine, ofd, "/bandit/gc/zs", "device_type", "serial");
	of_add_prop_str(machine, ofd, "/bandit/gc/zs", "name", "escc");
	of_store_32bit_in_host(zs_reg + 0, 0x13000);
	of_store_32bit_in_host(zs_reg + 4, 0x40);
	of_store_32bit_in_host(zs_reg + 8, 0x100);
	of_store_32bit_in_host(zs_reg + 12, 0x100);
	of_store_32bit_in_host(zs_reg + 16, 0x200);
	of_store_32bit_in_host(zs_reg + 20, 0x100);
	of_add_prop(ofd, "/bandit/gc/zs", "reg", zs_reg, 6*sizeof(uint32_t), 0);

	/*  Port 1:  */
	CHECK_ALLOCATION(zs_interrupts = (unsigned char *) malloc(3 * sizeof(uint32_t)));
	CHECK_ALLOCATION(zs_reg = (unsigned char *) malloc(6 * sizeof(uint32_t)));

	of_add_device(ofd, "zstty1", "/bandit/gc/zs");
	of_add_prop_str(machine, ofd, "/bandit/gc/zs/zstty1", "name", "ch-a");
	of_store_32bit_in_host(zs_interrupts + 0, 16);
	of_store_32bit_in_host(zs_interrupts + 4, 0);
	of_store_32bit_in_host(zs_interrupts + 8, 0);
	of_add_prop(ofd, "/bandit/gc/zs/zstty1", "interrupts", zs_interrupts,
	    3*sizeof(uint32_t), 0);
	of_store_32bit_in_host(zs_reg + 0, 0x13800);
	of_store_32bit_in_host(zs_reg + 4, 0x100);
	of_store_32bit_in_host(zs_reg + 8, 0x100);
	of_store_32bit_in_host(zs_reg + 12, 0x100);
	of_store_32bit_in_host(zs_reg + 16, 0x200);
	of_store_32bit_in_host(zs_reg + 20, 0x100);
	of_add_prop(ofd, "/bandit/gc/zs/zstty1",
	    "reg", zs_reg, 6*sizeof(uint32_t), 0);

	/*  Port 0:  */
	CHECK_ALLOCATION(zs_interrupts = (unsigned char *) malloc(3 * sizeof(uint32_t)));
	CHECK_ALLOCATION(zs_reg = (unsigned char *) malloc(6 * sizeof(uint32_t)));

	of_add_device(ofd, "zstty0", "/bandit/gc/zs");
	of_add_prop_str(machine, ofd, "/bandit/gc/zs/zstty0", "name", "ch-b");
	of_store_32bit_in_host(zs_interrupts + 0, 15);
	of_store_32bit_in_host(zs_interrupts + 4, 0);
	of_store_32bit_in_host(zs_interrupts + 8, 0);
	of_add_prop(ofd, "/bandit/gc/zs/zstty0", "interrupts", zs_interrupts,
	    3*sizeof(uint32_t), 0);
	of_store_32bit_in_host(zs_reg + 0, 0x13400);
	of_store_32bit_in_host(zs_reg + 4, 0x100);
	of_store_32bit_in_host(zs_reg + 8, 0x100);
	of_store_32bit_in_host(zs_reg + 12, 0x100);
	of_store_32bit_in_host(zs_reg + 16, 0x200);
	of_store_32bit_in_host(zs_reg + 20, 0x100);
	of_add_prop(ofd, "/bandit/gc/zs/zstty0",
	    "reg", zs_reg, 6*sizeof(uint32_t), 0);
}


/*
 *  of_emul_init_uninorth():
 */
void of_emul_init_uninorth(struct machine *machine)
{
	struct of_data *ofd = machine->md.of_data;
	unsigned char *uninorth_reg, *uninorth_bus_range, *uninorth_ranges;
	unsigned char *macio_aa, *ata_interrupts, *ata_reg;
	struct of_device *ic;
	const char *n = "pci@e2000000";
	const char *macio = "mac-io";

	of_add_device(ofd, n, "/");
	of_add_prop_str(machine, ofd, n, "name", "pci");
	of_add_prop_str(machine, ofd, n, "device_type", "pci");
	of_add_prop_str(machine, ofd, n, "compatible", "uni-north");

	CHECK_ALLOCATION(uninorth_reg = (unsigned char *) malloc(2 * sizeof(uint32_t)));
	CHECK_ALLOCATION(uninorth_bus_range = (unsigned char *) malloc(2 * sizeof(uint32_t)));
	CHECK_ALLOCATION(uninorth_ranges = (unsigned char *) malloc(12 * sizeof(uint32_t)));
	CHECK_ALLOCATION(macio_aa = (unsigned char *) malloc(5 * sizeof(uint32_t)));
	CHECK_ALLOCATION(ata_interrupts = (unsigned char *) malloc(6 * sizeof(uint32_t)));
	CHECK_ALLOCATION(ata_reg = (unsigned char *) malloc(8 * sizeof(uint32_t)));

	of_store_32bit_in_host(uninorth_reg + 0, 0xe2000000);
	of_store_32bit_in_host(uninorth_reg + 4, 0);	/*  not used?  */
	of_add_prop(ofd, n, "reg", uninorth_reg, 2*sizeof(uint32_t), 0);

	of_store_32bit_in_host(uninorth_bus_range + 0, 0);
	of_store_32bit_in_host(uninorth_bus_range + 4, 0);
	of_add_prop(ofd, n, "bus-range", uninorth_bus_range,
	    2*sizeof(uint32_t), 0);

	/*  MEM:  */
	of_store_32bit_in_host(uninorth_ranges + 0, 0x02000000);
	of_store_32bit_in_host(uninorth_ranges + 4, 0);
	of_store_32bit_in_host(uninorth_ranges + 8, 0);
	of_store_32bit_in_host(uninorth_ranges + 12, 0xd0000000);
	of_store_32bit_in_host(uninorth_ranges + 16, 0);
	of_store_32bit_in_host(uninorth_ranges + 20, 0x04000000);
	/*  IO:  */
	of_store_32bit_in_host(uninorth_ranges + 24, 0x01000000);
	of_store_32bit_in_host(uninorth_ranges + 28, 0);
	of_store_32bit_in_host(uninorth_ranges + 32, 0);
	of_store_32bit_in_host(uninorth_ranges + 36, 0xe2000000);
	of_store_32bit_in_host(uninorth_ranges + 40, 0);
	of_store_32bit_in_host(uninorth_ranges + 44, 0x01000000);
	of_add_prop(ofd, n, "ranges", uninorth_ranges,
	    12*sizeof(uint32_t), 0);

	ic = of_add_device(ofd, macio, "/");
	memset(macio_aa, 0, 20);
	of_store_32bit_in_host(macio_aa + 0, 15 << 11); /* pci tag */
	of_store_32bit_in_host(macio_aa + 8, 0xf3000000);
	of_add_prop(ofd, macio, "assigned-addresses", macio_aa,
	    5*sizeof(uint32_t), 0);
/*	of_add_prop(ofd, n, "assigned-addresses", macio_aa,
	    5*sizeof(uint32_t), 0); */
	of_add_prop_int32(ofd, "/chosen", "interrupt-controller", ic->handle);

	of_add_device(ofd, "bandit", "/");
	of_add_device(ofd, "gc", "/bandit");
	of_add_prop(ofd, "/bandit/gc", "assigned-addresses", macio_aa,
	    5*sizeof(uint32_t), 0);

	if (diskimage_exist(machine, 0, DISKIMAGE_IDE) ||
	    diskimage_exist(machine, 1, DISKIMAGE_IDE)) {
		char tmpstr[400];
		of_add_device(ofd, "ata", "/bandit/gc");
		of_add_prop_str(machine, ofd, "/bandit/gc/ata", "name", "ata");
		of_add_prop_str(machine, ofd, "/bandit/gc/ata", "compatible",
		    "heathrow-ata");
		of_store_32bit_in_host(ata_interrupts + 0, 13);
		of_store_32bit_in_host(ata_interrupts + 4, 0);
		of_store_32bit_in_host(ata_interrupts + 8, 0);
		of_store_32bit_in_host(ata_interrupts + 12, 0);
		of_store_32bit_in_host(ata_interrupts + 16, 0);
		of_store_32bit_in_host(ata_interrupts + 20, 0);
		of_add_prop(ofd, "/bandit/gc/ata", "interrupts", ata_interrupts,
		    6*sizeof(uint32_t), 0);
		of_store_32bit_in_host(ata_reg + 0, 0x20000);
		of_store_32bit_in_host(ata_reg + 4, 0);
		of_store_32bit_in_host(ata_reg + 8, 0x21000);
		of_store_32bit_in_host(ata_reg + 12, 0x22000);
		of_store_32bit_in_host(ata_reg + 16, 0);
		of_store_32bit_in_host(ata_reg + 20, 0);
		of_store_32bit_in_host(ata_reg + 24, 0);
		of_store_32bit_in_host(ata_reg + 28, 0);
		of_add_prop(ofd, "/bandit/gc/ata", "reg", ata_reg,
		    8*sizeof(uint32_t), 0);

		snprintf(tmpstr, sizeof(tmpstr), "wdc addr=0xf3020000 "
		    "irq=%s.cpu[%i].gc.lo.21 addr_mult=0x10", machine->path,
		    machine->bootstrap_cpu);
		device_add(machine, tmpstr);
	}
}


/*
 *  of_emul_init():
 *
 *  This function creates an OpenFirmware emulation instance.
 */
struct of_data *of_emul_init(struct machine *machine, struct vfb_data *vfb_data,
	uint64_t fb_addr, int fb_xsize, int fb_ysize)
{
	unsigned char *memory_reg, *memory_av;
	unsigned char *zs_assigned_addresses;
	struct of_device *memory_dev, *mmu, *devstdout, *devstdin;
	struct of_data *ofd;
	int i;

	CHECK_ALLOCATION(ofd = (struct of_data *) malloc(sizeof(struct of_data)));
	memset(ofd, 0, sizeof(struct of_data));

	ofd->vfb_data = vfb_data;

	/*  Devices:  */

	/*  Root = device 1  */
	of_add_device(ofd, "", "");

	of_add_device(ofd, "io", "/");
	devstdin  = of_add_device(ofd, "stdin", "/io");
	devstdout = of_add_device(ofd, "stdout", "/io");

	if (machine->x11_md.in_use) {
		fatal("!\n!  TODO: keyboard + framebuffer for MacPPC\n!\n");

		of_add_prop_str(machine, ofd, "/io/stdin", "name",
		    "keyboard");
		of_add_prop_str(machine, ofd, "/io", "name", "adb");

		of_add_prop_str(machine, ofd, "/io/stdout", "device_type",
		    "display");
		of_add_prop_int32(ofd, "/io/stdout", "width", fb_xsize);
		of_add_prop_int32(ofd, "/io/stdout", "height", fb_ysize);
		of_add_prop_int32(ofd, "/io/stdout", "linebytes", fb_xsize * 1);
		of_add_prop_int32(ofd, "/io/stdout", "depth", 8);
		of_add_prop_int32(ofd, "/io/stdout", "address", fb_addr);
	} else {
		CHECK_ALLOCATION(zs_assigned_addresses = (unsigned char *) malloc(12));
		memset(zs_assigned_addresses, 0, 12);

		of_add_prop_str(machine, ofd, "/io/stdin", "name", "ch-b");
		of_add_prop_str(machine, ofd, "/io/stdin", "device_type",
		    "serial");
		of_add_prop_int32(ofd, "/io/stdin", "reg", 0xf3013000);
		of_add_prop(ofd, "/io/stdin", "assigned-addresses",
		    zs_assigned_addresses, 12, 0);

		of_add_prop_str(machine, ofd, "/io/stdout", "device_type",
		    "serial");
	}

	of_add_device(ofd, "cpus", "/");
	for (i=0; i<machine->ncpus; i++) {
		char tmp[50];
		snprintf(tmp, sizeof(tmp), "@%x", i);
		of_add_device(ofd, tmp, "/cpus");
		snprintf(tmp, sizeof(tmp), "/cpus/@%x", i);
		of_add_prop_str(machine, ofd, tmp, "device_type", "cpu");
		of_add_prop_int32(ofd, tmp, "timebase-frequency",
		    machine->emulated_hz / 4);
		of_add_prop_int32(ofd, tmp, "clock-frequency",
		    machine->emulated_hz);
		of_add_prop_int32(ofd, tmp, "reg", i);
	}

	mmu = of_add_device(ofd, "mmu", "/");

	/*  TODO:  */
	of_add_prop(ofd, "/mmu", "translations", NULL, 0, 0);

	of_add_device(ofd, "chosen", "/");
	of_add_prop_int32(ofd, "/chosen", "mmu", mmu->handle);
	of_add_prop_int32(ofd, "/chosen", "stdin", devstdin->handle);
	of_add_prop_int32(ofd, "/chosen", "stdout", devstdout->handle);

	memory_dev = of_add_device(ofd, "memory", "/");
	CHECK_ALLOCATION(memory_reg = (unsigned char *) malloc(2 * sizeof(uint32_t)));
	CHECK_ALLOCATION(memory_av = (unsigned char *) malloc(2 * sizeof(uint32_t)));

	of_store_32bit_in_host(memory_reg + 0, 0);
	of_store_32bit_in_host(memory_reg + 4, machine->physical_ram_in_mb<<20);
	of_store_32bit_in_host(memory_av + 0, 10 << 20);
	of_store_32bit_in_host(memory_av + 4,
	    (machine->physical_ram_in_mb - 10) << 20);
	of_add_prop(ofd, "/memory", "reg", memory_reg, 2 * sizeof(uint32_t), 0);
	of_add_prop(ofd, "/memory", "available",memory_av,2*sizeof(uint32_t),0);
	of_add_prop_str(machine, ofd, "/memory","device_type","memory"/*?*/);

	of_add_prop_int32(ofd, "/chosen", "memory", memory_dev->handle);

	/*  Services:  */
	of_add_service(ofd, "call-method", of__call_method_2_2, 2, 2);
	of_add_service(ofd, "call-method", of__call_method_3_4, 3, 4);
	of_add_service(ofd, "call-method", of__call_method_5_2, 5, 2);
	of_add_service(ofd, "call-method", of__call_method_6_1, 6, 1);
	of_add_service(ofd, "call-method", of__call_method_6_2, 6, 2);
	of_add_service(ofd, "child", of__child, 1, 1);
	of_add_service(ofd, "claim", of__claim, 3, 1);
	of_add_service(ofd, "exit", of__exit, 0, 0);
	of_add_service(ofd, "finddevice", of__finddevice, 1, 1);
	of_add_service(ofd, "getprop", of__getprop, 4, 1);
	of_add_service(ofd, "getproplen", of__getproplen, 2, 1);
	of_add_service(ofd, "instance-to-package",
	    of__instance_to_package, 1, 1);
	of_add_service(ofd, "interpret", of__interpret_1, 1, 1);
	of_add_service(ofd, "interpret", of__interpret_2, 1, 2);
	of_add_service(ofd, "package-to-path", of__package_to_path, 3, 1);
	of_add_service(ofd, "parent", of__parent, 1, 1);
	of_add_service(ofd, "peer", of__peer, 1, 1);
	of_add_service(ofd, "read", of__read, 3, 1);
	of_add_service(ofd, "write", of__write, 3, 1);

	if (verbose >= 2)
		of_dump_all(ofd);

	machine->md.of_data = ofd;

	return ofd;
}


/*
 *  of_emul():
 *
 *  OpenFirmware call emulation.
 */
int of_emul(struct cpu *cpu)
{
	int i, nargs, nret, ofs, retval = 0;
	char service[50];
	char *arg[OF_N_MAX_ARGS];
	uint64_t base, ptr;
	struct of_service *os;
	struct of_data *of_data = cpu->machine->md.of_data;

	if (of_data == NULL) {
		fatal("of_emul(): no of_data struct?\n");
		exit(1);
	}

	/*
	 *  The first argument register points to "prom_args":
	 *
	 *	char *service;		(probably 32 bit)
	 *	int nargs;
	 *	int nret;
	 *	char *args[10];
	 */

	switch (cpu->machine->arch) {
	case ARCH_ARM:
		base = cpu->cd.arm.r[0];
		break;
	case ARCH_PPC:
		base = cpu->cd.ppc.gpr[3];
		break;
	default:fatal("of_emul(): unimplemented arch (TODO)\n");
		exit(1);
	}

	/*  TODO: how about 64-bit OpenFirmware?  */
	ptr   = load_32bit_word(cpu, base);
	nargs = load_32bit_word(cpu, base + 4);
	nret  = load_32bit_word(cpu, base + 8);

	readstr(cpu, ptr, service, sizeof(service));

	debug("[ of: %s(", service);
	ofs = 12;
	for (i=0; i<nargs; i++) {
		int x;
		if (i > 0)
			debug(", ");
		if (i >= OF_N_MAX_ARGS) {
			fatal("TOO MANY ARGS!");
			continue;
		}

		ptr = load_32bit_word(cpu, base + ofs);

		CHECK_ALLOCATION(arg[i] = (char *) malloc(OF_ARG_MAX_LEN + 1));
		memset(arg[i], 0, OF_ARG_MAX_LEN + 1);

		x = ptr;
		if (x > -256 && x < 256) {
			debug("%i", x);
		} else {
			readstr(cpu, ptr, arg[i], OF_ARG_MAX_LEN);
			if (arg[i][0])
				debug("\"%s\"", arg[i]);
			else
				debug("0x%x", x);
		}
		ofs += sizeof(uint32_t);
	}
	debug(") ]\n");

	/*  Note: base + ofs points to the first return slot.  */

	os = of_data->of_services;
	while (os != NULL) {
		if (strcmp(service, os->name) == 0 &&
		    nargs == os->n_args && nret == os->n_ret_args) {
			retval = os->f(cpu, arg, base, ofs);
			break;
		}
		os = os->next;
	}

	if (os == NULL) {
		quiet_mode = 0;
		cpu_register_dump(cpu->machine, cpu, 1, 0);
		printf("\n");
		fatal("[ of: unimplemented service \"%s\" with %i input "
		    "args and %i output values ]\n", service, nargs, nret);
		cpu->running = 0;
	}

	for (i=0; i<nargs; i++)
		free(arg[i]);

	/*  Return:  */
	switch (cpu->machine->arch) {
	case ARCH_ARM:
		cpu->cd.arm.r[0] = retval;
		break;
	case ARCH_PPC:
		cpu->cd.ppc.gpr[3] = retval;
		break;
	default:fatal("of_emul(): TODO: unimplemented arch (Retval)\n");
		exit(1);
	}

	return 1;
}

