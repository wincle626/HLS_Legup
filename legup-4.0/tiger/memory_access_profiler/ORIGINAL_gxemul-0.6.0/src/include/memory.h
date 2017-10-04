#ifndef	MEMORY_H
#define	MEMORY_H

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
 *  Memory related functions.
 */

#include <sys/types.h>
#include <inttypes.h>

#include "misc.h"


#define	DEFAULT_RAM_IN_MB		32

struct cpu;


/*
 *  Memory mapped device
 */
struct memory_device {
	uint64_t	baseaddr;
	uint64_t	endaddr;	/*  NOTE: after the last byte!  */
	uint64_t	length;
	int		flags;

	const char	*name;

	int		(*f)(struct cpu *,struct memory *,
			    uint64_t,unsigned char *,size_t,int,void *);
	void		*extra;

	unsigned char	*dyntrans_data;

	uint64_t	dyntrans_write_low;
	uint64_t	dyntrans_write_high;
};


/*
 *  Memory
 *  ------
 *
 *  This struct defines a memory object. Most machines only use one memory
 *  object (the main memory), but if necessary, multiple memories can be
 *  used.
 */
struct memory {
	uint64_t	physical_max;
	void		*pagetable;

	int		dev_dyntrans_alignment;

	int		n_mmapped_devices;
	int		last_accessed_device;
	/*  The following two might speed up things a little bit.  */
	/*  (actually maxaddr is the addr after the last address)  */
	uint64_t	mmap_dev_minaddr;
	uint64_t	mmap_dev_maxaddr;

	struct memory_device *devices;
};

#define	BITS_PER_PAGETABLE	20
#define	BITS_PER_MEMBLOCK	20
#define	MAX_BITS		40


/*  memory.c:  */
#define	MEM_PCI_LITTLE_ENDIAN	128
uint64_t memory_readmax64(struct cpu *cpu, unsigned char *buf, int len);
void memory_writemax64(struct cpu *cpu, unsigned char *buf, int len,
	uint64_t data);

void *zeroed_alloc(size_t s);

struct memory *memory_new(uint64_t physical_max, int arch);

int memory_points_to_string(struct cpu *cpu, struct memory *mem,
	uint64_t addr, int min_string_length);
char *memory_conv_to_string(struct cpu *cpu, struct memory *mem,
	uint64_t addr, char *buf, int bufsize);

unsigned char *memory_paddr_to_hostaddr(struct memory *mem,
	uint64_t paddr, int writeflag);


/*  Writeflag:  */
#define	MEM_READ			0
#define	MEM_WRITE			1
#define	MEM_DOWNGRADE			128

/*  Misc. flags:  */
#define	CACHE_DATA			0
#define	CACHE_INSTRUCTION		1
#define	CACHE_NONE			2
#define	CACHE_FLAGS_MASK		0x3
#define	NO_EXCEPTIONS			16
#define	PHYSICAL			32
#define	MEMORY_USER_ACCESS		64	/*  for ARM and M88K  */

/*  Dyntrans Memory flags:  */
#define	DM_DEFAULT				0
#define	DM_DYNTRANS_OK				1
#define	DM_DYNTRANS_WRITE_OK			2
#define	DM_READS_HAVE_NO_SIDE_EFFECTS		4
#define	DM_EMULATED_RAM				8

#define FLAG_WRITEFLAG          1
#define FLAG_NOEXCEPTIONS       2
#define FLAG_INSTR              4

#define	MEMORY_ACCESS_FAILED		0
#define	MEMORY_ACCESS_OK		1
#define	MEMORY_ACCESS_OK_WRITE		2
#define	MEMORY_NOT_FULL_PAGE		256

void memory_device_dyntrans_access(struct cpu *, struct memory *mem,
	void *extra, uint64_t *low, uint64_t *high);

#define DEVICE_ACCESS(x)	int dev_ ## x ## _access(struct cpu *cpu, \
	struct memory *mem, uint64_t relative_addr, unsigned char *data,  \
	size_t len, int writeflag, void *extra)

void memory_device_update_data(struct memory *mem, void *extra,
	unsigned char *data);

void memory_device_register(struct memory *mem, const char *,
	uint64_t baseaddr, uint64_t len, int (*f)(struct cpu *,
	    struct memory *,uint64_t,unsigned char *,size_t,int,void *),
	void *extra, int flags, unsigned char *dyntrans_data);
void memory_device_remove(struct memory *mem, int i);

uint64_t memory_checksum(struct memory *mem);

void dump_mem_string(struct cpu *cpu, uint64_t addr);
void store_string(struct cpu *cpu, uint64_t addr, const char *s);
int store_64bit_word(struct cpu *cpu, uint64_t addr, uint64_t data64);
int store_32bit_word(struct cpu *cpu, uint64_t addr, uint64_t data32);
int store_16bit_word(struct cpu *cpu, uint64_t addr, uint64_t data16);
void store_byte(struct cpu *cpu, uint64_t addr, uint8_t data);
void store_64bit_word_in_host(struct cpu *cpu, unsigned char *data,
        uint64_t data32);
void store_32bit_word_in_host(struct cpu *cpu, unsigned char *data,
        uint64_t data32);
void store_16bit_word_in_host(struct cpu *cpu, unsigned char *data,
        uint16_t data16);
uint64_t load_64bit_word(struct cpu *cpu, uint64_t addr);
uint32_t load_32bit_word(struct cpu *cpu, uint64_t addr);
uint16_t load_16bit_word(struct cpu *cpu, uint64_t addr);
void store_buf(struct cpu *cpu, uint64_t addr, const char *s, size_t len);
void add_environment_string(struct cpu *cpu, const char *s, uint64_t *addr);
void add_environment_string_dual(struct cpu *cpu,
        uint64_t *ptrp, uint64_t *addrp, const char *s1, const char *s2);
void store_pointer_and_advance(struct cpu *cpu, uint64_t *addrp,
        uint64_t data, int flag64);

void memory_warn_about_unimplemented_addr(struct cpu *cpu, struct memory *mem,
	int writeflag, uint64_t paddr, uint8_t *data, size_t len);


#endif	/*  MEMORY_H  */
