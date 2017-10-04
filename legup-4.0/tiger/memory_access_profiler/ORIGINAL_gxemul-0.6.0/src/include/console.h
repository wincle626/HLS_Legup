#ifndef	CONSOLE_H
#define	CONSOLE_H

/*
 *  Copyright (C) 2003-2010  Anders Gavare.  All rights reserved.
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
 *  Console functions.  (See console.c for more info.)
 */

#include "misc.h"

/*  Fixed default console handle for the main console:  */
#define	MAIN_CONSOLE		0

#define	CONSOLE_OUTPUT_ONLY	-1

void console_deinit_main(void);
void console_sigcont(int x);
void console_makeavail(int handle, char ch);
int console_charavail(int handle);
int console_readchar(int handle);
void console_putchar(int handle, int ch);
void console_flush(void);
void console_mouse_coordinates(int x, int y, int fb_nr);
void console_mouse_button(int, int);
void console_getmouse(int *x, int *y, int *buttons, int *fb_nr);
void console_slave(const char *arg);
int console_are_slaves_allowed(void);
int console_warn_if_slaves_are_needed(int init);
int console_start_slave(struct machine *, const char *consolename, int use_for_input);
int console_start_slave_inputonly(struct machine *, const char *consolename,
	int use_for_input);
int console_change_inputability(int handle, int inputability);
void console_init_main(struct emul *);
void console_debug_dump(struct machine *);
void console_allow_slaves(int);

void console_init(void);
void console_deinit(void);


#endif	/*  CONSOLE_H  */
