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
 *  COMMENT: ARCBIOS emulation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/resource.h>

#include "arcbios.h"
#include "console.h"
#include "cpu.h"
#include "cpu_mips.h"
#include "diskimage.h"
#include "machine.h"
#include "machine_arc.h"
#include "memory.h"
#include "misc.h"

#include "thirdparty/arcbios_other.h"


extern int quiet_mode;


/*
 *  arcbios_add_string_to_component():
 */
void arcbios_add_string_to_component(struct machine *machine,
	char *string, uint64_t component)
{
	if (machine->md.arc->n_string_to_components
	    >= MAX_STRING_TO_COMPONENT) {
		printf("Too many string-to-component mappings.\n");
		exit(1);
	}

	CHECK_ALLOCATION(machine->md.arc->string_to_component[machine->
	    md.arc->n_string_to_components] = strdup(string));

	debug("adding ARC component mapping: 0x%08x = %s\n",
	    (int)component, string);

	machine->md.arc->string_to_component_value[
	    machine->md.arc->n_string_to_components] = component;

	machine->md.arc->n_string_to_components ++;
}


/*
 *  arcbios_get_dsp_stat():
 *
 *  Fills in an arcbios_dsp_stat struct with valid data.
 */
static void arcbios_get_dsp_stat(struct cpu *cpu,
	struct arcbios_dsp_stat *dspstat)
{
	memset(dspstat, 0, sizeof(struct arcbios_dsp_stat));

	store_16bit_word_in_host(cpu, (unsigned char *)&dspstat->
	    CursorXPosition, cpu->machine->md.arc->console_curx + 1);
	store_16bit_word_in_host(cpu, (unsigned char *)&dspstat->
	    CursorYPosition, cpu->machine->md.arc->console_cury + 1);
	store_16bit_word_in_host(cpu, (unsigned char *)&dspstat->
	    CursorMaxXPosition, ARC_CONSOLE_MAX_X);
	store_16bit_word_in_host(cpu, (unsigned char *)&dspstat->
	    CursorMaxYPosition, ARC_CONSOLE_MAX_Y);
	dspstat->ForegroundColor = cpu->machine->md.arc->console_curcolor;
	dspstat->HighIntensity = cpu->machine->md.arc->console_curcolor ^ 0x08;
}


/*
 *  arcbios_putcell():
 */
static void arcbios_putcell(struct cpu *cpu, int ch, int x, int y)
{
	unsigned char buf[2];
	buf[0] = ch;
	buf[1] = cpu->machine->md.arc->console_curcolor;
	if (cpu->machine->md.arc->console_reverse)
		buf[1] = ((buf[1] & 0x70) >> 4) | ((buf[1] & 7) << 4)
		    | (buf[1] & 0x88);
	cpu->memory_rw(cpu, cpu->mem, cpu->machine->md.arc->console_vram +
	    2*(x + cpu->machine->md.arc->console_maxx * y),
	    &buf[0], sizeof(buf), MEM_WRITE,
	    CACHE_NONE | PHYSICAL);
}


/*
 *  handle_esc_seq():
 *
 *  Used by arcbios_putchar().
 */
static void handle_esc_seq(struct cpu *cpu)
{
	int i, len = strlen(cpu->machine->md.arc->escape_sequence);
	int row, col, color, code, start, stop;
	char *p;

	if (cpu->machine->md.arc->escape_sequence[0] != '[')
		return;

	code = cpu->machine->md.arc->escape_sequence[len-1];
	cpu->machine->md.arc->escape_sequence[len-1] = '\0';

	switch (code) {
	case 'm':
		color = atoi(cpu->machine->md.arc->escape_sequence + 1);
		switch (color) {
		case 0:	/*  Default.  */
			cpu->machine->md.arc->console_curcolor = 0x1f;
			cpu->machine->md.arc->console_reverse = 0; break;
		case 1:	/*  "Bold".  */
			cpu->machine->md.arc->console_curcolor |= 0x08; break;
		case 7:	/*  "Reverse".  */
			cpu->machine->md.arc->console_reverse = 1; break;
		case 30: /*  Black foreground.  */
			cpu->machine->md.arc->console_curcolor &= 0xf0;
			cpu->machine->md.arc->console_curcolor |= 0x00; break;
		case 31: /*  Red foreground.  */
			cpu->machine->md.arc->console_curcolor &= 0xf0;
			cpu->machine->md.arc->console_curcolor |= 0x04; break;
		case 32: /*  Green foreground.  */
			cpu->machine->md.arc->console_curcolor &= 0xf0;
			cpu->machine->md.arc->console_curcolor |= 0x02; break;
		case 33: /*  Yellow foreground.  */
			cpu->machine->md.arc->console_curcolor &= 0xf0;
			cpu->machine->md.arc->console_curcolor |= 0x06; break;
		case 34: /*  Blue foreground.  */
			cpu->machine->md.arc->console_curcolor &= 0xf0;
			cpu->machine->md.arc->console_curcolor |= 0x01; break;
		case 35: /*  Red-blue foreground.  */
			cpu->machine->md.arc->console_curcolor &= 0xf0;
			cpu->machine->md.arc->console_curcolor |= 0x05; break;
		case 36: /*  Green-blue foreground.  */
			cpu->machine->md.arc->console_curcolor &= 0xf0;
			cpu->machine->md.arc->console_curcolor |= 0x03; break;
		case 37: /*  White foreground.  */
			cpu->machine->md.arc->console_curcolor &= 0xf0;
			cpu->machine->md.arc->console_curcolor |= 0x07; break;
		case 40: /*  Black background.  */
			cpu->machine->md.arc->console_curcolor &= 0x0f;
			cpu->machine->md.arc->console_curcolor |= 0x00; break;
		case 41: /*  Red background.  */
			cpu->machine->md.arc->console_curcolor &= 0x0f;
			cpu->machine->md.arc->console_curcolor |= 0x40; break;
		case 42: /*  Green background.  */
			cpu->machine->md.arc->console_curcolor &= 0x0f;
			cpu->machine->md.arc->console_curcolor |= 0x20; break;
		case 43: /*  Yellow background.  */
			cpu->machine->md.arc->console_curcolor &= 0x0f;
			cpu->machine->md.arc->console_curcolor |= 0x60; break;
		case 44: /*  Blue background.  */
			cpu->machine->md.arc->console_curcolor &= 0x0f;
			cpu->machine->md.arc->console_curcolor |= 0x10; break;
		case 45: /*  Red-blue background.  */
			cpu->machine->md.arc->console_curcolor &= 0x0f;
			cpu->machine->md.arc->console_curcolor |= 0x50; break;
		case 46: /*  Green-blue background.  */
			cpu->machine->md.arc->console_curcolor &= 0x0f;
			cpu->machine->md.arc->console_curcolor |= 0x30; break;
		case 47: /*  White background.  */
			cpu->machine->md.arc->console_curcolor &= 0x0f;
			cpu->machine->md.arc->console_curcolor |= 0x70; break;
		default:fatal("{ handle_esc_seq: color %i }\n", color);
		}
		return;
	case 'H':
		p = strchr(cpu->machine->md.arc->escape_sequence, ';');
		if (p == NULL)
			return;		/*  TODO  */
		row = atoi(cpu->machine->md.arc->escape_sequence + 1);
		col = atoi(p + 1);
		if (col < 1)
			col = 1;
		if (row < 1)
			row = 1;
		cpu->machine->md.arc->console_curx = col - 1;
		cpu->machine->md.arc->console_cury = row - 1;
		return;
	case 'J':
		/*
		 *  J = clear screen below cursor, and the rest of the
		 *      current line,
		 *  2J = clear whole screen.
		 */
		i = atoi(cpu->machine->md.arc->escape_sequence + 1);
		if (i != 0 && i != 2)
			fatal("{ handle_esc_seq(): %iJ }\n", i);
		if (i == 0)
			for (col = cpu->machine->md.arc->console_curx;
			    col < cpu->machine->md.arc->console_maxx; col++)
				arcbios_putcell(cpu, ' ', col,
				    cpu->machine->md.arc->console_cury);
		for (col = 0; col < cpu->machine->md.arc->console_maxx; col++)
			for (row = i? 0 : cpu->machine->md.arc->console_cury+1;
			    row < cpu->machine->md.arc->console_maxy; row++)
				arcbios_putcell(cpu, ' ', col, row);
		return;
	case 'K':
		col = atoi(cpu->machine->md.arc->escape_sequence + 1);
		/*  2 = clear line to the right. 1 = to the left (?)  */
		start = 0; stop = cpu->machine->md.arc->console_curx;
		if (col == 2) {
			start = cpu->machine->md.arc->console_curx;
			stop = cpu->machine->md.arc->console_maxx - 1;
		}
		for (i=start; i<=stop; i++)
			arcbios_putcell(cpu, ' ', i,
			    cpu->machine->md.arc->console_cury);

		return;
	}

	fatal("{ handle_esc_seq(): unimplemented escape sequence: ");
	for (i=0; i<len; i++) {
		int x = cpu->machine->md.arc->escape_sequence[i];
		if (i == len-1)
			x = code;

		if (x >= ' ' && x < 127)
			fatal("%c", x);
		else
			fatal("[0x%02x]", x);
	}
	fatal(" }\n");
}


/*
 *  scroll_if_necessary():
 */
static void scroll_if_necessary(struct cpu *cpu)
{
	/*  Scroll?  */
	if (cpu->machine->md.arc->console_cury >=
	    cpu->machine->md.arc->console_maxy) {
		unsigned char buf[2];
		int x, y;
		for (y=0; y<cpu->machine->md.arc->console_maxy-1; y++)
			for (x=0; x<cpu->machine->md.arc->console_maxx;
			    x++) {
				cpu->memory_rw(cpu, cpu->mem,
				    cpu->machine->md.arc->console_vram +
				    2*(x + cpu->machine->md.arc->
					console_maxx * (y+1)),
				    &buf[0], sizeof(buf), MEM_READ,
				    CACHE_NONE | PHYSICAL);
				cpu->memory_rw(cpu, cpu->mem,
				    cpu->machine->md.arc->console_vram +
				    2*(x + cpu->machine->md.arc->
					console_maxx * y),
				    &buf[0], sizeof(buf), MEM_WRITE,
				    CACHE_NONE | PHYSICAL);
			}

		cpu->machine->md.arc->console_cury =
		    cpu->machine->md.arc->console_maxy - 1;

		for (x=0; x<cpu->machine->md.arc->console_maxx; x++)
			arcbios_putcell(cpu, ' ', x,
			    cpu->machine->md.arc->console_cury);
	}
}


/*
 *  arcbios_putchar():
 *
 *  If we're using X11 with VGA-style console, then output to that console.
 *  Otherwise, use console_putchar().
 */
static void arcbios_putchar(struct cpu *cpu, int ch)
{
	int addr;
	unsigned char byte;

	if (!cpu->machine->md.arc->vgaconsole) {
		/*  Text console output:  */

		/*  Hack for Windows NT, which uses 0x9b instead of ESC + [  */
		if (ch == 0x9b) {
			console_putchar(cpu->machine->main_console_handle, 27);
			ch = '[';
		}
		console_putchar(cpu->machine->main_console_handle, ch);
		return;
	}

	if (cpu->machine->md.arc->in_escape_sequence) {
		int len = strlen(cpu->machine->md.arc->escape_sequence);
		cpu->machine->md.arc->escape_sequence[len] = ch;
		len++;
		if (len >= ARC_MAX_ESC)
			len = ARC_MAX_ESC;
		cpu->machine->md.arc->escape_sequence[len] = '\0';
		if ((ch >= 'a' && ch <= 'z') ||
		    (ch >= 'A' && ch <= 'Z') || len >= ARC_MAX_ESC) {
			handle_esc_seq(cpu);
			cpu->machine->md.arc->in_escape_sequence = 0;
		}
	} else {
		if (ch == 27) {
			cpu->machine->md.arc->in_escape_sequence = 1;
			cpu->machine->md.arc->escape_sequence[0] = '\0';
		} else if (ch == 0x9b) {
			cpu->machine->md.arc->in_escape_sequence = 1;
			cpu->machine->md.arc->escape_sequence[0] = '[';
			cpu->machine->md.arc->escape_sequence[1] = '\0';
		} else if (ch == '\b') {
			if (cpu->machine->md.arc->console_curx > 0)
				cpu->machine->md.arc->console_curx --;
		} else if (ch == '\r') {
			cpu->machine->md.arc->console_curx = 0;
		} else if (ch == '\n') {
			cpu->machine->md.arc->console_cury ++;
		} else if (ch == '\t') {
			cpu->machine->md.arc->console_curx =
			    ((cpu->machine->md.arc->console_curx - 1)
			    | 7) + 1;
			/*  TODO: Print spaces?  */
		} else {
			/*  Put char:  */
			if (cpu->machine->md.arc->console_curx >=
			    cpu->machine->md.arc->console_maxx) {
				cpu->machine->md.arc->console_curx = 0;
				cpu->machine->md.arc->console_cury ++;
				scroll_if_necessary(cpu);
			}
			arcbios_putcell(cpu, ch,
			    cpu->machine->md.arc->console_curx,
			    cpu->machine->md.arc->console_cury);
			cpu->machine->md.arc->console_curx ++;
		}
	}

	scroll_if_necessary(cpu);

	/*  Update cursor position:  */
	addr = (cpu->machine->md.arc->console_curx >=
	    cpu->machine->md.arc->console_maxx?
	    cpu->machine->md.arc->console_maxx - 1 :
		cpu->machine->md.arc->console_curx) +
	    cpu->machine->md.arc->console_cury *
	    cpu->machine->md.arc->console_maxx;
	byte = 0x0e;
	cpu->memory_rw(cpu, cpu->mem, cpu->machine->md.arc->
	    console_ctrlregs + 0x14,
	    &byte, sizeof(byte), MEM_WRITE, CACHE_NONE | PHYSICAL);
	byte = (addr >> 8) & 255;
	cpu->memory_rw(cpu, cpu->mem, cpu->machine->md.arc->
	    console_ctrlregs + 0x15,
	    &byte, sizeof(byte), MEM_WRITE, CACHE_NONE | PHYSICAL);
	byte = 0x0f;
	cpu->memory_rw(cpu, cpu->mem, cpu->machine->md.arc->
	    console_ctrlregs + 0x14,
	    &byte, sizeof(byte), MEM_WRITE, CACHE_NONE | PHYSICAL);
	byte = addr & 255;
	cpu->memory_rw(cpu, cpu->mem, cpu->machine->md.arc->
	    console_ctrlregs + 0x15,
	    &byte, sizeof(byte), MEM_WRITE, CACHE_NONE | PHYSICAL);
}


/*
 *  arcbios_putstring():
 */
static void arcbios_putstring(struct cpu *cpu, const char *s)
{
	while (*s) {
		if (*s == '\n')
			arcbios_putchar(cpu, '\r');
		arcbios_putchar(cpu, *s++);
	}
}


/*
 *  arcbios_register_scsicontroller():
 */
void arcbios_register_scsicontroller(struct machine *machine,
	uint64_t scsicontroller_component)
{
	machine->md.arc->scsicontroller = scsicontroller_component;
}


/*
 *  arcbios_get_scsicontroller():
 */
uint64_t arcbios_get_scsicontroller(struct machine *machine)
{
	return machine->md.arc->scsicontroller;
}


