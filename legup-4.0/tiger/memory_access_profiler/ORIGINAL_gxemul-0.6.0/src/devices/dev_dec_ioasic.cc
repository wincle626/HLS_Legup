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
 *  COMMENT: IOASIC device used in the DECstation "3MIN" and "3MAX" machines
 *
 *  TODO:  Lots of stuff, such as DMA and all bits in the control registers.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "devices.h"
#include "memory.h"
#include "misc.h"

#include "thirdparty/dec_kn03.h"
#include "thirdparty/tc_ioasicreg.h"


#define IOASIC_DEBUG
/* #define debug fatal */


DEVICE_ACCESS(dec_ioasic)
{
	struct dec_ioasic_data *d = (struct dec_ioasic_data *) extra;
	uint64_t idata = 0, odata = 0;
	uint64_t curptr;
	uint32_t csr;
	int dma_len, dma_res, regnr;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	regnr = (relative_addr - IOASIC_SLOT_1_START) / 0x10;
	if (relative_addr < 0x80000 && (relative_addr & 0xf) != 0)
		fatal("[ dec_ioasic: unaligned access? relative_addr = "
		    "0x%x ]\n", (int)relative_addr);

	if (regnr >= 0 && regnr < N_DEC_IOASIC_REGS) {
		if (writeflag == MEM_WRITE)
			d->reg[regnr] = idata;
		else
			odata = d->reg[regnr];
	}

#ifdef IOASIC_DEBUG
	if (writeflag == MEM_WRITE)
		debug("[ dec_ioasic: write to address 0x%llx, data=0x"
		    "%016llx ]\n", (long long)relative_addr, (long long)idata);
	else
		debug("[ dec_ioasic: read from address 0x%llx ]\n",
		    (long long)relative_addr);
#endif

	switch (relative_addr) {

	/*  Don't print warnings for these:  */
	case IOASIC_SCSI_DMAPTR:
	case IOASIC_SCC_T1_DMAPTR:
	case IOASIC_SCC_T2_DMAPTR:
	case IOASIC_SCC_R1_DMAPTR:
	case IOASIC_SCC_R2_DMAPTR:
		break;

	case IOASIC_CSR:
		if (writeflag == MEM_WRITE) {
			csr = d->reg[(IOASIC_CSR - IOASIC_SLOT_1_START) / 0x10];

			d->reg[(IOASIC_INTR - IOASIC_SLOT_1_START) / 0x10] &=
			    ~IOASIC_INTR_T2_PAGE_END;

			if (csr & IOASIC_CSR_DMAEN_T2) {
				/*  Transmit data:  */
				curptr = (d->reg[(IOASIC_SCC_T2_DMAPTR -
				    IOASIC_SLOT_1_START) / 0x10] >> 3)
				    | ((d->reg[(IOASIC_SCC_T2_DMAPTR -
				    IOASIC_SLOT_1_START) / 0x10] & 0x1f) << 29);
				dma_len = 0x1000 - (curptr & 0xffc);

				if ((curptr & 0xfff) == 0)
					break;

				if (d->dma_func[3] != NULL) {
					d->dma_func[3](cpu,
					    d->dma_func_extra[3], curptr,
					    dma_len, 1);
				} else
					fatal("[ dec_ioasic: DMA tx: data @ "
					    "%08x, len %i bytes, but no "
					    "handler? ]\n", (int)curptr,
					    dma_len);

				/*  and signal the end of page:  */
				d->reg[(IOASIC_INTR - IOASIC_SLOT_1_START) /
				    0x10] |= IOASIC_INTR_T2_PAGE_END;

				d->reg[(IOASIC_CSR - IOASIC_SLOT_1_START) /
				    0x10] &= ~IOASIC_CSR_DMAEN_T2;
				curptr |= 0xfff;
				curptr ++;

				d->reg[(IOASIC_SCC_T2_DMAPTR -
				    IOASIC_SLOT_1_START) / 0x10] = ((curptr <<
				    3) & ~0x1f) | ((curptr >> 29) & 0x1f);
			}

			if (csr & IOASIC_CSR_DMAEN_R2) {
				/*  Receive data:  */
				curptr = (d->reg[(IOASIC_SCC_R2_DMAPTR -
				    IOASIC_SLOT_1_START) / 0x10] >> 3)
				    | ((d->reg[(IOASIC_SCC_R2_DMAPTR -
				    IOASIC_SLOT_1_START) / 0x10] & 0x1f) << 29);
				dma_len = 0x1000 - (curptr & 0xffc);

				dma_res = 0;
				if (d->dma_func[3] != NULL) {
					dma_res = d->dma_func[3](cpu,
					    d->dma_func_extra[3], curptr,
					    dma_len, 0);
				} else
					fatal("[ dec_ioasic: DMA tx: data @ "
					    "%08x, len %i bytes, but no "
					    "handler? ]\n", (int)curptr,
					    dma_len);

				/*  and signal the end of page:  */
				if (dma_res > 0) {
					if ((curptr & 0x800) != ((curptr +
					    dma_res) & 0x800))
						d->reg[(IOASIC_INTR -
						    IOASIC_SLOT_1_START) / 0x10]
						    |= IOASIC_INTR_R2_HALF_PAGE;
					curptr += dma_res;
/*					d->reg[(IOASIC_CSR - IOASIC_SLOT_1_START
					   ) / 0x10] &= ~IOASIC_CSR_DMAEN_R2; */
					d->reg[(IOASIC_SCC_R2_DMAPTR -
					    IOASIC_SLOT_1_START) / 0x10] =
					    ((curptr << 3) & ~0x1f) | ((curptr
					    >> 29) & 0x1f);
				}
			}
		}
		break;

	case IOASIC_INTR:
		if (writeflag == MEM_READ) {
			odata = d->reg[(IOASIC_INTR - IOASIC_SLOT_1_START)
			    / 0x10];
			/*  Note/TODO: How about other models than KN03?  */
			if (!d->rackmount_flag)
				odata |= KN03_INTR_PROD_JUMPER;
		} else {
			/*  Clear bits on write.  */
			d->reg[(IOASIC_INTR - IOASIC_SLOT_1_START) / 0x10] &=
			    ~idata;

			/*  Make sure that the CPU interrupt is deasserted as
			    well:  */
fatal("TODO: interrupt rewrite!\n");
abort();
//			if (idata != 0)
//				cpu_interrupt_ack(cpu, 8 + idata);
		}
		break;

	case IOASIC_IMSK:
		if (writeflag == MEM_WRITE) {
			d->reg[(IOASIC_IMSK - IOASIC_SLOT_1_START) / 0x10] =
			    idata;
fatal("TODO: interrupt rewrite!\n");
abort();
//			cpu_interrupt_ack(cpu, 8 + 0);
		} else
			odata = d->reg[(IOASIC_IMSK - IOASIC_SLOT_1_START) /
			    0x10];
		break;

	case IOASIC_CTR:
		if (writeflag == MEM_READ)
			odata = 0;
		break;

	case 0x80000:
	case 0x80004:
	case 0x80008:
	case 0x8000c:
	case 0x80010:
	case 0x80014:
		/*  Station's ethernet address:  */
		if (writeflag == MEM_WRITE) {
			fatal("[ dec_ioasic: attempt to write to the station's"
			    " ethernet address? ]\n");
		} else {
			odata = ((relative_addr - 0x80000) / 4 + 1) * 0x10;
		}
		break;

	default:
		if (writeflag == MEM_WRITE)
			fatal("[ dec_ioasic: unimplemented write to address "
			    "0x%llx, data=0x%016llx ]\n",
			    (long long)relative_addr, (long long)idata);
		else
			fatal("[ dec_ioasic: unimplemented read from address "
			    "0x%llx ]\n", (long long)relative_addr);
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


/*
 *  dev_dec_ioasic_init():
 *
 *  For DECstation "type 4", the rackmount_flag selects which model type
 *  the IOASIC should identify itself as (5000 for zero, 5900 if rackmount_flag
 *  is non-zero). It is probably not meaningful on other machines than
 *  type 4.
 */
struct dec_ioasic_data *dev_dec_ioasic_init(struct cpu *cpu,
	struct memory *mem, uint64_t baseaddr, int rackmount_flag)
{
	struct dec_ioasic_data *d;

	CHECK_ALLOCATION(d = (struct dec_ioasic_data *) malloc(sizeof(struct dec_ioasic_data)));
	memset(d, 0, sizeof(struct dec_ioasic_data));

	d->rackmount_flag = rackmount_flag;

	memory_device_register(mem, "dec_ioasic", baseaddr,
	    DEV_DEC_IOASIC_LENGTH, dev_dec_ioasic_access, (void *)d,
	    DM_DEFAULT, NULL);

	return d;
}

