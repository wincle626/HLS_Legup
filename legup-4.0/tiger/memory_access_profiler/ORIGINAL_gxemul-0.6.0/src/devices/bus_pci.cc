/*
 *  Copyright (C) 2004-2009  Anders Gavare.  All rights reserved.
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
 *  COMMENT: Generic PCI bus framework
 *
 *  This is not a normal "device", but is used by individual PCI controllers
 *  and devices.
 *
 *  See NetBSD's pcidevs.h for more PCI vendor and device identifiers.
 *
 *  TODO:
 *
 *	x) Allow guest OSes to do runtime address fixups (i.e. actually
 *	   move a device from one address to another).
 *
 *	x) Generalize the PCI and legacy ISA interrupt routing stuff.
 *
 *	x) Make sure that pci_little_endian is used correctly everywhere.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUS_PCI_C

#include "bus_isa.h"
#include "bus_pci.h"
#include "cpu.h"
#include "device.h"
#include "devices.h"
#include "diskimage.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"
#include "wdc.h"

#include "thirdparty/cpc700reg.h"

extern int verbose;


#ifdef UNSTABLE_DEVEL
#define debug fatal
#endif


/*
 *  bus_pci_decompose_1():
 *
 *  Helper function for decomposing Mechanism 1 tags.
 */
void bus_pci_decompose_1(uint32_t t, int *bus, int *dev, int *func, int *reg)
{
	*bus  = (t >> 16) & 0xff;
	*dev  = (t >> 11) & 0x1f;
	*func = (t >>  8) &  0x7;
	*reg  =  t        & 0xff;

	/*  Warn about unaligned register access:  */
	if (t & 3)
		fatal("[ bus_pci_decompose_1: WARNING: reg = 0x%02x ]\n",
		    t & 0xff);
}


/*
 *  bus_pci_data_access():
 *
 *  Reads from or writes to the PCI configuration registers of a device.
 */
void bus_pci_data_access(struct cpu *cpu, struct pci_data *pci_data,
	uint64_t *data, int len, int writeflag)
{
	struct pci_device *dev;
	unsigned char *cfg_base;
	uint64_t x, idata = *data;
	int i;

	/*  Scan through the list of pci_device entries.  */
	dev = pci_data->first_device;
	while (dev != NULL) {
		if (dev->bus      == pci_data->cur_bus &&
		    dev->function == pci_data->cur_func &&
		    dev->device   == pci_data->cur_device)
			break;
		dev = dev->next;
	}

	/*  No device? Then return emptiness.  */
	if (dev == NULL) {
		if (writeflag == MEM_READ) {
			if (pci_data->cur_reg == 0)
				*data = (uint64_t) -1;
			else
				*data = 0;
		} else {
			fatal("[ bus_pci_data_access(): write to non-existant"
			    " device? ]\n");
		}
		return;
	}

	/*  Return normal config data, or length data?  */
	if (pci_data->last_was_write_ffffffff &&
	    pci_data->cur_reg >= PCI_MAPREG_START &&
	    pci_data->cur_reg <= PCI_MAPREG_END - 4)
		cfg_base = dev->cfg_mem_size;
	else
		cfg_base = dev->cfg_mem;

	/*  Read data as little-endian:  */
	x = 0;
	for (i=len-1; i>=0; i--) {
		int ofs = pci_data->cur_reg + i;
		x <<= 8;
		x |= cfg_base[ofs & (PCI_CFG_MEM_SIZE - 1)];
	}

	/*  Register write:  */
	if (writeflag == MEM_WRITE) {
		debug("[ bus_pci: write to PCI DATA: data = 0x%08llx ]\n",
		    (long long)idata);
		if (idata == 0xffffffffULL &&
		    pci_data->cur_reg >= PCI_MAPREG_START &&
		    pci_data->cur_reg <= PCI_MAPREG_END - 4) {
			pci_data->last_was_write_ffffffff = 1;
			return;
		}

		if (dev->cfg_reg_write == NULL ||
		    dev->cfg_reg_write(dev, pci_data->cur_reg, *data) == 0) {
			/*  Print a warning for unhandled writes:  */
			debug("[ bus_pci: write to PCI DATA: data = 0x%08llx"
			    " (current value = 0x%08llx); NOT YET"
			    " SUPPORTED. bus %i, device %i, function %i (%s)"
			    " register 0x%02x ]\n", (long long)idata,
			    (long long)x, pci_data->cur_bus,
			    pci_data->cur_device, pci_data->cur_func,
			    dev->name, pci_data->cur_reg);

			/*  Special warning, to detect if NetBSD's special
			    detection of PCI devices fails:  */
			if (pci_data->cur_reg == PCI_COMMAND_STATUS_REG
			    && !((*data) & PCI_COMMAND_IO_ENABLE)) {
				fatal("\n[ NetBSD PCI detection stuff not"
				    " yet implemented for device '%s' ]\n",
				    dev->name);
			}
		}
		return;
	}

	/*  Register read:  */
	*data = x;

	pci_data->last_was_write_ffffffff = 0;

	debug("[ bus_pci: read from PCI DATA, bus %i, device "
	    "%i, function %i (%s) register 0x%02x: (len=%i) 0x%08lx ]\n",
	    pci_data->cur_bus, pci_data->cur_device, pci_data->cur_func,
	    dev->name, pci_data->cur_reg, len, (long)*data);
}


/*
 *  bus_pci_setaddr():
 *
 *  Sets the address in preparation for a PCI register transfer.
 */
void bus_pci_setaddr(struct cpu *cpu, struct pci_data *pci_data,
	int bus, int device, int function, int reg)
{
	if (cpu == NULL || pci_data == NULL) {
		fatal("bus_pci_setaddr(): NULL ptr\n");
		exit(1);
	}

	pci_data->cur_bus = bus;
	pci_data->cur_device = device;
	pci_data->cur_func = function;
	pci_data->cur_reg = reg;
}


/*
 *  bus_pci_add():
 *
 *  Add a PCI device to a bus_pci device.
 */
void bus_pci_add(struct machine *machine, struct pci_data *pci_data,
	struct memory *mem, int bus, int device, int function,
	const char *name)
{
	struct pci_device *pd;
	int ofs;
	void (*init)(struct machine *, struct memory *, struct pci_device *);

	if (pci_data == NULL) {
		fatal("bus_pci_add(): pci_data == NULL!\n");
		abort();
	}

	/*  Find the PCI device:  */
	init = pci_lookup_initf(name);

	/*  Make sure this bus/device/function number isn't already in use:  */
	pd = pci_data->first_device;
	while (pd != NULL) {
		if (pd->bus == bus && pd->device == device &&
		    pd->function == function) {
			fatal("bus_pci_add(): (bus %i, device %i, function"
			    " %i) already in use\n", bus, device, function);
			exit(1);
		}
		pd = pd->next;
	}

	CHECK_ALLOCATION(pd = (struct pci_device *) malloc(sizeof(struct pci_device)));
	memset(pd, 0, sizeof(struct pci_device));

	/*  Add the new device first in the PCI bus' chain:  */
	pd->next = pci_data->first_device;
	pci_data->first_device = pd;

	CHECK_ALLOCATION(pd->name = strdup(name));
	pd->pcibus   = pci_data;
	pd->bus      = bus;
	pd->device   = device;
	pd->function = function;

	/*
	 *  Initialize with some default values:
	 *
	 *  TODO:  The command status register is best to set up per device.
	 *  The size registers should also be set up on a per-device basis.
	 */
	PCI_SET_DATA(PCI_COMMAND_STATUS_REG,
	    PCI_COMMAND_IO_ENABLE | PCI_COMMAND_MEM_ENABLE);
	for (ofs = PCI_MAPREG_START; ofs < PCI_MAPREG_END; ofs += 4)
		PCI_SET_DATA_SIZE(ofs, 0x00100000 - 1);

	if (init == NULL) {
		fatal("No init function for PCI device \"%s\"?\n", name);
		exit(1);
	}

	/*  Call the PCI device' init function:  */
	init(machine, mem, pd);
}