/*
 *  arcbios_add_memory_descriptor():
 *
 *  NOTE: arctype is the ARC type, not the SGI type. This function takes
 *  care of converting, when necessary.
 */
void arcbios_add_memory_descriptor(struct cpu *cpu,
	uint64_t base, uint64_t len, int arctype)
{
	uint64_t memdesc_addr;
	int s;
	struct arcbios_mem arcbios_mem;
	struct arcbios_mem64 arcbios_mem64;

	base /= 4096;
	len /= 4096;

	/*
	 *  TODO: Huh? Why isn't it necessary to convert from arc to sgi types?
	 *
	 *  TODO 2: It seems that it _is_ necessary, but NetBSD's arcdiag
	 *  doesn't handle the sgi case separately.
	 */
#if 1
	if (cpu->machine->machine_type == MACHINE_SGI) {
		/*  arctype is SGI style  */
		/*  printf("%i => ", arctype); */
		switch (arctype) {
		case 0:	arctype = 0; break;
		case 1:	arctype = 1; break;
		case 2:	arctype = 3; break;
		case 3:	arctype = 4; break;
		case 4:	arctype = 5; break;
		case 5:	arctype = 6; break;
		case 6:	arctype = 7; break;
		case 7:	arctype = 2; break;
		}
		/*  printf("%i\n", arctype);  */
	}
#endif
	if (cpu->machine->md.arc->arc_64bit)
		s = sizeof(arcbios_mem64);
	else
		s = sizeof(arcbios_mem);

	memdesc_addr = cpu->machine->md.arc->memdescriptor_base +
	    cpu->machine->md.arc->n_memdescriptors * s;

	if (cpu->machine->md.arc->arc_64bit) {
		memset(&arcbios_mem64, 0, s);
		store_32bit_word_in_host(cpu,
		    (unsigned char *)&arcbios_mem64.Type, arctype);
		store_64bit_word_in_host(cpu,
		    (unsigned char *)&arcbios_mem64.BasePage, base);
		store_64bit_word_in_host(cpu,
		    (unsigned char *)&arcbios_mem64.PageCount, len);
		store_buf(cpu, memdesc_addr, (char *)&arcbios_mem64, s);
	} else {
		memset(&arcbios_mem, 0, s);
		store_32bit_word_in_host(cpu,
		    (unsigned char *)&arcbios_mem.Type, arctype);
		store_32bit_word_in_host(cpu,
		    (unsigned char *)&arcbios_mem.BasePage, base);
		store_32bit_word_in_host(cpu,
		    (unsigned char *)&arcbios_mem.PageCount, len);
		store_buf(cpu, memdesc_addr, (char *)&arcbios_mem, s);
	}

	cpu->machine->md.arc->n_memdescriptors ++;
}


/*
 *  arcbios_addchild():
 *
 *  host_tmp_component is a temporary component, with data formated for
 *  the host system.  It needs to be translated/copied into emulated RAM.
 *
 *  Return value is the virtual (emulated) address of the added component.
 *
 *  TODO:  This function doesn't care about memory management, but simply
 *         stores the new child after the last stored child.
 *  TODO:  This stuff is really ugly.
 */
static uint64_t arcbios_addchild(struct cpu *cpu,
	struct arcbios_component *host_tmp_component,
	const char *identifier, uint32_t parent)
{
	struct machine *machine = cpu->machine;
	uint64_t a = machine->md.arc->next_component_address;
	uint32_t peer=0;
	uint32_t child=0;
	int n_left;
	uint64_t peeraddr = FIRST_ARC_COMPONENT;

	/*
	 *  This component has no children yet, but it may have peers (that is,
	 *  other components that share this component's parent) so we have to
	 *  set the peer value correctly.
	 *
	 *  Also, if this is the first child of some parent, the parent's child
	 *  pointer should be set to point to this component.  (But only if it
	 *  is the first.)
	 *
	 *  This is really ugly:  scan through all components, starting from
	 *  FIRST_ARC_COMPONENT, to find a component with the same parent as
	 *  this component will have.  If such a component is found, and its
	 *  'peer' value is NULL, then set it to this component's address (a).
	 *
	 *  TODO:  make this nicer
	 */

	n_left = machine->md.arc->n_components;
	while (n_left > 0) {
		/*  Load parent, child, and peer values:  */
		uint32_t eparent, echild, epeer, tmp;
		unsigned char buf[4];

		/*  debug("[ addchild: peeraddr = 0x%08x ]\n",
		    (int)peeraddr);  */

		cpu->memory_rw(cpu, cpu->mem,
		    peeraddr + 0 * machine->md.arc->wordlen, &buf[0],
		    sizeof(eparent), MEM_READ, CACHE_NONE);
		if (cpu->byte_order == EMUL_BIG_ENDIAN) {
			unsigned char tmp;
			tmp = buf[0]; buf[0] = buf[3]; buf[3] = tmp;
			tmp = buf[1]; buf[1] = buf[2]; buf[2] = tmp;
		}
		epeer   = buf[0] + (buf[1]<<8) + (buf[2]<<16) + (buf[3]<<24);

		cpu->memory_rw(cpu, cpu->mem, peeraddr + 1 *
		    machine->md.arc->wordlen,
		    &buf[0], sizeof(eparent), MEM_READ, CACHE_NONE);
		if (cpu->byte_order == EMUL_BIG_ENDIAN) {
			unsigned char tmp; tmp = buf[0];
			buf[0] = buf[3]; buf[3] = tmp;
			tmp = buf[1]; buf[1] = buf[2]; buf[2] = tmp;
		}
		echild  = buf[0] + (buf[1]<<8) + (buf[2]<<16) + (buf[3]<<24);

		cpu->memory_rw(cpu, cpu->mem, peeraddr + 2 *
		    machine->md.arc->wordlen,
		    &buf[0], sizeof(eparent), MEM_READ, CACHE_NONE);
		if (cpu->byte_order == EMUL_BIG_ENDIAN) {
			unsigned char tmp; tmp = buf[0];
			buf[0] = buf[3]; buf[3] = tmp;
			tmp = buf[1]; buf[1] = buf[2]; buf[2] = tmp;
		}
		eparent = buf[0] + (buf[1]<<8) + (buf[2]<<16) + (buf[3]<<24);

		/*  debug("  epeer=%x echild=%x eparent=%x\n",
		    (int)epeer,(int)echild,(int)eparent);  */

		if ((uint32_t)eparent == (uint32_t)parent &&
		    (uint32_t)epeer == 0) {
			epeer = a;
			store_32bit_word(cpu, peeraddr + 0x00, epeer);
			/*  debug("[ addchild: adding 0x%08x as peer "
			    "to 0x%08x ]\n", (int)a, (int)peeraddr);  */
		}
		if ((uint32_t)peeraddr == (uint32_t)parent &&
		    (uint32_t)echild == 0) {
			echild = a;
			store_32bit_word(cpu, peeraddr + 0x04, echild);
			/*  debug("[ addchild: adding 0x%08x as "
			    "child to 0x%08x ]\n", (int)a, (int)peeraddr);  */
		}

		/*  Go to the next component:  */
		cpu->memory_rw(cpu, cpu->mem, peeraddr + 0x28, &buf[0],
		    sizeof(eparent), MEM_READ, CACHE_NONE);
		if (cpu->byte_order == EMUL_BIG_ENDIAN) {
			unsigned char tmp; tmp = buf[0];
			buf[0] = buf[3]; buf[3] = tmp;
			tmp = buf[1]; buf[1] = buf[2]; buf[2] = tmp;
		}
		tmp = buf[0] + (buf[1]<<8) + (buf[2]<<16) + (buf[3]<<24);
		peeraddr += 0x30;
		peeraddr += tmp + 1;
		peeraddr = ((peeraddr - 1) | 3) + 1;

		n_left --;
	}

	store_32bit_word(cpu, a + 0x00, peer);
	store_32bit_word(cpu, a + 0x04, child);
	store_32bit_word(cpu, a + 0x08, parent);
	store_32bit_word(cpu, a+  0x0c, host_tmp_component->Class);
	store_32bit_word(cpu, a+  0x10, host_tmp_component->Type);
	store_32bit_word(cpu, a+  0x14, host_tmp_component->Flags +
	    65536 * host_tmp_component->Version);
	store_32bit_word(cpu, a+  0x18, host_tmp_component->Revision);
	store_32bit_word(cpu, a+  0x1c, host_tmp_component->Key);
	store_32bit_word(cpu, a+  0x20, host_tmp_component->AffinityMask);
	store_32bit_word(cpu, a+  0x24, host_tmp_component->
	    ConfigurationDataSize);
	store_32bit_word(cpu, a+  0x28, host_tmp_component->IdentifierLength);
	store_32bit_word(cpu, a+  0x2c, host_tmp_component->Identifier);

	machine->md.arc->next_component_address += 0x30;

	if (host_tmp_component->IdentifierLength != 0) {
		store_32bit_word(cpu, a + 0x2c, a + 0x30);
		store_string(cpu, a + 0x30, identifier);
		if (identifier != NULL)
			machine->md.arc->next_component_address +=
			    strlen(identifier) + 1;
	}

	machine->md.arc->next_component_address ++;

	/*  Round up to next 0x4 bytes:  */
	machine->md.arc->next_component_address =
	    ((machine->md.arc->next_component_address - 1) | 3) + 1;

	machine->md.arc->n_components ++;

	return a;
}


/*
 *  arcbios_addchild64():
 *
 *  host_tmp_component is a temporary component, with data formated for
 *  the host system.  It needs to be translated/copied into emulated RAM.
 *
 *  Return value is the virtual (emulated) address of the added component.
 *
 *  TODO:  This function doesn't care about memory management, but simply
 *         stores the new child after the last stored child.
 *  TODO:  This stuff is really ugly.
 */
static uint64_t arcbios_addchild64(struct cpu *cpu,
	struct arcbios_component64 *host_tmp_component,
	const char *identifier, uint64_t parent)
{
	struct machine *machine = cpu->machine;
	uint64_t a = machine->md.arc->next_component_address;
	uint64_t peer=0;
	uint64_t child=0;
	int n_left;
	uint64_t peeraddr = FIRST_ARC_COMPONENT;

	/*
	 *  This component has no children yet, but it may have peers (that is,
	 *  other components that share this component's parent) so we have to
	 *  set the peer value correctly.
	 *
	 *  Also, if this is the first child of some parent, the parent's child
	 *  pointer should be set to point to this component.  (But only if it
	 *  is the first.)
	 *
	 *  This is really ugly:  scan through all components, starting from
	 *  FIRST_ARC_COMPONENT, to find a component with the same parent as
	 *  this component will have.  If such a component is found, and its
	 *  'peer' value is NULL, then set it to this component's address (a).
	 *
	 *  TODO:  make this nicer
	 */

	n_left = machine->md.arc->n_components;
	while (n_left > 0) {
		/*  Load parent, child, and peer values:  */
		uint64_t eparent, echild, epeer, tmp;
		unsigned char buf[8];

		/*  debug("[ addchild: peeraddr = 0x%016"PRIx64" ]\n",
		    (uint64_t) peeraddr);  */

		cpu->memory_rw(cpu, cpu->mem,
		    peeraddr + 0 * machine->md.arc->wordlen, &buf[0],
		    sizeof(eparent), MEM_READ, CACHE_NONE);
		if (cpu->byte_order == EMUL_BIG_ENDIAN) {
			unsigned char tmp;
			tmp = buf[0]; buf[0] = buf[7]; buf[7] = tmp;
			tmp = buf[1]; buf[1] = buf[6]; buf[6] = tmp;
			tmp = buf[2]; buf[2] = buf[5]; buf[5] = tmp;
			tmp = buf[3]; buf[3] = buf[4]; buf[4] = tmp;
		}
		epeer   = buf[0] + (buf[1]<<8) + (buf[2]<<16) + (buf[3]<<24)
		    + ((uint64_t)buf[4] << 32) + ((uint64_t)buf[5] << 40)
		    + ((uint64_t)buf[6] << 48) + ((uint64_t)buf[7] << 56);

		cpu->memory_rw(cpu, cpu->mem, peeraddr + 1 *
		    machine->md.arc->wordlen,
		    &buf[0], sizeof(eparent), MEM_READ, CACHE_NONE);
		if (cpu->byte_order == EMUL_BIG_ENDIAN) {
			unsigned char tmp;
			tmp = buf[0]; buf[0] = buf[7]; buf[7] = tmp;
			tmp = buf[1]; buf[1] = buf[6]; buf[6] = tmp;
			tmp = buf[2]; buf[2] = buf[5]; buf[5] = tmp;
			tmp = buf[3]; buf[3] = buf[4]; buf[4] = tmp;
		}
		echild  = buf[0] + (buf[1]<<8) + (buf[2]<<16) + (buf[3]<<24)
		    + ((uint64_t)buf[4] << 32) + ((uint64_t)buf[5] << 40)
		    + ((uint64_t)buf[6] << 48) + ((uint64_t)buf[7] << 56);

		cpu->memory_rw(cpu, cpu->mem, peeraddr + 2 *
		    machine->md.arc->wordlen,
		    &buf[0], sizeof(eparent), MEM_READ, CACHE_NONE);
		if (cpu->byte_order == EMUL_BIG_ENDIAN) {
			unsigned char tmp;
			tmp = buf[0]; buf[0] = buf[7]; buf[7] = tmp;
			tmp = buf[1]; buf[1] = buf[6]; buf[6] = tmp;
			tmp = buf[2]; buf[2] = buf[5]; buf[5] = tmp;
			tmp = buf[3]; buf[3] = buf[4]; buf[4] = tmp;
		}
		eparent = buf[0] + (buf[1]<<8) + (buf[2]<<16) + (buf[3]<<24)
		    + ((uint64_t)buf[4] << 32) + ((uint64_t)buf[5] << 40)
		    + ((uint64_t)buf[6] << 48) + ((uint64_t)buf[7] << 56);

		/*  debug("  epeer=%"PRIx64" echild=%"PRIx64" eparent=%"PRIx64
		    "\n", (uint64_t) epeer, (uint64_t) echild,
		    (uint64_t) eparent);  */

		if (eparent == parent && epeer == 0) {
			epeer = a;
			store_64bit_word(cpu, peeraddr + 0 *
			    machine->md.arc->wordlen, epeer);
			/*  debug("[ addchild: adding 0x%016"PRIx64" as peer "
			    "to 0x%016"PRIx64" ]\n", (uint64_t) a,
			    (uint64_t) peeraddr);  */
		}
		if (peeraddr == parent && echild == 0) {
			echild = a;
			store_64bit_word(cpu, peeraddr + 1 *
			    machine->md.arc->wordlen, echild);
			/*  debug("[ addchild: adding 0x%016"PRIx64" as child "
			    "to 0x%016"PRIx64" ]\n", (uint64_t) a,
			    (uint64_t) peeraddr);  */
		}

		/*  Go to the next component:  */
		cpu->memory_rw(cpu, cpu->mem, peeraddr + 0x34,
		    &buf[0], sizeof(uint32_t), MEM_READ, CACHE_NONE);
		if (cpu->byte_order == EMUL_BIG_ENDIAN) {
			unsigned char tmp;
			tmp = buf[0]; buf[0] = buf[3]; buf[3] = tmp;
			tmp = buf[1]; buf[1] = buf[2]; buf[2] = tmp;
		}
		tmp = buf[0] + (buf[1]<<8) + (buf[2]<<16) + (buf[3]<<24);

		tmp &= 0xfffff;

		peeraddr += 0x50;
		peeraddr += tmp + 1;
		peeraddr = ((peeraddr - 1) | 3) + 1;

		n_left --;
	}

	store_64bit_word(cpu, a + 0x00, peer);
	store_64bit_word(cpu, a + 0x08, child);
	store_64bit_word(cpu, a + 0x10, parent);
	store_32bit_word(cpu, a+  0x18, host_tmp_component->Class);
	store_32bit_word(cpu, a+  0x1c, host_tmp_component->Type);
	store_32bit_word(cpu, a+  0x20, host_tmp_component->Flags);
	store_32bit_word(cpu, a+  0x24, host_tmp_component->Version +
	    ((uint64_t)host_tmp_component->Revision << 16));
	store_32bit_word(cpu, a+  0x28, host_tmp_component->Key);
	store_64bit_word(cpu, a+  0x30, host_tmp_component->AffinityMask);
	store_64bit_word(cpu, a+  0x38, host_tmp_component->
	    ConfigurationDataSize);
	store_64bit_word(cpu, a+  0x40, host_tmp_component->IdentifierLength);
	store_64bit_word(cpu, a+  0x48, host_tmp_component->Identifier);

	/*  TODO: Find out how a REAL ARCS64 implementation does it.  */

	machine->md.arc->next_component_address += 0x50;

	if (host_tmp_component->IdentifierLength != 0) {
		store_64bit_word(cpu, a + 0x48, a + 0x50);
		store_string(cpu, a + 0x50, identifier);
		if (identifier != NULL)
			machine->md.arc->next_component_address +=
			    strlen(identifier) + 1;
	}

	machine->md.arc->next_component_address ++;

	/*  Round up to next 0x8 bytes:  */
	machine->md.arc->next_component_address =
	    ((machine->md.arc->next_component_address - 1) | 7) + 1;

	machine->md.arc->n_components ++;
	return a;
}


