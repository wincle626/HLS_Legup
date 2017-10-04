/*
 *  Copyright (C) 2005-2009  Anders Gavare.  All rights reserved.
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
 *  Given a list of common ARM load/store multiple opcodes, figure out (using 
 *  simple brute force), which n bits (where n is low, e.g. 7) that cause the
 *  best separation of the 24 bit opcode space into linear lists, where "best"
 *  means to optimize the length of the longest such linear list.
 *
 *  The result is a set of bits, such as this:
 *
 *	  xxxx100P USWLnnnn llllllll llllllll
 *	         ^ ^ ^^       ^^    (in this case, n = 6)
 *
 *  (it's a 24-bit space, because the s-bit isn't used).
 */

#include <stdio.h>
#include <stdlib.h>


int bit_count(unsigned int x)
{
	static const int c[16] = { 0,1,1,2, 1,2,2,3, 1,2,2,3, 2,3,3,4 };
	return c[x & 15] + c[(x>>4) & 15] +
	    c[(x>>8) & 15] + c[(x>>12) & 15] +
	    c[(x>>16) & 15] + c[(x>>20) & 15] +
	    c[(x>>24) & 15] + c[(x>>28) & 15];
}


int cmpfunc(const void *a, const void *b)
{
	int *pa = (int *) a, *pb = (int *) b;
	if (*pa < *pb)
		return -1;
	if (*pa > *pb)
		return  1;
	return 0;
}


int calc_max_list_length(int *opcodes, int *tmp_table,
	int n_opcodes, int bit_mask)
{
	int i, maxlen, curlen;

	for (i=0; i<n_opcodes; i++)
		tmp_table[i] = opcodes[i] & bit_mask;

	qsort(tmp_table, n_opcodes, sizeof(int), cmpfunc);
	curlen = maxlen = 1;

	for (i=1; i<n_opcodes; i++)
		if (tmp_table[i] == tmp_table[i-1]) {
			curlen ++;
			if (curlen > maxlen)
				maxlen = curlen;
		} else
			curlen = 1;

	return maxlen;
}


int main(int argc, char *argv[])
{
	FILE *f = fopen("cpu_arm_multi.txt", "r");
	int n;
	const int max = 10000;
	int opcode[max];
	int tmp_table[max];
	int n_opcodes = 0;
	int *max_len;
	int bit_mask, best_bit_mask, best_bit_mask_len;

	if (argc < 2) {
		fprintf(stderr, "usage: %s n\n", argv[0]);
		fprintf(stderr, "where n=6 might be a good choice\n");
		exit(1);
	}

	n = atoi(argv[1]);

	if (f == NULL) {
		fprintf(stderr, "could not open cpu_arm_multi.txt\n");
		exit(1);
	}

	/*  Read the opcodes:  */
	while (!feof(f)) {
		char s[100];
		s[0] = s[sizeof(s)-1] = '\0';
		fgets(s, sizeof(s), f);
		if (s[0] == '0') {
			if (n_opcodes > max) {
				fprintf(stderr, "too many opcodes\n");
				exit(1);
			}
			opcode[n_opcodes++] = strtol(s, NULL, 0);
		}
	}

	printf("nr of opcodes = %i\n", n_opcodes);

	max_len = malloc(sizeof(int) * (1 << 25));
	if (max_len == NULL) {
		fprintf(stderr, "out of memory\n");
		exit(1);
	}

	best_bit_mask_len = -1;

	for (bit_mask = 0; bit_mask <= 0x01ffffff; bit_mask ++) {
		/*  Skip the s-bit:  */
		if (bit_mask & 0x00400000)
			continue;

		if (bit_count(bit_mask) != n)
			continue;

		/*  Calculate the max list length for this bit_mask:  */
		max_len[bit_mask] = calc_max_list_length(opcode,
		    tmp_table, n_opcodes, bit_mask);

		if (best_bit_mask_len == -1 ||
		    max_len[bit_mask] < best_bit_mask_len) {
			best_bit_mask_len = max_len[bit_mask];
			best_bit_mask = bit_mask;
			printf("best bit_mask so far: 0x%08x: %i\n",
			    best_bit_mask, best_bit_mask_len);
		}
	}

	return 0;
}

