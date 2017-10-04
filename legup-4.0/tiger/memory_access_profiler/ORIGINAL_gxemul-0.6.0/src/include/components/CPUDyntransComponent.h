#ifndef CPUDYNTRANSCOMPONENT_H
#define	CPUDYNTRANSCOMPONENT_H

/*
 *  Copyright (C) 2008-2010  Anders Gavare.  All rights reserved.
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

// Note: Not included in the component registry.


#include "CPUComponent.h"
#include "UnitTest.h"

#include <assert.h>


class CPUDyntransComponent;


#define N_DYNTRANS_IC_ARGS	3
/**
 * \brief A dyntrans instruction call.
 *
 * f points to a function to be executed.
 * arg[] contains arguments, such as pointers to registers, or immediate values.
 */
struct DyntransIC
{
	void (*f)(CPUDyntransComponent*, DyntransIC*);

	union {
		void* p;
		uint32_t u32;
	} arg[N_DYNTRANS_IC_ARGS];
};

/*
 * A dyntrans page contains DyntransIC calls for each instruction slot, followed
 * by some special entries, which handle execution going over the end of a page
 * (by changing the PC to the start of the next virtual page).
 */
#define	DYNTRANS_PAGE_NSPECIALENTRIES	2


/*
 * Some helpers for implementing dyntrans instructions.
 */
#define DECLARE_DYNTRANS_INSTR(name) static void instr_##name(CPUDyntransComponent* cpubase, DyntransIC* ic);
#define DYNTRANS_INSTR(class,name) void class::instr_##name(CPUDyntransComponent* cpubase, DyntransIC* ic)
#define DYNTRANS_INSTR_HEAD(class)  class* cpu = (class*) cpubase;

#define REG32(arg)	(*((uint32_t*)((arg).p)))
#define REG64(arg)	(*((uint64_t*)((arg).p)))

#define DYNTRANS_SYNCH_PC	cpu->m_nextIC = ic; cpu->DyntransResyncPC()


/**
 * \brief A base-class for processors Component implementations that
 *	use dynamic translation.
 */