/*
 *  arcbios_addchild_manual():
 *
 *  Used internally to set up component structures.
 *  Parent may only be NULL for the first (system) component.
 *
 *  Return value is the virtual (emulated) address of the added component.
 */
uint64_t arcbios_addchild_manual(struct cpu *cpu,
	uint64_t cclass, uint64_t type, uint64_t flags,
	uint64_t version, uint64_t revision, uint64_t key,
	uint64_t affinitymask, const char *identifier, uint64_t parent,
	void *config_data, size_t config_len)
{
	struct machine *machine = cpu->machine;
	/*  This component is only for temporary use:  */
	struct arcbios_component component;
	struct arcbios_component64 component64;

	if (config_data != NULL) {
		unsigned char *p = (unsigned char *) config_data;
		size_t i;

		if (machine->md.arc->n_configuration_data >= MAX_CONFIG_DATA) {
			printf("fatal error: you need to increase "
			    "MAX_CONFIG_DATA\n");
			exit(1);
		}

		for (i=0; i<config_len; i++) {
			unsigned char ch = p[i];
			cpu->memory_rw(cpu, cpu->mem,
			    machine->md.arc->configuration_data_next_addr + i,
			    &ch, 1, MEM_WRITE, CACHE_NONE);
		}

		machine->md.arc->configuration_data_len[
		    machine->md.arc->n_configuration_data] = config_len;
		machine->md.arc->configuration_data_configdata[
		    machine->md.arc->n_configuration_data] =
		    machine->md.arc->configuration_data_next_addr;
		machine->md.arc->configuration_data_next_addr += config_len;
		machine->md.arc->configuration_data_component[
		    machine->md.arc->n_configuration_data] =
		    machine->md.arc->next_component_address +
		    (cpu->machine->md.arc->arc_64bit? 0x18 : 0x0c);

		/*  printf("& ADDING %i: configdata=0x%016"PRIx64" "
		    "component=0x%016"PRIx64"\n",
		     machine->md.arc->n_configuration_data,
		    (uint64_t) machine->md.arc->configuration_data_configdata[
			machine->md.arc->n_configuration_data],
		    (uint64_t) machine->md.arc->configuration_data_component[
			machine->md.arc->n_configuration_data]);  */

		machine->md.arc->n_configuration_data ++;
	}

	if (!cpu->machine->md.arc->arc_64bit) {
		component.Class                 = cclass;
		component.Type                  = type;
		component.Flags                 = flags;
		component.Version               = version;
		component.Revision              = revision;
		component.Key                   = key;
		component.AffinityMask          = affinitymask;
		component.ConfigurationDataSize = config_len;
		component.IdentifierLength      = 0;
		component.Identifier            = 0;
		if (identifier != NULL) {
			component.IdentifierLength = strlen(identifier) + 1;
		}

		return arcbios_addchild(cpu, &component, identifier, parent);
	} else {
		component64.Class                 = cclass;
		component64.Type                  = type;
		component64.Flags                 = flags;
		component64.Version               = version;
		component64.Revision              = revision;
		component64.Key                   = key;
		component64.AffinityMask          = affinitymask;
		component64.ConfigurationDataSize = config_len;
		component64.IdentifierLength      = 0;
		component64.Identifier            = 0;
		if (identifier != NULL) {
			component64.IdentifierLength = strlen(identifier) + 1;
		}

		return arcbios_addchild64(cpu, &component64, identifier,
		    parent);
	}
}


/*
 *  arcbios_get_msdos_partition_size():
 *
 *  This function tries to parse MSDOS-style partition tables on a disk
 *  image, and return the starting offset (counted in bytes), and the
 *  size, of a specific partition.
 *
 *  NOTE: partition_nr is 1-based!
 *
 *  TODO: This is buggy, it doesn't really handle extended partitions.
 *
 *  See http://www.nondot.org/sabre/os/files/Partitions/Partitions.html
 *  for more info.
 */
static void arcbios_get_msdos_partition_size(struct machine *machine,
	int disk_id, int disk_type, int partition_nr, uint64_t *start,
	uint64_t *size)
{
	int res, i, partition_type, cur_partition = 0;
	unsigned char sector[512];
	unsigned char buf[16];
	uint64_t offset = 0, st;

	/*  Partition 0 is the entire disk image:  */
	*start = 0;
	*size = diskimage_getsize(machine, disk_id, disk_type);
	if (partition_nr == 0)
		return;

ugly_goto:
	*start = 0; *size = 0;

	/*  printf("reading MSDOS partition from offset 0x%"PRIx64"\n",
	    (uint64_t) offset);  */

	res = diskimage_access(machine, disk_id, disk_type, 0, offset,
	    sector, sizeof(sector));
	if (!res) {
		fatal("[ arcbios_get_msdos_partition_size(): couldn't "
		    "read the disk image, id %i, offset 0x%"PRIx64" ]\n",
		    disk_id, (uint64_t) offset);
		return;
	}

	if (sector[510] != 0x55 || sector[511] != 0xaa) {
		fatal("[ arcbios_get_msdos_partition_size(): not an "
		    "MSDOS partition table ]\n");
	}

#if 0
	/*  Debug dump:  */
	for (i=0; i<4; i++) {
		int j;
		printf("  partition %i: ", i+1);
		for (j=0; j<16; j++)
			printf(" %02x", sector[446 + i*16 + j]);
		printf("\n");
	}
#endif

	for (i=0; i<4; i++) {
		memmove(buf, sector + 446 + 16*i, 16);

		partition_type = buf[4];

		if (partition_type == 0)
			continue;

		st = (buf[8] + (buf[9] << 8) + (buf[10] << 16) +
		    (buf[11] << 24)) * 512;

		if (start != NULL)
			*start = st;
		if (size != NULL)
			*size = (buf[12] + (buf[13] << 8) + (buf[14] << 16) +
			    (buf[15] << 24)) * 512;

		/*  Extended DOS partition:  */
		if (partition_type == 5) {
			offset += st;
			goto ugly_goto;
		}

		/*  Found the right partition? Then return.  */
		cur_partition ++;
		if (cur_partition == partition_nr)
			return;
	}

	fatal("[ partition(%i) NOT found ]\n", partition_nr);
}


/*
 *  arcbios_handle_to_disk_id_and_type():
 */
static int arcbios_handle_to_disk_id_and_type(struct machine *machine,
	int handle, int *typep)
{
	int id, cdrom;
	char *s;

	if (handle < 0 || handle >= ARC_MAX_HANDLES)
		return -1;

	s = machine->md.arc->file_handle_string[handle];
	if (s == NULL)
		return -1;

	/*
	 *  s is something like "scsi(0)disk(0)rdisk(0)partition(0)".
	 *  TODO: This is really ugly and hardcoded.
	 */

	if (strncmp(s, "scsi(", 5) != 0 || strlen(s) < 13)
		return -1;

	*typep = DISKIMAGE_SCSI;

	cdrom = (s[7] == 'c');
	id = cdrom? atoi(s + 13) : atoi(s + 12);

	return id;
}


/*
 *  arcbios_handle_to_start_and_size():
 */
static void arcbios_handle_to_start_and_size(struct machine *machine,
	int handle, uint64_t *start, uint64_t *size)
{
	char *s = machine->md.arc->file_handle_string[handle];
	char *s2;
	int disk_id, disk_type;

	disk_id = arcbios_handle_to_disk_id_and_type(machine,
	    handle, &disk_type);

	if (disk_id < 0)
		return;

	/*  This works for "partition(0)":  */
	*start = 0;
	*size = diskimage_getsize(machine, disk_id, disk_type);

	s2 = strstr(s, "partition(");
	if (s2 != NULL) {
		int partition_nr = atoi(s2 + 10);
		/*  printf("partition_nr = %i\n", partition_nr);  */
		if (partition_nr != 0)
			arcbios_get_msdos_partition_size(machine,
			    disk_id, disk_type, partition_nr, start, size);
	}
}


/*
 *  arcbios_getfileinformation():
 *
 *  Fill in a GetFileInformation struct in emulated memory,
 *  for a specific file handle. (This is used to get the size
 *  and offsets of partitions on disk images.)
 */
static int arcbios_getfileinformation(struct cpu *cpu)
{
	int handle = cpu->cd.mips.gpr[MIPS_GPR_A0];
	uint64_t addr = cpu->cd.mips.gpr[MIPS_GPR_A1];
	uint64_t start, size;

	arcbios_handle_to_start_and_size(cpu->machine, handle, &start, &size);

	store_64bit_word(cpu, addr + 0, 0);
	store_64bit_word(cpu, addr + 8, size);
	store_64bit_word(cpu, addr + 16, 0);
	store_32bit_word(cpu, addr + 24, 1);
	store_32bit_word(cpu, addr + 28, 0);
	store_32bit_word(cpu, addr + 32, 0);

	/*  printf("\n!!! size=0x%x start=0x%x\n", (int)size, (int)start);  */

	return ARCBIOS_ESUCCESS;
}


/*
 *  arcbios_private_emul():
 *
 *  TODO:  This is probably SGI specific. (?)
 *
 *	0x04	get nvram table
 */
void arcbios_private_emul(struct cpu *cpu)
{
	int vector = cpu->pc & 0xfff;

	switch (vector) {
	case 0x04:
		debug("[ ARCBIOS PRIVATE get nvram table(): TODO ]\n");
		cpu->cd.mips.gpr[MIPS_GPR_V0] = 0;
		break;
	default:
		cpu_register_dump(cpu->machine, cpu, 1, 0x1);
		debug("a0 points to: ");
		dump_mem_string(cpu, cpu->cd.mips.gpr[MIPS_GPR_A0]);
		debug("\n");
		fatal("ARCBIOS: unimplemented PRIVATE vector 0x%x\n", vector);
		cpu->running = 0;
	}
}


/*
 *  arcbios_emul():  ARCBIOS emulation
 *
 *	0x0c	Halt()
 *	0x10	PowerDown()
 *	0x14	Restart()
 *	0x18	Reboot()
 *	0x1c	EnterInteractiveMode()
 *	0x20	ReturnFromMain()
 *	0x24	GetPeer(node)
 *	0x28	GetChild(node)
 *	0x2c	GetParent(node)
 *	0x30	GetConfigurationData(config_data, node)
 *	0x3c	GetComponent(name)
 *	0x44	GetSystemId()
 *	0x48	GetMemoryDescriptor(void *)
 *	0x50	GetTime()
 *	0x54	GetRelativeTime()
 *	0x5c	Open(path, mode, &fileid)
 *	0x60	Close(handle)
 *	0x64	Read(handle, &buf, len, &actuallen)
 *	0x6c	Write(handle, buf, len, &returnlen)
 *	0x70	Seek(handle, &offset, len)
 *	0x78	GetEnvironmentVariable(char *)
 *	0x7c	SetEnvironmentVariable(char *, char *)
 *	0x80	GetFileInformation(handle, buf)
 *	0x88	FlushAllCaches()
 *	0x90	GetDisplayStatus(uint32_t handle)
 *	0x100	undocumented IRIX (?)
 */
