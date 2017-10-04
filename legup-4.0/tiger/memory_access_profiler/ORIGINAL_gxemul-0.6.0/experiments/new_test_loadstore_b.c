/*
 *  $Id: new_test_loadstore_b.c,v 1.4 2005-07-22 20:01:25 debug Exp $
 *
 *  Experimenting with dynamic-but-not-binary-translation load/store.
 */

#include <stdio.h>
#include <stdlib.h>

#include "new_test_loadstore.h"

/*  These are in new_test_loadstore_a.c:  */
void x(struct cpu *cpu, struct ic *ic);
void y(struct cpu *cpu, struct ic *ic);

int main(int argc, char *argv[])
{
	int x1 = 72, x2 = 1234;
	struct ic ic = { 0, &x1, 5, &x2 };
	struct ic ic2 = { 0, &x2, 9, &x1 };
	struct cpu cpu;
	int i;
	char *page = malloc(4096);

#ifdef AAA
/*	cpu.table0 = malloc(sizeof(void *) * 1048576);  */
	for (i=0; i<1048576; i++)
		cpu.table0[i] = page;
#else
	cpu.table0[0] = malloc(sizeof(void *) * 2 * 1024);
	for (i=0; i<1024; i++) {
		cpu.table0[0][i*2+0] = page;
		cpu.table0[0][i*2+1] = page;
	}
#endif
	printf("A: 100 Million loads + 100 Million stores\n");
	printf("y=%i\n", x2);
	for (i=0; i<10000000; i++) {
		x(&cpu, &ic);
		y(&cpu, &ic2);

		x(&cpu, &ic);
		y(&cpu, &ic2);

		x(&cpu, &ic);
		y(&cpu, &ic2);

		x(&cpu, &ic);
		y(&cpu, &ic2);

		x(&cpu, &ic);
		y(&cpu, &ic2);

		x(&cpu, &ic);
		y(&cpu, &ic2);

		x(&cpu, &ic);
		y(&cpu, &ic2);

		x(&cpu, &ic);
		y(&cpu, &ic2);

		x(&cpu, &ic);
		y(&cpu, &ic2);

		x(&cpu, &ic);
		y(&cpu, &ic2);
	}
	printf("y=%i\n", x2);
	printf("B\n");

	return 0;
}
