#ifndef	ARCBIOS_H
#define	ARCBIOS_H

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
 *  Headerfile for src/arcbios.c.
 *
 *  (Note: There are also files called arcbios_other.h and sgi_arcbios.h,
 *  which are copied from NetBSD.)
 */

#include "misc.h"

#include "thirdparty/sgi_arcbios.h"


struct cpu;

/*  arcbios.c:  */
void arcbios_add_string_to_component(struct machine *machine,
	char *string, uint64_t component);
void arcbios_register_scsicontroller(struct machine *machine,
	uint64_t scsicontroller_component);
uint64_t arcbios_get_scsicontroller(struct machine *machine);
void arcbios_add_memory_descriptor(struct cpu *cpu,
	uint64_t base, uint64_t len, int arctype);
uint64_t arcbios_addchild_manual(struct cpu *cpu,
	uint64_t cclass, uint64_t type, uint64_t flags, uint64_t version,
	uint64_t revision, uint64_t key, uint64_t affinitymask,
	const char *identifier, uint64_t parent, void *config_data,
	size_t config_len);
int arcbios_emul(struct cpu *cpu);
void arcbios_set_default_exception_handler(struct cpu *cpu);

void arcbios_console_init(struct machine *machine,
	uint64_t vram, uint64_t ctrlregs);
void arcbios_init(struct machine *machine, int is64bit, uint64_t sgi_ram_offset,
	const char *primary_ether_string, uint8_t *primary_ether_macaddr);


/*  For internal use in arcbios.c:  */

struct emul_arc_child {
	uint32_t			ptr_peer;
	uint32_t			ptr_child;
	uint32_t			ptr_parent;
	struct arcbios_component	component;
};

struct emul_arc_child64 {
	uint64_t			ptr_peer;
	uint64_t			ptr_child;
	uint64_t			ptr_parent;
	struct arcbios_component64	component;
};

#define	ARC_BOOTSTR_BUFLEN		1000


/*
 *  Problem: kernels seem to be loaded at low addresses in RAM, so
 *  storing environment strings and memory descriptors there is a bad
 *  idea. They are stored at 0xbfc..... instead.  The ARC SPB must
 *  be at physical address 0x1000 though.
 */

#define SGI_SPB_ADDR            0xffffffff80001000ULL
/*  0xbfc10000 is firmware callback vector stuff  */
#define ARC_FIRMWARE_VECTORS    0xffffffffbfc80000ULL
#define ARC_FIRMWARE_ENTRIES    0xffffffffbfc88000ULL
#define ARC_ARGV_START          0xffffffffbfc90000ULL
#define ARC_ENV_STRINGS         0xffffffffbfc98000ULL
#define ARC_ENV_POINTERS        0xffffffffbfc9d000ULL
#define SGI_SYSID_ADDR          0xffffffffbfca1800ULL
#define ARC_DSPSTAT_ADDR        0xffffffffbfca1c00ULL
#define ARC_MEMDESC_ADDR        0xffffffffbfca1c80ULL
#define ARC_CONFIG_DATA_ADDR    0xffffffffbfca2000ULL
#define FIRST_ARC_COMPONENT     0xffffffffbfca8000ULL
#define ARC_PRIVATE_VECTORS     0xffffffffbfcb0000ULL
#define ARC_PRIVATE_ENTRIES     0xffffffffbfcb8000ULL


#endif	/*  ARCBIOS_H  */