int arcbios_emul(struct cpu *cpu)
{
	struct machine *machine = cpu->machine;
	int vector = cpu->pc & 0xfff;
	int i, j, handle;
	unsigned char ch2;
	unsigned char buf[40];

	if (cpu->pc >= ARC_PRIVATE_ENTRIES &&
	    cpu->pc < ARC_PRIVATE_ENTRIES + 100*sizeof(uint32_t)) {
		arcbios_private_emul(cpu);
		return 1;
	}

	if (machine->md.arc->arc_64bit)
		vector /= 2;

	/*  Special case for reboot by jumping to 0xbfc00000:  */
	if (vector == 0 && (cpu->pc & 0xffffffffULL) == 0xbfc00000ULL)
		vector = 0x18;

	switch (vector) {
	case 0x0c:		/*  Halt()  */
	case 0x10:		/*  PowerDown()  */
	case 0x14:		/*  Restart()  */
	case 0x18:		/*  Reboot()  */
	case 0x1c:		/*  EnterInteractiveMode()  */
	case 0x20:		/*  ReturnFromMain()  */
		debug("[ ARCBIOS Halt() or similar ]\n");
		/*  Halt all CPUs.  */
		for (i=0; i<machine->ncpus; i++) {
			machine->cpus[i]->running = 0;
		}
		machine->exit_without_entering_debugger = 1;
		break;
	case 0x24:		/*  GetPeer(node)  */
		if (cpu->cd.mips.gpr[MIPS_GPR_A0] == 0) {
			/*  NULL ptr argument: return NULL.  */
			cpu->cd.mips.gpr[MIPS_GPR_V0] = 0;
		} else {
			uint64_t peer;
			cpu->memory_rw(cpu, cpu->mem,
			    cpu->cd.mips.gpr[MIPS_GPR_A0] - 3 *
			    machine->md.arc->wordlen, &buf[0],
			    machine->md.arc->wordlen, MEM_READ, CACHE_NONE);
			if (machine->md.arc->arc_64bit) {
				if (cpu->byte_order == EMUL_BIG_ENDIAN) {
					unsigned char tmp; tmp = buf[0];
					buf[0] = buf[7]; buf[7] = tmp;
					tmp = buf[1]; buf[1] = buf[6];
					buf[6] = tmp;
					tmp = buf[2]; buf[2] = buf[5];
					buf[5] = tmp;
					tmp = buf[3]; buf[3] = buf[4];
					buf[4] = tmp;
				}
				peer = (uint64_t)buf[0] + ((uint64_t)buf[1]<<8)
				    + ((uint64_t)buf[2]<<16)
				    + ((uint64_t)buf[3]<<24)
				    + ((uint64_t)buf[4]<<32)
				    + ((uint64_t)buf[5]<<40)
				    + ((uint64_t)buf[6]<<48)
				    + ((uint64_t)buf[7]<<56);
			} else {
				if (cpu->byte_order == EMUL_BIG_ENDIAN) {
					unsigned char tmp; tmp = buf[0];
					buf[0] = buf[3]; buf[3] = tmp;
					tmp = buf[1]; buf[1] = buf[2];
					buf[2] = tmp;
				}
				peer = buf[0] + (buf[1]<<8) + (buf[2]<<16)
				    + (buf[3]<<24);
			}

			cpu->cd.mips.gpr[MIPS_GPR_V0] = peer?
			    (peer + 3 * machine->md.arc->wordlen) : 0;
			if (!machine->md.arc->arc_64bit)
				cpu->cd.mips.gpr[MIPS_GPR_V0] = (int64_t)
				    (int32_t) cpu->cd.mips.gpr[MIPS_GPR_V0];
		}
		debug("[ ARCBIOS GetPeer(node 0x%016"PRIx64"): 0x%016"PRIx64
		    " ]\n", (uint64_t) cpu->cd.mips.gpr[MIPS_GPR_A0],
		    (uint64_t) cpu->cd.mips.gpr[MIPS_GPR_V0]);
		break;
	case 0x28:		/*  GetChild(node)  */
		/*  0 for the root, non-0 for children:  */
		if (cpu->cd.mips.gpr[MIPS_GPR_A0] == 0)
			cpu->cd.mips.gpr[MIPS_GPR_V0] = FIRST_ARC_COMPONENT
			    + machine->md.arc->wordlen * 3;
		else {
			uint64_t child = 0;
			cpu->memory_rw(cpu, cpu->mem,
			    cpu->cd.mips.gpr[MIPS_GPR_A0] - 2 *
			    machine->md.arc->wordlen, &buf[0], machine->
			    md.arc->wordlen, MEM_READ, CACHE_NONE);
			if (machine->md.arc->arc_64bit) {
				if (cpu->byte_order == EMUL_BIG_ENDIAN) {
					unsigned char tmp; tmp = buf[0];
					buf[0] = buf[7]; buf[7] = tmp;
					tmp = buf[1]; buf[1] = buf[6];
					buf[6] = tmp;
					tmp = buf[2]; buf[2] = buf[5];
					buf[5] = tmp;
					tmp = buf[3]; buf[3] = buf[4];
					buf[4] = tmp;
				}
				child = (uint64_t)buf[0] +
				    ((uint64_t)buf[1]<<8) +
				    ((uint64_t)buf[2]<<16) +
				    ((uint64_t)buf[3]<<24) +
				    ((uint64_t)buf[4]<<32) +
				    ((uint64_t)buf[5]<<40) +
				    ((uint64_t)buf[6]<<48) +
				    ((uint64_t)buf[7]<<56);
			} else {
				if (cpu->byte_order == EMUL_BIG_ENDIAN) {
					unsigned char tmp; tmp = buf[0];
					buf[0] = buf[3]; buf[3] = tmp;
					tmp = buf[1]; buf[1] = buf[2];
					buf[2] = tmp;
				}
				child = buf[0] + (buf[1]<<8) + (buf[2]<<16) +
				    (buf[3]<<24);
			}

			cpu->cd.mips.gpr[MIPS_GPR_V0] = child?
			    (child + 3 * machine->md.arc->wordlen) : 0;
			if (!machine->md.arc->arc_64bit)
				cpu->cd.mips.gpr[MIPS_GPR_V0] = (int64_t)
				    (int32_t)cpu->cd.mips.gpr[MIPS_GPR_V0];
		}
		debug("[ ARCBIOS GetChild(node 0x%016"PRIx64"): 0x%016"
		    PRIx64" ]\n", (uint64_t) cpu->cd.mips.gpr[MIPS_GPR_A0],
		    (uint64_t) cpu->cd.mips.gpr[MIPS_GPR_V0]);
		break;
	case 0x2c:		/*  GetParent(node)  */
		{
			uint64_t parent;

			cpu->memory_rw(cpu, cpu->mem,
			    cpu->cd.mips.gpr[MIPS_GPR_A0] - 1 * machine->
			    md.arc->wordlen, &buf[0], machine->md.arc->wordlen,
			    MEM_READ, CACHE_NONE);

			if (machine->md.arc->arc_64bit) {
				if (cpu->byte_order == EMUL_BIG_ENDIAN) {
					unsigned char tmp; tmp = buf[0];
					buf[0] = buf[7]; buf[7] = tmp;
					tmp = buf[1]; buf[1] = buf[6];
					buf[6] = tmp;
					tmp = buf[2]; buf[2] = buf[5];
					buf[5] = tmp;
					tmp = buf[3]; buf[3] = buf[4];
					buf[4] = tmp;
				}
				parent = (uint64_t)buf[0] +
				    ((uint64_t)buf[1]<<8) +
				    ((uint64_t)buf[2]<<16) +
				    ((uint64_t)buf[3]<<24) +
				    ((uint64_t)buf[4]<<32) +
				    ((uint64_t)buf[5]<<40) +
				    ((uint64_t)buf[6]<<48) +
				    ((uint64_t)buf[7]<<56);
			} else {
				if (cpu->byte_order == EMUL_BIG_ENDIAN) {
					unsigned char tmp; tmp = buf[0];
					buf[0] = buf[3]; buf[3] = tmp;
					tmp = buf[1]; buf[1] = buf[2];
					buf[2] = tmp;
				}
				parent = buf[0] + (buf[1]<<8) +
				    (buf[2]<<16) + (buf[3]<<24);
			}

			cpu->cd.mips.gpr[MIPS_GPR_V0] = parent?
			    (parent + 3 * machine->md.arc->wordlen) : 0;
			if (!machine->md.arc->arc_64bit)
				cpu->cd.mips.gpr[MIPS_GPR_V0] = (int64_t)
				    (int32_t) cpu->cd.mips.gpr[MIPS_GPR_V0];
		}
		debug("[ ARCBIOS GetParent(node 0x%016"PRIx64"): 0x%016"
		    PRIx64" ]\n", (uint64_t) cpu->cd.mips.gpr[MIPS_GPR_A0],
		    (uint64_t) cpu->cd.mips.gpr[MIPS_GPR_V0]);
		break;
	case 0x30:  /*  GetConfigurationData(void *configdata, void *node)  */
		/*  fatal("[ ARCBIOS GetConfigurationData(0x%016"PRIx64","
		    "0x%016"PRIx64") ]\n",
		    (uint64_t) cpu->cd.mips.gpr[MIPS_GPR_A0],
		    (uint64_t) cpu->cd.mips.gpr[MIPS_GPR_A1]);  */
		cpu->cd.mips.gpr[MIPS_GPR_V0] = ARCBIOS_EINVAL;
		for (i=0; i<machine->md.arc->n_configuration_data; i++) {
			/*  fatal("configuration_data_component[%i] = "
			    "0x%016"PRIx64"\n", i, (uint64_t) machine->
			    md.arc->configuration_data_component[i]);  */
			if (cpu->cd.mips.gpr[MIPS_GPR_A1] ==
			    machine->md.arc->configuration_data_component[i]) {
				cpu->cd.mips.gpr[MIPS_GPR_V0] = 0;
				for (j=0; j<machine->
				    md.arc->configuration_data_len[i]; j++) {
					unsigned char ch;
					cpu->memory_rw(cpu, cpu->mem,
					    machine->md.arc->
					    configuration_data_configdata[i] +
					    j, &ch, 1, MEM_READ, CACHE_NONE);
					cpu->memory_rw(cpu, cpu->mem,
					    cpu->cd.mips.gpr[MIPS_GPR_A0] + j,
					    &ch, 1, MEM_WRITE, CACHE_NONE);
				}
				break;
			}
		}
		break;
	case 0x3c:		/*  GetComponent(char *name)  */
		debug("[ ARCBIOS GetComponent(\"");
		dump_mem_string(cpu, cpu->cd.mips.gpr[MIPS_GPR_A0]);
		debug("\") ]\n");

		if (cpu->cd.mips.gpr[MIPS_GPR_A0] == 0) {
			fatal("[ ARCBIOS GetComponent: NULL ptr ]\n");
		} else {
			unsigned char buf[500];
			int match_index = -1;
			int match_len = 0;

			memset(buf, 0, sizeof(buf));
			for (i=0; i<(ssize_t)sizeof(buf); i++) {
				cpu->memory_rw(cpu, cpu->mem,
				    cpu->cd.mips.gpr[MIPS_GPR_A0] + i,
				    &buf[i], 1, MEM_READ, CACHE_NONE);
				if (buf[i] == '\0')
					i = sizeof(buf);
			}
			buf[sizeof(buf) - 1] = '\0';

			/*  "scsi(0)disk(0)rdisk(0)partition(0)" and such.  */
			/*  printf("GetComponent(\"%s\")\n", buf);  */

			/*  Default to NULL return value.  */
			cpu->cd.mips.gpr[MIPS_GPR_V0] = 0;

			/*  Scan the string to component table:  */
			for (i=0; i<machine->md.arc->n_string_to_components;
			    i++) {
				int m = 0;
				while (buf[m] && machine->md.arc->
				    string_to_component[i][m] &&
				    machine->md.arc->string_to_component[i][m]
				    == buf[m])
					m++;
				if (m > match_len) {
					match_len = m;
					match_index = i;
				}
			}

			if (match_index >= 0) {
				/*  printf("Longest match: '%s'\n",
				    machine->md.arc->string_to_component[
				    match_index]);  */
				cpu->cd.mips.gpr[MIPS_GPR_V0] =
				    machine->md.arc->string_to_component_value[
				    match_index];
			}
		}
		break;
	case 0x44:		/*  GetSystemId()  */
		debug("[ ARCBIOS GetSystemId() ]\n");
		cpu->cd.mips.gpr[MIPS_GPR_V0] = SGI_SYSID_ADDR;
		break;
	case 0x48:		/*  void *GetMemoryDescriptor(void *ptr)  */
		debug("[ ARCBIOS GetMemoryDescriptor(0x%08x) ]\n",
		    (int)cpu->cd.mips.gpr[MIPS_GPR_A0]);

		/*  If a0=NULL, then return the first descriptor:  */
		if ((uint32_t)cpu->cd.mips.gpr[MIPS_GPR_A0] == 0)
			cpu->cd.mips.gpr[MIPS_GPR_V0] =
			    machine->md.arc->memdescriptor_base;
		else {
			int s = machine->md.arc->arc_64bit?
			    sizeof(struct arcbios_mem64)
			    : sizeof(struct arcbios_mem);
			int nr = cpu->cd.mips.gpr[MIPS_GPR_A0] -
			    machine->md.arc->memdescriptor_base;
			nr /= s;
			nr ++;
			cpu->cd.mips.gpr[MIPS_GPR_V0] =
			    machine->md.arc->memdescriptor_base + s * nr;
			if (nr >= machine->md.arc->n_memdescriptors)
				cpu->cd.mips.gpr[MIPS_GPR_V0] = 0;
		}
		break;
	case 0x50:		/*  GetTime()  */
		debug("[ ARCBIOS GetTime() ]\n");
		cpu->cd.mips.gpr[MIPS_GPR_V0] = 0xffffffff80001000ULL;
		/*  TODO!  */
		break;
	case 0x54:		/*  GetRelativeTime()  */
		debug("[ ARCBIOS GetRelativeTime() ]\n");
		cpu->cd.mips.gpr[MIPS_GPR_V0] = (int64_t)(int32_t)time(NULL);
		break;
	case 0x5c:  /*  Open(char *path, uint32_t mode, uint32_t *fileID)  */
		debug("[ ARCBIOS Open(\"");
		dump_mem_string(cpu, cpu->cd.mips.gpr[MIPS_GPR_A0]);
		debug("\",0x%x,0x%x)", (int)cpu->cd.mips.gpr[MIPS_GPR_A0],
		    (int)cpu->cd.mips.gpr[MIPS_GPR_A1],
		    (int)cpu->cd.mips.gpr[MIPS_GPR_A2]);

		cpu->cd.mips.gpr[MIPS_GPR_V0] = ARCBIOS_ENOENT;

		handle = 3;
		/*  TODO: Starting at 0 would require some updates...  */
		while (machine->md.arc->file_handle_in_use[handle]) {
			handle ++;
			if (handle >= ARC_MAX_HANDLES) {
				cpu->cd.mips.gpr[MIPS_GPR_V0] = ARCBIOS_EMFILE;
				break;
			}
		}

		if (handle >= ARC_MAX_HANDLES) {
			fatal("[ ARCBIOS Open: out of file handles ]\n");
		} else if (cpu->cd.mips.gpr[MIPS_GPR_A0] == 0) {
			fatal("[ ARCBIOS Open: NULL ptr ]\n");
		} else {
			/*
			 *  TODO: This is hardcoded to successfully open
			 *  anything. It is used by the Windows NT SETUPLDR
			 *  program to load stuff from the boot partition.
			 */
			unsigned char *buf;
			CHECK_ALLOCATION(buf = (unsigned char *) malloc(MAX_OPEN_STRINGLEN));
			memset(buf, 0, MAX_OPEN_STRINGLEN);
			for (i=0; i<MAX_OPEN_STRINGLEN; i++) {
				cpu->memory_rw(cpu, cpu->mem,
				    cpu->cd.mips.gpr[MIPS_GPR_A0] + i,
				    &buf[i], 1, MEM_READ, CACHE_NONE);
				if (buf[i] == '\0')
					i = MAX_OPEN_STRINGLEN;
			}
			buf[MAX_OPEN_STRINGLEN - 1] = '\0';
			machine->md.arc->file_handle_string[handle] =
			    (char *)buf;
			machine->md.arc->current_seek_offset[handle] = 0;
			cpu->cd.mips.gpr[MIPS_GPR_V0] = ARCBIOS_ESUCCESS;
		}

		if (cpu->cd.mips.gpr[MIPS_GPR_V0] == ARCBIOS_ESUCCESS) {
			debug(" = handle %i ]\n", (int)handle);
			store_32bit_word(cpu, cpu->cd.mips.gpr[MIPS_GPR_A2],
			    handle);
			machine->md.arc->file_handle_in_use[handle] = 1;
		} else
			debug(" = ERROR %i ]\n",
			    (int)cpu->cd.mips.gpr[MIPS_GPR_V0]);
		break;
	case 0x60:		/*  Close(uint32_t handle)  */
		debug("[ ARCBIOS Close(%i) ]\n",
		    (int)cpu->cd.mips.gpr[MIPS_GPR_A0]);
		if (!machine->md.arc->file_handle_in_use[cpu->cd.mips.gpr[
		    MIPS_GPR_A0]]) {
			fatal("ARCBIOS Close(%i): bad handle\n",
			    (int)cpu->cd.mips.gpr[MIPS_GPR_A0]);
			cpu->cd.mips.gpr[MIPS_GPR_V0] = ARCBIOS_EBADF;
		} else {
			machine->md.arc->file_handle_in_use[
			    cpu->cd.mips.gpr[MIPS_GPR_A0]] = 0;
			if (machine->md.arc->file_handle_string[
			    cpu->cd.mips.gpr[MIPS_GPR_A0]] != NULL)
				free(machine->md.arc->file_handle_string[
				    cpu->cd.mips.gpr[MIPS_GPR_A0]]);
			machine->md.arc->file_handle_string[cpu->cd.mips.
			    gpr[MIPS_GPR_A0]] = NULL;
			cpu->cd.mips.gpr[MIPS_GPR_V0] = ARCBIOS_ESUCCESS;
		}
		break;
	case 0x64:  /*  Read(handle, void *buf, length, uint32_t *count)  */
		if (cpu->cd.mips.gpr[MIPS_GPR_A0] == ARCBIOS_STDIN) {
			int i, nread = 0, a2;
			/*
			 *  Before going into the loop, make sure stdout
			 *  is flushed.  If we're using an X11 VGA console,
			 *  then it needs to be flushed as well.
			 */
			fflush(stdin);
			fflush(stdout);
			/*  NOTE/TODO: This gives a tick to _everything_  */
			for (i=0; i<machine->tick_functions.n_entries; i++)
				machine->tick_functions.f[i](cpu,
				    machine->tick_functions.extra[i]);

			a2 = cpu->cd.mips.gpr[MIPS_GPR_A2];
			for (i=0; i<a2; i++) {
				int x;
				unsigned char ch;

				/*  Read from STDIN is blocking (at least
				    that seems to be how NetBSD's arcdiag
				    wants it)  */
				x = console_readchar(
				    machine->main_console_handle);
				if (x < 0)
					return 0;

				/*
				 *  ESC + '[' should be transformed into 0x9b:
				 *
				 *  NOTE/TODO: This makes the behaviour of just
				 *  pressing ESC a bit harder to define.
				 */
				if (x == 27) {
					x = console_readchar(cpu->
					    machine->main_console_handle);
					if (x == '[' || x == 'O')
						x = 0x9b;
				}

				ch = x;
				nread ++;
				cpu->memory_rw(cpu, cpu->mem,
				    cpu->cd.mips.gpr[MIPS_GPR_A1] + i,
				    &ch, 1, MEM_WRITE, CACHE_NONE);

				/*  NOTE: Only one char, from STDIN:  */
				i = cpu->cd.mips.gpr[MIPS_GPR_A2];  /*  :-)  */
			}
			store_32bit_word(cpu, cpu->cd.mips.gpr[MIPS_GPR_A3],
			    nread);
			/*  TODO: not EAGAIN?  */
			cpu->cd.mips.gpr[MIPS_GPR_V0] =
			    nread? ARCBIOS_ESUCCESS: ARCBIOS_EAGAIN;
		} else {
			int handle = cpu->cd.mips.gpr[MIPS_GPR_A0];
			int disk_type = 0;
			int disk_id = arcbios_handle_to_disk_id_and_type(
			    machine, handle, &disk_type);
			uint64_t partition_offset = 0;
			int res;
			uint64_t size;		/*  dummy  */
			unsigned char *tmp_buf;

			arcbios_handle_to_start_and_size(machine, handle,
			    &partition_offset, &size);

			debug("[ ARCBIOS Read(%i,0x%08x,0x%08x,0x%08x) ]\n",
			    (int)cpu->cd.mips.gpr[MIPS_GPR_A0],
			    (int)cpu->cd.mips.gpr[MIPS_GPR_A1],
			    (int)cpu->cd.mips.gpr[MIPS_GPR_A2],
			    (int)cpu->cd.mips.gpr[MIPS_GPR_A3]);

			CHECK_ALLOCATION(tmp_buf = (unsigned char *)
			    malloc(cpu->cd.mips.gpr[MIPS_GPR_A2]));

			res = diskimage_access(machine, disk_id, disk_type,
			    0, partition_offset + machine->md.arc->
			    current_seek_offset[handle], tmp_buf,
			    cpu->cd.mips.gpr[MIPS_GPR_A2]);

			/*  If the transfer was successful, transfer the
			    data to emulated memory:  */
			if (res) {
				uint64_t dst = cpu->cd.mips.gpr[MIPS_GPR_A1];
				store_buf(cpu, dst, (char *)tmp_buf,
				    cpu->cd.mips.gpr[MIPS_GPR_A2]);
				store_32bit_word(cpu,
				    cpu->cd.mips.gpr[MIPS_GPR_A3],
				    cpu->cd.mips.gpr[MIPS_GPR_A2]);
				machine->md.arc->current_seek_offset[handle] +=
				    cpu->cd.mips.gpr[MIPS_GPR_A2];
				cpu->cd.mips.gpr[MIPS_GPR_V0] = 0;
			} else
				cpu->cd.mips.gpr[MIPS_GPR_V0] = ARCBIOS_EIO;
			free(tmp_buf);
		}
		break;
	case 0x68:		/*  GetReadStatus(handle)  */
		/*
		 *  According to arcbios_tty_getchar() in NetBSD's
		 *  dev/arcbios/arcbios_tty.c, GetReadStatus should
		 *  return 0 if there is something available.
		 *
		 *  TODO: Error codes are things like ARCBIOS_EAGAIN.
		 */
		if (cpu->cd.mips.gpr[MIPS_GPR_A0] == ARCBIOS_STDIN) {
			cpu->cd.mips.gpr[MIPS_GPR_V0] = console_charavail(
			    machine->main_console_handle)? 0 : 1;
		} else {
			fatal("[ ARCBIOS GetReadStatus(%i) from "
			    "something other than STDIN: TODO ]\n",
			    (int)cpu->cd.mips.gpr[MIPS_GPR_A0]);
			/*  TODO  */
			cpu->cd.mips.gpr[MIPS_GPR_V0] = 1;
		}
		break;
	case 0x6c:		/*  Write(handle, buf, len, &returnlen)  */
		if (cpu->cd.mips.gpr[MIPS_GPR_A0] != ARCBIOS_STDOUT) {
			/*
			 *  TODO: this is just a test
			 */
			int handle = cpu->cd.mips.gpr[MIPS_GPR_A0];
			int disk_type = 0;
			int disk_id = arcbios_handle_to_disk_id_and_type(
			    machine, handle, &disk_type);
			uint64_t partition_offset = 0;
			int res, i;
			uint64_t size;		/*  dummy  */
			unsigned char *tmp_buf;

			arcbios_handle_to_start_and_size(machine,
			    handle, &partition_offset, &size);

			debug("[ ARCBIOS Write(%i,0x%08"PRIx64",%i,0x%08"
			    PRIx64") ]\n", (int) cpu->cd.mips.gpr[MIPS_GPR_A0],
			    (uint64_t) cpu->cd.mips.gpr[MIPS_GPR_A1],
			    (int) cpu->cd.mips.gpr[MIPS_GPR_A2],
			    (uint64_t) cpu->cd.mips.gpr[MIPS_GPR_A3]);

			CHECK_ALLOCATION(tmp_buf = (unsigned char *)
			    malloc(cpu->cd.mips.gpr[MIPS_GPR_A2]));

			for (i=0; i<(int32_t)cpu->cd.mips.gpr[MIPS_GPR_A2]; i++)
				cpu->memory_rw(cpu, cpu->mem,
				    cpu->cd.mips.gpr[MIPS_GPR_A1] + i,
				    &tmp_buf[i], sizeof(char), MEM_READ,
				    CACHE_NONE);

			res = diskimage_access(machine, disk_id, disk_type,
			    1, partition_offset + machine->md.arc->
			    current_seek_offset[handle], tmp_buf,
			    cpu->cd.mips.gpr[MIPS_GPR_A2]);

			if (res) {
				store_32bit_word(cpu,
				    cpu->cd.mips.gpr[MIPS_GPR_A3],
				    cpu->cd.mips.gpr[MIPS_GPR_A2]);
				machine->md.arc->current_seek_offset[handle] +=
				    cpu->cd.mips.gpr[MIPS_GPR_A2];
				cpu->cd.mips.gpr[MIPS_GPR_V0] = 0;
			} else
				cpu->cd.mips.gpr[MIPS_GPR_V0] = ARCBIOS_EIO;
			free(tmp_buf);
		} else {
			for (i=0; i<(int32_t)cpu->cd.mips.gpr[MIPS_GPR_A2];
			    i++) {
				unsigned char ch = '\0';
				cpu->memory_rw(cpu, cpu->mem,
				    cpu->cd.mips.gpr[MIPS_GPR_A1] + i,
				    &ch, sizeof(ch), MEM_READ, CACHE_NONE);

				arcbios_putchar(cpu, ch);
			}
		}
		store_32bit_word(cpu, cpu->cd.mips.gpr[MIPS_GPR_A3],
		    cpu->cd.mips.gpr[MIPS_GPR_A2]);
		cpu->cd.mips.gpr[MIPS_GPR_V0] = 0;	/*  Success.  */
		break;
	case 0x70:	/*  Seek(uint32_t handle, int64_t *ofs,
				 uint32_t whence): uint32_t  */
		debug("[ ARCBIOS Seek(%i,0x%08"PRIx64",%i): ",
		    (int) cpu->cd.mips.gpr[MIPS_GPR_A0],
		    (uint64_t)cpu->cd.mips.gpr[MIPS_GPR_A1],
		    (int) cpu->cd.mips.gpr[MIPS_GPR_A2]);

		if (cpu->cd.mips.gpr[MIPS_GPR_A2] != 0) {
			fatal("[ ARCBIOS Seek(%i,0x%08"PRIx64",%i): "
			    "UNIMPLEMENTED whence=%i ]\n",
			    (int) cpu->cd.mips.gpr[MIPS_GPR_A0],
			    (uint64_t) cpu->cd.mips.gpr[MIPS_GPR_A1],
			    (int) cpu->cd.mips.gpr[MIPS_GPR_A2],
			    (int) cpu->cd.mips.gpr[MIPS_GPR_A2]);
		}

		{
			unsigned char buf[8];
			uint64_t ofs;
			cpu->memory_rw(cpu, cpu->mem,
			    cpu->cd.mips.gpr[MIPS_GPR_A1], &buf[0],
			    sizeof(buf), MEM_READ, CACHE_NONE);
			if (cpu->byte_order == EMUL_BIG_ENDIAN) {
				unsigned char tmp;
				tmp = buf[0]; buf[0] = buf[7]; buf[7] = tmp;
				tmp = buf[1]; buf[1] = buf[6]; buf[6] = tmp;
				tmp = buf[2]; buf[2] = buf[5]; buf[5] = tmp;
				tmp = buf[3]; buf[3] = buf[4]; buf[4] = tmp;
			}
			ofs = buf[0] + (buf[1] << 8) + (buf[2] << 16) +
			    (buf[3] << 24) + ((uint64_t)buf[4] << 32) +
			    ((uint64_t)buf[5] << 40) + ((uint64_t)buf[6] << 48)
			    + ((uint64_t)buf[7] << 56);
			machine->md.arc->current_seek_offset[
			    cpu->cd.mips.gpr[MIPS_GPR_A0]] = ofs;
			debug("%016"PRIx64" ]\n", (uint64_t) ofs);
		}

		cpu->cd.mips.gpr[MIPS_GPR_V0] = 0;	/*  Success.  */

		break;
	case 0x78:		/*  GetEnvironmentVariable(char *)  */
		/*  Find the environment variable given by a0:  */
		for (i=0; i<(ssize_t)sizeof(buf); i++)
			cpu->memory_rw(cpu, cpu->mem,
			    cpu->cd.mips.gpr[MIPS_GPR_A0] + i,
			    &buf[i], sizeof(char), MEM_READ, CACHE_NONE);
		buf[sizeof(buf)-1] = '\0';
		debug("[ ARCBIOS GetEnvironmentVariable(\"%s\") ]\n", buf);
		for (i=0; i<0x1000; i++) {
			/*  Matching string at offset i?  */
			int nmatches = 0;
			for (j=0; j<(ssize_t)strlen((char *)buf); j++) {
				cpu->memory_rw(cpu, cpu->mem,
				    (uint64_t)(ARC_ENV_STRINGS + i + j),
				    &ch2, sizeof(char), MEM_READ, CACHE_NONE);
				if (ch2 == buf[j])
					nmatches++;
			}
			cpu->memory_rw(cpu, cpu->mem,
			    (uint64_t)(ARC_ENV_STRINGS + i +
			    strlen((char *)buf)), &ch2, sizeof(char),
			    MEM_READ, CACHE_NONE);
			if (nmatches == (int)strlen((char *)buf) && ch2=='=') {
				cpu->cd.mips.gpr[MIPS_GPR_V0] =
				    ARC_ENV_STRINGS + i +
				    strlen((char *)buf) + 1;
				return 1;
			}
		}
		/*  Return NULL if string wasn't found.  */
		cpu->cd.mips.gpr[MIPS_GPR_V0] = 0;
		break;
	case 0x7c:		/*  SetEnvironmentVariable(char *, char *)  */
		debug("[ ARCBIOS SetEnvironmentVariable(\"");
		dump_mem_string(cpu, cpu->cd.mips.gpr[MIPS_GPR_A0]);
		debug("\",\"");
		dump_mem_string(cpu, cpu->cd.mips.gpr[MIPS_GPR_A1]);
		debug("\") ]\n");
		/*  TODO: This is a dummy.  */
		cpu->cd.mips.gpr[MIPS_GPR_V0] = ARCBIOS_ESUCCESS;
		break;
	case 0x80:		/*  GetFileInformation()  */
		debug("[ ARCBIOS GetFileInformation(%i,0x%x): ",
		    (int)cpu->cd.mips.gpr[MIPS_GPR_A0],
		    (int)cpu->cd.mips.gpr[MIPS_GPR_A1]);

		if (cpu->cd.mips.gpr[MIPS_GPR_A0] >= ARC_MAX_HANDLES) {
			debug("invalid file handle ]\n");
			cpu->cd.mips.gpr[MIPS_GPR_V0] = ARCBIOS_EINVAL;
		} else if (!machine->md.arc->file_handle_in_use[cpu->cd.
		    mips.gpr[MIPS_GPR_A0]]) {
			debug("file handle not in use! ]\n");
			cpu->cd.mips.gpr[MIPS_GPR_V0] = ARCBIOS_EBADF;
		} else {
			debug("'%s' ]\n", machine->md.arc->file_handle_string[
			    cpu->cd.mips.gpr[MIPS_GPR_A0]]);
			cpu->cd.mips.gpr[MIPS_GPR_V0] =
			    arcbios_getfileinformation(cpu);
		}
		break;
	case 0x88:		/*  FlushAllCaches()  */
		debug("[ ARCBIOS FlushAllCaches(): TODO ]\n");
		cpu->cd.mips.gpr[MIPS_GPR_V0] = 0;
		break;
	case 0x90:		/*  void *GetDisplayStatus(handle)  */
		debug("[ ARCBIOS GetDisplayStatus(%i) ]\n",
		    (int)cpu->cd.mips.gpr[MIPS_GPR_A0]);
		/*  TODO:  handle different values of 'handle'?  */
		cpu->cd.mips.gpr[MIPS_GPR_V0] = ARC_DSPSTAT_ADDR;
		break;
	case 0x100:
		/*
		 *  Undocumented, used by IRIX.
		 */
		debug("[ ARCBIOS: IRIX 0x100 (?) ]\n");
		/*  TODO  */
		cpu->cd.mips.gpr[MIPS_GPR_V0] = 0;
		break;
	case 0x888:
		/*
		 *  Magical crash if there is no exception handling code.
		 */
		fatal("EXCEPTION, but no exception handler installed yet.\n");
		quiet_mode = 0;
		cpu_register_dump(machine, cpu, 1, 0x1);
		cpu->running = 0;
		break;
	default:
		quiet_mode = 0;
		cpu_register_dump(machine, cpu, 1, 0x1);
		debug("a0 points to: ");
		dump_mem_string(cpu, cpu->cd.mips.gpr[MIPS_GPR_A0]);
		debug("\n");
		fatal("ARCBIOS: unimplemented vector 0x%x\n", vector);
		cpu->running = 0;
	}

	return 1;
}


