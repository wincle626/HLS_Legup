/*
 *  $Id: disk.c,v 1.2 2006-05-22 04:53:52 debug Exp $
 *
 *  GXemul demo:  Disk image access
 *
 *  This file is in the Public Domain.
 */

#include "dev_cons.h"
#include "dev_disk.h"


#ifdef MIPS
/*  Note: The ugly cast to a signed int (32-bit) causes the address to be
	sign-extended correctly on MIPS when compiled in 64-bit mode  */ 
#define PHYSADDR_OFFSET         ((signed int)0xa0000000)
#else
#define PHYSADDR_OFFSET         0
#endif


#define PUTCHAR_ADDRESS		(PHYSADDR_OFFSET +              \
				DEV_CONS_ADDRESS + DEV_CONS_PUTGETCHAR)
#define HALT_ADDRESS            (PHYSADDR_OFFSET +              \
				DEV_CONS_ADDRESS + DEV_CONS_HALT)
#define DISK_ADDRESS            (PHYSADDR_OFFSET + DEV_DISK_ADDRESS)


void printchar(char ch)
{
	*((volatile unsigned char *) PUTCHAR_ADDRESS) = ch;
}


void printstr(char *s)
{
	while (*s)
		printchar(*s++);
}


void halt(void)
{
	*((volatile unsigned char *) HALT_ADDRESS) = 0;
}


void printhex2(int i)
{
	printchar("0123456789abcdef"[i >> 4]);
	printchar("0123456789abcdef"[i & 15]);
}


void printhex4(int i)
{
	printhex2(i >> 8);
	printhex2(i & 255);
}


void f(void)
{
	int ofs, ide_id = 0, status, i;
	unsigned char ch;

	printstr("Testing dev_disk.\n");
	printstr("Assuming that IDE ID 0 (primary master) is available.\n");

	for (ofs = 0; ofs < 1024; ofs += 512) {
		printstr("\n");

		*((volatile int *) (DISK_ADDRESS + DEV_DISK_OFFSET)) = ofs;
		*((volatile int *) (DISK_ADDRESS + DEV_DISK_ID)) = ide_id;

		*((volatile int *) (DISK_ADDRESS + DEV_DISK_START_OPERATION)) =
		    DEV_DISK_OPERATION_READ;

		/*  Get status:  */
		status = *((volatile int *) (DISK_ADDRESS + DEV_DISK_STATUS));

		if (status == 0) {
			printstr("Read failed.\n");
			halt();
		}

		printstr("Sector dump:\n");
		for (i = 0; i < 512; i++) {
			if ((i % 16) == 0) {
				printhex4(i);
				printstr(" ");
			}
			printstr(" ");
			ch = *((volatile unsigned char *) DISK_ADDRESS
			    + DEV_DISK_BUFFER + i);
			printhex2(ch);
			if ((i % 16) == 15) {
				int j;
				printstr("  ");
				for (j = i-15; j <= i; j++) {
					ch = *((volatile unsigned char *)
					    DISK_ADDRESS + DEV_DISK_BUFFER + j);
					if (ch < 32 || ch >= 127)
						ch = '.';
					printchar(ch);
				}
				printstr("\n");
			}
		}
	}

	printstr("\nDone.\n");
	halt();
}

