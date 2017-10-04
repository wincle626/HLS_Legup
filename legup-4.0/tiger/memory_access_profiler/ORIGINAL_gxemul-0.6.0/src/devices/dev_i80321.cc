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
 *  COMMENT: Intel i80321 (ARM) core functionality
 *
 *	o)  Interrupt controller
 *	o)  Timer
 *	o)  PCI controller
 *	o)  Memory controller
 *	o)  I2C
 *
 *  TODO:
 *	o)  More or less everything.
 *	o)  This is hardcoded for little endian emulation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bus_pci.h"
#include "cpu.h"
#include "device.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"
#include "timer.h"

#include "thirdparty/i80321reg.h"
#include "thirdparty/iopi2creg.h"


#define	TICK_SHIFT		15
#define	DEV_I80321_LENGTH	VERDE_PMMR_SIZE

struct i80321_data {
	/*  Interrupt Controller  */
	struct interrupt irq;
	uint32_t	*status;	/*  Note: these point to i80321_isrc  */
	uint32_t	*enable;	/*  and i80321_inten in the CPU!  */

	/*  Timer:  */
	struct timer	*timer;
	double		hz;
	int		pending_tmr0_interrupts;

	/*  PCI Controller:  */
	uint32_t	pci_addr;
	struct pci_data *pci_bus;

	/*  Memory Controller:  */
	uint32_t	mcu_reg[0x100 / sizeof(uint32_t)];

	/*  I2C Controller:  */
	uint32_t	i2c_reg[VERDE_I2C_SIZE / sizeof(uint32_t)];
};


static void i80321_assert(struct i80321_data *d, uint32_t linemask)
{
	*d->status |= linemask;
	if (*d->status & *d->enable)
		INTERRUPT_ASSERT(d->irq);
}
static void i80321_deassert(struct i80321_data *d, uint32_t linemask)
{
	*d->status &= ~linemask;
	if (!(*d->status & *d->enable))
		INTERRUPT_DEASSERT(d->irq);
}


/*  
 *  i80321_interrupt_assert():
 *  i80321_interrupt_deassert():
 *
 *  Called whenever an i80321 interrupt is asserted/deasserted.
 */
void i80321_interrupt_assert(struct interrupt *interrupt)
{ i80321_assert((struct i80321_data *)interrupt->extra, interrupt->line); }
void i80321_interrupt_deassert(struct interrupt *interrupt)
{
	struct i80321_data *d = (struct i80321_data *) interrupt->extra;

	/*  Ack. timer interrupts:  */
	if (interrupt->line == 1 << 9 &&
	    d->pending_tmr0_interrupts > 0)
		d->pending_tmr0_interrupts --;

	i80321_deassert(d, interrupt->line);
}


/*  TMR0 ticks, called d->hz times per second.  */
static void tmr0_tick(struct timer *t, void *extra)
{
	struct i80321_data *d = (struct i80321_data *) extra;
	d->pending_tmr0_interrupts ++;
}