/*
 *  arcbios_set_default_exception_handler():
 */
void arcbios_set_default_exception_handler(struct cpu *cpu)
{
	/*
	 *  The default exception handlers simply jump to 0xbfc88888,
	 *  which is then taken care of in arcbios_emul() above.
	 *
	 *  3c1abfc8        lui     k0,0xbfc8
	 *  375a8888        ori     k0,k0,0x8888
	 *  03400008        jr      k0
	 *  00000000        nop
	 */
	store_32bit_word(cpu, 0xffffffff80000000ULL, 0x3c1abfc8);
	store_32bit_word(cpu, 0xffffffff80000004ULL, 0x375a8888);
	store_32bit_word(cpu, 0xffffffff80000008ULL, 0x03400008);
	store_32bit_word(cpu, 0xffffffff8000000cULL, 0x00000000);

	store_32bit_word(cpu, 0xffffffff80000080ULL, 0x3c1abfc8);
	store_32bit_word(cpu, 0xffffffff80000084ULL, 0x375a8888);
	store_32bit_word(cpu, 0xffffffff80000088ULL, 0x03400008);
	store_32bit_word(cpu, 0xffffffff8000008cULL, 0x00000000);

	store_32bit_word(cpu, 0xffffffff80000180ULL, 0x3c1abfc8);
	store_32bit_word(cpu, 0xffffffff80000184ULL, 0x375a8888);
	store_32bit_word(cpu, 0xffffffff80000188ULL, 0x03400008);
	store_32bit_word(cpu, 0xffffffff8000018cULL, 0x00000000);
}