/*
 *  allocate_device_space():
 *
 *  Used by glue code (see below) to allocate space for a PCI device.
 *
 *  The returned values in portp and memp are the actual (emulated) addresses
 *  that the device should use. (Normally only one of these is actually used.)
 *
 *  TODO: PCI irqs?
 */
static void allocate_device_space(struct pci_device *pd,
	uint64_t portsize, uint64_t memsize,
	uint64_t *portp, uint64_t *memp)
{
	uint64_t port, mem;

	/*  Calculate an aligned starting port:  */
	port = pd->pcibus->cur_pci_portbase;
	if (portsize != 0) {
		port = ((port - 1) | (portsize - 1)) + 1;
		pd->pcibus->cur_pci_portbase = port;
		PCI_SET_DATA(PCI_MAPREG_START + pd->cur_mapreg_offset,
		    port | PCI_MAPREG_TYPE_IO);
		PCI_SET_DATA_SIZE(PCI_MAPREG_START + pd->cur_mapreg_offset,
		    ((portsize - 1) & ~0xf) | 0xd);
		pd->cur_mapreg_offset += sizeof(uint32_t);
	}

	/*  Calculate an aligned starting memory location:  */
	mem = pd->pcibus->cur_pci_membase;
	if (memsize != 0) {
		mem = ((mem - 1) | (memsize - 1)) + 1;
		pd->pcibus->cur_pci_membase = mem;
		PCI_SET_DATA(PCI_MAPREG_START + pd->cur_mapreg_offset, mem);
		PCI_SET_DATA_SIZE(PCI_MAPREG_START + pd->cur_mapreg_offset,
		    ((memsize - 1) & ~0xf) | 0x0);
		pd->cur_mapreg_offset += sizeof(uint32_t);
	}

	*portp = port + pd->pcibus->pci_actual_io_offset;
	*memp  = mem  + pd->pcibus->pci_actual_mem_offset;

	if (verbose >= 2) {
		debug("pci device '%s' at", pd->name);
		if (portsize != 0)
			debug(" port 0x%llx-0x%llx", (long long)pd->pcibus->
			    cur_pci_portbase, (long long)(pd->pcibus->
			    cur_pci_portbase + portsize - 1));
		if (memsize != 0)
			debug(" mem 0x%llx-0x%llx", (long long)pd->pcibus->
			    cur_pci_membase, (long long)(pd->pcibus->
			    cur_pci_membase + memsize - 1));
		debug("\n");
	}

	pd->pcibus->cur_pci_portbase += portsize;
	pd->pcibus->cur_pci_membase += memsize;
}


/*
 *  bus_pci_init():
 *
 *  This doesn't register a device, but instead returns a pointer to a struct
 *  which should be passed to other bus_pci functions when accessing the bus.
 *
 *  irq_path is the interrupt path to the PCI controller.
 *
 *  pci_portbase, pci_membase, and pci_irqbase are the port, memory, and
 *  interrupt bases for PCI devices (as found in the configuration registers).
 *
 *  pci_actual_io_offset and pci_actual_mem_offset are the offset from
 *  the values in the configuration registers to the actual (emulated) device.
 *
 *  isa_portbase, isa_membase, and isa_irqbase are the port, memory, and
 *  interrupt bases for legacy ISA devices.
 */
struct pci_data *bus_pci_init(struct machine *machine, const char *irq_path,
	uint64_t pci_actual_io_offset, uint64_t pci_actual_mem_offset,
	uint64_t pci_portbase, uint64_t pci_membase, const char *pci_irqbase,
	uint64_t isa_portbase, uint64_t isa_membase, const char *isa_irqbase)
{
	struct pci_data *d;

	CHECK_ALLOCATION(d = (struct pci_data *) malloc(sizeof(struct pci_data)));
	memset(d, 0, sizeof(struct pci_data));

	CHECK_ALLOCATION(d->irq_path     = strdup(irq_path));
	CHECK_ALLOCATION(d->irq_path_isa = strdup(isa_irqbase));
	CHECK_ALLOCATION(d->irq_path_pci = strdup(pci_irqbase));

	d->pci_actual_io_offset  = pci_actual_io_offset;
	d->pci_actual_mem_offset = pci_actual_mem_offset;
	d->pci_portbase          = pci_portbase;
	d->pci_membase           = pci_membase;
	d->isa_portbase          = isa_portbase;
	d->isa_membase           = isa_membase;

	d->cur_pci_portbase = d->pci_portbase;
	d->cur_pci_membase  = d->pci_membase;

	/*  Assume that the first 64KB could be used by legacy ISA devices:  */
	if (d->isa_portbase != 0 || d->isa_membase != 0) {
		d->cur_pci_portbase += 0x10000;
		d->cur_pci_membase  += 0x10000;
	}

	return d;
}



/******************************************************************************
 *                                                                            *
 *  The following is glue code for PCI controllers and devices. The glue      *
 *  code does the minimal stuff necessary to get an emulated OS to detect     *
 *  the device (i.e. set up PCI configuration registers), and then if         *
 *  necessary adds a "normal" device.                                         *
 *                                                                            *
 ******************************************************************************/



/*
 *  Integraphics Systems "igsfb" Framebuffer (graphics) card, used in at
 *  least the NetWinder.
 */

#define	PCI_VENDOR_INTEGRAPHICS		0x10ea

PCIINIT(igsfb)
{
	char tmpstr[200];

	PCI_SET_DATA(PCI_ID_REG,
	    PCI_ID_CODE(PCI_VENDOR_INTEGRAPHICS, 0x2010));

	PCI_SET_DATA(PCI_CLASS_REG,
	    PCI_CLASS_CODE(PCI_CLASS_DISPLAY,
	    PCI_SUBCLASS_DISPLAY_VGA, 0) + 0x01);

	/*  TODO  */
	PCI_SET_DATA(0x10, 0x08000000);

	snprintf(tmpstr, sizeof(tmpstr), "igsfb addr=0x%llx",
	    (long long)(pd->pcibus->isa_membase + 0x08000000));
	device_add(machine, tmpstr);
}



/*
 *  S3 ViRGE graphics.
 *
 *  TODO: Only emulates a standard VGA card, so far.
 */

