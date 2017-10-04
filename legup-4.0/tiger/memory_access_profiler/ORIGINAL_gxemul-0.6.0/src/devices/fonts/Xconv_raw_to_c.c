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
 *  Convert a raw binary file into an unsigned char array, usable from C.
 *  Example:
 *
 *	./Xconv_raw_to_c somefile.raw hello
 *
 *  gives the following output:
 *
 *	unsigned char hello[] = {
 *		35,10,35,32,32,77,97,107,101,102,
 *		105,108,101,32,105,115,32,103,101,110,
 *		101,114,97,116,101,100,32,102,114,111,
 *		109,32,77,97,107,101,102,105,108,101,
 *		46,115,107,101,108,46,32,68,111,110,
 *	...
 *	}
 *
 *  This is useful to include things such as trampoline code, which needs
 *  to be copied to some specific address at runtime, into a kernel image.
 */

#include <stdio.h>
#include <stdlib.h>


int main(int argc, char *argv[])
{
	FILE *f;
	unsigned char buf[50000];
	int len, i;

	if (argc < 3) {
		fprintf(stderr, "syntax: %s input_binary symbol_name\n",
		    argv[0]);
		fprintf(stderr, "Output is written to stdout.\n");
		exit(1);
	}

	f = fopen(argv[1], "r");
	printf("unsigned char %s[] = {", argv[2]);
	len = fread(&buf, 1, sizeof(buf), f);

	for (i=0; i<len; i++) {
		if ((i%10)==0)
			printf("\n\t");
		printf("%u", buf[i]);
		if (i < len-1)
			printf(",");
	}
	printf("\n};\n");

	return 0;
}

