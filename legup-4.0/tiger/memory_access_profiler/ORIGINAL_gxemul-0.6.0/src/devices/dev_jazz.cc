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
 *  COMMENT: Microsoft Jazz-related stuff (Acer PICA-61, etc)
 *
 *  TODO/NOTE: This is mostly a quick hack, it doesn't really implement
 *  much of the Jazz architecture.  Also, the a0/20 isa-like stuff is
 *  not supposed to be here.
 *
 *  TODO: Figure out how the int enable mask works; it seems to be shifted
 *  10 bits (?) according to NetBSD/arc sources.
 *
 *  TODO: Don't hardcode the timer to 100 Hz.
 *
 *  JAZZ interrupts 0..14 are connected to MIPS irq 3,
 *  JAZZ interrupt 15 (the timer) is connected to MIPS irq 6,
 *  and ISA interrupts 0..15 are connected to MIPS irq 4.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "device.h"
#include "devices.h"
#include "interrupt.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"
#include "timer.h"

#include "thirdparty/jazz_r4030_dma.h"
#include "thirdparty/pica.h"


#define	DEV_JAZZ_LENGTH			0x280
#define	DEV_JAZZ_TICKSHIFT		14
#define	PICA_TIMER_IRQ			15

struct jazz_data {
	struct interrupt mips_irq_3;
	struct interrupt mips_irq_4;
	struct interrupt mips_irq_6;

	struct cpu	*cpu;

	/*  Jazz stuff:  */
	uint32_t	int_enable_mask;	/*  TODO!  */
	uint32_t	int_asserted;

	/*  ISA stuff:  */
	uint32_t	isa_int_enable_mask;
	uint32_t	isa_int_asserted;

	int		interval;
	int		interval_start;

	struct timer	*timer;
	int		pending_timer_interrupts;
	int		jazz_timer_value;
	int		jazz_timer_current;
	struct interrupt jazz_timer_irq;

	uint64_t	dma_translation_table_base;
	uint64_t	dma_translation_table_limit;

	uint32_t	dma0_mode;
	uint32_t	dma0_enable;
	uint32_t	dma0_count;
	uint32_t	dma0_addr;

	uint32_t	dma1_mode;
	/*  same for dma1,2,3 actually (TODO)  */

	int		led;
};


void reassert_isa_interrupts(struct jazz_data *d)
{
	if (d->isa_int_asserted & d->isa_int_enable_mask)
		INTERRUPT_ASSERT(d->mips_irq_4);
	else
		INTERRUPT_DEASSERT(d->mips_irq_4);
}


void jazz_interrupt_assert(struct interrupt *interrupt)
{
	struct jazz_data *d = (struct jazz_data *) interrupt->extra;
	d->int_asserted |= (1 << interrupt->line);

	if (d->int_asserted & 0x7fff)
		INTERRUPT_ASSERT(d->mips_irq_3);
	if (d->int_asserted & 0x8000)
		INTERRUPT_ASSERT(d->mips_irq_6);
}
void jazz_interrupt_deassert(struct interrupt *interrupt)
{
	struct jazz_data *d = (struct jazz_data *) interrupt->extra;
	d->int_asserted &= ~(1 << interrupt->line);

	if (!(d->int_asserted & 0x7fff))
		INTERRUPT_DEASSERT(d->mips_irq_3);
	if (!(d->int_asserted & 0x8000))
		INTERRUPT_DEASSERT(d->mips_irq_6);
}
void jazz_isa_interrupt_assert(struct interrupt *interrupt)
{
	struct jazz_data *d = (struct jazz_data *) interrupt->extra;
	d->isa_int_asserted |= (1 << interrupt->line);
	reassert_isa_interrupts(d);
}
void jazz_isa_interrupt_deassert(struct interrupt *interrupt)
{
	struct jazz_data *d = (struct jazz_data *) interrupt->extra;
	d->isa_int_asserted &= ~(1 << interrupt->line);
	reassert_isa_interrupts(d);
}
 

/*
 *  dev_jazz_dma_controller():
 */
