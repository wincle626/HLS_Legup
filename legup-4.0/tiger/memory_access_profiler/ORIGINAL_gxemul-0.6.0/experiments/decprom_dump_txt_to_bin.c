/*
 *  Copyright (C) 2003-2005  Anders Gavare.  All rights reserved.
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
 *  $Id: decprom_dump_txt_to_bin.c,v 1.4 2005-02-21 07:18:10 debug Exp $
 *
 *  decprom_dump_txt_to_bin.c
 *
 *  This program takes a textfile containing a DECstation
 *  PROM memory dump of the following format:
 *
 *	bfc00000: 0x12345678
 *	bfc00004: 0x9a8a8a7f
 *	and so on
 *
 *  and turns it into a binary image (32-bit little-endian words -> bytes),
 *
 *	0x78 0x56 0x34 0x12 0x7f 0x8a ...
 *
 *  To produce such a dump, hook up a serial terminal capable of capturing
 *  data to a file, and enter the commands
 *
 *	setenv more 0
 *	e -w 0xbfc00000:0xbfffffff
 *
 *  at the DECstation's PROM prompt.
 */

#include <stdio.h>
#include <string.h>


int main(int argc, char *argv[])
{
	FILE *fin, *fout;
	char s[500];

	if (argc != 3) {
		fprintf(stderr, "usage: %s dump.txt dump.bin\n", argv[0]);
		fprintf(stderr, "The resulting dump binary will be written"
		    " to dump.bin\n");
		exit(1);
	}

	fin = fopen(argv[1], "r");
	if (fin == NULL) {
		perror(argv[1]);
		exit(1);
	}

	fout = fopen(argv[2], "w");
	if (fout == NULL) {
		perror(argv[2]);
		exit(1);
	}

	while (!feof(fin)) {
		s[0] = 0;
		fgets(s, sizeof(s), fin);
		if (s[0]=='b' && s[1]=='f' && strlen(s)>15) {
			unsigned long addr = strtoul(s, NULL, 16);
			unsigned long data = strtoul(s + 12, NULL, 16);
			unsigned char obuf[4];

			printf("addr = 0x%0l8x data = 0x%08lx\n",
			    (long)addr, (long)data);

			addr -= 0xbfc00000L;
			obuf[0] = data & 255;
			obuf[1] = (data >> 8) & 255;
			obuf[2] = (data >> 16) & 255;
			obuf[3] = (data >> 24) & 255;

			fseek(fout, addr, SEEK_SET);
			fwrite(obuf, 1, 4, fout);
		}
	}

	fclose(fin);
	fclose(fout);

	return 0;
}

