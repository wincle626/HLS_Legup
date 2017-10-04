#ifndef	SYMBOL_H
#define	SYMBOL_H

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
 *  Symbol handling routines.
 */

#include "misc.h"

/*  This should actually only be used within symbol.c:  */
struct symbol {
	struct symbol	*next;
	uint64_t	addr;
	uint64_t	len;
	char		*name;
	int		type;

	int		n_args;
	/*  TODO: argument types  */
};


struct symbol_context {
	struct symbol	*first_symbol;
	int		sorted_array;
	int		n_symbols;

};

/*  symbol.c:  */
int symbol_nsymbols(struct symbol_context *);
int get_symbol_addr(struct symbol_context *, const char *symbol, uint64_t *addr);
char *get_symbol_name_and_n_args(struct symbol_context *, uint64_t addr,
	uint64_t *offset, int *n_argsp);
char *get_symbol_name(struct symbol_context *, uint64_t addr, uint64_t *offset);
void add_symbol_name(struct symbol_context *, uint64_t addr,
	uint64_t len, const char *name, int type, int n_args);
void symbol_readfile(struct symbol_context *, char *fname);
void symbol_recalc_sizes(struct symbol_context *);
void symbol_init(struct symbol_context *);

/*  symbol_demangle.c:  */
char *symbol_demangle_cplusplus(const char *name);

#endif	/*  SYMBOL_H  */
