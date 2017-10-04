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
 *
 *
 *  MIPS-specific memory routines. Included from cpu_mips.c.
 */

#include <sys/types.h>
#include <sys/mman.h>


/*
 *  memory_cache_R3000():
 *
 *  R2000/R3000 specific cache handling.
 *
 *  Return value is 1 if a jump to do_return_ok is supposed to happen directly
 *  after this routine is finished, 0 otherwise.
 */
int memory_cache_R3000(struct cpu *cpu, int cache, uint64_t paddr,
	int writeflag, size_t len, unsigned char *data)
{
	unsigned int i;
	int cache_isolated = 0, addr, hit, which_cache = cache;


	if (len > 4 || cache == CACHE_NONE)
		return 0;


	/*
	 *  R2000/R3000 without correct cache emulation:
	 *
	 *  TODO: This is just enough to trick NetBSD/pmax and Ultrix into
	 *  being able to detect the cache sizes and think that the caches
	 *  are actually working, but they are not.
	 */

	if (cache != CACHE_DATA)
		return 0;

	/*  Is this a cache hit or miss?  */
	hit = (cpu->cd.mips.cache_last_paddr[which_cache]
		& ~cpu->cd.mips.cache_mask[which_cache])
	    == (paddr & ~(cpu->cd.mips.cache_mask[which_cache]));

	/*
	 *  The cache miss bit is only set on cache reads, and only to the
	 *  data cache. (?)
	 *
	 *  (TODO: is this correct? I don't remember where I got this from.)
	 */
	if (cache == CACHE_DATA && writeflag==MEM_READ) {
		cpu->cd.mips.coproc[0]->reg[COP0_STATUS] &= ~MIPS1_CACHE_MISS;
		if (!hit)
			cpu->cd.mips.coproc[0]->reg[COP0_STATUS] |=
			    MIPS1_CACHE_MISS;
	}

	/*
	 *  Is the Data cache isolated?  Then don't access main memory:
	 */
	if (cache == CACHE_DATA &&
	    cpu->cd.mips.coproc[0]->reg[COP0_STATUS] & MIPS1_ISOL_CACHES)
		cache_isolated = 1;

	addr = paddr & cpu->cd.mips.cache_mask[which_cache];

	/*  Data cache isolated?  Then don't access main memory:  */
	if (cache_isolated) {
		/*  debug("ISOLATED write=%i cache=%i vaddr=%016"PRIx64" "
		    "paddr=%016"PRIx64" => addr in cache = 0x%lx\n",
		    writeflag, cache, (uint64_t) vaddr,
		    (uint64_t) paddr, addr);  */

		if (writeflag==MEM_READ) {
			for (i=0; i<len; i++)
				data[i] = cpu->cd.mips.cache[cache][(addr+i) &
				    cpu->cd.mips.cache_mask[cache]];
		} else {
			for (i=0; i<len; i++)
				cpu->cd.mips.cache[cache][(addr+i) &
				    cpu->cd.mips.cache_mask[cache]] = data[i];
		}
		return 1;
	} else {
		/*  Reload caches if necessary:  */

		/*  No!  Not when not emulating caches fully. (TODO?)  */
		cpu->cd.mips.cache_last_paddr[cache] = paddr;
	}

	return 0;
}


#define TRANSLATE_ADDRESS	translate_v2p_mmu3k
#define	V2P_MMU3K
#include "memory_mips_v2p.cc"
#undef TRANSLATE_ADDRESS
#undef V2P_MMU3K

#define TRANSLATE_ADDRESS	translate_v2p_mmu8k
#define	V2P_MMU8K
#include "memory_mips_v2p.cc"
#undef TRANSLATE_ADDRESS
#undef V2P_MMU8K

#define TRANSLATE_ADDRESS	translate_v2p_mmu10k
#define	V2P_MMU10K
#include "memory_mips_v2p.cc"
#undef TRANSLATE_ADDRESS
#undef V2P_MMU10K

/*  Almost generic  :-)  */
#define TRANSLATE_ADDRESS	translate_v2p_mmu4100
#define	V2P_MMU4100
#include "memory_mips_v2p.cc"
#undef TRANSLATE_ADDRESS
#undef V2P_MMU4100

#define TRANSLATE_ADDRESS	translate_v2p_generic
#include "memory_mips_v2p.cc"