#define PCI_VENDOR_S3			0x5333
#define PCI_PRODUCT_S3_VIRGE		0x5631
#define	PCI_PRODUCT_S3_VIRGE_DX		0x8a01

PCIINIT(s3_virge)
{
	PCI_SET_DATA(PCI_ID_REG,
	    PCI_ID_CODE(PCI_VENDOR_S3, PCI_PRODUCT_S3_VIRGE_DX));

	PCI_SET_DATA(PCI_CLASS_REG,
	    PCI_CLASS_CODE(PCI_CLASS_DISPLAY,
	    PCI_SUBCLASS_DISPLAY_VGA, 0) + 0x01);

	dev_vga_init(machine, mem, pd->pcibus->isa_membase + 0xa0000,
	    pd->pcibus->isa_portbase + 0x3c0, machine->machine_name);
}



/*
 *  Acer Labs M5229 PCI-IDE (UDMA) controller.
 *  Acer Labs M1543 PCI->ISA bridge.
 */

#define PCI_VENDOR_ALI			0x10b9
#define PCI_PRODUCT_ALI_M1543		0x1533	/*  NOTE: not 1543  */
#define	PCI_PRODUCT_ALI_M5229		0x5229

PCIINIT(ali_m1543)
{
	PCI_SET_DATA(PCI_ID_REG,
	    PCI_ID_CODE(PCI_VENDOR_ALI, PCI_PRODUCT_ALI_M1543));

	PCI_SET_DATA(PCI_CLASS_REG, PCI_CLASS_CODE(PCI_CLASS_BRIDGE,
	    PCI_SUBCLASS_BRIDGE_ISA, 0) + 0xc3);

	PCI_SET_DATA(PCI_BHLC_REG,
	    PCI_BHLC_CODE(0,0, 1 /* multi-function */, 0x40,0));

	/*  Linux uses these to detect which IRQ the IDE controller uses:  */
	PCI_SET_DATA(0x44, 0x0000000e);
	PCI_SET_DATA(0x58, 0x00000003);

	switch (machine->machine_type) {
	case MACHINE_CATS:
		bus_isa_init(machine, pd->pcibus->irq_path_isa,
		    BUS_ISA_PCKBC_FORCE_USE | BUS_ISA_PCKBC_NONPCSTYLE,
		    0x7c000000, 0x80000000);
		break;
	default:fatal("ali_m1543 init: unimplemented machine type\n");
		exit(1);
	}
}

PCIINIT(ali_m5229)
{
	char tmpstr[300], irqstr[300];

	PCI_SET_DATA(PCI_ID_REG,
	    PCI_ID_CODE(PCI_VENDOR_ALI, PCI_PRODUCT_ALI_M5229));

	PCI_SET_DATA(PCI_CLASS_REG, PCI_CLASS_CODE(PCI_CLASS_MASS_STORAGE,
	    PCI_SUBCLASS_MASS_STORAGE_IDE, 0x60) + 0xc1);

	switch (machine->machine_type) {
	case MACHINE_CATS:
		/*  CATS ISA interrupts are at footbridge irq 10:  */
		snprintf(irqstr, sizeof(irqstr), "%s.10.isa",
		    pd->pcibus->irq_path);
		break;
	default:fatal("ali_m5229 init: unimplemented machine type\n");
		exit(1);
	}

	if (diskimage_exist(machine, 0, DISKIMAGE_IDE) ||
	    diskimage_exist(machine, 1, DISKIMAGE_IDE)) {
		snprintf(tmpstr, sizeof(tmpstr), "wdc addr=0x%llx irq=%s.%i",
		    (long long)(pd->pcibus->isa_portbase + 0x1f0),
		    irqstr, 14);
		device_add(machine, tmpstr);
	}

	/*  The secondary channel is disabled. TODO: fix this.  */
}



/*
 *  Adaptec AHC SCSI controller.
 */

#define PCI_VENDOR_ADP  0x9004          /* Adaptec */
#define PCI_VENDOR_ADP2 0x9005          /* Adaptec (2nd PCI Vendor ID) */
#define PCI_PRODUCT_ADP_2940U   0x8178          /* AHA-2940 Ultra */
#define PCI_PRODUCT_ADP_2940UP  0x8778          /* AHA-2940 Ultra Pro */

PCIINIT(ahc)
{
	/*  Numbers taken from a Adaptec 2940U:  */
	/*  http://mail-index.netbsd.org/netbsd-bugs/2000/04/29/0000.html  */

	PCI_SET_DATA(PCI_ID_REG, PCI_ID_CODE(PCI_VENDOR_ADP,
	    PCI_PRODUCT_ADP_2940U));

	PCI_SET_DATA(PCI_COMMAND_STATUS_REG, 0x02900007);

	PCI_SET_DATA(PCI_CLASS_REG, PCI_CLASS_CODE(PCI_CLASS_MASS_STORAGE,
	    PCI_SUBCLASS_MASS_STORAGE_SCSI, 0) + 0x01);

	PCI_SET_DATA(PCI_BHLC_REG, 0x00004008);

	/*  1 = type i/o. 0x0000e801;  address?  */
	/*  second address reg = 0xf1002000?  */
	PCI_SET_DATA(PCI_MAPREG_START + 0x00, 0x00000001);
	PCI_SET_DATA(PCI_MAPREG_START + 0x04, 0x00000000);

	PCI_SET_DATA(PCI_MAPREG_START + 0x08, 0x00000000);
	PCI_SET_DATA(PCI_MAPREG_START + 0x0c, 0x00000000);
	PCI_SET_DATA(PCI_MAPREG_START + 0x10, 0x00000000);
	PCI_SET_DATA(PCI_MAPREG_START + 0x14, 0x00000000);
	PCI_SET_DATA(PCI_MAPREG_START + 0x18, 0x00000000);

	/*  Subsystem vendor ID? 0x78819004?  */
	PCI_SET_DATA(PCI_MAPREG_START + 0x1c, 0x00000000);

	PCI_SET_DATA(0x30, 0xef000000);
	PCI_SET_DATA(PCI_CAPLISTPTR_REG, 0x000000dc);
	PCI_SET_DATA(0x38, 0x00000000);
	PCI_SET_DATA(PCI_INTERRUPT_REG, 0x08080109);	/*  interrupt pin A  */

	/*
	 *  TODO:  this address is based on what NetBSD/sgimips uses
	 *  on SGI IP32 (O2). Fix this!
	 */

	device_add(machine, "ahc addr=0x18000000");

	/*  OpenBSD/sgi snapshots sometime between 2005-03-11 and
	    2005-04-04 changed to using 0x1a000000:  */
	dev_ram_init(machine, 0x1a000000, 0x2000000, DEV_RAM_MIRROR,
	    0x18000000);
}



/*
 *  Galileo Technology GT-64xxx PCI controller.
 *
 *	GT-64011	Used in Cobalt machines.
 *	GT-64120	Used in evbmips machines (Malta).
 *
 *  NOTE: This works in the opposite way compared to other devices; the PCI
 *  device is added from the normal device instead of the other way around.
 */

