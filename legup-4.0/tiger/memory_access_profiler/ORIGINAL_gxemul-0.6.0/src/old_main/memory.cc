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
 *  Functions for handling the memory of an emulated machine.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/mman.h>

#include "cpu.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"


extern int verbose;
extern int quiet_mode;


/*
 *  memory_readmax64():
 *
 *  Read at most 64 bits of data from a buffer.  Length is given by
 *  len, and the byte order by cpu->byte_order.
 *
 *  This function should not be called with cpu == NULL.
 */
uint64_t memory_readmax64(struct cpu *cpu, unsigned char *buf, int len)
{
	int i, byte_order = cpu->byte_order;
	uint64_t x = 0;

	if (len & MEM_PCI_LITTLE_ENDIAN) {
		len &= ~MEM_PCI_LITTLE_ENDIAN;
		byte_order = EMUL_LITTLE_ENDIAN;
	}

	/*  Switch byte order for incoming data, if necessary:  */
	if (byte_order == EMUL_BIG_ENDIAN)
		for (i=0; i<len; i++) {
			x <<= 8;
			x |= buf[i];
		}
	else
		for (i=len-1; i>=0; i--) {
			x <<= 8;
			x |= buf[i];
		}

	return x;
}


/*
 *  memory_writemax64():
 *
 *  Write at most 64 bits of data to a buffer.  Length is given by
 *  len, and the byte order by cpu->byte_order.
 *
 *  This function should not be called with cpu == NULL.
 */
void memory_writemax64(struct cpu *cpu, unsigned char *buf, int len,
	uint64_t data)
{
	int i, byte_order = cpu->byte_order;

	if (len & MEM_PCI_LITTLE_ENDIAN) {
		len &= ~MEM_PCI_LITTLE_ENDIAN;
		byte_order = EMUL_LITTLE_ENDIAN;
	}

	if (byte_order == EMUL_LITTLE_ENDIAN)
		for (i=0; i<len; i++) {
			buf[i] = data & 255;
			data >>= 8;
		}
	else
		for (i=0; i<len; i++) {
			buf[len - 1 - i] = data & 255;
			data >>= 8;
		}
}


/*
 *  zeroed_alloc():
 *
 *  Allocates a block of memory using mmap(), and if that fails, try
 *  malloc() + memset(). The returned memory block contains only zeroes.
 */
void *zeroed_alloc(size_t s)
{
	void *p = mmap(NULL, s, PROT_READ | PROT_WRITE,
	    MAP_ANON | MAP_PRIVATE, -1, 0);

	if (p == NULL) {
#if 1
		fprintf(stderr, "zeroed_alloc(): mmap() failed. This should"
		    " not usually happen. If you can reproduce this, then"
		    " please contact me with details about your run-time"
		    " environment.\n");
		exit(1);
#else
		CHECK_ALLOCATION(p = malloc(s));
		memset(p, 0, s);
#endif
	}

	return p;
}


/*
 *  memory_new():
 *
 *  This function creates a new memory object. An emulated machine needs one
 *  of these.
 */
struct memory *memory_new(uint64_t physical_max, int arch)
{
	struct memory *mem;
	int bits_per_pagetable = BITS_PER_PAGETABLE;
	int bits_per_memblock = BITS_PER_MEMBLOCK;
	int entries_per_pagetable = 1 << BITS_PER_PAGETABLE;
	int max_bits = MAX_BITS;
	size_t s;

	CHECK_ALLOCATION(mem = (struct memory *) malloc(sizeof(struct memory)));
	memset(mem, 0, sizeof(struct memory));

	/*  Check bits_per_pagetable and bits_per_memblock for sanity:  */
	if (bits_per_pagetable + bits_per_memblock != max_bits) {
		fprintf(stderr, "memory_new(): bits_per_pagetable and "
		    "bits_per_memblock mismatch\n");
		exit(1);
	}

	mem->physical_max = physical_max;
	mem->dev_dyntrans_alignment = 4095;

	s = entries_per_pagetable * sizeof(void *);