DEVICE_TICK(i80321)
{
	struct i80321_data *d = (struct i80321_data *) extra;

	if (cpu->cd.arm.tmr0 & TMRx_ENABLE && d->pending_tmr0_interrupts > 0) {
		i80321_assert(d, 1 << 9);
		cpu->cd.arm.tisr |= TISR_TMR0;
	} else {
		i80321_deassert(d, 1 << 9);
		cpu->cd.arm.tisr &= ~TISR_TMR0;
	}
}

        
DEVICE_ACCESS(i80321)
{
	struct i80321_data *d = (struct i80321_data *) extra;
	uint64_t idata = 0, odata = 0;
	const char *n = NULL;
	int bus, dev, func, reg;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	/*  PCI configuration space:  */
	if (relative_addr >= 0x100 && relative_addr < 0x140) {
		/*  TODO  */
		goto ret;
	}

	/*  MCU registers:  */
	if (relative_addr >= VERDE_MCU_BASE &&
	    relative_addr <  VERDE_MCU_BASE + VERDE_MCU_SIZE) {
		int regnr = (relative_addr - VERDE_MCU_BASE) / sizeof(uint32_t);
		if (writeflag == MEM_WRITE)
			d->mcu_reg[regnr] = idata;
		else
			odata = d->mcu_reg[regnr];
	}

	/*  I2C registers:  */
	if (relative_addr >= VERDE_I2C_BASE &&
	    relative_addr <  VERDE_I2C_BASE + VERDE_I2C_SIZE) {
		int regnr = (relative_addr - VERDE_I2C_BASE) / sizeof(uint32_t);
		if (writeflag == MEM_WRITE)
			d->i2c_reg[regnr] = idata;
		else
			odata = d->i2c_reg[regnr];
	}


	switch (relative_addr) {

	/*  Address Translation Unit:  */
	case VERDE_ATU_BASE + ATU_IALR0:
	case VERDE_ATU_BASE + ATU_IATVR0:
	case VERDE_ATU_BASE + ATU_IALR1:
	case VERDE_ATU_BASE + ATU_IALR2:
	case VERDE_ATU_BASE + ATU_IATVR2:
	case VERDE_ATU_BASE + ATU_OIOWTVR:
	case VERDE_ATU_BASE + ATU_OMWTVR0:
	case VERDE_ATU_BASE + ATU_OUMWTVR0:
	case VERDE_ATU_BASE + ATU_OMWTVR1:
	case VERDE_ATU_BASE + ATU_OUMWTVR1:
		/*  Ignoring these for now. TODO  */
		break;
	case VERDE_ATU_BASE + ATU_ATUCR:
		/*  ATU configuration register; ignored for now. TODO  */
		break;
	case VERDE_ATU_BASE + ATU_PCSR:
		/*  TODO: Temporary hack to allow NetBSD/evbarm to
		    reboot itself.  Should be rewritten as soon as possible!  */
		if (writeflag == MEM_WRITE && idata == 0x30) {
			int j;
			for (j=0; j<cpu->machine->ncpus; j++)
				cpu->machine->cpus[j]->running = 0;
			cpu->machine->exit_without_entering_debugger = 1;
		}
		break;
	case VERDE_ATU_BASE + ATU_ATUIMR:
	case VERDE_ATU_BASE + ATU_IABAR3:
	case VERDE_ATU_BASE + ATU_IAUBAR3:
	case VERDE_ATU_BASE + ATU_IALR3:
	case VERDE_ATU_BASE + ATU_IATVR3:
		/*  Ignoring these for now. TODO  */
		break;
	case VERDE_ATU_BASE + ATU_OCCAR:
		/*  PCI address  */
		if (writeflag == MEM_WRITE) {
			d->pci_addr = idata;
			bus_pci_decompose_1(idata, &bus, &dev, &func, &reg);
			bus = 0;	/*  NOTE  */
			bus_pci_setaddr(cpu, d->pci_bus, bus, dev, func, reg);
		} else {
			odata = d->pci_addr;
		}
		break;
	case VERDE_ATU_BASE + ATU_OCCDR:
	case VERDE_ATU_BASE + ATU_OCCDR + 1:
	case VERDE_ATU_BASE + ATU_OCCDR + 2:
	case VERDE_ATU_BASE + ATU_OCCDR + 3:
		/*  PCI data  */
		if (writeflag == MEM_READ) {
			uint64_t tmp;
			bus_pci_data_access(cpu, d->pci_bus, &tmp,
			    sizeof(uint32_t), MEM_READ);
			switch (relative_addr) {
			case VERDE_ATU_BASE + ATU_OCCDR + 1:
				odata = tmp >> 8; break;
			case VERDE_ATU_BASE + ATU_OCCDR + 2:
				odata = tmp >> 16; break;
			case VERDE_ATU_BASE + ATU_OCCDR + 3:
				odata = tmp >> 24; break;
			default:odata = tmp;
			}
		} else {
			uint64_t tmp;
			unsigned int i;
			int r = relative_addr - (VERDE_ATU_BASE + ATU_OCCDR);
			bus_pci_data_access(cpu, d->pci_bus, &tmp,
			    sizeof(uint32_t), MEM_READ);
			for (i=0; i<len; i++) {
				uint8_t b = idata >> (i*8);
				tmp &= ~(0xff << ((r+i)*8));
				tmp |= b << ((r+i)*8);
			}
			tmp &= 0xffffffff;  /* needed because << is 32-bit */
			bus_pci_data_access(cpu, d->pci_bus, &tmp,
			    sizeof(uint32_t), MEM_WRITE);
		}
		break;
	case VERDE_ATU_BASE + ATU_PCIXSR:
		odata = 0;		/*  TODO  */
		break;

	/*  Memory Controller Unit:  */
	case VERDE_MCU_BASE + MCU_SDIR:
		n = "MCU_SDIR (DDR SDRAM Init Register)";
		break;
	case VERDE_MCU_BASE + MCU_SDCR:
		n = "MCU_SDCR (DDR SDRAM Control Register)";
		break;
	case VERDE_MCU_BASE + MCU_SDBR:
		n = "MCU_SDBR (SDRAM Base Register)";
		break;
	case VERDE_MCU_BASE + MCU_SBR0:
		n = "MCU_SBR0 (SDRAM Boundary 0)";
		break;
	case VERDE_MCU_BASE + MCU_SBR1:
		n = "MCU_SBR1 (SDRAM Boundary 1)";
		break;
	case VERDE_MCU_BASE + MCU_ECCR:
		n = "MCU_ECCR (ECC Control Register)";
		break;
	case VERDE_MCU_BASE + MCU_RFR:
		n = "MCU_RFR (Refresh Frequency Register)";
		break;
	case VERDE_MCU_BASE + MCU_DBUDSR:
		n = "MCU_DBUDSR (Data Bus Pull-up Drive Strength)";
		break;
	case VERDE_MCU_BASE + MCU_DBDDSR:
		n = "MCU_DBDDSR (Data Bus Pull-down Drive Strength)";
		break;
	case VERDE_MCU_BASE + MCU_CUDSR:
		n = "MCU_CUDSR (Clock Pull-up Drive Strength)";
		break;
	case VERDE_MCU_BASE + MCU_CDDSR:
		n = "MCU_CDDSR (Clock Pull-down Drive Strength)";
		break;
	case VERDE_MCU_BASE + MCU_CEUDSR:
		n = "MCU_CEUDSR (Clock En Pull-up Drive Strength)";
		break;
	case VERDE_MCU_BASE + MCU_CEDDSR:
		n = "MCU_CEDDSR (Clock En Pull-down Drive Strength)";
		break;
	case VERDE_MCU_BASE + MCU_CSUDSR:
		n = "MCU_CSUDSR (Chip Sel Pull-up Drive Strength)";
		break;
	case VERDE_MCU_BASE + MCU_CSDDSR:
		n = "MCU_CSDDSR (Chip Sel Pull-down Drive Strength)";
		break;
	case VERDE_MCU_BASE + MCU_REUDSR:
		n = "MCU_REUDSR (Rx En Pull-up Drive Strength)";
		break;
	case VERDE_MCU_BASE + MCU_REDDSR:
		n = "MCU_REDDSR (Rx En Pull-down Drive Strength)";
		break;

	case VERDE_MCU_BASE + MCU_ABUDSR:
		n = "MCU_ABUDSR (Addr Bus Pull-up Drive Strength)";
		break;
	case VERDE_MCU_BASE + MCU_ABDDSR:
		n = "MCU_ABDDSR (Addr Bus Pull-down Drive Strength)";
		break;

	/*  Peripheral Bus Interface Unit  */
	case VERDE_PBIU_BASE + PBIU_PBCR:
		n = "PBIU_PBCR (PBIU Control Register)";
		break;
	case VERDE_PBIU_BASE + PBIU_PBBAR0:
		n = "PBIU_PBBAR0 (PBIU Base Address Register 0)";
		break;
	case VERDE_PBIU_BASE + PBIU_PBLR0:
		n = "PBIU_PBLR0 (PBIU Limit Register 0)";
		break;
	case VERDE_PBIU_BASE + PBIU_PBBAR1:
		n = "PBIU_PBBAR1 (PBIU Base Address Register 1)";
		break;
	case VERDE_PBIU_BASE + PBIU_PBLR1:
		n = "PBIU_PBLR1 (PBIU Limit Register 1)";
		break;
	case VERDE_PBIU_BASE + PBIU_PBBAR2:
		n = "PBIU_PBBAR2 (PBIU Base Address Register 2)";
		break;
	case VERDE_PBIU_BASE + PBIU_PBLR2:
		n = "PBIU_PBLR2 (PBIU Limit Register 2)";
		break;

	/*  TODO:  */
	case 0x7cc:
		n = "0x7cc_TODO";
		break;

	case VERDE_I2C_BASE0 + IIC_ICR:
		n = "I2C 0, IIC_ICR (control register)";
		break;
	case VERDE_I2C_BASE0 + IIC_ISR:
		n = "I2C 0, IIC_ISR (status register)";
		break;
	case VERDE_I2C_BASE0 + IIC_ISAR:
		n = "I2C 0, IIC_ISAR (slave address register)";
		break;
	case VERDE_I2C_BASE0 + IIC_IDBR:
		n = "I2C 0, IIC_IDBR (data buffer register)";
		break;

	case VERDE_I2C_BASE1 + IIC_ICR:
		n = "I2C 1, IIC_ICR (control register)";
		break;
	case VERDE_I2C_BASE1 + IIC_ISR:
		n = "I2C 1, IIC_ISR (status register)";
		odata = IIC_ISR_ITE;	/*  IDBR Tx empty  */
		odata |= IIC_ISR_IRF;	/*  IDBR Rx full  */
		break;
	case VERDE_I2C_BASE1 + IIC_ISAR:
		n = "I2C 1, IIC_ISAR (slave address register)";
		break;
	case VERDE_I2C_BASE1 + IIC_IDBR:
		n = "I2C 1, IIC_IDBR (data buffer register)";
		odata = 7;	/*  TODO  */
		break;

	default:if (writeflag == MEM_READ) {
			fatal("[ i80321: read from 0x%x ]\n",
			    (int)relative_addr);
		} else {
			fatal("[ i80321: write to 0x%x: 0x%llx ]\n",
			    (int)relative_addr, (long long)idata);
		}
		exit(1);
	}

	if (n != NULL) {
		if (writeflag == MEM_READ) {
			debug("[ i80321: read from %s: 0x%llx ]\n",
			    n, (long long)idata);
		} else {
			debug("[ i80321: write to %s: 0x%llx ]\n",
			    n, (long long)idata);
		}
	}

ret:
	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVINIT(i80321)
{
	struct i80321_data *d;
	uint32_t memsize = devinit->machine->physical_ram_in_mb * 1048576;
	uint32_t base;
	char tmpstr[300];
	struct cpu *cpu = devinit->machine->cpus[devinit->
	    machine->bootstrap_cpu];
	int i;

	CHECK_ALLOCATION(d = (struct i80321_data *) malloc(sizeof(struct i80321_data)));
	memset(d, 0, sizeof(struct i80321_data));

	/*  Connect to the CPU interrupt pin:  */
	INTERRUPT_CONNECT(devinit->interrupt_path, d->irq);

	/*  Register 32 i80321 interrupts:  */
	for (i=0; i<32; i++) {
		struct interrupt templ;
		char tmpstr[300];
		snprintf(tmpstr, sizeof(tmpstr), "%s.i80321.%i",
		    devinit->interrupt_path, i);
		memset(&templ, 0, sizeof(templ));
		templ.line = 1 << i;
		templ.name = tmpstr;
		templ.extra = d;
		templ.interrupt_assert = i80321_interrupt_assert;
		templ.interrupt_deassert = i80321_interrupt_deassert;
		interrupt_handler_register(&templ);

		/*
		 *  Connect the CPU's TMR0 and TMR1 interrupts to these
		 *  i80321 timer interrupts (nr 9 and 10):
		 */
		if (i == 9)
			INTERRUPT_CONNECT(tmpstr, cpu->cd.arm.tmr0_irq);
		if (i == 10)
			INTERRUPT_CONNECT(tmpstr, cpu->cd.arm.tmr1_irq);
	}

	d->status = &cpu->cd.arm.i80321_isrc;
	d->enable = &cpu->cd.arm.i80321_inten;

	/*  TODO: base = 0 on Iyonix?  */
	d->mcu_reg[MCU_SDBR / sizeof(uint32_t)] = base = 0xa0000000;
	d->mcu_reg[MCU_SBR0 / sizeof(uint32_t)] = (base + memsize) >> 25;
	d->mcu_reg[MCU_SBR1 / sizeof(uint32_t)] = (base + memsize) >> 25;

	snprintf(tmpstr, sizeof(tmpstr), "%s.i80321", devinit->interrupt_path);

	d->pci_bus = bus_pci_init(devinit->machine,
	    tmpstr	/*  pciirq  */,
	    0x90000000	/*  TODO: pci_io_offset  */,
	    0x90010000	/*  TODO: pci_mem_offset  */,
	    0xffff0000	/*  TODO: pci_portbase  */,
	    0x00000000	/*  TODO: pci_membase  */,
	    tmpstr	/*  pci_irqbase  */,
	    0x90000000	/*  TODO: isa_portbase  */,
	    0x90010000	/*  TODO: isa_membase  */,
	    "TODO: isa_irqbase" /*  TODO: isa_irqbase  */);

	memory_device_register(devinit->machine->memory, devinit->name,
	    devinit->addr, DEV_I80321_LENGTH,
	    dev_i80321_access, d, DM_DEFAULT, NULL);

	/*  TODO: Don't hardcode to 100 Hz!  */
	d->hz = 100;
	d->timer = timer_add(d->hz, tmr0_tick, d);

	machine_add_tickfunction(devinit->machine, dev_i80321_tick,
	    d, TICK_SHIFT);

	devinit->return_ptr = d->pci_bus;

	return 1;
}