#define PCI_VENDOR_GALILEO           0x11ab    /* Galileo Technology */
#define PCI_PRODUCT_GALILEO_GT64011  0x4146    /* GT-64011 System Controller */
#define	PCI_PRODUCT_GALILEO_GT64120  0x4620    /* GT-64120 */
#define	PCI_PRODUCT_GALILEO_GT64260  0x6430    /* GT-64260 */

PCIINIT(gt64011)
{
	PCI_SET_DATA(PCI_ID_REG, PCI_ID_CODE(PCI_VENDOR_GALILEO,
	    PCI_PRODUCT_GALILEO_GT64011));

	PCI_SET_DATA(PCI_CLASS_REG, PCI_CLASS_CODE(PCI_CLASS_BRIDGE,
	    PCI_SUBCLASS_BRIDGE_HOST, 0) + 0x01);	/*  Revision 1  */
}

PCIINIT(gt64120)
{
	PCI_SET_DATA(PCI_ID_REG, PCI_ID_CODE(PCI_VENDOR_GALILEO,
	    PCI_PRODUCT_GALILEO_GT64120));

	PCI_SET_DATA(PCI_CLASS_REG, PCI_CLASS_CODE(PCI_CLASS_BRIDGE,
	    PCI_SUBCLASS_BRIDGE_HOST, 0) + 0x02);	/*  Revision 2?  */

	switch (machine->machine_type) {
	case MACHINE_EVBMIPS:
		PCI_SET_DATA(PCI_MAPREG_START + 0x10, 0x1be00000);
		break;
	}
}

PCIINIT(gt64260)
{
	PCI_SET_DATA(PCI_ID_REG, PCI_ID_CODE(PCI_VENDOR_GALILEO,
	    PCI_PRODUCT_GALILEO_GT64260));

	PCI_SET_DATA(PCI_CLASS_REG, PCI_CLASS_CODE(PCI_CLASS_BRIDGE,
	    PCI_SUBCLASS_BRIDGE_HOST, 0) + 0x01);	/*  Revision 1?  */
}



/*
 *  AMD PCnet Ethernet card.
 *
 *  "Am79c970A PCnet-PCI II rev 0" or similar.
 */

#define	PCI_VENDOR_AMD			0x1022	/* Advanced Micro Devices */
#define	PCI_PRODUCT_AMD_PCNET_PCI	0x2000	/* PCnet-PCI Ethernet */

PCIINIT(pcn)
{
	int irq;

	PCI_SET_DATA(PCI_ID_REG, PCI_ID_CODE(PCI_VENDOR_AMD,
	    PCI_PRODUCT_AMD_PCNET_PCI));

	PCI_SET_DATA(PCI_CLASS_REG, PCI_CLASS_CODE(PCI_CLASS_NETWORK,
	    PCI_SUBCLASS_NETWORK_ETHERNET, 0) + 0x00);	/*  Revision 0  */

	switch (machine->machine_type) {

	case MACHINE_EVBMIPS:
		irq = (1 << 8) + 10;	/*  TODO  */
		break;

	default:fatal("pcn in non-implemented machine type %i\n",
		    machine->machine_type);
		exit(1);
	}

	PCI_SET_DATA(PCI_INTERRUPT_REG, 0x01100000 | irq);

	/*
	 *  TODO: Add the pcn device here. The pcn device will need to work as
	 *  a wrapper for dev_le + all the DMA magic and whatever is required.
	 *  It's too much to implement right now.
	 */
}



/*
 *  Intel 31244   Serial ATA Controller
 *  Intel 82371SB PIIX3 PCI-ISA bridge
 *  Intel 82371AB PIIX4 PCI-ISA bridge
 *  Intel 82371SB IDE controller
 *  Intel 82371AB IDE controller
 *  Intel 82378ZB System I/O controller.
 */

#define	PCI_VENDOR_INTEL		0x8086
#define	PCI_PRODUCT_INTEL_31244		0x3200
#define	PCI_PRODUCT_INTEL_82371SB_ISA	0x7000
#define	PCI_PRODUCT_INTEL_82371SB_IDE	0x7010
#define	PCI_PRODUCT_INTEL_82371AB_ISA	0x7110
#define	PCI_PRODUCT_INTEL_82371AB_IDE	0x7111
#define	PCI_PRODUCT_INTEL_SIO		0x0484

PCIINIT(i31244)
{
	uint64_t port, memaddr;
	int irq = 0;

	PCI_SET_DATA(PCI_ID_REG, PCI_ID_CODE(PCI_VENDOR_INTEL,
	    PCI_PRODUCT_INTEL_31244));

	PCI_SET_DATA(PCI_CLASS_REG, PCI_CLASS_CODE(PCI_CLASS_MASS_STORAGE,
	    PCI_SUBCLASS_MASS_STORAGE_IDE, 0x33) + 0x00);

	switch (machine->machine_type) {
	case MACHINE_IQ80321:
		/*  S-PCI-X slot uses PCI IRQ A, int 29  */
		irq = (1 << 8) + 29;
		break;
	default:fatal("i31244 in non-implemented machine type %i\n",
		    machine->machine_type);
		exit(1);
	}

	PCI_SET_DATA(PCI_INTERRUPT_REG, 0x01100000 | irq);

	allocate_device_space(pd, 0x1000, 0, &port, &memaddr);
	allocate_device_space(pd, 0x1000, 0, &port, &memaddr);

	/*  PCI IDE using dev_wdc:  */
	if (diskimage_exist(machine, 0, DISKIMAGE_IDE) ||
	    diskimage_exist(machine, 1, DISKIMAGE_IDE)) {
		char tmpstr[150];
		snprintf(tmpstr, sizeof(tmpstr), "wdc addr=0x%llx irq=%s.%i",
		    (long long)(pd->pcibus->pci_actual_io_offset + 0),
		    pd->pcibus->irq_path_pci, irq & 255);
		device_add(machine, tmpstr);
	}
}

int piix_isa_cfg_reg_write(struct pci_device *pd, int reg, uint32_t value)
{
	switch (reg) {
	case PCI_MAPREG_START:
	case PCI_MAPREG_START + 4:
	case PCI_MAPREG_START + 8:
	case PCI_MAPREG_START + 12:
	case PCI_MAPREG_START + 16:
	case PCI_MAPREG_START + 20:
		PCI_SET_DATA(reg, value);
		return 1;
	}

	return 0;
}

PCIINIT(piix3_isa)
{
	PCI_SET_DATA(PCI_ID_REG, PCI_ID_CODE(PCI_VENDOR_INTEL,
	    PCI_PRODUCT_INTEL_82371SB_ISA));

	PCI_SET_DATA(PCI_CLASS_REG, PCI_CLASS_CODE(PCI_CLASS_BRIDGE,
	    PCI_SUBCLASS_BRIDGE_ISA, 0) + 0x01);	/*  Rev 1  */

	PCI_SET_DATA(PCI_BHLC_REG,
	    PCI_BHLC_CODE(0,0, 1 /* multi-function */, 0x40,0));
}

