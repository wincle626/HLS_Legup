/*
 *  Copyright (C) 2004  Anders Gavare.  All rights reserved.
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
 *  $Id: try_runlen.c,v 1.4 2005-01-09 01:55:27 debug Exp $
 *
 *  Run length test.
 *  This program takes a text file of the following format as input:
 *
 *	8003005c
 *	80030060
 *	80030064
 *	80030068
 *	8003006c
 *	80030070
 *	8013f740
 *	8013f744
 *	8013f748
 *	8013f74c
 *
 *  and returns the number of successive instructions in a block. Using
 *  the example above, the output would be:
 *
 *	6
 *	4
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>


int main(int argc, char *argv[])
{
	FILE *f;
	long long addr = 1;	/*  Magic number 1  */
	long long newaddr;
	char buf[100];
	int count;

	if (argc < 2) {
		fprintf(stderr, "usage: %s inputfile\n", argv[0]);
		exit(1);
	}

	f = fopen(argv[1], "r");
	if (f == NULL) {
		perror(argv[1]);
		exit(1);
	}

	count = 0;
	buf[0] = '0';
	buf[1] = 'x';

	while (!feof(f)) {
		buf[2] = buf[sizeof(buf)-1] = 0;
		fgets(buf + 2, sizeof(buf) - 2, f);

		/*  printf("buf = '%s'\n", buf);  */

		if (strlen(buf) < 4)
			break;

		newaddr = strtoull(buf, NULL, 0);
		if (addr != 1 && newaddr != addr + 4) {
			printf("%i\n", count);
			count = 0;
		}
		count ++;

		addr = newaddr;
	}

	/*  Print the length of the last block:  */
	printf("%i\n", count);

	return 0;
}

