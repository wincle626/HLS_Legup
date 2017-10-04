#ifndef	DEBUGGER_H
#define	DEBUGGER_H

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
 *  See src/debugger/debugger.c.
 */

#include "misc.h"

struct emul;
struct machine;

/*  debugger.c:  */
void debugger_activate(int x);
void debugger_execute_cmd(char *cmd, int cmd_len);
void debugger(void);
void debugger_reset(void);
void debugger_init(struct emul *emul);

/*  single_step values:  */
#define	NOT_SINGLE_STEPPING		0
#define	ENTER_SINGLE_STEPPING		1
#define	SINGLE_STEPPING			2

/*  debugger_expr.c:  */
#define	PARSE_NOMATCH		0
#define	PARSE_MULTIPLE		1
#define	PARSE_SETTINGS		2
#define	PARSE_NUMBER		3
#define	PARSE_SYMBOL		4

int debugger_parse_expression(struct machine *m, char *expr, int writeflag,
	uint64_t *valuep);

#endif	/*  DEBUGGER_H  */