PCIINIT(piix4_isa)
{
	PCI_SET_DATA(PCI_ID_REG, PCI_ID_CODE(PCI_VENDOR_INTEL,
	    PCI_PRODUCT_INTEL_82371AB_ISA));

	PCI_SET_DATA(PCI_CLASS_REG, PCI_CLASS_CODE(PCI_CLASS_BRIDGE,
	    PCI_SUBCLASS_BRIDGE_ISA, 0) + 0x01);	/*  Rev 1  */

	PCI_SET_DATA(PCI_BHLC_REG,
	    PCI_BHLC_CODE(0,0, 1 /* multi-function */, 0x40,0));

	pd->cfg_reg_write = piix_isa_cfg_reg_write;
}

PCIINIT(i82378zb)
{
	PCI_SET_DATA(PCI_ID_REG, PCI_ID_CODE(PCI_VENDOR_INTEL,
	    PCI_PRODUCT_INTEL_SIO));

	PCI_SET_DATA(PCI_CLASS_REG, PCI_CLASS_CODE(PCI_CLASS_BRIDGE,
	    PCI_SUBCLASS_BRIDGE_ISA, 0) + 0x43);

	PCI_SET_DATA(PCI_BHLC_REG,
	    PCI_BHLC_CODE(0,0, 1 /* multi-function */, 0x40,0));

	PCI_SET_DATA(0x40, 0x20);

	/*  PIRQ[0]=10 PIRQ[1]=11 PIRQ[2]=14 PIRQ[3]=15  */
	PCI_SET_DATA(0x60, 0x0f0e0b0a);
}

struct piix_ide_extra {
	void	*wdc0;
	void	*wdc1;
};

int piix_ide_cfg_reg_write(struct pci_device *pd, int reg, uint32_t value)
{
	void *wdc0 = ((struct piix_ide_extra *)pd->extra)->wdc0;
	void *wdc1 = ((struct piix_ide_extra *)pd->extra)->wdc1;
	int enabled = 0;

	PCI_SET_DATA(reg, value);

	switch (reg) {
	case PCI_COMMAND_STATUS_REG:
		if (value & PCI_COMMAND_IO_ENABLE)
			enabled = 1;
		if (wdc0 != NULL)
			wdc_set_io_enabled((struct wdc_data *) wdc0, enabled);
		if (wdc1 != NULL)
			wdc_set_io_enabled((struct wdc_data *) wdc1, enabled);
		return 1;
	case PCI_MAPREG_START:
	case PCI_MAPREG_START + 4:
	case PCI_MAPREG_START + 8:
	case PCI_MAPREG_START + 12:
	case PCI_MAPREG_START + 16:
	case PCI_MAPREG_START + 20:
		return 1;
	}

	return 0;
}

PCIINIT(piix3_ide)
{
	char tmpstr[100];

	PCI_SET_DATA(PCI_ID_REG, PCI_ID_CODE(PCI_VENDOR_INTEL,
	    PCI_PRODUCT_INTEL_82371SB_IDE));

	/*  Possibly not correct:  */
	PCI_SET_DATA(PCI_CLASS_REG, PCI_CLASS_CODE(PCI_CLASS_MASS_STORAGE,
	    PCI_SUBCLASS_MASS_STORAGE_IDE, 0x00) + 0x00);

	/*  PIIX_IDETIM (see NetBSD's pciide_piix_reg.h)  */
	/*  channel 0 and 1 enabled as IDE  */
	PCI_SET_DATA(0x40, 0x80008000);

	CHECK_ALLOCATION(pd->extra = malloc(sizeof(struct piix_ide_extra)));
	((struct piix_ide_extra *)pd->extra)->wdc0 = NULL;
	((struct piix_ide_extra *)pd->extra)->wdc1 = NULL;

	if (diskimage_exist(machine, 0, DISKIMAGE_IDE) ||
	    diskimage_exist(machine, 1, DISKIMAGE_IDE)) {
		snprintf(tmpstr, sizeof(tmpstr), "wdc addr=0x%llx "
		    "irq=%s.isa.%i", (long long)(pd->pcibus->isa_portbase +
		    0x1f0), pd->pcibus->irq_path_isa, 14);
		((struct piix_ide_extra *)pd->extra)->wdc0 =
		    device_add(machine, tmpstr);
	}

	if (diskimage_exist(machine, 2, DISKIMAGE_IDE) ||
	    diskimage_exist(machine, 3, DISKIMAGE_IDE)) {
		snprintf(tmpstr, sizeof(tmpstr), "wdc addr=0x%llx "
		    "irq=%s.isa.%i", (long long)(pd->pcibus->isa_portbase +
		    0x170), pd->pcibus->irq_path_isa, 15);
		((struct piix_ide_extra *)pd->extra)->wdc1 =
		    device_add(machine, tmpstr);
	}

	pd->cfg_reg_write = piix_ide_cfg_reg_write;
}

PCIINIT(piix4_ide)
{
	char tmpstr[100];

	PCI_SET_DATA(PCI_ID_REG, PCI_ID_CODE(PCI_VENDOR_INTEL,
	    PCI_PRODUCT_INTEL_82371AB_IDE));

	/*  Possibly not correct:  */
	PCI_SET_DATA(PCI_CLASS_REG, PCI_CLASS_CODE(PCI_CLASS_MASS_STORAGE,
	    PCI_SUBCLASS_MASS_STORAGE_IDE, 0x80) + 0x01);

	/*  PIIX_BMIBA (see NetBSD's pciide_piix_reg.h)  */
	/*  2009-05-18: Needs to have the lowest bit set, or Linux/Malta
	    crashes.  */
	PCI_SET_DATA(0x20, 1);

	/*  PIIX_IDETIM (see NetBSD's pciide_piix_reg.h)  */
	/*  channel 0 and 1 enabled as IDE  */
	PCI_SET_DATA(0x40, 0x80008000);

	CHECK_ALLOCATION(pd->extra = malloc(sizeof(struct piix_ide_extra)));
	((struct piix_ide_extra *)pd->extra)->wdc0 = NULL;
	((struct piix_ide_extra *)pd->extra)->wdc1 = NULL;

	if (diskimage_exist(machine, 0, DISKIMAGE_IDE) ||
	    diskimage_exist(machine, 1, DISKIMAGE_IDE)) {
		snprintf(tmpstr, sizeof(tmpstr), "wdc addr=0x%llx irq=%s."
		    "isa.%i", (long long)(pd->pcibus->isa_portbase + 0x1f0),
		    pd->pcibus->irq_path_isa, 14);
		((struct piix_ide_extra *)pd->extra)->wdc0 =
		    device_add(machine, tmpstr);
	}

	if (diskimage_exist(machine, 2, DISKIMAGE_IDE) ||
	    diskimage_exist(machine, 3, DISKIMAGE_IDE)) {
		snprintf(tmpstr, sizeof(tmpstr), "wdc addr=0x%llx irq=%s."
		    "isa.%i", (long long)(pd->pcibus->isa_portbase + 0x170),
		    pd->pcibus->irq_path_isa, 15);
		((struct piix_ide_extra *)pd->extra)->wdc1 =
		    device_add(machine, tmpstr);
	}

	pd->cfg_reg_write = piix_ide_cfg_reg_write;
}