class CPUDyntransComponent
	: public CPUComponent
{
public:
	/**
	 * \brief Constructs a CPUDyntransComponent.
	 *
	 * @param className The class name for the component.
	 * @param cpuKind The CPU kind, e.g. "MIPS R4400" for a
	 *	MIPS R4400 processor.
	 */
	CPUDyntransComponent(const string& className, const string& cpuKind);

	virtual int Execute(GXemul* gxemul, int nrOfCycles);


	/********************************************************************/

	static void RunUnitTests(int& nSucceeded, int& nFailures);

protected:
	// Implemented by specific CPU families:
	virtual int GetDyntransICshift() const = 0;
	virtual void (*GetDyntransToBeTranslated())(CPUDyntransComponent* cpu, DyntransIC* ic) const = 0;

	void DyntransToBeTranslatedBegin(struct DyntransIC*);
	bool DyntransReadInstruction(uint16_t& iword);
	bool DyntransReadInstruction(uint32_t& iword);
	void DyntransToBeTranslatedDone(struct DyntransIC*);

	/**
	 * \brief Calculate m_pc based on m_nextIC and m_firstIConPage.
	 */
	void DyntransResyncPC();

	/**
	 * \brief Calculate m_nextIC and m_firstIConPage, based on m_pc.
	 *
	 * This function may return pointers to within an existing translation
	 * page (hopefully the most common case, since it is the fastest), or
	 * it may allocate a new empty page.
	 */
	void DyntransPCtoPointers();

private:
	void DyntransInit();
	struct DyntransIC* DyntransGetICPage(uint64_t addr);
	void DyntransClearICPage(struct DyntransIC* icpage);

protected:
	/*
	 * Generic dyntrans instruction implementations, that may be used by
	 * several different cpu architectures.
	 */
	DECLARE_DYNTRANS_INSTR(nop);
	DECLARE_DYNTRANS_INSTR(abort);
	DECLARE_DYNTRANS_INSTR(endOfPage);
	DECLARE_DYNTRANS_INSTR(endOfPage2);

	// Branches.
	DECLARE_DYNTRANS_INSTR(branch_samepage);

	// Data movement.
	DECLARE_DYNTRANS_INSTR(set_u64_imms32);
	DECLARE_DYNTRANS_INSTR(mov_u64_u64);

	// Arithmetic.
	DECLARE_DYNTRANS_INSTR(add_u32_u32_immu32);
	DECLARE_DYNTRANS_INSTR(add_u32_u32_u32);
	DECLARE_DYNTRANS_INSTR(add_u64_u64_imms32_truncS32);
	DECLARE_DYNTRANS_INSTR(add_u64_u64_u64_truncS32);
	DECLARE_DYNTRANS_INSTR(add_u64_u64_imms32);
	DECLARE_DYNTRANS_INSTR(sub_u32_u32_immu32);
	DECLARE_DYNTRANS_INSTR(sub_u32_u32_u32);
	DECLARE_DYNTRANS_INSTR(sub_u64_u64_u64_truncS32);

	// Logic.
	DECLARE_DYNTRANS_INSTR(and_u32_u32_immu32);
	DECLARE_DYNTRANS_INSTR(and_u64_u64_immu32);
	DECLARE_DYNTRANS_INSTR(or_u32_u32_immu32);
	DECLARE_DYNTRANS_INSTR(or_u32_u32_u32);
	DECLARE_DYNTRANS_INSTR(or_u64_u64_immu32);
	DECLARE_DYNTRANS_INSTR(xor_u32_u32_immu32);
	DECLARE_DYNTRANS_INSTR(xor_u32_u32_u32);
	DECLARE_DYNTRANS_INSTR(xor_u64_u64_immu32);
	DECLARE_DYNTRANS_INSTR(xor_u64_u64_u64);

	// Shifts, rotates.
	DECLARE_DYNTRANS_INSTR(shift_left_u64_u64_imm5_truncS32);
	DECLARE_DYNTRANS_INSTR(shift_right_u64_u64asu32_imm5_truncS32);

private:
	class DyntransTranslationPage
	{
	public:
		DyntransTranslationPage(int nICentriesPerpage)
			: m_prev(-1)
			, m_next(-1)
			, m_nextCacheEntryForAddr(-1)
			, m_addr(0)
			, m_showFunctionTraceCall(false)
		{
			m_ic.resize(nICentriesPerpage);
		}

	public:
		// Linked list: Freelist or MRU list
		int				m_prev;
		int				m_next;

		// Address match:
		int				m_nextCacheEntryForAddr;
		uint64_t			m_addr;

		// Flags for this page:
		// TODO: In the future, this could be stuff like different
		// instruction encodings (MIPS16/Thumb vs 32-bit encoding),
		// or other mode switches.
		bool				m_showFunctionTraceCall;

		// Translated instructions:
		vector< struct DyntransIC >	m_ic;
	};

	class DyntransTranslationCache
	{
	public:
		DyntransTranslationCache()
			: m_nICentriesPerpage(0)
			, m_pageShift(0)
			, m_firstFree(-1)
			, m_lastFree(-1)
			, m_firstMRU(-1)
			, m_lastMRU(-1)
		{
		}

		void Reinit(size_t approximateSize, int nICentriesPerpage, int pageShift)
		{
			size_t approximateSizePerPage = sizeof(struct DyntransIC) * nICentriesPerpage + 64;
			size_t nrOfPages = approximateSize / approximateSizePerPage;

			if (nrOfPages < 2) {
				std::cerr << "Too small translation cache!\n";
				throw std::exception();
			}

			if (nICentriesPerpage == m_nICentriesPerpage &&
			    nrOfPages == m_pageCache.size() &&
			    pageShift == m_pageShift)
				return;

			m_nICentriesPerpage = nICentriesPerpage;
			m_pageShift = pageShift;

			// Generate empty pages:
			m_pageCache.clear();
			m_pageCache.resize(nrOfPages, DyntransTranslationPage(nICentriesPerpage));

			// Set up the free-list to connect all pages:
			m_firstFree = 0;
			m_lastFree = nrOfPages - 1;
			for (int i=m_firstFree; i<=m_lastFree; i++) {
				m_pageCache[i].m_prev = i-1;	// note: i=0 (first page)
								// results in prev = -1.

				if (i == m_lastFree)
					m_pageCache[i].m_next = -1;
				else
					m_pageCache[i].m_next = i+1;
			}

			// No pages in use yet, so nothing on the MRU list:
			m_firstMRU = m_lastMRU = -1;

			// Reset the quick lookup table:
			// (Currently hardcoded to allow the first 1 GB of emulated physical memory
			// to be looked up with 1 call, if more than 1 GB of RAM is used, the
			// m_nextCacheEntryForAddr chain must be traversed...)
			m_addrToFirstPageIndex.resize(1024 * 1048576 >> m_pageShift);
			for (size_t i=0; i<m_addrToFirstPageIndex.size(); ++i)
				m_addrToFirstPageIndex[i] = -1;

			ValidateConsistency();
		}

		void ValidateConsistency()
		{
			vector<bool> pageIsInMRUList;
			vector<bool> pageIsInFreeList;
			vector<bool> pageIsInMRUListReverse;
			vector<bool> pageIsInFreeListReverse;

			pageIsInMRUList.resize(m_pageCache.size(), false);
			pageIsInFreeList.resize(m_pageCache.size(), false);
			pageIsInMRUListReverse.resize(m_pageCache.size(), false);
			pageIsInFreeListReverse.resize(m_pageCache.size(), false);

			// Follow free-list forward:
			int i = m_firstFree;
			while (i >= 0) {
				pageIsInFreeList[i] = true;
				i = m_pageCache[i].m_next;
			}

			// Follow free-list backwards:
			i = m_lastFree;
			while (i >= 0) {
				pageIsInFreeListReverse[i] = true;
				i = m_pageCache[i].m_prev;
			}

			// Follow MRU-list forward:
			i = m_firstMRU;
			while (i >= 0) {
				pageIsInMRUList[i] = true;
				i = m_pageCache[i].m_next;
			}

			// Follow MRU-list backwards:
			i = m_lastMRU;
			while (i >= 0) {
				pageIsInMRUListReverse[i] = true;
				i = m_pageCache[i].m_prev;
			}

			// The forward and reverse sets should match:
			for (size_t j=0; j<m_pageCache.size(); ++j) {
				if (pageIsInFreeList[j] != pageIsInFreeListReverse[j]) {
					std::cerr << "Forward and reverse Free-list iteration mismatch, position " << j << "!\n";
					throw std::exception();
				}

				if (pageIsInMRUList[j] != pageIsInMRUListReverse[j]) {
					std::cerr << "Forward and reverse MRU-list iteration mismatch, position " << j << "!\n";
					throw std::exception();
				}

				if ((pageIsInMRUList[j] ^ pageIsInFreeList[j]) == false) {
					std::cerr << "Each page should be in exactly ONE of the two lists, position " << j << "!\n";
					throw std::exception();
				}
			}

			vector<bool> pageIsPointedToByQuickLookupTable;
			vector<bool> pageIsPointedToByQLTChain;
			pageIsPointedToByQuickLookupTable.resize(m_pageCache.size(), false);
			pageIsPointedToByQLTChain.resize(m_pageCache.size(), false);

			for (size_t k=0; k<m_addrToFirstPageIndex.size(); ++k)
				if (m_addrToFirstPageIndex[k] >= 0)
					pageIsPointedToByQuickLookupTable[m_addrToFirstPageIndex[k]] = true;

			for (size_t k=0; k<m_pageCache.size(); ++k) {
				int index = m_pageCache[k].m_nextCacheEntryForAddr;
				if (index >= 0)
					pageIsPointedToByQLTChain[index] = true;
			}

			for (size_t k=0; k<pageIsInFreeList.size(); ++k) {
				if (!pageIsInFreeList[k])
					continue;

				if (m_pageCache[k].m_nextCacheEntryForAddr >= 0) {
					std::cerr << "Pages on the free-list should not have m_nextCacheEntryForAddr set!\n";
					throw std::exception();
				}

				if (pageIsPointedToByQuickLookupTable[k]) {
					std::cerr << "Pages on the free-list should not be pointed to by the quick lookup table!\n";
					throw std::exception();
				}

				if (pageIsPointedToByQLTChain[k]) {
					std::cerr << "Pages on the free-list should not be in the quick lookup table chain!\n";
					throw std::exception();
				}
			}

			for (size_t k=0; k<pageIsInMRUList.size(); ++k) {
				if (!pageIsInMRUList[k])
					continue;

				uint64_t addr = m_pageCache[k].m_addr;
				uint64_t physPageNumber = addr >> m_pageShift;
				int quickLookupIndex = physPageNumber & (m_addrToFirstPageIndex.size() - 1);
				int pageIndex = m_addrToFirstPageIndex[quickLookupIndex];

				while (pageIndex >= 0) {
					if (m_pageCache[pageIndex].m_addr == addr)
						break;

					pageIndex = m_pageCache[pageIndex].m_nextCacheEntryForAddr;
				}

				if (pageIndex < 0) {
					std::cerr << "Pages in the MRU list must be reachable from the quick lookup table!\n";
					throw std::exception();
				}
			}
		}

		void FreeLeastRecentlyUsedPage()
		{
			// This function should only be called if it is really necessary to
			// free a page, i.e. if there is no free page at all.
			assert(m_firstFree < 0);

			if (m_firstMRU == m_lastMRU) {
				std::cerr << "Attempt to free a page, but there's only one page in the MRU list. Too small!\n";
				throw std::exception();
			}

			// This is the one we will free.
			int index = m_lastMRU;
			assert(m_pageCache[index].m_prev >= 0);
			assert(m_pageCache[index].m_next < 0);

			// Disconnect it from the MRU list...
			m_lastMRU = m_pageCache[index].m_prev;
			m_pageCache[m_lastMRU].m_next = -1;

			// ... and add it first in the free-list:
			if (m_firstFree < 0) {
				// In fact, the free-list was empty.
				m_firstFree = m_lastFree = index;
				m_pageCache[index].m_prev = -1;
				m_pageCache[index].m_next = -1;
			} else {
				m_pageCache[index].m_prev = -1;
				m_pageCache[index].m_next = m_firstFree;
				m_pageCache[m_firstFree].m_prev = index;
				m_firstFree = index;
			}

			// Remove from the quick lookup chain:
			uint64_t physPageNumber = m_pageCache[index].m_addr >> m_pageShift;
			int quickLookupIndex = physPageNumber & (m_addrToFirstPageIndex.size() - 1);
			int pageIndex = m_addrToFirstPageIndex[quickLookupIndex];
			if (pageIndex == index) {
				// Direct hit? Then remove from the base quick look up table...
				m_addrToFirstPageIndex[quickLookupIndex] = m_pageCache[index].m_nextCacheEntryForAddr;
			} else {
				// ... otherwise traverse the chain until the next entry is the one we are removing.
				while (true) {
					if (m_pageCache[pageIndex].m_nextCacheEntryForAddr == index) {
						m_pageCache[pageIndex].m_nextCacheEntryForAddr = m_pageCache[index].m_nextCacheEntryForAddr;
						break;
					}

					pageIndex = m_pageCache[pageIndex].m_nextCacheEntryForAddr;
				}
			}

			m_pageCache[index].m_nextCacheEntryForAddr = -1;

			ValidateConsistency();
		}

		struct DyntransIC *AllocateNewPage(uint64_t addr, bool showFunctionTraceCall)
		{
			int index = m_firstFree;
			assert(index >= 0);

			// Let's grab this index for ourselves...

			// Was this the only free page?
			if (index == m_lastFree) {
				// Then there will be no more free pages.
				m_firstFree = m_lastFree = -1;
			} else {
				// No, just update the chain:
				m_firstFree = m_pageCache[index].m_next;
				m_pageCache[m_firstFree].m_prev = -1;
			}

			// The page's prev and next fields should be updated to
			// be part of the MRU list now.

			// Is there nothing in the MRU list?
			if (m_firstMRU == -1) {
				// Then we are the only one.
				m_firstMRU = m_lastMRU = index;
				m_pageCache[index].m_next = m_pageCache[index].m_prev = -1;
			} else {
				// Then we place ourselves first:
				m_pageCache[m_firstMRU].m_prev = index;
				m_pageCache[index].m_next = m_firstMRU;
				m_firstMRU = index;
				m_pageCache[index].m_prev = -1;
			}

			// Set attributes for the page: address and other flags.
			m_pageCache[index].m_addr = addr;
			m_pageCache[index].m_showFunctionTraceCall = showFunctionTraceCall;

			// Insert into quick lookup table:
			uint64_t physPageNumber = addr >> m_pageShift;
			int quickLookupIndex = physPageNumber & (m_addrToFirstPageIndex.size() - 1);

			// Are we the only one? (I.e. the first page for this quick lookup index.)
			if (m_addrToFirstPageIndex[quickLookupIndex] < 0) {
				m_addrToFirstPageIndex[quickLookupIndex] = index;
				m_pageCache[index].m_nextCacheEntryForAddr = -1;
			} else {
				// No. Let's add ourselves first in the chain:
				m_pageCache[index].m_nextCacheEntryForAddr = m_addrToFirstPageIndex[quickLookupIndex];
				m_addrToFirstPageIndex[quickLookupIndex] = index;
			}

			ValidateConsistency();

			return &(m_pageCache[index].m_ic[0]);
		}

		struct DyntransIC *GetICPage(uint64_t addr, bool showFunctionTraceCall, bool& clear)
		{
			clear = false;

			// Strip of the low bits:
			addr >>= m_pageShift;
			uint64_t physPageNumber = addr;
			addr <<= m_pageShift;

			int quickLookupIndex = physPageNumber & (m_addrToFirstPageIndex.size() - 1);
			int pageIndex = m_addrToFirstPageIndex[quickLookupIndex];

			// std::cerr << "addr " << addr << ", physPageNumber " << physPageNumber << ", quickLookupIndex " << quickLookupIndex << ", pageIndex " << pageIndex << "\n";

			// If pageIndex >= 0, then pageIndex points to a page which _may_ be for this addr.
			while (pageIndex >= 0) {
				if (m_pageCache[pageIndex].m_addr == addr)
					break;

				// If the page for pageIndex was for some other address, then
				// let's continue searching the chain...
				pageIndex = m_pageCache[pageIndex].m_nextCacheEntryForAddr;
			}

			// If we have a definite page index, then return a pointer that page's ICs:
			if (pageIndex >= 0) {
				// Update the MRU list, unless this page was already first,
				// so that m_firstMRU points to the page.
				if (m_firstMRU != pageIndex) {
					// Disconnect from previous place...
					int prev = m_pageCache[pageIndex].m_prev;
					int next = m_pageCache[pageIndex].m_next;
					if (prev >= 0)
						m_pageCache[prev].m_next = next;
					if (next >= 0)
						m_pageCache[next].m_prev = prev;

					// ... disconnect from "end of list":
					if (pageIndex == m_lastMRU)
						m_lastMRU = prev;

					// ... and insert first in list:
					m_pageCache[pageIndex].m_prev = -1;
					m_pageCache[pageIndex].m_next = m_firstMRU;
					m_pageCache[m_firstMRU].m_prev = pageIndex;
					m_firstMRU = pageIndex;
				}

				// TODO: Hm... also move to front of the Quick Lookup chain?
				// Only necessary if more memory is used/emulated than the
				// size of the quick lookup table, e.g. if 2 GB ram are used,
				// and the size of the lookup table is 1 GB, and pages
				// at exactly 0 GB and 1 GB are used in this order:
				// 1 0 1 1 1 1 1 1 1 1 1 1 1
				// then 1 would first be placed in the chain, then 0 (which
				// would insert it first in the chain). But all lookups after
				// that would have to search the whole chain (0, 1) to find 1.

				ValidateConsistency();

				// If flags are not the same, then let's clear the page:
				if (m_pageCache[pageIndex].m_showFunctionTraceCall != showFunctionTraceCall) {
					m_pageCache[pageIndex].m_showFunctionTraceCall = showFunctionTraceCall;
					clear = true;
				}

				return &(m_pageCache[pageIndex].m_ic[0]);
			}

			// The address was NOT in the translation cache at all. So we have
			// to create a new page.

			// If the free-list is all used up, that means we have to free something
			// before we can allocate a new page...
			if (m_firstFree < 0)
				FreeLeastRecentlyUsedPage();

			// ... and then finally allocate a new page:
			clear = true;
			return AllocateNewPage(addr, showFunctionTraceCall);
		}

	private:
		// Number of translated instructions per page, and number of bits
		// to shift to convert address to page number:
		int				m_nICentriesPerpage;
		int				m_pageShift;

		// Free-list of pages:
		int				m_firstFree;
		int				m_lastFree;

		// Most-Recently-Used list of pages:
		int				m_firstMRU;
		int				m_lastMRU;

		// "Usually" quick lookup table, address to page index:
		vector<int>			m_addrToFirstPageIndex;

		// The actual pages:
		vector<DyntransTranslationPage>	m_pageCache;
	};

protected:
	/*
	 * Volatile state:
	 */
	struct DyntransIC *	m_firstIConPage;
	struct DyntransIC *	m_nextIC;
	int			m_dyntransPageMask;
	int			m_dyntransICentriesPerPage;
	int			m_dyntransICshift;
	int			m_executedCycles;
	int			m_nrOfCyclesToExecute;

	/*
	 * Translation cache:
	 */
	DyntransTranslationCache	m_translationCache;

	/*
	 * Special always present DyntransIC structs, for aborting emulation:
	 */
	struct DyntransIC	m_abortIC;
};


#endif	// CPUDYNTRANSCOMPONENT_H