/*
 *  arcbios_add_other_components():
 *
 *  TODO: How should this be synched with the hardware devices
 *  added in machine.c?
 */
static void arcbios_add_other_components(struct machine *machine,
	uint64_t system)
{
	struct cpu *cpu = machine->cpus[0];

	if (machine->machine_type == MACHINE_ARC &&
	    (machine->machine_subtype == MACHINE_ARC_JAZZ_PICA
	    || machine->machine_subtype == MACHINE_ARC_JAZZ_MAGNUM)) {
		uint64_t jazzbus, ali_s3, vxl;
		uint64_t diskcontroller, floppy, kbdctl, kbd;
		uint64_t ptrctl, ptr, paral, audio;
		uint64_t eisa, scsi;
		/*  uint64_t serial1, serial2;  */

		jazzbus = arcbios_addchild_manual(cpu,
		    COMPONENT_CLASS_AdapterClass,
		    COMPONENT_TYPE_MultiFunctionAdapter,
		    0, 1, 2, 0, 0xffffffff, "Jazz-Internal Bus",
		    system, NULL, 0);

		/*
		 *  DisplayController, needed by NetBSD:
		 *  TODO: NetBSD still doesn't use it :(
		 */
		switch (machine->machine_subtype) {
		case MACHINE_ARC_JAZZ_PICA:
			/*  Default TLB entries on PICA-61:  */

			/* 7: 256K, asid: 0x0, v: 0xe1000000,
			   p0: 0xfff00000(2.VG), p1: 0x0(0..G)  */
			mips_coproc_tlb_set_entry(cpu, 7, 262144,
			    0xffffffffe1000000ULL,
			    0x0fff00000ULL, 0, 1, 0, 0, 0, 1, 0, 2, 0);

			/* 8: 64K, asid: 0x0, v: 0xe0000000,
			   p0: 0x80000000(2DVG), p1: 0x0(0..G) */
			mips_coproc_tlb_set_entry(cpu, 8, 65536,
			    0xffffffffe0000000ULL,
			    0x080000000ULL, 0, 1, 0, 1, 0, 1, 0, 2, 0);

			/* 9: 64K, asid: 0x0, v: 0xe00e0000,
			   p0: 0x800e0000(2DVG), p1: 0x800f0000(2DVG) */
			mips_coproc_tlb_set_entry(cpu, 9, 65536,
			    (uint64_t)0xffffffffe00e0000ULL,
			    (uint64_t)0x0800e0000ULL,
			    (uint64_t)0x0800f0000ULL, 1, 1, 1, 1, 1, 0, 2, 2);

			/* 10: 4K, asid: 0x0, v: 0xe0100000,
			   p0: 0xf0000000(2DVG), p1: 0x0(0..G) */
			mips_coproc_tlb_set_entry(cpu, 10, 4096,
			    (uint64_t)0xffffffffe0100000ULL,
			    (uint64_t)0x0f0000000ULL, 0,1, 0, 1, 0, 1, 0, 2, 0);

			/* 11: 1M, asid: 0x0, v: 0xe0200000,
			   p0: 0x60000000(2DVG), p1: 0x60100000(2DVG) */
			mips_coproc_tlb_set_entry(cpu, 11, 1048576,
			    0xffffffffe0200000ULL,
			    0x060000000ULL, 0x060100000ULL,1,1,1,1,1, 0, 2, 2);

			/* 12: 1M, asid: 0x0, v: 0xe0400000,
			   p0: 0x60200000(2DVG), p1: 0x60300000(2DVG) */
			mips_coproc_tlb_set_entry(cpu, 12, 1048576,
			    0xffffffffe0400000ULL, 0x060200000ULL,
			    0x060300000ULL, 1, 1, 1, 1, 1, 0, 2, 2);

			/* 13: 4M, asid: 0x0, v: 0xe0800000,
			   p0: 0x40000000(2DVG), p1: 0x40400000(2DVG) */
			mips_coproc_tlb_set_entry(cpu, 13, 1048576*4,
			    0xffffffffe0800000ULL, 0x040000000ULL,
			    0x040400000ULL, 1, 1, 1, 1, 1, 0, 2, 2);

			/* 14: 16M, asid: 0x0, v: 0xe2000000,
			   p0: 0x90000000(2DVG), p1: 0x91000000(2DVG) */
			mips_coproc_tlb_set_entry(cpu, 14, 1048576*16,
			    0xffffffffe2000000ULL, 0x090000000ULL,
			    0x091000000ULL, 1, 1, 1, 1, 1, 0, 2, 2);

			if (machine->x11_md.in_use) {
				ali_s3 = arcbios_addchild_manual(cpu,
				    COMPONENT_CLASS_ControllerClass,
				    COMPONENT_TYPE_DisplayController,
				    COMPONENT_FLAG_ConsoleOut |
					COMPONENT_FLAG_Output,
				    1, 2, 0, 0xffffffff, "ALI_S3",
				    jazzbus, NULL, 0);

				arcbios_addchild_manual(cpu,
				    COMPONENT_CLASS_PeripheralClass,
				    COMPONENT_TYPE_MonitorPeripheral,
				    COMPONENT_FLAG_ConsoleOut |
					COMPONENT_FLAG_Output,
				    1, 2, 0, 0xffffffff, "1024x768",
				    ali_s3, NULL, 0);
			}
			break;
		case MACHINE_ARC_JAZZ_MAGNUM:
			if (machine->x11_md.in_use) {
				vxl = arcbios_addchild_manual(cpu,
				    COMPONENT_CLASS_ControllerClass,
				    COMPONENT_TYPE_DisplayController,
				    COMPONENT_FLAG_ConsoleOut |
					COMPONENT_FLAG_Output,
				    1, 2, 0, 0xffffffff, "VXL",
				    jazzbus, NULL, 0);

				arcbios_addchild_manual(cpu,
				    COMPONENT_CLASS_PeripheralClass,
				    COMPONENT_TYPE_MonitorPeripheral,
				    COMPONENT_FLAG_ConsoleOut |
					COMPONENT_FLAG_Output,
				    1, 2, 0, 0xffffffff, "1024x768",
				    vxl, NULL, 0);
			}
			break;
		}

		diskcontroller = arcbios_addchild_manual(cpu,
		    COMPONENT_CLASS_ControllerClass,
		    COMPONENT_TYPE_DiskController,
			COMPONENT_FLAG_Input | COMPONENT_FLAG_Output,
		    1, 2, 0, 0xffffffff, "I82077", jazzbus, NULL, 0);

		floppy = arcbios_addchild_manual(cpu,
		    COMPONENT_CLASS_PeripheralClass,
		    COMPONENT_TYPE_FloppyDiskPeripheral,
			COMPONENT_FLAG_Removable |
			COMPONENT_FLAG_Input | COMPONENT_FLAG_Output,
		    1, 2, 0, 0xffffffff, NULL, diskcontroller, NULL, 0);

		kbdctl = arcbios_addchild_manual(cpu,
		    COMPONENT_CLASS_ControllerClass,
		    COMPONENT_TYPE_KeyboardController,
			COMPONENT_FLAG_ConsoleIn | COMPONENT_FLAG_Input,
		    1, 2, 0, 0xffffffff, "I8742", jazzbus, NULL, 0);

		kbd = arcbios_addchild_manual(cpu,
		    COMPONENT_CLASS_PeripheralClass,
		    COMPONENT_TYPE_KeyboardPeripheral,
			COMPONENT_FLAG_ConsoleIn | COMPONENT_FLAG_Input,
		    1, 2, 0, 0xffffffff, "PCAT_ENHANCED", kbdctl, NULL, 0);

		ptrctl = arcbios_addchild_manual(cpu,
		    COMPONENT_CLASS_ControllerClass,
		    COMPONENT_TYPE_PointerController, COMPONENT_FLAG_Input,
		    1, 2, 0, 0xffffffff, "I8742", jazzbus, NULL, 0);

		ptr = arcbios_addchild_manual(cpu,
		    COMPONENT_CLASS_PeripheralClass,
		    COMPONENT_TYPE_PointerPeripheral, COMPONENT_FLAG_Input,
		    1, 2, 0, 0xffffffff, "PS2 MOUSE", ptrctl, NULL, 0);

/*  These cause Windows NT to bug out.  */
#if 0
		serial1 = arcbios_addchild_manual(cpu,
		    COMPONENT_CLASS_ControllerClass,
		    COMPONENT_TYPE_SerialController,
			COMPONENT_FLAG_Input | COMPONENT_FLAG_Output,
		    1, 2, 0, 0xffffffff, "COM1", jazzbus, NULL, 0);

		serial2 = arcbios_addchild_manual(cpu,
		    COMPONENT_CLASS_ControllerClass,
		    COMPONENT_TYPE_SerialController,
			COMPONENT_FLAG_Input | COMPONENT_FLAG_Output,
		    1, 2, 0, 0xffffffff, "COM1", jazzbus, NULL, 0);
#endif

		paral = arcbios_addchild_manual(cpu,
		    COMPONENT_CLASS_ControllerClass,
		    COMPONENT_TYPE_ParallelController,
			COMPONENT_FLAG_Input | COMPONENT_FLAG_Output,
		    1, 2, 0, 0xffffffff, "LPT1", jazzbus, NULL, 0);

		audio = arcbios_addchild_manual(cpu,
		    COMPONENT_CLASS_ControllerClass,
		    COMPONENT_TYPE_AudioController,
			COMPONENT_FLAG_Input | COMPONENT_FLAG_Output,
		    1, 2, 0, 0xffffffff, "MAGNUM", jazzbus, NULL, 0);

		eisa = arcbios_addchild_manual(cpu,
		    COMPONENT_CLASS_AdapterClass, COMPONENT_TYPE_EISAAdapter,
		    0, 1, 2, 0, 0xffffffff, "EISA", system, NULL, 0);

		{
			unsigned char config[78];
			memset(config, 0, sizeof(config));

/*  config data version: 1, revision: 2, count: 4  */
config[0] = 0x01; config[1] = 0x00;
config[2] = 0x02; config[3] = 0x00;
config[4] = 0x04; config[5] = 0x00; config[6] = 0x00; config[7] = 0x00;

/*
          type: Interrupt
           share_disposition: DeviceExclusive, flags: LevelSensitive
           level: 4, vector: 22, reserved1: 0
*/
			config[8] = arc_CmResourceTypeInterrupt;
			config[9] = arc_CmResourceShareDeviceExclusive;
			config[10] = arc_CmResourceInterruptLevelSensitive;
			config[12] = 4;
			config[16] = 22;
			config[20] = 0;

/*
          type: Memory
           share_disposition: DeviceExclusive, flags: ReadWrite
           start: 0x 0 80002000, length: 0x1000
*/
			config[24] = arc_CmResourceTypeMemory;
			config[25] = arc_CmResourceShareDeviceExclusive;
			config[26] = arc_CmResourceMemoryReadWrite;
config[28] = 0x00; config[29] = 0x20; config[30] = 0x00; config[31] = 0x80;
  config[32] = 0x00; config[33] = 0x00; config[34] = 0x00; config[35] = 0x00;
config[36] = 0x00; config[37] = 0x10; config[38] = 0x00; config[39] = 0x00;

/*
          type: DMA
           share_disposition: DeviceExclusive, flags: 0x0
           channel: 0, port: 0, reserved1: 0
*/
			config[40] = arc_CmResourceTypeDMA;
			config[41] = arc_CmResourceShareDeviceExclusive;
/*  42..43 = flags, 44,45,46,47 = channel, 48,49,50,51 = port, 52,53,54,55
 = reserved  */

/*          type: DeviceSpecific
           share_disposition: DeviceExclusive, flags: 0x0
           datasize: 6, reserved1: 0, reserved2: 0
           data: [0x1:0x0:0x2:0x0:0x7:0x30]
*/
			config[56] = arc_CmResourceTypeDeviceSpecific;
			config[57] = arc_CmResourceShareDeviceExclusive;
/*  58,59 = flags  60,61,62,63 = data size, 64..71 = reserved  */
			config[60] = 6;
/*  72..77 = the data  */
			config[72] = 0x01; config[73] = 0x00; config[74] = 0x02;
			config[75] = 0x00; config[76] = 0x07; config[77] = 0x30;
			scsi = arcbios_addchild_manual(cpu,
			    COMPONENT_CLASS_AdapterClass,
			    COMPONENT_TYPE_SCSIAdapter,
			    0, 1, 2, 0, 0xffffffff, "ESP216",
			    system, config, sizeof(config));

			arcbios_register_scsicontroller(machine, scsi);
		}
	}
}


/*
 *  arcbios_console_init():
 *
 *  Called from machine.c whenever an ARC-based machine is running with
 *  a graphical VGA-style framebuffer, which can be used as console.
 */