/*
 *  IBM ISA bridge (used by at least one PReP machine).
 */

#define	PCI_VENDOR_IBM			0x1014
#define	PCI_PRODUCT_IBM_ISABRIDGE	0x000a

PCIINIT(ibm_isa)
{
	PCI_SET_DATA(PCI_ID_REG, PCI_ID_CODE(PCI_VENDOR_IBM,
	    PCI_PRODUCT_IBM_ISABRIDGE));

	PCI_SET_DATA(PCI_CLASS_REG, PCI_CLASS_CODE(PCI_CLASS_BRIDGE,
	    PCI_SUBCLASS_BRIDGE_ISA, 0) + 0x02);

	PCI_SET_DATA(PCI_BHLC_REG,
	    PCI_BHLC_CODE(0,0, 1 /* multi-function */, 0x40,0));
}



/*
 *  Heuricon PCI host bridge for PM/PPC.
 */

#define	PCI_VENDOR_HEURICON		0x1223
#define	PCI_PRODUCT_HEURICON_PMPPC	0x000e

PCIINIT(heuricon_pmppc)
{
	PCI_SET_DATA(PCI_ID_REG, PCI_ID_CODE(PCI_VENDOR_HEURICON,
	    PCI_PRODUCT_HEURICON_PMPPC));

	PCI_SET_DATA(PCI_CLASS_REG, PCI_CLASS_CODE(PCI_CLASS_BRIDGE,
	    PCI_SUBCLASS_BRIDGE_HOST, 0) + 0x00);   /*  Revision?  */

	PCI_SET_DATA(PCI_BHLC_REG,
	    PCI_BHLC_CODE(0,0, 1 /* multi-function */, 0x40,0));
}



/*
 *  VIATECH VT82C586 devices:
 *
 *	vt82c586_isa	PCI->ISA bridge
 *	vt82c586_ide	IDE controller
 *
 *  TODO:  This more or less just a dummy device, so far.
 */

#define PCI_VENDOR_VIATECH                0x1106  /* VIA Technologies */
#define PCI_PRODUCT_VIATECH_VT82C586_IDE  0x1571  /* VT82C586 (Apollo VP)
					             IDE Controller */
#define PCI_PRODUCT_VIATECH_VT82C586_ISA  0x0586  /* VT82C586 (Apollo VP)
						     PCI-ISA Bridge */

PCIINIT(vt82c586_isa)
{
	PCI_SET_DATA(PCI_ID_REG, PCI_ID_CODE(PCI_VENDOR_VIATECH,
	    PCI_PRODUCT_VIATECH_VT82C586_ISA));

	PCI_SET_DATA(PCI_CLASS_REG, PCI_CLASS_CODE(PCI_CLASS_BRIDGE,
	    PCI_SUBCLASS_BRIDGE_ISA, 0) + 0x39);   /*  Revision 37 or 39  */

	PCI_SET_DATA(PCI_BHLC_REG,
	    PCI_BHLC_CODE(0,0, 1 /* multi-function */, 0x40,0));

	/*
	 *  NetBSD/cobalt specific: bits 7..4 are the "model id". See
	 *  netbsd/usr/src/sys/arch/cobalt/cobalt/machdep.c read_board_id()
	 *  for details.
	 */
	if (machine->machine_type == MACHINE_COBALT) {
#define	COBALT_PCIB_BOARD_ID_REG	0x94
#define	COBALT_QUBE2_ID			5
		PCI_SET_DATA(COBALT_PCIB_BOARD_ID_REG, COBALT_QUBE2_ID << 4);
	}
}

struct vt82c586_ide_extra {
	void	*wdc0;
	void	*wdc1;
};

int vt82c586_ide_cfg_reg_write(struct pci_device *pd, int reg, uint32_t value)
{
	void *wdc0 = ((struct vt82c586_ide_extra *)pd->extra)->wdc0;
	void *wdc1 = ((struct vt82c586_ide_extra *)pd->extra)->wdc1;
	int enabled = 0;

	switch (reg) {
	case PCI_COMMAND_STATUS_REG:
		if (value & PCI_COMMAND_IO_ENABLE)
			enabled = 1;
		if (wdc0 != NULL)
			wdc_set_io_enabled((struct wdc_data *) wdc0, enabled);
		if (wdc1 != NULL)
			wdc_set_io_enabled((struct wdc_data *) wdc1, enabled);
		return 1;
	}

	return 0;
}

PCIINIT(vt82c586_ide)
{
	char tmpstr[100];

	PCI_SET_DATA(PCI_ID_REG, PCI_ID_CODE(PCI_VENDOR_VIATECH,
	    PCI_PRODUCT_VIATECH_VT82C586_IDE));

	/*  Possibly not correct:  */
	PCI_SET_DATA(PCI_CLASS_REG, PCI_CLASS_CODE(PCI_CLASS_MASS_STORAGE,
	    PCI_SUBCLASS_MASS_STORAGE_IDE, 0x00) + 0x01);

	/*  APO_IDECONF  */
	/*  channel 0 and 1 enabled  */
	PCI_SET_DATA(0x40, 0x00000003);

	CHECK_ALLOCATION(pd->extra = malloc(sizeof(struct vt82c586_ide_extra)));
	((struct vt82c586_ide_extra *)pd->extra)->wdc0 = NULL;
	((struct vt82c586_ide_extra *)pd->extra)->wdc1 = NULL;

	if (diskimage_exist(machine, 0, DISKIMAGE_IDE) ||
	    diskimage_exist(machine, 1, DISKIMAGE_IDE)) {
		snprintf(tmpstr, sizeof(tmpstr), "wdc addr=0x%llx irq=%s."
		    "isa.%i", (long long)(pd->pcibus->isa_portbase + 0x1f0),
		    pd->pcibus->irq_path_isa, 14);
		((struct vt82c586_ide_extra *)pd->extra)->wdc0 =
		    device_add(machine, tmpstr);
	}

	if (diskimage_exist(machine, 2, DISKIMAGE_IDE) ||
	    diskimage_exist(machine, 3, DISKIMAGE_IDE)) {
		snprintf(tmpstr, sizeof(tmpstr), "wdc addr=0x%llx irq=%s."
		    "isa.%i", (long long)(pd->pcibus->isa_portbase + 0x170),
		    pd->pcibus->irq_path_isa, 15);
		((struct vt82c586_ide_extra *)pd->extra)->wdc1 =
		    device_add(machine, tmpstr);
	}

	pd->cfg_reg_write = vt82c586_ide_cfg_reg_write;
}



/*
 *  Symphony Labs 83C553 PCI->ISA bridge.
 *  Symphony Labs 82C105 PCIIDE controller.
 */

#define PCI_VENDOR_SYMPHONY		0x10ad
#define PCI_PRODUCT_SYMPHONY_83C553	0x0565
#define	PCI_PRODUCT_SYMPHONY_82C105	0x0105

