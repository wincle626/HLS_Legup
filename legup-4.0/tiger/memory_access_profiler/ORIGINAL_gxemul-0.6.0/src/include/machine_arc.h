#ifndef	MACHINE_ARC_H
#define	MACHINE_ARC_H

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
 *  ARC machine specific structure.
 */

#include <sys/types.h>


#define	ARC_CONSOLE_MAX_X		80
#define	ARC_CONSOLE_MAX_Y		25

#define	ARC_MAX_ESC			16

#define	MAX_OPEN_STRINGLEN		200
#define	ARC_MAX_HANDLES			10

#define	MAX_STRING_TO_COMPONENT		20
#define	MAX_CONFIG_DATA			50

struct machine_arcbios {
	/*  General stuff:  */
	int		arc_64bit;
	int		wordlen;		/*  cached  */

	/*  VGA Console I/O:  */
	int		vgaconsole;		/*  1 or 0  */
	uint64_t	console_vram;
	uint64_t	console_ctrlregs;
	char		escape_sequence[ARC_MAX_ESC+1];
	int		in_escape_sequence;
	int		console_maxx;
	int		console_maxy;
	int		console_curx;
	int		console_cury;
	int		console_reverse;
	int		console_curcolor;

	/*  File handles:  */
	int		file_handle_in_use[ARC_MAX_HANDLES];
	char		*file_handle_string[ARC_MAX_HANDLES];
	uint64_t	current_seek_offset[ARC_MAX_HANDLES];

	/*  Memory:  */
	int		n_memdescriptors;
	uint64_t	memdescriptor_base;

	/*  Component tree:  */
	uint64_t	next_component_address;
	int		n_components;

	char		*string_to_component[MAX_STRING_TO_COMPONENT];
	uint64_t	string_to_component_value[MAX_STRING_TO_COMPONENT];
	int		n_string_to_components;

	/*  Configuration data:  */
	int		n_configuration_data;
	uint64_t	configuration_data_next_addr;
	uint64_t	configuration_data_component[MAX_CONFIG_DATA];
	int		configuration_data_len[MAX_CONFIG_DATA];
	uint64_t	configuration_data_configdata[MAX_CONFIG_DATA];

	/*  SCSI:  */
	uint64_t	scsicontroller;		/*  component addr  */
};


#endif	/*  MACHINE_ARC_H  */
