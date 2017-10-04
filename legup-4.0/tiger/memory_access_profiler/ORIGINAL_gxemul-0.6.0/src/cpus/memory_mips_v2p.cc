/*
 *  Copyright (C) 2003-2009  Anders Gavare.  All rights reserved.
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
 */


/*
 *  translate_v2p():
 *
 *  Translate a virtual MIPS address to a physical address, by looking up the
 *  address in the TLB. On failure, an exception is generated (except for the
 *  case when FLAG_NOEXCEPTIONS is used).
 *
 *  Note: This function is long and hairy, it is included several times with
 *        various defines set, to produce more or less optimized versions of
 *        the function for different emulated CPU types.
 *
 *  V2P_MMU3K       Defined for R2000/R3000 emulation. If it is not defined,
 *                  R4000+/MIPS32/MIPS64 is presumed.
 *  V2P_MMU10K      This enables the use of 44 userspace bits, instead of 40.
 *  V2P_MMU4100     VR41xx processors support 1 KB pages, so their page mask
 *                  is slightly different. (The emulator only supports 4 KB
 *                  pages, though.)
 *  V2P_MMU8K       Not yet. (TODO.)
 *
 *
 *  Note:  Unfortunately, the variable name vpn2 is poorly choosen for R2K/R3K,
 *         since it actual contains the vpn.
 *
 *  Return values:
 *	0  Failure
 *	1  Success, the page is readable only
 *	2  Success, the page is read/write
 */