void arcbios_console_init(struct machine *machine,
	uint64_t vram, uint64_t ctrlregs)
{
	if (machine->md.arc == NULL) {
		CHECK_ALLOCATION(machine->md.arc = (struct machine_arcbios *)
		    malloc(sizeof(struct machine_arcbios)));
		memset(machine->md.arc, 0, sizeof(struct machine_arcbios));
	}

	machine->md.arc->vgaconsole = 1;

	machine->md.arc->console_vram = vram;
	machine->md.arc->console_ctrlregs = ctrlregs;
	machine->md.arc->console_maxx = ARC_CONSOLE_MAX_X;
	machine->md.arc->console_maxy = ARC_CONSOLE_MAX_Y;
	machine->md.arc->in_escape_sequence = 0;
	machine->md.arc->escape_sequence[0] = '\0';
}


/*
 *  arc_environment_setup():
 *
 *  Initialize the emulated environment variables.
 */
static void arc_environment_setup(struct machine *machine, int is64bit,
	const char *primary_ether_addr)
{
	size_t bootpath_len = 500;
	char *init_bootpath;
	uint64_t addr, addr2;
	struct cpu *cpu = machine->cpus[0];

	/*
	 *  Boot string in ARC format:
	 *
	 *  TODO: How about floppies? multi()disk()fdisk()
	 *        Is tftp() good for netbooting?
	 */
	CHECK_ALLOCATION(init_bootpath = (char *) malloc(bootpath_len));
	init_bootpath[0] = '\0';

	if (machine->bootdev_id < 0 || machine->force_netboot) {
		snprintf(init_bootpath, bootpath_len, "tftp()");
	} else {
		/*  TODO: Make this nicer.  */
		if (machine->machine_type == MACHINE_SGI) {
			if (machine->machine_subtype == 30)
				strlcat(init_bootpath, "xio(0)pci(15)",
				    bootpath_len);
			if (machine->machine_subtype == 32)
				strlcat(init_bootpath, "pci(0)",
				    bootpath_len);
		}

		if (diskimage_is_a_cdrom(machine, machine->bootdev_id,
		    machine->bootdev_type))
			snprintf(init_bootpath + strlen(init_bootpath),
			    bootpath_len - strlen(init_bootpath),
			    "scsi(0)cdrom(%i)fdisk(0)", machine->bootdev_id);
		else
			snprintf(init_bootpath + strlen(init_bootpath),
			    bootpath_len - strlen(init_bootpath),
			    "scsi(0)disk(%i)rdisk(0)partition(1)",
			    machine->bootdev_id);
	}

	if (machine->machine_type == MACHINE_ARC)
		strlcat(init_bootpath, "\\", bootpath_len);

	CHECK_ALLOCATION(machine->bootstr = (char *) malloc(ARC_BOOTSTR_BUFLEN));

	strlcpy(machine->bootstr, init_bootpath, ARC_BOOTSTR_BUFLEN);
	if (strlcat(machine->bootstr, machine->boot_kernel_filename,
	    ARC_BOOTSTR_BUFLEN) >= ARC_BOOTSTR_BUFLEN) {
		fprintf(stderr, "boot string too long?\n");
		exit(1);
	}

	/*  Boot args., eg "-a"  */
	machine->bootarg = machine->boot_string_argument;

	/*  argc, argv, envp in a0, a1, a2:  */
	cpu->cd.mips.gpr[MIPS_GPR_A0] = 0; /*  note: argc is increased later  */

	/*  TODO:  not needed?  */
	cpu->cd.mips.gpr[MIPS_GPR_SP] = (int64_t)(int32_t)
	    (machine->physical_ram_in_mb * 1048576 + 0x80000000 - 0x2080);

	/*  Set up argc/argv:  */
	addr = ARC_ENV_STRINGS;
	addr2 = ARC_ARGV_START;
	cpu->cd.mips.gpr[MIPS_GPR_A1] = addr2;

	/*  bootstr:  */
	store_pointer_and_advance(cpu, &addr2, addr, is64bit);
	add_environment_string(cpu, machine->bootstr, &addr);
	cpu->cd.mips.gpr[MIPS_GPR_A0] ++;

	/*  bootarg:  */
	if (machine->bootarg[0] != '\0') {
		store_pointer_and_advance(cpu, &addr2, addr, is64bit);
		add_environment_string(cpu, machine->bootarg, &addr);
		cpu->cd.mips.gpr[MIPS_GPR_A0] ++;
	}

	cpu->cd.mips.gpr[MIPS_GPR_A2] = addr2;

	/*
	 *  Add environment variables.  For each variable, add it
	 *  as a string using add_environment_string(), and add a
	 *  pointer to it to the ARC_ENV_POINTERS array.
	 */
	if (machine->x11_md.in_use) {
		if (machine->machine_type == MACHINE_ARC) {
			store_pointer_and_advance(cpu, &addr2, addr, is64bit);
			add_environment_string(cpu,
			    "CONSOLEIN=multi()key()keyboard()console()", &addr);
			store_pointer_and_advance(cpu, &addr2, addr, is64bit);
			add_environment_string(cpu,
			    "CONSOLEOUT=multi()video()monitor()console()",
			    &addr);
		} else {
			store_pointer_and_advance(cpu, &addr2, addr, is64bit);
			add_environment_string(cpu, "ConsoleIn=keyboard()",
			    &addr);
			store_pointer_and_advance(cpu, &addr2, addr, is64bit);
			add_environment_string(cpu, "ConsoleOut=video()",
			    &addr);

			/*  g for graphical mode. G for graphical mode
			    with SGI logo visible on Irix?  */
			store_pointer_and_advance(cpu, &addr2, addr, is64bit);
			add_environment_string(cpu, "console=g", &addr);
		}
	} else {
		if (machine->machine_type == MACHINE_ARC) {
			/*  TODO: serial console for ARC?  */
			store_pointer_and_advance(cpu, &addr2, addr, is64bit);
			add_environment_string(cpu,
			    "CONSOLEIN=multi()serial(0)", &addr);
			store_pointer_and_advance(cpu, &addr2, addr, is64bit);
			add_environment_string(cpu,
			    "CONSOLEOUT=multi()serial(0)", &addr);
		} else {
			store_pointer_and_advance(cpu, &addr2, addr, is64bit);
			add_environment_string(cpu, "ConsoleIn=serial(0)",
			    &addr);
			store_pointer_and_advance(cpu, &addr2, addr, is64bit);
			add_environment_string(cpu, "ConsoleOut=serial(0)",
			    &addr);

			/*  'd' or 'd2' in Irix, 'ttyS0' in Linux?  */
			store_pointer_and_advance(cpu, &addr2, addr, is64bit);
			add_environment_string(cpu, "console=d", &addr);
		}
	}

	if (machine->machine_type == MACHINE_SGI) {
		store_pointer_and_advance(cpu, &addr2, addr, is64bit);
		add_environment_string(cpu, "AutoLoad=No", &addr);
		store_pointer_and_advance(cpu, &addr2, addr, is64bit);
		add_environment_string(cpu, "diskless=0", &addr);
		store_pointer_and_advance(cpu, &addr2, addr, is64bit);
		add_environment_string(cpu, "volume=80", &addr);
		store_pointer_and_advance(cpu, &addr2, addr, is64bit);
		add_environment_string(cpu, "sgilogo=y", &addr);

		store_pointer_and_advance(cpu, &addr2, addr, is64bit);
		add_environment_string(cpu, "monitor=h", &addr);
		store_pointer_and_advance(cpu, &addr2, addr, is64bit);
		add_environment_string(cpu, "TimeZone=GMT", &addr);
		store_pointer_and_advance(cpu, &addr2, addr, is64bit);
		add_environment_string(cpu, "nogfxkbd=1", &addr);

		/*  TODO: 'xio(0)pci(15)scsi(0)disk(1)rdisk(0)partition(0)'
		    on IP30 at least  */

		store_pointer_and_advance(cpu, &addr2, addr, is64bit);
		add_environment_string(cpu,
		    "SystemPartition=pci(0)scsi(0)disk(2)rdisk(0)partition(8)",
		    &addr);
		store_pointer_and_advance(cpu, &addr2, addr, is64bit);
		add_environment_string(cpu,
		    "OSLoadPartition=pci(0)scsi(0)disk(2)rdisk(0)partition(0)",
		    &addr);
		store_pointer_and_advance(cpu, &addr2, addr, is64bit);
		add_environment_string(cpu, "OSLoadFilename=/unix", &addr);
		store_pointer_and_advance(cpu, &addr2, addr, is64bit);
		add_environment_string(cpu, "OSLoader=sash", &addr);

		store_pointer_and_advance(cpu, &addr2, addr, is64bit);
		add_environment_string(cpu, "rbaud=9600", &addr);
		store_pointer_and_advance(cpu, &addr2, addr, is64bit);
		add_environment_string(cpu, "rebound=y", &addr);
		store_pointer_and_advance(cpu, &addr2, addr, is64bit);
		add_environment_string(cpu, "crt_option=1", &addr);
		store_pointer_and_advance(cpu, &addr2, addr, is64bit);
		add_environment_string(cpu, "netaddr=10.0.0.1", &addr);

		store_pointer_and_advance(cpu, &addr2, addr, is64bit);
		add_environment_string(cpu, "keybd=US", &addr);

		store_pointer_and_advance(cpu, &addr2, addr, is64bit);
		add_environment_string(cpu, "cpufreq=3", &addr);
		store_pointer_and_advance(cpu, &addr2, addr, is64bit);
		add_environment_string(cpu, "dbaud=9600", &addr);
		store_pointer_and_advance(cpu, &addr2, addr, is64bit);
		add_environment_string(cpu, primary_ether_addr, &addr);
		store_pointer_and_advance(cpu, &addr2, addr, is64bit);
		add_environment_string(cpu, "verbose=istrue", &addr);
		store_pointer_and_advance(cpu, &addr2, addr, is64bit);
		add_environment_string(cpu, "showconfig=istrue", &addr);
		store_pointer_and_advance(cpu, &addr2, addr, is64bit);
		add_environment_string(cpu, "diagmode=v", &addr);
		store_pointer_and_advance(cpu, &addr2, addr, is64bit);
		add_environment_string(cpu, "kernname=unix", &addr);
	} else {
		char *tmp;
		size_t mlen = ARC_BOOTSTR_BUFLEN;
		CHECK_ALLOCATION(tmp = (char *) malloc(mlen));
		snprintf(tmp, mlen, "OSLOADOPTIONS=%s", machine->bootarg);

		store_pointer_and_advance(cpu, &addr2, addr, is64bit);
		add_environment_string(cpu, tmp, &addr);

		store_pointer_and_advance(cpu, &addr2, addr, is64bit);
		snprintf(tmp, mlen,
		    "OSLOADPARTITION=scsi(0)disk(%d)rdisk(0)partition(1)",
		    machine->bootdev_id);
		add_environment_string(cpu, tmp, &addr);
		free(tmp);
	}

	/*  End the environment strings with an empty zero-terminated
	    string, and the envp array with a NULL pointer.  */
	add_environment_string(cpu, "", &addr);	/*  the end  */
	store_pointer_and_advance(cpu, &addr2, 0, is64bit);

	/*  Return address:  (0x20 = ReturnFromMain())  */
	cpu->cd.mips.gpr[MIPS_GPR_RA] = ARC_FIRMWARE_ENTRIES + 0x20;
}


/*
 *  arcbios_init():
 *
 *  Should be called before any other arcbios function is used. An exception
 *  is arcbios_console_init(), which may be called before this function.
 *
 *  TODO: Refactor; this is too long.
 */