size_t dev_jazz_dma_controller(void *dma_controller_data,
	unsigned char *data, size_t len, int writeflag)
{
	struct jazz_data *d = (struct jazz_data *) dma_controller_data;
	struct cpu *cpu = d->cpu;
	int i, enab_writeflag;
	int res, ncpy;
	uint32_t dma_addr;
	unsigned char tr[sizeof(uint32_t)];
	uint32_t phys_addr;

#if 0
	fatal("[ dev_jazz_dma_controller(): writeflag=%i, len=%i, data =",
	    writeflag, (int)len);
	for (i=0; i<len; i++)
		fatal(" %02x", data[i]);
	fatal(" mode=%08x enable=%08x count=%08x addr=%08x",
	    d->dma0_mode, d->dma0_enable, d->dma0_count, d->dma0_addr);
	fatal(" table=%08x",
	    d->dma_translation_table_base);
	fatal(" ]\n");
#endif

	if (!(d->dma0_enable & R4030_DMA_ENAB_RUN)) {
		fatal("[ dev_jazz_dma_controller(): dma not enabled? ]\n");
		/*  return 0;  */
	}

	/*  R4030 "write" means write to the device, writeflag as the
	    argument to this function means write to memory.  */
	enab_writeflag = (d->dma0_enable & R4030_DMA_ENAB_WRITE)? 0 : 1;
	if (enab_writeflag != writeflag) {
		fatal("[ dev_jazz_dma_controller(): wrong direction? ]\n");
		return 0;
	}

	dma_addr = d->dma0_addr;
	i = 0;
	while (dma_addr < d->dma0_addr + d->dma0_count && i < (int32_t)len) {

		res = cpu->memory_rw(cpu, cpu->mem,
		    d->dma_translation_table_base + (dma_addr >> 12) * 8,
		    tr, sizeof(tr), 0, PHYSICAL | NO_EXCEPTIONS);

		if (cpu->byte_order==EMUL_BIG_ENDIAN)
			phys_addr = (tr[0] << 24) + (tr[1] << 16) +
			    (tr[2] << 8) + tr[3];
		else
			phys_addr = (tr[3] << 24) + (tr[2] << 16) +
			    (tr[1] << 8) + tr[0];
		phys_addr &= ~0xfff;	/*  just in case  */
		phys_addr += (dma_addr & 0xfff);

		/*  fatal(" !!! dma_addr = %08x, phys_addr = %08x\n",
		    (int)dma_addr, (int)phys_addr);  */

		/*  Speed up the copying by copying 16 or 256 bytes:  */
		ncpy = 1;
		if ((phys_addr & 15) == 0 && i + 15 <= (int32_t)len)
			ncpy = 15;
		if ((phys_addr & 255) == 0 && i + 255 <= (int32_t)len)
			ncpy = 255;

		res = cpu->memory_rw(cpu, cpu->mem, phys_addr,
		    &data[i], ncpy, writeflag, PHYSICAL | NO_EXCEPTIONS);

		dma_addr += ncpy;
		i += ncpy;
	}

	/*  TODO: Is this correct?  */
	d->dma0_count = 0;

	return len;
}


static void timer_tick(struct timer *t, void *extra)
{
	struct jazz_data *d = (struct jazz_data *) extra;
	d->pending_timer_interrupts ++;
}


DEVICE_TICK(jazz)
{
	struct jazz_data *d = (struct jazz_data *) extra;

	/*  Used by NetBSD/arc and OpenBSD/arc:  */
	if (d->interval_start > 0 && d->interval > 0
	    && (d->int_enable_mask & 2) /* Hm? */ ) {
		d->interval -= 2;
		if (d->interval <= 0) {
			/*  debug("[ jazz: interval timer interrupt ]\n");
			  INTERRUPT_ASSERT(d->jazz_timer_irq);  */
		}

		/*  New timer system:  */
		if (d->pending_timer_interrupts > 0)
			INTERRUPT_ASSERT(d->jazz_timer_irq);
	}

	/*  Linux?  */
	if (d->jazz_timer_value != 0) {
		d->jazz_timer_current -= 5;
		if (d->jazz_timer_current < 1) {
			d->jazz_timer_current = d->jazz_timer_value;
			/*  INTERRUPT_ASSERT(d->mips_irq_6);  */
		}

		/*  New timer system:  */
		if (d->pending_timer_interrupts > 0)
			INTERRUPT_ASSERT(d->mips_irq_6);
	}
}