int TRANSLATE_ADDRESS(struct cpu *cpu, uint64_t vaddr,
	uint64_t *return_paddr, int flags)
{
	int writeflag = flags & FLAG_WRITEFLAG? MEM_WRITE : MEM_READ;
	int no_exceptions = flags & FLAG_NOEXCEPTIONS;
	int ksu, use_tlb, status, i;
	uint64_t vaddr_vpn2=0, vaddr_asid=0;
	int exccode, tlb_refill;
	struct mips_coproc *cp0;

#ifdef V2P_MMU3K
	const int x_64 = 0;
	const int n_tlbs = 64;
	const uint32_t pmask = 0xfff;
	uint64_t xuseg_top;		/*  Well, useg actually.  */
#else
#ifdef V2P_MMU10K
	const uint64_t vpn2_mask = ENTRYHI_R_MASK | ENTRYHI_VPN2_MASK_R10K;
	uint64_t xuseg_top = ENTRYHI_VPN2_MASK_R10K | 0x1fffULL;
#else
#ifdef V2P_MMU4100
	const uint64_t vpn2_mask = ENTRYHI_R_MASK | ENTRYHI_VPN2_MASK | 0x1800;
#else
	const uint64_t vpn2_mask = ENTRYHI_R_MASK | ENTRYHI_VPN2_MASK;
#endif
	uint64_t xuseg_top = ENTRYHI_VPN2_MASK | 0x1fffULL;
#endif
	int x_64;	/*  non-zero for 64-bit address space accesses  */
	int pageshift, n_tlbs;
	uint32_t pmask;
#ifdef V2P_MMU4100
	const int pagemask_mask = PAGEMASK_MASK_R4100;
	const int pagemask_shift = PAGEMASK_SHIFT_R4100;
	const int pfn_shift = 10;
#else
	const int pagemask_mask = PAGEMASK_MASK;
	const int pagemask_shift = PAGEMASK_SHIFT;
	const int pfn_shift = 12;
#endif
#endif	/*  !V2P_MMU3K  */


	exccode = -1;
	tlb_refill = 1;

	/*  Cached values:  */
	cp0 = cpu->cd.mips.coproc[0];
	status = cp0->reg[COP0_STATUS];

	/*
	 *  MIPS R4000+ and MIPS64 Address Translation:
	 *
	 *  An address may be in one of the kernel segments, that are directly
	 *  mapped to physical addresses, or the address needs to be looked up
	 *  in the TLB entries.
	 *
	 *  KSU: EXL: ERL: X:  Name:   Range:
	 *  ---- ---- ---- --  -----   ------
	 *
	 *   10   0    0    0  useg    0 - 0x7fffffff    (2GB)  (via TLB)
	 *   10   0    0    1  xuseg   0 - 0xffffffffff  (1TB)  (via TLB)
	 *
	 *   01   0    0    0  suseg   0          - 0x7fffffff  (2GB via TLB)
	 *   01   0    0    0  ssseg   0xc0000000 - 0xdfffffff  (0.5 GB via TLB)
	 *   01   0    0    1  xsuseg  0 - 0xffffffffff         (1TB)  (via TLB)
	 *   01   0    0    1  xsseg   0x4000000000000000 - 0x400000ffffffffff
	 *					  (1TB)  (via TLB)
	 *   01   0    0    1  csseg   0xffffffffc0000000 - 0xffffffffdfffffff
	 *					  (0.5TB)  (via TLB)
	 *
	 *   00   x    x    0  kuseg   0 - 0x7fffffff  (2GB)  (via TLB)  (*)
	 *   00   x    x    0  kseg0   0x80000000 - 0x9fffffff (0.5GB)
	 *					  unmapped, cached
	 *   00   x    x    0  kseg1   0xa0000000 - 0xbfffffff (0.5GB)
	 *					  unmapped, uncached
	 *   00   x    x    0  ksseg   0xc0000000 - 0xdfffffff (0.5GB) (via TLB)
	 *   00   x    x    0  kseg3   0xe0000000 - 0xffffffff (0.5GB) (via TLB)
	 *   00   x    x    1  xksuseg 0 - 0xffffffffff (1TB) (via TLB) (*)
	 *   00   x    x    1  xksseg  0x4000000000000000 - 0x400000ffffffffff
	 *					  (1TB)  (via TLB)
	 *   00   x    x    1  xkphys  0x8000000000000000 - 0xbfffffffffffffff
	 *   00   x    x    1  xkseg   0xc000000000000000 - 0xc00000ff7fffffff
	 *   00   x    x    1  ckseg0  0xffffffff80000000 - 0xffffffff9fffffff
	 *   00   x    x    1  ckseg1  0xffffffffa0000000 - 0xffffffffbfffffff
	 *   00   x    x    1  cksseg  0xffffffffc0000000 - 0xffffffffdfffffff
	 *   00   x    x    1  ckseg3  0xffffffffe0000000 - 0xffffffffffffffff
	 *					  like 0x80000000 - 0xffffffff
	 *
	 *  (*) = if ERL=1 then kuseg is not via TLB, but unmapped,
	 *  uncached physical memory.
	 *
	 *  (KSU==0 or EXL=1 or ERL=1 is enough to use k*seg*.)
	 *
	 *  An invalid address causes an Address Error.
	 *
	 *  See chapter 4, page 96, in the R4000 manual for more info!
	 */

#ifdef V2P_MMU3K
	if (status & MIPS1_SR_KU_CUR)
		ksu = KSU_USER;
	else
		ksu = KSU_KERNEL;

	/*  These are needed later:  */
	vaddr_asid = cp0->reg[COP0_ENTRYHI] & R2K3K_ENTRYHI_ASID_MASK;
	vaddr_vpn2 = vaddr & R2K3K_ENTRYHI_VPN_MASK;
#else
	/*  kx,sx,ux = 0 for 32-bit addressing, 1 for 64-bit addressing.  */
	ksu = (status & STATUS_KSU_MASK) >> STATUS_KSU_SHIFT;
	if (status & (STATUS_EXL | STATUS_ERL))
		ksu = KSU_KERNEL;

	switch (ksu) {
	case KSU_USER:
		x_64 = status & STATUS_UX;
		break;
	case KSU_KERNEL:
		x_64 = status & STATUS_KX;
		break;
	case KSU_SUPERVISOR:
		x_64 = status & STATUS_SX;
		/*  FALLTHROUGH, since supervisor address spaces are not
		    really implemented yet.  */
	default:fatal("memory_mips_v2p.c: ksu=%i not yet implemented yet\n",
		    ksu);
		exit(1);
	}

	n_tlbs = cpu->cd.mips.cpu_type.nr_of_tlb_entries;

	/*  Having this here suppresses a compiler warning:  */
	pageshift = 12;

	/*  KUSEG: 0x00000000 - 0x7fffffff if ERL = 1 and KSU = kernel:  */
	if (ksu == KSU_KERNEL && (status & STATUS_ERL) &&
	    vaddr <= 0x7fffffff) {
		*return_paddr = vaddr & 0x7fffffff;
		return 2;
	}

	/*
	 *  XKPHYS: 0x8000000000000000 - 0xbfffffffffffffff
	 *
	 *  TODO: Is the correct error generated if accessing XKPHYS from
	 *        usermode?
	 *
	 *  TODO: Magic on SGI machines... Cache control, NUMA, etc.:
	 *        0x9000000080000000   = disable L2 cache (?)
	 *        0x90000000a0000000   = something on IP30?
	 *        0x92.... and 0x96... = NUMA on IP27
	 */
	if (ksu == KSU_KERNEL && (vaddr & ENTRYHI_R_MASK) == ENTRYHI_R_XKPHYS) {
		*return_paddr = vaddr & (((uint64_t)1 << 44) - 1);
		return 2;
	}

	/*  This is needed later:  */
	vaddr_asid = cp0->reg[COP0_ENTRYHI] & ENTRYHI_ASID;
	/*  vpn2 depends on pagemask, which is not fixed on R4000  */
#endif

	/*  If 32-bit, truncate address and sign extend:  */
	if (x_64 == 0) {
		vaddr = (int32_t) vaddr;
		xuseg_top = 0x7fffffff;
		/*  (Actually useg for R2000/R3000)  */
	}

	if (vaddr <= xuseg_top) {
		use_tlb = 1;
	} else {
		if (ksu == KSU_KERNEL) {
			/*  kseg0, kseg1:  */
			if (vaddr >= (uint64_t)0xffffffff80000000ULL &&
			    vaddr <= (uint64_t)0xffffffffbfffffffULL) {
				*return_paddr = vaddr & 0x1fffffff;
				return 2;
			}

			/*  other segments:  */
			use_tlb = 1;
		} else {
			use_tlb = 0;
		}
	}

	if (use_tlb) {
#ifndef V2P_MMU3K
		int odd = 0;
		uint64_t cached_lo1 = 0;
#endif
		int g_bit, v_bit, d_bit;
		uint64_t cached_hi, cached_lo0;
		uint64_t entry_vpn2 = 0, entry_asid, pfn;
		int i_end;

		i = cpu->cd.mips.last_written_tlb_index;
		i_end = i == 0? n_tlbs-1 : i - 1;

		/*  Scan all TLB entries:  */
		for (;;) {
#ifdef V2P_MMU3K
			/*  R3000 or similar:  */
			cached_hi = cp0->tlbs[i].hi;
			cached_lo0 = cp0->tlbs[i].lo0;

			entry_vpn2 = cached_hi & R2K3K_ENTRYHI_VPN_MASK;
			entry_asid = cached_hi & R2K3K_ENTRYHI_ASID_MASK;
			g_bit = cached_lo0 & R2K3K_ENTRYLO_G;
			v_bit = cached_lo0 & R2K3K_ENTRYLO_V;
			d_bit = cached_lo0 & R2K3K_ENTRYLO_D;
#else
			/*  R4000 or similar:  */
			pmask = cp0->tlbs[i].mask & pagemask_mask;
			cached_hi = cp0->tlbs[i].hi;
			cached_lo0 = cp0->tlbs[i].lo0;
			cached_lo1 = cp0->tlbs[i].lo1;

			/*  Optimized for minimum page size:  */
			if (pmask == 0) {
				pageshift = pagemask_shift - 1;
				entry_vpn2 = (cached_hi & vpn2_mask)
				    >> pagemask_shift;
				vaddr_vpn2 = (vaddr & vpn2_mask)
				    >> pagemask_shift;
				pmask = (1 << (pagemask_shift-1)) - 1;
				odd = (vaddr >> (pagemask_shift-1)) & 1;
			} else {
				/*  Non-standard page mask:  */
				switch (pmask | ((1 << pagemask_shift) - 1)) {
				case 0x00007ff:	pageshift = 10; break;
				case 0x0001fff:	pageshift = 12; break;
				case 0x0007fff:	pageshift = 14; break;
				case 0x001ffff:	pageshift = 16; break;
				case 0x007ffff:	pageshift = 18; break;
				case 0x01fffff:	pageshift = 20; break;
				case 0x07fffff:	pageshift = 22; break;
				case 0x1ffffff:	pageshift = 24; break;
				case 0x7ffffff:	pageshift = 26; break;
				default:fatal("pmask=%08"PRIx32"\n", pmask);
					exit(1);
				}

				entry_vpn2 = (cached_hi &
				    vpn2_mask) >> (pageshift + 1);
				vaddr_vpn2 = (vaddr & vpn2_mask) >>
				    (pageshift + 1);
				pmask = (1 << pageshift) - 1;
				odd = (vaddr >> pageshift) & 1;
			}

			/*  Assume even virtual page...  */
			v_bit = cached_lo0 & ENTRYLO_V;
			d_bit = cached_lo0 & ENTRYLO_D;

#ifdef V2P_MMU8K
			/*
			 *  TODO:  I don't really know anything about the R8000.
			 *  http://futuretech.mirror.vuurwerk.net/i2sec7.html
			 *  says that it has a three-way associative TLB with
			 *  384 entries, 16KB page size, and some other things.
			 *
			 *  It feels like things like the valid bit (ala R4000)
			 *  and dirty bit are not implemented the same on R8000.
			 *
			 *  http://sgistuff.tastensuppe.de/documents/
			 *		R8000_chipset.html
			 *  also has some info, but no details.
			 */
			v_bit = 1;	/*  Big TODO  */
			d_bit = 1;
#endif

			entry_asid = cached_hi & ENTRYHI_ASID;

			/*  ... reload pfn, v_bit, d_bit if
			    it was the odd virtual page:  */
			if (odd) {
				v_bit = cached_lo1 & ENTRYLO_V;
				d_bit = cached_lo1 & ENTRYLO_D;
			}
#ifdef V2P_MMU4100
			g_bit = cached_lo1 & cached_lo0 & ENTRYLO_G;
#else
			g_bit = cached_hi & TLB_G;
#endif

#endif

			/*  Is there a VPN and ASID match?  */
			if (entry_vpn2 == vaddr_vpn2 &&
			    (entry_asid == vaddr_asid || g_bit)) {
				/*  debug("OK MAP 1, i=%i { vaddr=%016"PRIx64" "
				    "==> paddr %016"PRIx64" v=%i d=%i "
				    "asid=0x%02x }\n", i, (uint64_t) vaddr,
				    (uint64_t) *return_paddr, v_bit?1:0,
				    d_bit?1:0, vaddr_asid);  */
				if (v_bit) {
					if (d_bit || (!d_bit &&
					    writeflag == MEM_READ)) {
						uint64_t paddr;
						/*  debug("OK MAP 2!!! { w=%i "
						    "vaddr=%016"PRIx64" ==> "
						    "d=%i v=%i paddr %016"
						    PRIx64" ",
						    writeflag, (uint64_t)vaddr,
						    d_bit?1:0, v_bit?1:0,
						    (uint64_t) *return_paddr);
						    debug(", tlb entry %2i: ma"
						    "sk=%016"PRIx64" hi=%016"
						    PRIx64" lo0=%016"PRIx64
						    " lo1=%016"PRIx64"\n",
						    i, cp0->tlbs[i].mask, cp0->
						    tlbs[i].hi, cp0->tlbs[i].
						    lo0, cp0->tlbs[i].lo1);
						*/
#ifdef V2P_MMU3K
						pfn = cached_lo0 &
						    R2K3K_ENTRYLO_PFN_MASK;
						paddr = pfn | (vaddr & pmask);
#else
						pfn = ((odd? cached_lo1 :
						    cached_lo0)
						    & ENTRYLO_PFN_MASK)
						    >> ENTRYLO_PFN_SHIFT;
						paddr = ((pfn << pfn_shift) &
						    ~pmask) | (vaddr & pmask);
#endif

						*return_paddr = paddr;
						return d_bit? 2 : 1;
					} else {
						/*  TLB modif. exception  */
						tlb_refill = 0;
						exccode = EXCEPTION_MOD;
						goto exception;
					}
				} else {
					/*  TLB invalid exception  */
					tlb_refill = 0;
					goto exception;
				}
			}

			if (i == i_end)
				break;

			/*  Go to the next TLB entry:  */
			i ++;
			if (i == n_tlbs)
				i = 0;
		}
	}

	/*
	 *  We are here if for example userland code tried to access
	 *  kernel memory, OR if there was a TLB refill.
	 */

	if (!use_tlb) {
		tlb_refill = 0;
		if (writeflag == MEM_WRITE)
			exccode = EXCEPTION_ADES;
		else
			exccode = EXCEPTION_ADEL;
	}

exception:
	if (no_exceptions)
		return 0;

	/*  TLB Load or Store exception:  */
	if (exccode == -1) {
		if (writeflag == MEM_WRITE)
			exccode = EXCEPTION_TLBS;
		else
			exccode = EXCEPTION_TLBL;
	}

#ifdef V2P_MMU3K
	vaddr_asid >>= R2K3K_ENTRYHI_ASID_SHIFT;
	vaddr_vpn2 >>= 12;
#endif

	mips_cpu_exception(cpu, exccode, tlb_refill, vaddr,
	    0, vaddr_vpn2, vaddr_asid, x_64);

	/*  Return failure:  */
	return 0;
}

