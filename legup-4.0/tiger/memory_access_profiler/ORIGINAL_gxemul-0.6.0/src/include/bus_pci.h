#ifndef	BUS_PCI_H
#define	BUS_PCI_H

/*
 *  Copyright (C) 2004-2010  Anders Gavare.  All rights reserved.
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
 *  PCI bus.
 */

#include "misc.h"

#include "thirdparty/pcireg.h"

struct machine;
struct memory;

struct pci_device;


#ifndef BUS_PCI_C

struct pci_data;

#else

struct pci_data {
	/*
	 *  IRQ paths:
	 *
	 *  irq_path		Path of the controller itself.
	 *  irq_path_isa	Path base of ISA interrupts.
	 *  irq_path_pci	Path base of PCI interrupts.
	 */
	char		*irq_path;
	char		*irq_path_isa;
	char		*irq_path_pci;

	/*
	 *  Default I/O port, memory, and irq bases for PCI and legacy ISA
	 *  devices, and the base address for actual (emulated) devices:
	 *
	 *  pci_portbase etc are what is stored in the device configuration
	 *  registers. This address + pci_actual_{io,mem}_offset is where the
	 *  emulated device should be registered.
	 */
	uint64_t	pci_actual_io_offset;
	uint64_t	pci_actual_mem_offset;

	uint64_t	pci_portbase;
	uint64_t	pci_membase;

	uint64_t	isa_portbase;
	uint64_t	isa_membase;

	/*  Current base when allocating space for PCI devices:  */
	uint64_t	cur_pci_portbase;
	uint64_t	cur_pci_membase;

	/*  Current register access:  */
	int		cur_bus, cur_device, cur_func, cur_reg;
	int		last_was_write_ffffffff;

	struct pci_device *first_device;
};

#define	PCI_CFG_MEM_SIZE	0x100

struct pci_device {
	/*  Pointer to the next PCI device on this bus:  */
	struct pci_device	*next;

	/*  Pointer back to the bus this device is connected to:  */
	struct pci_data		*pcibus;

	/*  Short device name, and bus/device/function value:  */
	char			*name;
	int			bus, device, function;

	/*  Configuration memory:  */
	unsigned char		cfg_mem[PCI_CFG_MEM_SIZE];
	unsigned char		cfg_mem_size[PCI_CFG_MEM_SIZE];

	/*  Used when setting up the configuration registers:  */
	int			cur_mapreg_offset;

	/*  Function to handle device-specific cfg register writes:  */
	int			(*cfg_reg_write)(struct pci_device *pd,
				    int reg, uint32_t value);
	void			*extra;
};

#define	PCIINIT(name)	void pciinit_ ## name(struct machine *machine,	\
	struct memory *mem, struct pci_device *pd)

/*
 *  Store little-endian config data in the pci_data struct's cfg_mem[]
 *  or cfg_mem_size[], respectively.
 */
#define PCI_SET_DATA(ofs,value)	{					\
	pd->cfg_mem[(ofs)]     = (value) & 255;				\
	pd->cfg_mem[(ofs) + 1] = ((value) >> 8) & 255;			\
	pd->cfg_mem[(ofs) + 2] = ((value) >> 16) & 255;			\
	pd->cfg_mem[(ofs) + 3] = ((value) >> 24) & 255;			\
	}
#define PCI_SET_DATA_SIZE(ofs,value)	{				\
	pd->cfg_mem_size[(ofs)]     = (value) & 255;			\
	pd->cfg_mem_size[(ofs) + 1] = ((value) >> 8) & 255;		\
	pd->cfg_mem_size[(ofs) + 2] = ((value) >> 16) & 255;		\
	pd->cfg_mem_size[(ofs) + 3] = ((value) >> 24) & 255;		\
	}

#endif

#define	BUS_PCI_ADDR	0xcf8
#define	BUS_PCI_DATA	0xcfc


/*
 *  bus_pci.c:
 */

/*  Run-time access:  */
void bus_pci_decompose_1(uint32_t t, int *bus, int *dev, int *func, int *reg);
void bus_pci_setaddr(struct cpu *cpu, struct pci_data *pci_data,
	int bus, int device, int function, int reg);
void bus_pci_data_access(struct cpu *cpu, struct pci_data *pci_data,
	uint64_t *data, int len, int writeflag);

/*  Initialization:  */
struct pci_data *bus_pci_init(struct machine *machine, const char *irq_path,
	uint64_t pci_actual_io_offset, uint64_t pci_actual_mem_offset,
	uint64_t pci_portbase, uint64_t pci_membase, const char *pci_irqbase,
	uint64_t isa_portbase, uint64_t isa_membase, const char *isa_irqbase);

/*  Add a PCI device to a PCI bus:  */
void bus_pci_add(struct machine *machine, struct pci_data *pci_data,
	struct memory *mem, int bus, int device, int function,
	const char *name);


#endif	/*  BUS_PCI_H  */