void arcbios_init(struct machine *machine, int is64bit, uint64_t sgi_ram_offset,
	const char *primary_ether_addr, uint8_t *primary_ether_macaddr)
{
	int i, alloclen = 20;
	char *name;
	uint64_t arc_reserved, mem_base, mem_count;
	struct cpu *cpu = machine->cpus[0];
	struct arcbios_sysid arcbios_sysid;
	struct arcbios_dsp_stat arcbios_dsp_stat;
	uint64_t system = 0;
	struct arcbios_spb arcbios_spb;
	struct arcbios_spb_64 arcbios_spb_64;

	if (machine->md.arc == NULL) {
		CHECK_ALLOCATION(machine->md.arc = (struct machine_arcbios *)
		    malloc(sizeof(struct machine_arcbios)));
		memset(machine->md.arc, 0, sizeof(struct machine_arcbios));
	}

	machine->md.arc->arc_64bit = is64bit;
	machine->md.arc->wordlen = is64bit? sizeof(uint64_t) : sizeof(uint32_t);

	machine->md.arc->next_component_address = FIRST_ARC_COMPONENT;
	machine->md.arc->configuration_data_next_addr = ARC_CONFIG_DATA_ADDR;

	if (machine->physical_ram_in_mb < 16)
		fprintf(stderr, "WARNING! The ARC platform specification "
		    "doesn't allow less than 16 MB of RAM. Continuing "
		    "anyway.\n");

	/*  File handles 0, 1, and 2 are stdin, stdout, and stderr.  */
	for (i=0; i<ARC_MAX_HANDLES; i++) {
		machine->md.arc->file_handle_in_use[i] = i<3? 1 : 0;
		machine->md.arc->file_handle_string[i] = i>=3? NULL :
		    (i==0? (char*)"(stdin)" : (i==1? (char*)"(stdout)" : (char*)"(stderr)"));
		machine->md.arc->current_seek_offset[i] = 0;
	}

	if (!machine->x11_md.in_use)
		machine->md.arc->vgaconsole = 0;

	if (machine->md.arc->vgaconsole) {
		char tmpstr[100];
		int x, y;

		machine->md.arc->console_curcolor = 0x1f;
		for (y=0; y<machine->md.arc->console_maxy; y++)
			for (x=0; x<machine->md.arc->console_maxx; x++)
				arcbios_putcell(cpu, ' ', x, y);

		machine->md.arc->console_curx = 0;
		machine->md.arc->console_cury = 0;

		arcbios_putstring(cpu, "GXemul "VERSION"  ARCBIOS emulation\n");

		snprintf(tmpstr, sizeof(tmpstr), "%i cpu%s (%s), %i MB "
		    "memory\n\n", machine->ncpus, machine->ncpus > 1? "s" : "",
		    cpu->cd.mips.cpu_type.name,
		    machine->physical_ram_in_mb);
		arcbios_putstring(cpu, tmpstr);
	}

	arcbios_set_default_exception_handler(cpu);

	memset(&arcbios_sysid, 0, sizeof(arcbios_sysid));
	if (machine->machine_type == MACHINE_SGI) {
		/*  Vendor ID, max 8 chars:  */
		strncpy(arcbios_sysid.VendorId,  "SGI", 3);
		switch (machine->machine_subtype) {
		case 22:
			strncpy(arcbios_sysid.ProductId,
			    "87654321", 8);	/*  some kind of ID?  */
			break;
		case 32:
			strncpy(arcbios_sysid.ProductId, "8", 1);
			    /*  6 or 8 (?)  */
			break;
		default:
			snprintf(arcbios_sysid.ProductId, 8, "IP%i",
			    machine->machine_subtype);
		}
	} else {
		switch (machine->machine_subtype) {
		case MACHINE_ARC_JAZZ_PICA:
			strncpy(arcbios_sysid.VendorId,  "MIPS MAG", 8);
			strncpy(arcbios_sysid.ProductId, "ijkl", 4);
			break;
		case MACHINE_ARC_JAZZ_MAGNUM:
			strncpy(arcbios_sysid.VendorId,  "MIPS MAG", 8);
			strncpy(arcbios_sysid.ProductId, "ijkl", 4);
			break;
		default:
			fatal("error in machine.c sysid\n");
			exit(1);
		}
	}

	store_buf(cpu, SGI_SYSID_ADDR, (char *)&arcbios_sysid,
	    sizeof(arcbios_sysid));

	arcbios_get_dsp_stat(cpu, &arcbios_dsp_stat);
	store_buf(cpu, ARC_DSPSTAT_ADDR, (char *)&arcbios_dsp_stat,
	    sizeof(arcbios_dsp_stat));

	/*
	 *  The first 12 MBs of RAM are simply reserved... this simplifies
	 *  things a lot.  If there's more than 512MB of RAM, it has to be
	 *  split in two, according to the ARC spec.  This code creates a
	 *  number of chunks of at most 512MB each.
	 *
	 *  NOTE:  The region of physical address space between 0x10000000 and
	 *  0x1fffffff (256 - 512 MB) is usually occupied by memory mapped
	 *  devices, so that portion is "lost".
	 */
	machine->md.arc->memdescriptor_base = ARC_MEMDESC_ADDR;

	arc_reserved = 0x2000;
	if (machine->machine_type == MACHINE_SGI)
		arc_reserved = 0x4000;

	arcbios_add_memory_descriptor(cpu, 0, arc_reserved,
	    ARCBIOS_MEM_FirmwarePermanent);
	arcbios_add_memory_descriptor(cpu, sgi_ram_offset + arc_reserved,
	    0x60000-arc_reserved, ARCBIOS_MEM_FirmwareTemporary);

	mem_base = 12;
	mem_base += sgi_ram_offset / 1048576;

	while (mem_base < machine->physical_ram_in_mb+sgi_ram_offset/1048576) {
		mem_count = machine->physical_ram_in_mb+sgi_ram_offset/1048576
		    - mem_base;

		/*  Skip the 256-512MB region (for devices)  */
		if (mem_base < 256 && mem_base + mem_count > 256) {
			mem_count = 256-mem_base;
		}

		/*  At most 512MB per descriptor (at least the first 512MB
		    must be separated this way, according to the ARC spec)  */
		if (mem_count > 512)
			mem_count = 512;

		arcbios_add_memory_descriptor(cpu, mem_base * 1048576,
		    mem_count * 1048576, ARCBIOS_MEM_FreeMemory);

		mem_base += mem_count;

		/*  Skip the devices:  */
		if (mem_base == 256)
			mem_base = 512;
	}

	/*
	 *  Components:   (this is an example of what a system could look like)
	 *
	 *  [System]
	 *	[CPU]  (one for each cpu)
	 *	    [FPU]  (one for each cpu)
	 *	    [CPU Caches]
	 *	[Memory]
	 *	[Ethernet]
	 *	[Serial]
	 *	[SCSI]
	 *	    [Disk]
	 *
	 *  Here's a good list of what hardware is in different IP-models:
	 *  http://www.linux-mips.org/archives/linux-mips/2001-03/msg00101.html
	 */

	if (machine->machine_name == NULL)
		fatal("ERROR: machine_name == NULL\n");

	/*  Add the root node:  */
	switch (machine->machine_type) {
	case MACHINE_SGI:
		CHECK_ALLOCATION(name = (char *) malloc(alloclen));
		snprintf(name, alloclen, "SGI-IP%i",
		    machine->machine_subtype);

		/*  A very special case for IP24 (which identifies itself
		    as an IP22):  */
		if (machine->machine_subtype == 24)
			snprintf(name, alloclen, "SGI-IP22");
		break;
	case MACHINE_ARC:
		/*  ARC:  */
		switch (machine->machine_subtype) {
		case MACHINE_ARC_JAZZ_PICA:
			name = strdup("PICA-61");
			break;
		case MACHINE_ARC_JAZZ_MAGNUM:
			name = strdup("Microsoft-Jazz");
			break;
		default:
			fatal("Unimplemented ARC machine type %i\n",
			    machine->machine_subtype);
			exit(1);
		}
		break;
	default:
		fatal("ERROR: non-SGI and non-ARC?\n");
		exit(1);
	}

	system = arcbios_addchild_manual(cpu, COMPONENT_CLASS_SystemClass,
	    COMPONENT_TYPE_ARC, 0,1,2,0, 0xffffffff, name, 0/*ROOT*/, NULL, 0);
	debug("ARC system @ 0x%"PRIx64" (\"%s\")\n", (uint64_t) system, name);


	/*
	 *  Add tree nodes for CPUs and their caches:
	 */

	for (i=0; i<machine->ncpus; i++) {
		uint64_t cpuaddr, fpu=0, picache, pdcache, sdcache=0;
		int cache_size, cache_line_size;
		unsigned int jj;
		char arc_cpu_name[100];
		char arc_fpc_name[105];

		snprintf(arc_cpu_name, sizeof(arc_cpu_name),
		    "MIPS-%s", machine->cpu_name);

		arc_cpu_name[sizeof(arc_cpu_name)-1] = 0;
		for (jj=0; jj<strlen(arc_cpu_name); jj++)
			if (arc_cpu_name[jj] >= 'a' && arc_cpu_name[jj] <= 'z')
				arc_cpu_name[jj] += ('A' - 'a');

		strlcpy(arc_fpc_name, arc_cpu_name, sizeof(arc_fpc_name));
		strlcat(arc_fpc_name, "FPC", sizeof(arc_fpc_name));

		cpuaddr = arcbios_addchild_manual(cpu,
		    COMPONENT_CLASS_ProcessorClass, COMPONENT_TYPE_CPU,
		    0, 1, 2, i, 0xffffffff, arc_cpu_name, system, NULL, 0);

		/*
		 *  TODO: This was in the ARC specs, but it isn't really used
		 *  by ARC implementations?   At least SGI-IP32 uses it.
		 */
		if (machine->machine_type == MACHINE_SGI)
			fpu = arcbios_addchild_manual(cpu,
			    COMPONENT_CLASS_ProcessorClass, COMPONENT_TYPE_FPU,
			    0, 1, 2, 0, 0xffffffff, arc_fpc_name, cpuaddr,
			    NULL, 0);

		cache_size = DEFAULT_PCACHE_SIZE - 12;
		if (cpu->cd.mips.cache_picache)
			cache_size = cpu->cd.mips.cache_picache - 12;
		if (cache_size < 0)
			cache_size = 0;

		cache_line_size = DEFAULT_PCACHE_LINESIZE;
		if (cpu->cd.mips.cache_picache_linesize)
			cache_line_size = cpu->cd.mips.cache_picache_linesize;
		if (cache_line_size < 0)
			cache_line_size = 0;

		picache = arcbios_addchild_manual(cpu,
		    COMPONENT_CLASS_CacheClass, COMPONENT_TYPE_PrimaryICache,
		    0, 1, 2,
		    /*
		     *  Key bits:  0xXXYYZZZZ
		     *  XX is refill-size.
		     *  Cache line size is 1 << YY,
		     *  Cache size is 4KB << ZZZZ.
		     */
		    0x01000000 + (cache_line_size << 16) + cache_size,
			/*  32 bytes per line, default = 32 KB total  */
		    0xffffffff, NULL, cpuaddr, NULL, 0);

		cache_size = DEFAULT_PCACHE_SIZE - 12;
		if (cpu->cd.mips.cache_pdcache)
			cache_size = cpu->cd.mips.cache_pdcache - 12;
		if (cache_size < 0)
			cache_size = 0;

		cache_line_size = DEFAULT_PCACHE_LINESIZE;
		if (cpu->cd.mips.cache_pdcache_linesize)
			cache_line_size = cpu->cd.mips.cache_pdcache_linesize;
		if (cache_line_size < 0)
			cache_line_size = 0;

		pdcache = arcbios_addchild_manual(cpu,
		    COMPONENT_CLASS_CacheClass,
		    COMPONENT_TYPE_PrimaryDCache, 0, 1, 2,
		    /*
		     *  Key bits:  0xYYZZZZ
		     *  Cache line size is 1 << YY,
		     *  Cache size is 4KB << ZZZZ.
		     */
		    0x01000000 + (cache_line_size << 16) + cache_size,
			/*  32 bytes per line, default = 32 KB total  */
		    0xffffffff, NULL, cpuaddr, NULL, 0);

		if (cpu->cd.mips.cache_secondary >= 12) {
			cache_size = cpu->cd.mips.cache_secondary - 12;

			cache_line_size = 6;	/*  64 bytes default  */
			if (cpu->cd.mips.cache_secondary_linesize)
				cache_line_size = cpu->cd.mips.
				    cache_secondary_linesize;
			if (cache_line_size < 0)
				cache_line_size = 0;

			sdcache = arcbios_addchild_manual(cpu,
			    COMPONENT_CLASS_CacheClass,
			    COMPONENT_TYPE_SecondaryDCache, 0, 1, 2,
			    /*
			     *  Key bits:  0xYYZZZZ
			     *  Cache line size is 1 << YY,
			     *  Cache size is 4KB << ZZZZ.
			     */
			    0x01000000 + (cache_line_size << 16) + cache_size,
				/*  64 bytes per line, default = 1 MB total  */
			    0xffffffff, NULL, cpuaddr, NULL, 0);
		}

		debug("ARC cpu%i @ 0x%"PRIx64, i, (uint64_t) cpuaddr);

		if (fpu != 0)
			debug(" (fpu @ 0x%"PRIx64")\n", (uint64_t) fpu);
		else
			debug("\n");

		debug("    picache @ 0x%"PRIx64", pdcache @ 0x%"PRIx64"\n",
		    (uint64_t) picache, (uint64_t) pdcache);

		if (cpu->cd.mips.cache_secondary >= 12)
			debug("    sdcache @ 0x%"PRIx64"\n",
			    (uint64_t) sdcache);

		if (machine->machine_type == MACHINE_SGI) {
			/*  TODO:  Memory amount (and base address?)!  */
			uint64_t memory = arcbios_addchild_manual(cpu,
			    COMPONENT_CLASS_MemoryClass,
			    COMPONENT_TYPE_MemoryUnit, 0, 1, 2, 0,
			    0xffffffff, "memory", cpuaddr, NULL, 0);
			debug("ARC memory @ 0x%"PRIx64"\n", (uint64_t) memory);
		}
	}


	/*
	 *  Add other components:
	 *
	 *  TODO: How should this be synched with the hardware devices
	 *  added in machine.c?
	 */

	arcbios_add_other_components(machine, system);


	/*
	 *  Defalt TLB entry for 64-bit SGI machines:
	 */
	if (machine->machine_type == MACHINE_SGI &&
	    machine->machine_subtype != 12 /* TODO: ugly */ ) {
		/*  TODO: On which models is this required?  */
		mips_coproc_tlb_set_entry(cpu, 0, 1048576*16,
		    0xc000000000000000ULL, 0, 1048576*16, 1,1,1,1,1, 0, 2, 2);
	}


	/*
	 *  Set up Firmware Vectors:
	 */
	add_symbol_name(&machine->symbol_context,
	    ARC_FIRMWARE_ENTRIES, 0x10000, "[ARCBIOS entry]", 0, 1);

	for (i=0; i<100; i++) {
		if (is64bit) {
			store_64bit_word(cpu, ARC_FIRMWARE_VECTORS + i*8,
			    ARC_FIRMWARE_ENTRIES + i*8);
			store_64bit_word(cpu, ARC_PRIVATE_VECTORS + i*8,
			    ARC_PRIVATE_ENTRIES + i*8);

			/*  "Magic trap" instruction:  */
			store_32bit_word(cpu, ARC_FIRMWARE_ENTRIES + i*8,
			    0x00c0de0c);
			store_32bit_word(cpu, ARC_PRIVATE_ENTRIES + i*8,
			    0x00c0de0c);
		} else {
			store_32bit_word(cpu, ARC_FIRMWARE_VECTORS + i*4,
			    ARC_FIRMWARE_ENTRIES + i*4);
			store_32bit_word(cpu, ARC_PRIVATE_VECTORS + i*4,
			    ARC_PRIVATE_ENTRIES + i*4);

			/*  "Magic trap" instruction:  */
			store_32bit_word(cpu, ARC_FIRMWARE_ENTRIES + i*4,
			    0x00c0de0c);
			store_32bit_word(cpu, ARC_PRIVATE_ENTRIES + i*4,
			    0x00c0de0c);
		}
	}


	/*
	 *  Set up the ARC SPD:
	 */
	if (is64bit) {
		/*  ARCS64 SPD (TODO: This is just a guess)  */
		memset(&arcbios_spb_64, 0, sizeof(arcbios_spb_64));
		store_64bit_word_in_host(cpu, (unsigned char *)
		    &arcbios_spb_64.SPBSignature, ARCBIOS_SPB_SIGNATURE);
		store_16bit_word_in_host(cpu, (unsigned char *)
		    &arcbios_spb_64.Version, 64);
		store_16bit_word_in_host(cpu, (unsigned char *)
		    &arcbios_spb_64.Revision, 0);
		store_64bit_word_in_host(cpu, (unsigned char *)
		    &arcbios_spb_64.FirmwareVector, ARC_FIRMWARE_VECTORS);
		store_buf(cpu, SGI_SPB_ADDR, (char *)&arcbios_spb_64, 
		    sizeof(arcbios_spb_64));
	} else {
		/*  ARCBIOS SPB:  (For ARC and 32-bit SGI modes)  */
		memset(&arcbios_spb, 0, sizeof(arcbios_spb));
		store_32bit_word_in_host(cpu, (unsigned char *)
		    &arcbios_spb.SPBSignature, ARCBIOS_SPB_SIGNATURE);
		store_32bit_word_in_host(cpu, (unsigned char *)   
		    &arcbios_spb.SPBLength, sizeof(arcbios_spb));     
		store_16bit_word_in_host(cpu, (unsigned char *)   
		    &arcbios_spb.Version, 1);
		store_16bit_word_in_host(cpu, (unsigned char *)
		    &arcbios_spb.Revision, machine->machine_type ==
		    MACHINE_SGI? 10 : 2);
		store_32bit_word_in_host(cpu, (unsigned char *)
		    &arcbios_spb.FirmwareVector, ARC_FIRMWARE_VECTORS);
		store_32bit_word_in_host(cpu, (unsigned char *)
		    &arcbios_spb.FirmwareVectorLength, 100 * 4);    /*  ?  */
		store_32bit_word_in_host(cpu, (unsigned char *)
		    &arcbios_spb.PrivateVector, ARC_PRIVATE_VECTORS);
		store_32bit_word_in_host(cpu, (unsigned char *)  
		    &arcbios_spb.PrivateVectorLength, 100 * 4);     /*  ?  */
		store_buf(cpu, SGI_SPB_ADDR, (char *)&arcbios_spb,
		    sizeof(arcbios_spb));
	}


	/*
	 *  TODO: How to build the component tree intermixed with
	 *  the rest of device initialization?
	 */

	arc_environment_setup(machine, is64bit, primary_ether_addr);
}