PCIINIT(symphony_83c553)
{
	PCI_SET_DATA(PCI_ID_REG, PCI_ID_CODE(PCI_VENDOR_SYMPHONY,
	    PCI_PRODUCT_SYMPHONY_83C553));

	PCI_SET_DATA(PCI_CLASS_REG, PCI_CLASS_CODE(PCI_CLASS_BRIDGE,
	    PCI_SUBCLASS_BRIDGE_ISA, 0) + 0x10);

	PCI_SET_DATA(PCI_BHLC_REG,
	    PCI_BHLC_CODE(0,0, 1 /* multi-function */, 0x40,0));

	switch (machine->machine_type) {
	case MACHINE_NETWINDER:
		bus_isa_init(machine, pd->pcibus->irq_path_isa,
		    0, 0x7c000000, 0x80000000);
		break;
	default:fatal("symphony_83c553 init: unimplemented machine type\n");
		exit(1);
	}
}

struct symphony_82c105_extra {
	void	*wdc0;
	void	*wdc1;
};

int symphony_82c105_cfg_reg_write(struct pci_device *pd, int reg,
	uint32_t value)
{
	void *wdc0 = ((struct symphony_82c105_extra *)pd->extra)->wdc0;
	void *wdc1 = ((struct symphony_82c105_extra *)pd->extra)->wdc1;
	int enabled = 0;

printf("reg = 0x%x\n", reg);
	switch (reg) {
	case PCI_COMMAND_STATUS_REG:
		if (value & PCI_COMMAND_IO_ENABLE)
			enabled = 1;
printf("  value = 0x%"PRIx32"\n", value);
		if (wdc0 != NULL)
			wdc_set_io_enabled((struct wdc_data *) wdc0, enabled);
		if (wdc1 != NULL)
			wdc_set_io_enabled((struct wdc_data *) wdc1, enabled);
		/*  Set all bits:  */
		PCI_SET_DATA(reg, value);
		return 1;
	case PCI_MAPREG_START:
	case PCI_MAPREG_START + 4:
	case PCI_MAPREG_START + 8:
	case PCI_MAPREG_START + 12:
	case PCI_MAPREG_START + 16:
	case PCI_MAPREG_START + 20:
		PCI_SET_DATA(reg, value);
		return 1;
	}

	return 0;
}

PCIINIT(symphony_82c105)
{
	char tmpstr[100];

	PCI_SET_DATA(PCI_ID_REG, PCI_ID_CODE(PCI_VENDOR_SYMPHONY,
	    PCI_PRODUCT_SYMPHONY_82C105));

	/*  Possibly not correct:  */
	PCI_SET_DATA(PCI_CLASS_REG, PCI_CLASS_CODE(PCI_CLASS_MASS_STORAGE,
	    PCI_SUBCLASS_MASS_STORAGE_IDE, 0x00) + 0x05);

	/*  TODO: Interrupt line:  */
	/*  PCI_SET_DATA(PCI_INTERRUPT_REG, 0x28140000);  */

	/*  APO_IDECONF  */
	/*  channel 0 and 1 enabled  */
	PCI_SET_DATA(0x40, 0x00000003);

	CHECK_ALLOCATION(pd->extra =
	    malloc(sizeof(struct symphony_82c105_extra)));
	((struct symphony_82c105_extra *)pd->extra)->wdc0 = NULL;
	((struct symphony_82c105_extra *)pd->extra)->wdc1 = NULL;

	if (diskimage_exist(machine, 0, DISKIMAGE_IDE) ||
	    diskimage_exist(machine, 1, DISKIMAGE_IDE)) {
		snprintf(tmpstr, sizeof(tmpstr), "wdc addr=0x%llx irq=%s."
		    "isa.%i", (long long)(pd->pcibus->isa_portbase + 0x1f0),
		    pd->pcibus->irq_path_isa, 14);
		((struct symphony_82c105_extra *)pd->extra)->wdc0 =
		    device_add(machine, tmpstr);
	}

	if (diskimage_exist(machine, 2, DISKIMAGE_IDE) ||
	    diskimage_exist(machine, 3, DISKIMAGE_IDE)) {
		snprintf(tmpstr, sizeof(tmpstr), "wdc addr=0x%llx irq=%s."
		    "isa.%i", (long long)(pd->pcibus->isa_portbase + 0x170),
		    pd->pcibus->irq_path_isa, 15);
		((struct symphony_82c105_extra *)pd->extra)->wdc1 =
		    device_add(machine, tmpstr);
	}

	pd->cfg_reg_write = symphony_82c105_cfg_reg_write;
}



/*
 *  Realtek 8139C+ PCI ethernet.
 */

#define PCI_VENDOR_REALTEK		0x10ec
#define	PCI_PRODUCT_REALTEK_RT8139	0x8139

PCIINIT(rtl8139c)
{
	uint64_t port, memaddr;
	int pci_int_line = 0x101, irq = 0;
	char irqstr[200];
	char tmpstr[200];

	PCI_SET_DATA(PCI_ID_REG, PCI_ID_CODE(PCI_VENDOR_REALTEK,
	    PCI_PRODUCT_REALTEK_RT8139));

	PCI_SET_DATA(PCI_CLASS_REG, PCI_CLASS_CODE(PCI_CLASS_NETWORK,
	    PCI_SUBCLASS_NETWORK_ETHERNET, 0x00) + 0x20);

	switch (machine->machine_type) {
	case MACHINE_LANDISK:
		irq = 5;
		pci_int_line = 0x105;
		break;
	default:fatal("rtl8139c for this machine has not been "
		    "implemented yet\n");
		exit(1);
	}

	PCI_SET_DATA(PCI_INTERRUPT_REG, 0x28140000 | pci_int_line);

	allocate_device_space(pd, 0x100, 0, &port, &memaddr);

	snprintf(irqstr, sizeof(irqstr), "%s.%i",
	    pd->pcibus->irq_path_pci, irq);

	snprintf(tmpstr, sizeof(tmpstr), "rtl8139c addr=0x%llx "
	    "irq=%s pci_little_endian=1", (long long)port, irqstr);

	device_add(machine, tmpstr);
}



/*
 *  DEC 21143 ("Tulip") PCI ethernet.
 */

#define PCI_VENDOR_DEC          0x1011 /* Digital Equipment */
#define PCI_PRODUCT_DEC_21142   0x0019 /* DECchip 21142/21143 10/100 Ethernet */

