/*
 *  $Id: mp.c,v 1.1 2007-02-11 09:19:27 debug Exp $
 *
 *  GXemul demo:  Multi-Processor test
 *
 *  This file is in the Public Domain.
 */

#include "dev_cons.h"
#include "dev_mp.h"


#ifdef MIPS
/*  Note: The ugly cast to a signed int (32-bit) causes the address to be
	sign-extended correctly on MIPS when compiled in 64-bit mode  */
#define	PHYSADDR_OFFSET		((signed int)0xa0000000)
#else
#define	PHYSADDR_OFFSET		0
#endif


#define	PUTCHAR_ADDRESS		(PHYSADDR_OFFSET +		\
				DEV_CONS_ADDRESS + DEV_CONS_PUTGETCHAR)
#define	HALT_ADDRESS		(PHYSADDR_OFFSET +		\
				DEV_CONS_ADDRESS + DEV_CONS_HALT)

#define	NCPUS_ADDRESS		(PHYSADDR_OFFSET +		\
				DEV_MP_ADDRESS + DEV_MP_NCPUS)


void printchar(char ch)
{
	*((volatile unsigned char *) PUTCHAR_ADDRESS) = ch;
}


void halt(void)
{
	*((volatile unsigned char *) HALT_ADDRESS) = 0;
}


void printstr(char *s)
{
	while (*s)
		printchar(*s++);
}

void printuint_internal(unsigned int u)
{
	int z = u / 10;
	if (z > 0)
		printuint_internal(z);
	printchar('0' + (u - z*10));
}

void printuint(unsigned int u)
{
	if (u == 0)
		printchar('0');
	else
		printuint_internal(u);
}

int get_nr_of_cpus(void)
{
	return *((volatile int *) NCPUS_ADDRESS);
}

void f(void)
{
	printstr("Multi-Processor demo\n");
	printstr("--------------------\n\n");

	printstr("Number of CPUs: ");
	printuint(get_nr_of_cpus());
	printstr("\n\n");

	halt();
}

