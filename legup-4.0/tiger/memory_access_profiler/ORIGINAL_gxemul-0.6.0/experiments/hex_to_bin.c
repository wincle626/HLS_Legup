/*
 *  $Id: hex_to_bin.c,v 1.2 2007-05-11 07:51:56 debug Exp $
 *
 *  Quick hack to convert .hex files (such as the AVR Hello World program at
 *  http://www.tfs.net/~petek/atmel/hiworld/hiworld.hex) into raw binaries.
 *
 *  E.g.  hex_to_bin hiworld.hex hiworld.bin
 *
 *  and then:   gxemul -E bareavr .....  0:hiworld.bin
 *
 *  Note: The experimental AVR emulation was removed from GXemul at 20070511.
 *        Use versions prior to this versions if you want to play with AVR
 *        emulation.
 */

#include <stdio.h>


int fromhex(char *p)
{
	char c1 = p[0], c2 = p[1];
	if (c1 >= '0' && c1 <= '9')
		c1 -= '0';
	else if (c1 >= 'A' && c1 <= 'F')
		c1 = c1 - 'A' + 10;
	if (c2 >= '0' && c2 <= '9')
		c2 -= '0';
	else if (c2 >= 'A' && c2 <= 'F')
		c2 = c2 - 'A' + 10;
	return c1 * 16 + c2;
}


int hex_to_bin(char *fname, char *outname)
{
	FILE *f = fopen(fname, "r");
	FILE *fout = fopen(outname, "w");

	while (!feof(f)) {
		char s[80];
		int nbytes, i, addr;

		s[0] = '\0';
		fgets(s, sizeof(s), f);
		if (s[0] == 0)
			break;
		if (s[0] != ':')
			continue;
		nbytes = fromhex(s+1);
		addr = fromhex(s+3) * 256 + fromhex(s+5);
		fseek(fout, addr, SEEK_SET);
		for (i=0; i<nbytes; i++) {
			unsigned char b = fromhex(s+9+i*2);
			fwrite(&b, 1, 1, fout);
		}
	}

	fclose(fout);
	fclose(f);
	return 0;
}


int main(int argc, char *argv[])
{
	if (argc < 3) {
		fprintf(stderr, "usage: %s file.hex output.bin\n", argv[0]);
		exit(1);
	}

	return hex_to_bin(argv[1], argv[2]);
}

