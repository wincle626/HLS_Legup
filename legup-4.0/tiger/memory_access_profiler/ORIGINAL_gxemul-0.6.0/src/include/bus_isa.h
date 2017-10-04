#ifndef	BUS_ISA_H
#define	BUS_ISA_H

/*
 *  Copyright (C) 2005-2010  Anders Gavare.  All rights reserved.
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
 *  ISA bus.
 */

#include "misc.h"
#include "interrupt.h"

#ifdef BUS_ISA_C

struct bus_isa_data {
	struct interrupt irq;

	struct pic8259_data* pic1;
	struct pic8259_data* pic2;
	int		*ptr_to_pending_timer_interrupts;
	int		*ptr_to_last_int;

	uint64_t	isa_portbase;
	uint64_t	isa_membase;
};

#endif	/*  BUS_ISA_C  */

struct bus_isa_data *bus_isa_init(struct machine *machine,
	char *interrupt_base_path, uint32_t bus_isa_flags,
	uint64_t isa_portbase, uint64_t isa_membase);

/*  ISA bus flags:  */
#define	BUS_ISA_IDE0			1
#define	BUS_ISA_IDE1			2
#define	BUS_ISA_FDC			4
#define	BUS_ISA_VGA			8
#define	BUS_ISA_VGA_FORCE		16
#define	BUS_ISA_PCKBC_FORCE_USE		32
#define	BUS_ISA_PCKBC_NONPCSTYLE	64
#define	BUS_ISA_NO_SECOND_PIC		128
#define	BUS_ISA_LPTBASE_3BC		256
#define	BUS_ISA_EXTERNAL_PIC		512

#endif	/*  BUS_ISA_H  */