DEVICE_ACCESS(jazz)
{
	struct jazz_data *d = (struct jazz_data *) extra;
	uint64_t idata = 0, odata = 0;
	int regnr;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	regnr = relative_addr / sizeof(uint32_t);

	switch (relative_addr) {
	case R4030_SYS_CONFIG:
		if (writeflag == MEM_WRITE) {
			fatal("[ jazz: unimplemented write to R4030_SYS_CONFIG"
			    ", data=0x%08x ]\n", (int)idata);
		} else {
			/*  Reading the config register should give
			    0x0104 or 0x0410. Why? TODO  */
			odata = 0x104;
		}
		break;
	case R4030_SYS_TL_BASE:
		if (writeflag == MEM_WRITE) {
			d->dma_translation_table_base = idata;
		} else {
			odata = d->dma_translation_table_base;
		}
		break;
	case R4030_SYS_TL_LIMIT:
		if (writeflag == MEM_WRITE) {
			d->dma_translation_table_limit = idata;
		} else {
			odata = d->dma_translation_table_limit;
		}
		break;
	case R4030_SYS_TL_IVALID:
		/*  TODO: Does invalidation actually need to be implemented?  */
		break;
	case R4030_SYS_DMA0_REGS:
		if (writeflag == MEM_WRITE) {
			d->dma0_mode = idata;
		} else {
			odata = d->dma0_mode;
		}
		break;
	case R4030_SYS_DMA0_REGS + 0x8:
		if (writeflag == MEM_WRITE) {
			d->dma0_enable = idata;
		} else {
			odata = d->dma0_enable;
		}
		break;
	case R4030_SYS_DMA0_REGS + 0x10:
		if (writeflag == MEM_WRITE) {
			d->dma0_count = idata;
		} else {
			odata = d->dma0_count;
		}
		break;
	case R4030_SYS_DMA0_REGS + 0x18:
		if (writeflag == MEM_WRITE) {
			d->dma0_addr = idata;
		} else {
			odata = d->dma0_addr;
		}
		break;
	case R4030_SYS_DMA1_REGS:
		if (writeflag == MEM_WRITE) {
			d->dma1_mode = idata;
		} else {
			odata = d->dma1_mode;
		}
		break;
	case R4030_SYS_ISA_VECTOR:
		/*  ?  */
printf("R4030_SYS_ISA_VECTOR: w=%i\n", writeflag);
		{
			uint32_t x = d->isa_int_asserted
			    & d->isa_int_enable_mask;
			odata = 0;
			while (odata < 16) {
				if (x & (1 << odata))
					break;
				odata ++;
			}
			if (odata >= 16)
				odata = 0;
		}
		break;
	case R4030_SYS_IT_VALUE:  /*  Interval timer reload value  */
		if (writeflag == MEM_WRITE) {
			d->interval_start = idata;
			d->interval = d->interval_start;
		} else
			odata = d->interval_start;
		break;
	case R4030_SYS_IT_STAT:
		/*  Accessing this word seems to acknowledge interrupts?  */
		INTERRUPT_DEASSERT(d->jazz_timer_irq);
		if (d->pending_timer_interrupts > 0)
			d->pending_timer_interrupts --;

		if (writeflag == MEM_WRITE)
			d->interval = idata;
		else
			odata = d->interval;
		d->interval = d->interval_start;
		break;
	case R4030_SYS_EXT_IMASK:
		if (writeflag == MEM_WRITE) {
			int old_assert_3 = (0x7fff &
			    d->int_asserted & d->int_enable_mask);
			int old_assert_6 = (0x8000 &
			    d->int_asserted & d->int_enable_mask);
			int new_assert_3, new_assert_6;

			d->int_enable_mask = idata;

			new_assert_3 =
			    d->int_asserted & d->int_enable_mask & 0x7fff;
			new_assert_6 =
			    d->int_asserted & d->int_enable_mask & 0x8000;

			if (old_assert_3 && !new_assert_3)
				INTERRUPT_DEASSERT(d->mips_irq_3);
			else if (!old_assert_3 && new_assert_3)
				INTERRUPT_ASSERT(d->mips_irq_3);

			if (old_assert_6 && !new_assert_6)
				INTERRUPT_DEASSERT(d->mips_irq_6);
			else if (!old_assert_6 && new_assert_6)
				INTERRUPT_ASSERT(d->mips_irq_6);
		} else
			odata = d->int_enable_mask;
		break;
	default:
		if (writeflag == MEM_WRITE) {
			fatal("[ jazz: unimplemented write to address 0x%x"
			    ", data=0x%02x ]\n", (int)relative_addr,
			    (int)idata);
		} else {
			fatal("[ jazz: unimplemented read from address 0x%x"
			    " ]\n", (int)relative_addr);
		}
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVICE_ACCESS(jazz_led)
{
	struct jazz_data *d = (struct jazz_data *) extra;
	uint64_t idata = 0, odata = 0;
	int regnr;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	regnr = relative_addr / sizeof(uint32_t);

	switch (relative_addr) {
	case 0:
		if (writeflag == MEM_WRITE) {
			d->led = idata;
			debug("[ jazz_led: write to LED: 0x%02x ]\n",
			    (int)idata);
		} else {
			odata = d->led;
		}
		break;
	default:
		if (writeflag == MEM_WRITE) {
			fatal("[ jazz_led: unimplemented write to address 0x%x"
			    ", data=0x%02x ]\n", (int)relative_addr,
			    (int)idata);
		} else {
			fatal("[ jazz_led: unimplemented read from address 0x%x"
			    " ]\n", (int)relative_addr);
		}
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


/*
 *  dev_jazz_a0_access():
 *
 *  ISA interrupt stuff, high 8 interrupts.
 *
 *  TODO: use isa8 stuff instead!
 */
DEVICE_ACCESS(jazz_a0)
{
	struct jazz_data *d = (struct jazz_data *) extra;
	uint64_t idata = 0, odata = 0;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	switch (relative_addr) {
	case 0:
		if (writeflag == MEM_WRITE) {
			/*  TODO: only if idata == 0x20?  */
			d->isa_int_asserted &= 0xff;

			reassert_isa_interrupts(d);
		}
		break;
	case 1:
		if (writeflag == MEM_WRITE) {
			idata = ((idata ^ 0xff) & 0xff) << 8;
			d->isa_int_enable_mask =
			    (d->isa_int_enable_mask & 0xff) | idata;
			debug("[ jazz_isa_a0: setting isa_int_enable_mask "
			    "to 0x%04x ]\n", (int)d->isa_int_enable_mask);

			reassert_isa_interrupts(d);
		} else
			odata = d->isa_int_enable_mask;
		break;
	default:
		if (writeflag == MEM_WRITE) {
			fatal("[ jazz_isa_a0: unimplemented write to "
			    "address 0x%x, data=0x%02x ]\n",
			    (int)relative_addr, (int)idata);
		} else {
			fatal("[ jazz_isa_a0: unimplemented read from "
			    "address 0x%x ]\n", (int)relative_addr);
		}
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


/*
 *  dev_jazz_20_access():
 *
 *  ISA interrupt stuff, low 8 interrupts.
 */
DEVICE_ACCESS(jazz_20)
{
	struct jazz_data *d = (struct jazz_data *) extra;
	uint64_t idata = 0, odata = 0;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	switch (relative_addr) {
	case 0:
		if (writeflag == MEM_WRITE) {
			/*  TODO: only if idata == 0x20?  */
			d->isa_int_asserted &= 0xff00;
			reassert_isa_interrupts(d);
		}
		break;
	case 1:
		if (writeflag == MEM_WRITE) {
			idata = (idata ^ 0xff) & 0xff;
			d->isa_int_enable_mask =
			    (d->isa_int_enable_mask & 0xff00) | idata;
			debug("[ jazz_isa_20: setting isa_int_enable_mask "
			    "to 0x%04x ]\n", (int)d->isa_int_enable_mask);

			reassert_isa_interrupts(d);
		} else
			odata = d->isa_int_enable_mask;
		break;
	default:
		if (writeflag == MEM_WRITE) {
			fatal("[ jazz_isa_20: unimplemented write to "
			    "address 0x%x, data=0x%02x ]\n",
			    (int)relative_addr, (int)idata);
		} else {
			fatal("[ jazz_isa_20: unimplemented read from "
			    "address 0x%x ]\n", (int)relative_addr);
		}
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


/*
 *  dev_jazz_jazzio_access():
 *
 *  See jazzio_intr() in NetBSD's
 *  /usr/src/sys/arch/arc/jazz/jazzio.c for more info.
 */
DEVICE_ACCESS(jazz_jazzio)
{
	struct jazz_data *d = (struct jazz_data *) extra;
	uint64_t idata = 0, odata = 0;
	int i, v;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	switch (relative_addr) {
	case 0:
		v = 0;
		for (i=0; i<15; i++) {
			if (d->int_asserted & (1<<i)) {
				v = i+1;
				break;
			}
		}
		odata = v << 2;
		break;
	case 2:
		/*  TODO: Should this be here?!  */

		if (writeflag == MEM_WRITE)
			d->jazz_timer_value = idata;
		else
			odata = d->jazz_timer_value;
		break;
	default:
		if (writeflag == MEM_WRITE) {
			fatal("[ jazzio: unimplemented write to address 0x%x"
			    ", data=0x%02x ]\n", (int)relative_addr,
			    (int)idata);
		} else {
			fatal("[ jazzio: unimplemented read from address 0x%x"
			    " ]\n", (int)relative_addr);
		}
	}

	/*  This is needed by Windows NT during startup:  */
	INTERRUPT_DEASSERT(d->mips_irq_3);

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVINIT(jazz)
{
	struct jazz_data *d;
	char tmpstr[300];
	int i;

	CHECK_ALLOCATION(d = (struct jazz_data *) malloc(sizeof(struct jazz_data)));
	memset(d, 0, sizeof(struct jazz_data));

	d->cpu = devinit->machine->cpus[0];	/*  TODO  */

	d->isa_int_enable_mask = 0xffff;

	/*
	 *  Register 16 native JAZZ irqs, and 16 ISA irqs:
	 *
	 *  machine[y].cpu[z].jazz.%i		(native)
	 *  machine[y].cpu[z].jazz.isa.%i	(ISA)
	 */
	for (i=0; i<16; i++) {
		struct interrupt templ;
		char n[300];
		snprintf(n, sizeof(n), "%s.jazz.%i",
		    devinit->interrupt_path, i);
		memset(&templ, 0, sizeof(templ));
		templ.line = i;
		templ.name = n;
		templ.extra = d;
		templ.interrupt_assert = jazz_interrupt_assert;
		templ.interrupt_deassert = jazz_interrupt_deassert;
		interrupt_handler_register(&templ);
	}
	for (i=0; i<16; i++) {
		struct interrupt templ;
		char n[300];
		snprintf(n, sizeof(n), "%s.jazz.isa.%i",
		    devinit->interrupt_path, i);
		memset(&templ, 0, sizeof(templ));
		templ.line = i;
		templ.name = n;
		templ.extra = d;
		templ.interrupt_assert = jazz_isa_interrupt_assert;
		templ.interrupt_deassert = jazz_isa_interrupt_deassert;
		interrupt_handler_register(&templ);
	}

	/*  Connect to MIPS CPU interrupt lines:  */
	snprintf(tmpstr, sizeof(tmpstr), "%s.3", devinit->interrupt_path);
	INTERRUPT_CONNECT(tmpstr, d->mips_irq_3);
	snprintf(tmpstr, sizeof(tmpstr), "%s.4", devinit->interrupt_path);
	INTERRUPT_CONNECT(tmpstr, d->mips_irq_4);
	snprintf(tmpstr, sizeof(tmpstr), "%s.6", devinit->interrupt_path);
	INTERRUPT_CONNECT(tmpstr, d->mips_irq_6);

	/*  Connect to JAZZ timer interrupt:  */
	snprintf(tmpstr, sizeof(tmpstr), "%s.jazz.%i",
	    devinit->interrupt_path, PICA_TIMER_IRQ);
	INTERRUPT_CONNECT(tmpstr, d->jazz_timer_irq);

	memory_device_register(devinit->machine->memory, "jazz",
	    devinit->addr, DEV_JAZZ_LENGTH,
	    dev_jazz_access, (void *)d, DM_DEFAULT, NULL);

	/*  At least for Magnum and Pica-61:  */
	memory_device_register(devinit->machine->memory, "jazz_led",
	    0x08000f000ULL, 4, dev_jazz_led_access, (void *)d,
	    DM_DEFAULT, NULL);

	memory_device_register(devinit->machine->memory, "jazz_isa_20",
	    0x90000020ULL, 2, dev_jazz_20_access, (void *)d, DM_DEFAULT, NULL);

	memory_device_register(devinit->machine->memory, "jazz_isa_a0",
	    0x900000a0ULL, 2, dev_jazz_a0_access, (void *)d, DM_DEFAULT, NULL);

	memory_device_register(devinit->machine->memory, "pica_jazzio",
	    0xf0000000ULL, 4, dev_jazz_jazzio_access, (void *)d,
	    DM_DEFAULT, NULL);

	/*  Add a timer, hardcoded to 100 Hz. TODO: Don't hardcode!  */
	d->timer = timer_add(100.0, timer_tick, d);
	machine_add_tickfunction(devinit->machine, dev_jazz_tick,
	    d, DEV_JAZZ_TICKSHIFT);

	devinit->return_ptr = d;

	return 1;
}