PCIINIT(dec21143)
{
	uint64_t port, memaddr;
	int pci_int_line = 0x101, irq = 0, isa = 0;
	char irqstr[200];
	char tmpstr[200];

	PCI_SET_DATA(PCI_ID_REG, PCI_ID_CODE(PCI_VENDOR_DEC,
	    PCI_PRODUCT_DEC_21142));

	PCI_SET_DATA(PCI_COMMAND_STATUS_REG, 0x02000017);

	PCI_SET_DATA(PCI_CLASS_REG, PCI_CLASS_CODE(PCI_CLASS_NETWORK,
	    PCI_SUBCLASS_NETWORK_ETHERNET, 0x00) + 0x41);

	PCI_SET_DATA(PCI_BHLC_REG, PCI_BHLC_CODE(0,0,0, 0x40,0));

	switch (machine->machine_type) {
	case MACHINE_CATS:
		/*  CATS int 18 = PCI.  */
		irq = 18;
		pci_int_line = 0x101;
		break;
	case MACHINE_COBALT:
		/*  On Cobalt, IRQ 7 = PCI.  */
		irq = 8 + 7;
		pci_int_line = 0x407;
		break;
	case MACHINE_PREP:
		irq = 10;
		isa = 1;
		pci_int_line = 0x20a;
		break;
	case MACHINE_MVMEPPC:
		/*  TODO  */
		irq = 10;
		pci_int_line = 0x40a;
		break;
	case MACHINE_PMPPC:
		/*  TODO, not working yet  */
		irq = 31 - CPC_IB_EXT1;
		pci_int_line = 0x101;
		break;
	case MACHINE_MACPPC:
		/*  TODO, not working yet  */
		irq = 25;
		pci_int_line = 0x101;
		break;
	}

	PCI_SET_DATA(PCI_INTERRUPT_REG, 0x28140000 | pci_int_line);

	allocate_device_space(pd, 0x100, 0x100, &port, &memaddr);

	if (isa)
		snprintf(irqstr, sizeof(irqstr), "%s.isa.%i",
		    pd->pcibus->irq_path_isa, irq);
	else
		snprintf(irqstr, sizeof(irqstr), "%s.%i",
		    pd->pcibus->irq_path_pci, irq);

	snprintf(tmpstr, sizeof(tmpstr), "dec21143 addr=0x%llx addr2=0x%llx "
	    "irq=%s pci_little_endian=1", (long long)port,
	    (long long)memaddr, irqstr);

	device_add(machine, tmpstr);
}



/*
 *  DEC 21030 "tga" graphics.
 */

#define PCI_PRODUCT_DEC_21030  0x0004          /*  DECchip 21030 ("TGA")  */

PCIINIT(dec21030)
{
	uint64_t base = 0;
	char tmpstr[200];

	PCI_SET_DATA(PCI_ID_REG, PCI_ID_CODE(PCI_VENDOR_DEC,
	    PCI_PRODUCT_DEC_21030));

	PCI_SET_DATA(PCI_COMMAND_STATUS_REG, 0x02800087);  /*  TODO  */

	PCI_SET_DATA(PCI_CLASS_REG, PCI_CLASS_CODE(PCI_CLASS_DISPLAY,
	    PCI_SUBCLASS_DISPLAY_VGA, 0x00) + 0x03);

	/*
	 *  See http://mail-index.netbsd.org/port-arc/2001/08/13/0000.html
	 *  for more info.
	 */

	PCI_SET_DATA(PCI_BHLC_REG, 0x0000ff00);

	/*  8 = prefetchable  */
	PCI_SET_DATA(0x10, 0x00000008);
	PCI_SET_DATA(0x30, 0x08000001);
	PCI_SET_DATA(PCI_INTERRUPT_REG, 0x00000100);	/*  interrupt pin A?  */

	/*
	 *  Experimental:
	 *
	 *  TODO: Base address, pci_little_endian, ...
	 */

	switch (machine->machine_type) {
	case MACHINE_ARC:
		base = 0x100000000ULL;
		break;
	default:fatal("dec21030 in non-implemented machine type %i\n",
		    machine->machine_type);
		exit(1);
	}

	snprintf(tmpstr, sizeof(tmpstr), "dec21030 addr=0x%llx",
	    (long long)(base));
	device_add(machine, tmpstr);
}



/*
 *  Motorola MPC105 "Eagle" Host Bridge
 *
 *  Used in at least PReP and BeBox.
 */

#define	PCI_VENDOR_MOT			0x1057
#define	PCI_PRODUCT_MOT_MPC105		0x0001

PCIINIT(eagle)
{
	PCI_SET_DATA(PCI_ID_REG, PCI_ID_CODE(PCI_VENDOR_MOT,
	    PCI_PRODUCT_MOT_MPC105));

	PCI_SET_DATA(PCI_CLASS_REG, PCI_CLASS_CODE(PCI_CLASS_BRIDGE,
	    PCI_SUBCLASS_BRIDGE_HOST, 0) + 0x24);

	PCI_SET_DATA(PCI_BHLC_REG,
	    PCI_BHLC_CODE(0,0, 1 /* multi-function */, 0x40,0));
}



/*
 *  Apple (MacPPC) stuff:
 *
 *  Grand Central (I/O controller)
 *  Uni-North (PCI controller)
 */

#define	PCI_VENDOR_APPLE		0x106b
#define	PCI_PRODUCT_APPLE_GC		0x0002
#define	PCI_PRODUCT_APPLE_UNINORTH1	0x001e

PCIINIT(gc_obio)
{
	uint64_t port, memaddr;

	PCI_SET_DATA(PCI_ID_REG, PCI_ID_CODE(PCI_VENDOR_APPLE,
	    PCI_PRODUCT_APPLE_GC));

	/*  TODO:  */
	PCI_SET_DATA(PCI_CLASS_REG, PCI_CLASS_CODE(PCI_CLASS_SYSTEM,
	    PCI_SUBCLASS_SYSTEM_PIC, 0) + 0x00);

	PCI_SET_DATA(PCI_BHLC_REG,
	    PCI_BHLC_CODE(0,0, 1 /* multi-function */, 0x40,0));

	/*  TODO  */
	allocate_device_space(pd, 0x10000, 0x10000, &port, &memaddr);
}

PCIINIT(uninorth)
{
	uint64_t port, memaddr;

	PCI_SET_DATA(PCI_ID_REG, PCI_ID_CODE(PCI_VENDOR_APPLE,
	    PCI_PRODUCT_APPLE_UNINORTH1));

	PCI_SET_DATA(PCI_CLASS_REG, PCI_CLASS_CODE(PCI_CLASS_BRIDGE,
	    PCI_SUBCLASS_BRIDGE_HOST, 0) + 0xff);

	PCI_SET_DATA(PCI_BHLC_REG,
	    PCI_BHLC_CODE(0,0, 1 /* multi-function */, 0x40,0));

	/*  TODO  */
	allocate_device_space(pd, 0x10000, 0x10000, &port, &memaddr);
}



/*
 *  ATI graphics cards
 */

#define	PCI_VENDOR_ATI			0x1002
#define	PCI_PRODUCT_ATI_RADEON_9200_2	0x5962

PCIINIT(ati_radeon_9200_2)
{
	uint64_t port, memaddr;

	PCI_SET_DATA(PCI_ID_REG, PCI_ID_CODE(PCI_VENDOR_ATI,
	    PCI_PRODUCT_ATI_RADEON_9200_2));

	/*  TODO: other subclass?  */
	PCI_SET_DATA(PCI_CLASS_REG, PCI_CLASS_CODE(PCI_CLASS_DISPLAY,
	    PCI_SUBCLASS_DISPLAY_VGA, 0) + 0x03);

	/*  TODO  */
	allocate_device_space(pd, 0x1000, 0x400000, &port, &memaddr);
}