	mem->pagetable = (unsigned char *) mmap(NULL, s,
	    PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
	if (mem->pagetable == NULL) {
		CHECK_ALLOCATION(mem->pagetable = malloc(s));
		memset(mem->pagetable, 0, s);
	}

	mem->mmap_dev_minaddr = 0xffffffffffffffffULL;
	mem->mmap_dev_maxaddr = 0;

	return mem;
}


/*
 *  memory_points_to_string():
 *
 *  Returns 1 if there's something string-like in emulated memory at address
 *  addr, otherwise 0.
 */
int memory_points_to_string(struct cpu *cpu, struct memory *mem, uint64_t addr,
	int min_string_length)
{
	int cur_length = 0;
	unsigned char c;

	for (;;) {
		c = '\0';
		cpu->memory_rw(cpu, mem, addr+cur_length,
		    &c, sizeof(c), MEM_READ, CACHE_NONE | NO_EXCEPTIONS);
		if (c=='\n' || c=='\t' || c=='\r' || (c>=' ' && c<127)) {
			cur_length ++;
			if (cur_length >= min_string_length)
				return 1;
		} else {
			if (cur_length >= min_string_length)
				return 1;
			else
				return 0;
		}
	}
}


/*
 *  memory_conv_to_string():
 *
 *  Convert emulated memory contents to a string, placing it in a buffer
 *  provided by the caller.
 */
char *memory_conv_to_string(struct cpu *cpu, struct memory *mem, uint64_t addr,
	char *buf, int bufsize)
{
	int len = 0;
	int output_index = 0;
	unsigned char c, p='\0';

	while (output_index < bufsize-1) {
		c = '\0';
		cpu->memory_rw(cpu, mem, addr+len, &c, sizeof(c), MEM_READ,
		    CACHE_NONE | NO_EXCEPTIONS);
		buf[output_index] = c;
		if (c>=' ' && c<127) {
			len ++;
			output_index ++;
		} else if (c=='\n' || c=='\r' || c=='\t') {
			len ++;
			buf[output_index] = '\\';
			output_index ++;
			switch (c) {
			case '\n':	p = 'n'; break;
			case '\r':	p = 'r'; break;
			case '\t':	p = 't'; break;
			}
			if (output_index < bufsize-1) {
				buf[output_index] = p;
				output_index ++;
			}
		} else {
			buf[output_index] = '\0';
			return buf;
		}
	}

	buf[bufsize-1] = '\0';
	return buf;
}


/*
 *  memory_device_dyntrans_access():
 *
 *  Get the lowest and highest dyntrans access since last time.
 */
void memory_device_dyntrans_access(struct cpu *cpu, struct memory *mem,
	void *extra, uint64_t *low, uint64_t *high)
{
	size_t s;
	int i, need_inval = 0;

	/*  TODO: This is O(n), so it might be good to rewrite it some day.
	    For now, it will be enough, as long as this function is not
	    called too often.  */

	for (i=0; i<mem->n_mmapped_devices; i++) {
		if (mem->devices[i].extra == extra &&
		    mem->devices[i].flags & DM_DYNTRANS_WRITE_OK &&
		    mem->devices[i].dyntrans_data != NULL) {
			if (mem->devices[i].dyntrans_write_low != (uint64_t) -1)
				need_inval = 1;
			if (low != NULL)
				*low = mem->devices[i].dyntrans_write_low;
			mem->devices[i].dyntrans_write_low = (uint64_t) -1;

			if (high != NULL)
				*high = mem->devices[i].dyntrans_write_high;
			mem->devices[i].dyntrans_write_high = 0;

			if (!need_inval)
				return;

			/*  Invalidate any pages of this device that might
			    be in the dyntrans load/store cache, by marking
			    the pages read-only.  */
			if (cpu->invalidate_translation_caches != NULL) {
				for (s = *low; s <= *high;
				    s += cpu->machine->arch_pagesize)
					cpu->invalidate_translation_caches
					    (cpu, mem->devices[i].baseaddr + s,
					    JUST_MARK_AS_NON_WRITABLE
					    | INVALIDATE_PADDR);
			}

			return;
		}
	}
}


/*
 *  memory_device_update_data():
 *
 *  Update a device' dyntrans data pointer.
 *
 *  SUPER-IMPORTANT NOTE: Anyone who changes a dyntrans data pointer while
 *  things are running also needs to invalidate all CPUs' address translation
 *  caches!  Otherwise, these may contain old pointers to the old data.
 */
void memory_device_update_data(struct memory *mem, void *extra,
	unsigned char *data)
{
	int i;

	for (i=0; i<mem->n_mmapped_devices; i++) {
		if (mem->devices[i].extra != extra)
			continue;

		mem->devices[i].dyntrans_data = data;
		mem->devices[i].dyntrans_write_low = (uint64_t)-1;
		mem->devices[i].dyntrans_write_high = 0;
	}
}


/*
 *  memory_device_register():
 *
 *  Register a memory mapped device.
 */
void memory_device_register(struct memory *mem, const char *device_name,
	uint64_t baseaddr, uint64_t len,
	int (*f)(struct cpu *,struct memory *,uint64_t,unsigned char *,
		size_t,int,void *),
	void *extra, int flags, unsigned char *dyntrans_data)
{
	int i, newi = 0;

	/*
	 *  Figure out at which index to insert this device, and simultaneously
	 *  check for collisions:
	 */
	newi = -1;
	for (i=0; i<mem->n_mmapped_devices; i++) {
		if (i == 0 && baseaddr + len <= mem->devices[i].baseaddr)
			newi = i;
		if (i > 0 && baseaddr + len <= mem->devices[i].baseaddr &&
		    baseaddr >= mem->devices[i-1].endaddr)
			newi = i;
		if (i == mem->n_mmapped_devices - 1 &&
		    baseaddr >= mem->devices[i].endaddr)
			newi = i + 1;

		/*  If this is not colliding with device i, then continue:  */
		if (baseaddr + len <= mem->devices[i].baseaddr)
			continue;
		if (baseaddr >= mem->devices[i].endaddr)
			continue;

		fatal("\nERROR! \"%s\" collides with device %i (\"%s\")!\n",
		    device_name, i, mem->devices[i].name);
		exit(1);
	}
	if (mem->n_mmapped_devices == 0)
		newi = 0;
	if (newi == -1) {
		fatal("INTERNAL ERROR\n");
		exit(1);
	}

	if (verbose >= 2) {
		/*  (40 bits of physical address is displayed)  */
		debug("device at 0x%010"PRIx64": %s", (uint64_t) baseaddr,
		    device_name);

		if (flags & (DM_DYNTRANS_OK | DM_DYNTRANS_WRITE_OK)
		    && (baseaddr & mem->dev_dyntrans_alignment) != 0) {
			fatal("\nWARNING: Device dyntrans access, but unaligned"
			    " baseaddr 0x%"PRIx64".\n", (uint64_t) baseaddr);
		}

		if (flags & (DM_DYNTRANS_OK | DM_DYNTRANS_WRITE_OK)) {
			debug(" (dyntrans %s)",
			    (flags & DM_DYNTRANS_WRITE_OK)? "R/W" : "R");
		}
		debug("\n");
	}

	for (i=0; i<mem->n_mmapped_devices; i++) {
		if (dyntrans_data == mem->devices[i].dyntrans_data &&
		    mem->devices[i].flags&(DM_DYNTRANS_OK|DM_DYNTRANS_WRITE_OK)
		    && flags & (DM_DYNTRANS_OK | DM_DYNTRANS_WRITE_OK)) {
			fatal("ERROR: the data pointer used for dyntrans "
			    "accesses must only be used once!\n");
			fatal("(%p cannot be used by '%s'; already in use by '"
			    "%s')\n", dyntrans_data, device_name,
			    mem->devices[i].name);
			exit(1);
		}
	}

	mem->n_mmapped_devices++;

	CHECK_ALLOCATION(mem->devices = (struct memory_device *) realloc(mem->devices,
	    sizeof(struct memory_device) * mem->n_mmapped_devices));

	/*  Make space for the new entry:  */
	if (newi + 1 != mem->n_mmapped_devices)
		memmove(&mem->devices[newi+1], &mem->devices[newi],
		    sizeof(struct memory_device)
		    * (mem->n_mmapped_devices - newi - 1));

	CHECK_ALLOCATION(mem->devices[newi].name = strdup(device_name));
	mem->devices[newi].baseaddr = baseaddr;
	mem->devices[newi].endaddr = baseaddr + len;
	mem->devices[newi].length = len;
	mem->devices[newi].flags = flags;
	mem->devices[newi].dyntrans_data = dyntrans_data;

	if (flags & (DM_DYNTRANS_OK | DM_DYNTRANS_WRITE_OK)
	    && !(flags & DM_EMULATED_RAM) && dyntrans_data == NULL) {
		fatal("\nERROR: Device dyntrans access, but dyntrans_data"
		    " = NULL!\n");
		exit(1);
	}

	if ((size_t)dyntrans_data & (sizeof(void *) - 1)) {
		fprintf(stderr, "memory_device_register():"
		    " dyntrans_data not aligned correctly (%p)\n",
		    dyntrans_data);
		abort();
	}

	mem->devices[newi].dyntrans_write_low = (uint64_t)-1;
	mem->devices[newi].dyntrans_write_high = 0;
	mem->devices[newi].f = f;
	mem->devices[newi].extra = extra;

	if (baseaddr < mem->mmap_dev_minaddr)
		mem->mmap_dev_minaddr = baseaddr & ~mem->dev_dyntrans_alignment;
	if (baseaddr + len > mem->mmap_dev_maxaddr)
		mem->mmap_dev_maxaddr = (((baseaddr + len) - 1) |
		    mem->dev_dyntrans_alignment) + 1;

	if (newi < mem->last_accessed_device)
		mem->last_accessed_device ++;
}


/*
 *  memory_device_remove():
 *
 *  Unregister a memory mapped device from a memory object.
 */
void memory_device_remove(struct memory *mem, int i)
{
	if (i < 0 || i >= mem->n_mmapped_devices) {
		fatal("memory_device_remove(): invalid device number %i\n", i);
		exit(1);
	}

	mem->n_mmapped_devices --;

	if (i == mem->n_mmapped_devices)
		return;

	memmove(&mem->devices[i], &mem->devices[i+1],
	    sizeof(struct memory_device) * (mem->n_mmapped_devices - i));

	if (i <= mem->last_accessed_device)
		mem->last_accessed_device --;
	if (mem->last_accessed_device < 0)
		mem->last_accessed_device = 0;
}


/*
 *  memory_paddr_to_hostaddr():
 *
 *  Translate a physical address into a host address. The usual way to call
 *  this function is to make sure that paddr is page aligned, which will result
 *  in the host _page_ corresponding to that address.
 *
 *  Return value is a pointer to the address in the host, or NULL on failure.
 *  On reads, a NULL return value should be interpreted as reading all zeroes.
 */
unsigned char *memory_paddr_to_hostaddr(struct memory *mem,
	uint64_t paddr, int writeflag)
{
	void **table;
	int entry;
	const int mask = (1 << BITS_PER_PAGETABLE) - 1;
	const int shrcount = MAX_BITS - BITS_PER_PAGETABLE;
	unsigned char *hostptr;

	table = (void **) mem->pagetable;
	entry = (paddr >> shrcount) & mask;

	/*  printf("memory_paddr_to_hostaddr(): p=%16"PRIx64
	    " w=%i => entry=0x%x\n", (uint64_t) paddr, writeflag, entry);  */

	if (table[entry] == NULL) {
		size_t alloclen;

		/*
		 *  Special case:  reading from a nonexistant memblock
		 *  returns all zeroes, and doesn't allocate anything.
		 *  (If any intermediate pagetable is nonexistant, then
		 *  the same thing happens):
		 */
		if (writeflag == MEM_READ)
			return NULL;

		/*  Allocate a memblock:  */
		alloclen = 1 << BITS_PER_MEMBLOCK;

		/*  printf("  allocating for entry %i, len=%i\n",
		    entry, alloclen);  */

		/*  Anonymous mmap() should return zero-filled memory,
		    try malloc + memset if mmap failed.  */
		table[entry] = (void *) mmap(NULL, alloclen,
		    PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
		if (table[entry] == NULL) {
			CHECK_ALLOCATION(table[entry] = malloc(alloclen));
			memset(table[entry], 0, alloclen);
		}
	}

	hostptr = (unsigned char *) table[entry];

	if (hostptr != NULL)
		hostptr += (paddr & ((1 << BITS_PER_MEMBLOCK) - 1));

	return hostptr;
}


#define	UPDATE_CHECKSUM(value) {					\
		internal_state -= 0x118c7771c0c0a77fULL;		\
		internal_state = ((internal_state + (value)) << 7) ^	\
		    (checksum >> 11) ^ ((checksum - (value)) << 3) ^	\
		    (internal_state - checksum) ^ ((value) - internal_state); \
		checksum ^= internal_state;				\
	}


/*
 *  memory_checksum():
 *
 *  Calculate a 64-bit checksum of everything in a struct memory. This is
 *  useful for tracking down bugs; an old (presumably working) version of
 *  the emulator can be compared to a newer (buggy) version.
 */
uint64_t memory_checksum(struct memory *mem)
{
	uint64_t internal_state = 0x80624185376feff2ULL;
	uint64_t checksum = 0xcb9a87d5c010072cULL;
	const size_t n_entries = (1 << BITS_PER_PAGETABLE) - 1;
	const size_t len = (1 << BITS_PER_MEMBLOCK) / sizeof(uint64_t);
	size_t entry, i;

	for (entry=0; entry<=n_entries; entry++) {
		uint64_t **table = (uint64_t **) mem->pagetable;
		uint64_t *memblock = table[entry];

		if (memblock == NULL) {
			UPDATE_CHECKSUM(0x1198ab7c8174a76fULL);
			continue;
		}

		for (i=0; i<len; i++)
			UPDATE_CHECKSUM(memblock[i]);
	}

	return checksum;
}


/*
 *  memory_warn_about_unimplemented_addr():
 *
 *  Called from memory_rw whenever memory outside of the physical address space
 *  is accessed (and quiet_mode isn't set).
 */
void memory_warn_about_unimplemented_addr(struct cpu *cpu, struct memory *mem,
	int writeflag, uint64_t paddr, uint8_t *data, size_t len)
{
	uint64_t offset, old_pc = cpu->pc;
	char *symbol;

	/*
	 *  This allows guest OS kernels to probe memory a few KBs past the
	 *  end of memory, without giving too many warnings.
	 */
	if (paddr < mem->physical_max + 0x40000)
		return;

	if (!cpu->machine->halt_on_nonexistant_memaccess && quiet_mode)
		return;

	fatal("[ memory_rw(): %s ", writeflag? "write":"read");

	if (writeflag) {
		unsigned int i;
		debug("data={", writeflag);
		if (len > 16) {
			int start2 = len-16;
			for (i=0; i<16; i++)
				debug("%s%02x", i?",":"", data[i]);
			debug(" .. ");
			if (start2 < 16)
				start2 = 16;
			for (i=start2; i<len; i++)
				debug("%s%02x", i?",":"", data[i]);
		} else
			for (i=0; i<len; i++)
				debug("%s%02x", i?",":"", data[i]);
		debug("} ");
	}

	fatal("paddr=0x%"PRIx64" >= physical_max; pc=", paddr);
	if (cpu->is_32bit)
		fatal("0x%08"PRIx32, (uint32_t) old_pc);
	else
		fatal("0x%016"PRIx64, (uint64_t) old_pc);
	symbol = get_symbol_name(&cpu->machine->symbol_context,
	    old_pc, &offset);
	fatal(" <%s> ]\n", symbol? symbol : " no symbol ");

	if (cpu->machine->halt_on_nonexistant_memaccess) {
		/*  TODO: Halt in a nicer way. Not possible with the
		    current dyntrans system...  */
		exit(1);
	}
}


/*
 *  dump_mem_string():
 *
 *  Dump the contents of emulated RAM as readable text.  Bytes that aren't
 *  readable are dumped in [xx] notation, where xx is in hexadecimal.
 *  Dumping ends after DUMP_MEM_STRING_MAX bytes, or when a terminating
 *  zero byte is found.
 */
#define DUMP_MEM_STRING_MAX	45
void dump_mem_string(struct cpu *cpu, uint64_t addr)
{
	int i;
	for (i=0; i<DUMP_MEM_STRING_MAX; i++) {
		unsigned char ch = '\0';

		cpu->memory_rw(cpu, cpu->mem, addr + i, &ch, sizeof(ch),
		    MEM_READ, CACHE_DATA | NO_EXCEPTIONS);
		if (ch == '\0')
			return;
		if (ch >= ' ' && ch < 126)
			debug("%c", ch);  
		else
			debug("[%02x]", ch);
	}
}


/*
 *  store_byte():
 *
 *  Stores a byte in emulated ram. (Helper function.)
 */
void store_byte(struct cpu *cpu, uint64_t addr, uint8_t data)
{
	if ((addr >> 32) == 0)
		addr = (int64_t)(int32_t)addr;
	cpu->memory_rw(cpu, cpu->mem,
	    addr, &data, sizeof(data), MEM_WRITE, CACHE_DATA);
}


/*
 *  store_string():
 *
 *  Stores chars into emulated RAM until a zero byte (string terminating
 *  character) is found. The zero byte is also copied.
 *  (strcpy()-like helper function, host-RAM-to-emulated-RAM.)
 */
void store_string(struct cpu *cpu, uint64_t addr, const char *s)
{
	do {
		store_byte(cpu, addr++, *s);
	} while (*s++);
}


/*
 *  add_environment_string():
 *
 *  Like store_string(), but advances the pointer afterwards. The most
 *  obvious use is to place a number of strings (such as environment variable
 *  strings) after one-another in emulated memory.
 */
void add_environment_string(struct cpu *cpu, const char *s, uint64_t *addr)
{
	store_string(cpu, *addr, s);
	(*addr) += strlen(s) + 1;
}


/*
 *  add_environment_string_dual():
 *
 *  Add "dual" environment strings, one for the variable name and one for the
 *  value, and update pointers afterwards.
 */
void add_environment_string_dual(struct cpu *cpu,
	uint64_t *ptrp, uint64_t *addrp, const char *s1, const char *s2)
{
	uint64_t ptr = *ptrp, addr = *addrp;

	store_32bit_word(cpu, ptr, addr);
	ptr += sizeof(uint32_t);
	if (addr != 0) {
		store_string(cpu, addr, s1);
		addr += strlen(s1) + 1;
	}
	store_32bit_word(cpu, ptr, addr);
	ptr += sizeof(uint32_t);
	if (addr != 0) {
		store_string(cpu, addr, s2);
		addr += strlen(s2) + 1;
	}

	*ptrp = ptr;
	*addrp = addr;
}


/*
 *  store_64bit_word():
 *
 *  Stores a 64-bit word in emulated RAM.  Byte order is taken into account.
 *  Helper function.
 */
int store_64bit_word(struct cpu *cpu, uint64_t addr, uint64_t data64)
{
	unsigned char data[8];
	if ((addr >> 32) == 0)
		addr = (int64_t)(int32_t)addr;
	data[0] = (data64 >> 56) & 255;
	data[1] = (data64 >> 48) & 255;
	data[2] = (data64 >> 40) & 255;
	data[3] = (data64 >> 32) & 255;
	data[4] = (data64 >> 24) & 255;
	data[5] = (data64 >> 16) & 255;
	data[6] = (data64 >> 8) & 255;
	data[7] = (data64) & 255;
	if (cpu->byte_order == EMUL_LITTLE_ENDIAN) {
		int tmp = data[0]; data[0] = data[7]; data[7] = tmp;
		tmp = data[1]; data[1] = data[6]; data[6] = tmp;
		tmp = data[2]; data[2] = data[5]; data[5] = tmp;
		tmp = data[3]; data[3] = data[4]; data[4] = tmp;
	}
	return cpu->memory_rw(cpu, cpu->mem,
	    addr, data, sizeof(data), MEM_WRITE, CACHE_DATA);
}


/*
 *  store_32bit_word():
 *
 *  Stores a 32-bit word in emulated RAM.  Byte order is taken into account.
 *  (This function takes a 64-bit word as argument, to suppress some
 *  warnings, but only the lowest 32 bits are used.)
 */
int store_32bit_word(struct cpu *cpu, uint64_t addr, uint64_t data32)
{
	unsigned char data[4];

	data[0] = (data32 >> 24) & 255;
	data[1] = (data32 >> 16) & 255;
	data[2] = (data32 >> 8) & 255;
	data[3] = (data32) & 255;
	if (cpu->byte_order == EMUL_LITTLE_ENDIAN) {
		int tmp = data[0]; data[0] = data[3]; data[3] = tmp;
		tmp = data[1]; data[1] = data[2]; data[2] = tmp;
	}
	return cpu->memory_rw(cpu, cpu->mem,
	    addr, data, sizeof(data), MEM_WRITE, CACHE_DATA);
}


/*
 *  store_16bit_word():
 *
 *  Stores a 16-bit word in emulated RAM.  Byte order is taken into account.
 *  (This function takes a 64-bit word as argument, to suppress some
 *  warnings, but only the lowest 16 bits are used.)
 */
int store_16bit_word(struct cpu *cpu, uint64_t addr, uint64_t data16)
{
	unsigned char data[2];

	data[0] = (data16 >> 8) & 255;
	data[1] = (data16) & 255;
	if (cpu->byte_order == EMUL_LITTLE_ENDIAN) {
		int tmp = data[0]; data[0] = data[1]; data[1] = tmp;
	}
	return cpu->memory_rw(cpu, cpu->mem,
	    addr, data, sizeof(data), MEM_WRITE, CACHE_DATA);
}


/*
 *  store_buf():
 *
 *  memcpy()-like helper function, from host RAM to emulated RAM.
 */
void store_buf(struct cpu *cpu, uint64_t addr, const char *s, size_t len)
{
	size_t psize = 1024;	/*  1024 256 64 16 4 1  */

	while (len != 0) {
		if ((addr & (psize-1)) == 0) {
			while (len >= psize) {
				cpu->memory_rw(cpu, cpu->mem, addr,
				    (unsigned char *)s, psize, MEM_WRITE,
				    CACHE_DATA);
				addr += psize;
				s += psize;
				len -= psize;
			}
		}
		psize >>= 2;
	}

	while (len-- != 0)
		store_byte(cpu, addr++, *s++);
}


/*
 *  store_pointer_and_advance():
 *
 *  Stores a 32-bit or 64-bit pointer in emulated RAM, and advances the
 *  target address. (Useful for e.g. ARCBIOS environment initialization.)
 */
void store_pointer_and_advance(struct cpu *cpu, uint64_t *addrp,
	uint64_t data, int flag64)
{
	uint64_t addr = *addrp;
	if (flag64) {
		store_64bit_word(cpu, addr, data);
		addr += 8;
	} else {
		store_32bit_word(cpu, addr, data);
		addr += 4;
	}
	*addrp = addr;
}


/*
 *  load_64bit_word():
 *
 *  Helper function. Emulated byte order is taken into account.
 */
uint64_t load_64bit_word(struct cpu *cpu, uint64_t addr)
{
	unsigned char data[8];

	cpu->memory_rw(cpu, cpu->mem,
	    addr, data, sizeof(data), MEM_READ, CACHE_DATA);

	if (cpu->byte_order == EMUL_LITTLE_ENDIAN) {
		int tmp = data[0]; data[0] = data[7]; data[7] = tmp;
		tmp = data[1]; data[1] = data[6]; data[6] = tmp;
		tmp = data[2]; data[2] = data[5]; data[5] = tmp;
		tmp = data[3]; data[3] = data[4]; data[4] = tmp;
	}

	return
	    ((uint64_t)data[0] << 56) + ((uint64_t)data[1] << 48) +
	    ((uint64_t)data[2] << 40) + ((uint64_t)data[3] << 32) +
	    ((uint64_t)data[4] << 24) + ((uint64_t)data[5] << 16) +
	    ((uint64_t)data[6] << 8) + (uint64_t)data[7];
}


/*
 *  load_32bit_word():
 *
 *  Helper function. Emulated byte order is taken into account.
 */
uint32_t load_32bit_word(struct cpu *cpu, uint64_t addr)
{
	unsigned char data[4];

	cpu->memory_rw(cpu, cpu->mem,
	    addr, data, sizeof(data), MEM_READ, CACHE_DATA);

	if (cpu->byte_order == EMUL_LITTLE_ENDIAN) {
		int tmp = data[0]; data[0] = data[3]; data[3] = tmp;
		tmp = data[1]; data[1] = data[2]; data[2] = tmp;
	}

	return (data[0] << 24) + (data[1] << 16) + (data[2] << 8) + data[3];
}


/*
 *  load_16bit_word():
 *
 *  Helper function. Emulated byte order is taken into account.
 */
uint16_t load_16bit_word(struct cpu *cpu, uint64_t addr)
{
	unsigned char data[2];

	cpu->memory_rw(cpu, cpu->mem,
	    addr, data, sizeof(data), MEM_READ, CACHE_DATA);

	if (cpu->byte_order == EMUL_LITTLE_ENDIAN) {
		int tmp = data[0]; data[0] = data[1]; data[1] = tmp;
	}

	return (data[0] << 8) + data[1];
}


/*
 *  store_64bit_word_in_host():
 *
 *  Stores a 64-bit word in the _host's_ RAM.  Emulated byte order is taken
 *  into account.  This is useful when building structs in the host's RAM
 *  which will later be copied into emulated RAM.
 */
void store_64bit_word_in_host(struct cpu *cpu,
	unsigned char *data, uint64_t data64)
{
	data[0] = (data64 >> 56) & 255;
	data[1] = (data64 >> 48) & 255;
	data[2] = (data64 >> 40) & 255;
	data[3] = (data64 >> 32) & 255;
	data[4] = (data64 >> 24) & 255;
	data[5] = (data64 >> 16) & 255;
	data[6] = (data64 >> 8) & 255;
	data[7] = (data64) & 255;
	if (cpu->byte_order == EMUL_LITTLE_ENDIAN) {
		int tmp = data[0]; data[0] = data[7]; data[7] = tmp;
		tmp = data[1]; data[1] = data[6]; data[6] = tmp;
		tmp = data[2]; data[2] = data[5]; data[5] = tmp;
		tmp = data[3]; data[3] = data[4]; data[4] = tmp;
	}
}


/*
 *  store_32bit_word_in_host():
 *
 *  See comment for store_64bit_word_in_host().
 *
 *  (Note:  The data32 parameter is a uint64_t. This is done to suppress
 *  some warnings.)
 */
void store_32bit_word_in_host(struct cpu *cpu,
	unsigned char *data, uint64_t data32)
{
	data[0] = (data32 >> 24) & 255;
	data[1] = (data32 >> 16) & 255;
	data[2] = (data32 >> 8) & 255;
	data[3] = (data32) & 255;
	if (cpu->byte_order == EMUL_LITTLE_ENDIAN) {
		int tmp = data[0]; data[0] = data[3]; data[3] = tmp;
		tmp = data[1]; data[1] = data[2]; data[2] = tmp;
	}
}


/*
 *  store_16bit_word_in_host():
 *
 *  See comment for store_64bit_word_in_host().
 */
void store_16bit_word_in_host(struct cpu *cpu,
	unsigned char *data, uint16_t data16)
{
	data[0] = (data16 >> 8) & 255;
	data[1] = (data16) & 255;
	if (cpu->byte_order == EMUL_LITTLE_ENDIAN) {
		int tmp = data[0]; data[0] = data[1]; data[1] = tmp;
	}
}

