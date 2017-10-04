/*
 *  Copyright (C) 2004-2005  Anders Gavare.  All rights reserved.
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
 *  $Id: sgiprom_to_bin.c,v 1.5 2005-02-21 07:18:10 debug Exp $
 *
 *  sgiprom_to_bin.c
 *
 *  This program takes a textfile containing a SGI PROM memory dump of
 *  the following format:
 *
 *	>> dump -b 0xBFC00000:0xBFCF0000
 *	0xbfc00000:    b   f0    0   f0    0    0    0    0
 *	0xbfc00008:    b   f0    1   f6    0    0    0    0
 *	SAME
 *	0xbfc00200:    b   f0    3   c9    0    0    0    0
 *	0xbfc00208:    b   f0    1   f6    0    0    0    0
 *	SAME
 *	0xbfc00280:    b   f0    3   cb    0    0    0    0
 *	..
 *
 *  and turns it into a binary image.  Input is read from stdin.
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define	MAX	200


int main(int argc, char *argv[])
{
	FILE *f;
	unsigned char previous_line[8];
	int same_flag = 0;
	off_t same_start_offset = 0;

	if (argc < 2) {
		fprintf(stderr, "usage: %s output_filename\n", argv[0]);
		fprintf(stderr, "input is read from stdin\n");
		exit(1);
	}

	f = fopen(argv[1], "w");

	while (!feof(stdin)) {
		char s[MAX];
		s[0] = 0;
		fgets(s, sizeof(s), stdin);

		while (s[0] == '\r') {
			memcpy(s, s+1, sizeof(s)-1);
			s[MAX-1] = '\0';
		}

		/*  0xbfc00460:   24    5    0   10    0    5   28   c2  */
		if (s[0] == '0' && s[10]==':') {
			unsigned long x;
			int i;

			x = strtol(s, NULL, 0);
			if (x < 0xbfc00000) {
				printf("x = 0x%08lx, less than 0xbfc00000. "
				    "aborting\n", (long)x);
				exit(1);
			}
			x -= 0xbfc00000;

			if (same_flag) {
				/*
				 *  We should fill from same_start_offset to
				 *  just before x, using previous_line data.
				 */
				off_t ofs;
				printf("same_flag set, filling until just "
				    "before 0x%08lx\n", (long)x);
				fseek(f, same_start_offset, SEEK_SET);
				for (ofs = same_start_offset; ofs < x;
				    ofs += 8) {
					fwrite(previous_line, 1,
					    sizeof(previous_line), f);
				}
				same_flag = 0;
			}

			printf("x = 0x%08lx\n", (long)x);

			fseek(f, x, SEEK_SET);

			for (i=0; i<strlen(s); i++)
				if (s[i]==' ')
					s[i]='0';
			for (i=0; i<8; i++) {
				int ofs = i*5 + 14;
				int d1, d2;
				d1 = s[ofs];
				d2 = s[ofs+1];
				if (d1 >= '0' && d1<='9')
					d1 -= '0';
				else
					d1 = d1 - 'a' + 10;
				if (d2 >= '0' && d2<='9')
					d2 -= '0';
				else
					d2 = d2 - 'a' + 10;
				d1 = d1*16 + d2;

				printf(" %02x", d1);
				fprintf(f, "%c", d1);

				previous_line[i] = d1;
			}
			printf("\n");
		}

		/*  "SAME":  */
		if (s[0] == 'S' && s[1] == 'A') {
			/*
			 *  This should produce "same" output until the
			 *  next normal "0xbfc.." line.
			 */
			same_flag = 1;
			same_start_offset = ftell(f);
		}
	}

	fclose(f);

	return 0;
}

