/*
 *  $Id: new_test_loadstore_a.c,v 1.3 2005-07-22 20:01:25 debug Exp $
 *
 *  Experimenting with dynamic-but-not-binary-translation load/store.
 *  See new_test_loadstore_b.c for the main() function.
 */

#include "new_test_loadstore.h"

static void inline general_store(struct cpu *cpu, struct ic *ic)
{
	general_store(cpu, ic);
}

void x(struct cpu *cpu, struct ic *ic)
{
	unsigned int addr = *ic->arg1; /* + ic->arg2; */
	unsigned char **table1, *page;

#ifdef AAA
	page = cpu->table0[addr >> 12];
#else
	table1 = cpu->table0[addr >> 22];
	page = table1[((addr >> 12) & 1023)*2 + 1];
#endif

	if (page != 0)
		page[addr & 4095] = *(ic->arg3);
	else
		general_store(cpu, ic);
}

void y(struct cpu *cpu, struct ic *ic)
{
	unsigned int addr = *ic->arg1; /* + ic->arg2; */
	unsigned char **table1, *page;

#ifdef AAA
	page = cpu->table0[addr >> 12];
#else
	table1 = cpu->table0[addr >> 22];
	page = table1[((addr >> 12) & 1023)*2 + 0];
#endif
	if (page != 0)
		*(ic->arg3) = page[addr & 4095];
	else
		general_store(cpu, ic);
}

