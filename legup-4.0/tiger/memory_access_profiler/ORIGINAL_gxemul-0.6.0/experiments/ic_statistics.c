/*
 *  Copyright (C) 2005-2006  Anders Gavare.  All rights reserved.
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
 *  $Id: ic_statistics.c,v 1.4 2006-07-15 09:44:13 debug Exp $
 *
 *  This program is not optimized for speed, but it should work.
 *
 *  Run  gxemul -s i:log.txt blahblahblah, and then
 *
 *  for a in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20; do \
 *	./ic_statistics log.txt $a |sort -n > statistics.$a.txt; done
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <inttypes.h>


struct entry {
	uint64_t	*ptrs;
	long long	count;
};


struct entry *entries = NULL;
int n_entries = 0;


size_t *cache_s = NULL;
char **cache_symbol = NULL;
int n_cached_symbols = 0;


char *cached_size_t_to_symbol(uint64_t s)
{
	int i = 0;
	FILE *q;
	char tmp[200];
	char *urk, *urk2;

	while (i < n_cached_symbols) {
		if (cache_s[i] == s)
			return cache_symbol[i];
		i++;
	}

	n_cached_symbols ++;
	cache_s = realloc(cache_s, sizeof(size_t) * n_cached_symbols);
	cache_symbol = realloc(cache_symbol, sizeof(char *) * n_cached_symbols);
	cache_s[n_cached_symbols - 1] = s;

	snprintf(tmp, sizeof(tmp), "nm ../gxemul | grep %"PRIx64, s);
	q = popen(tmp, "r");
	if (q == NULL) {
		perror("popen()");
		exit(1);
	}
	fgets(tmp, sizeof(tmp), q);
	pclose(q);

	while (tmp[0] && (tmp[strlen(tmp)-1] == '\n' ||
	    tmp[strlen(tmp)-1] == '\r'))
		tmp[strlen(tmp)-1] = '\0';

	urk = strrchr(tmp, ' ');
	if (urk == NULL)
		urk = tmp;
	else
		urk ++;

	urk2 = strstr(urk, "instr_");
	if (urk2 != NULL)
		urk = urk2 + 6;

	cache_symbol[n_cached_symbols - 1] = strdup(urk);
}


void print_all(int n)
{
	int i = 0;
	while (i < n_entries) {
		uint64_t *pp = entries[i].ptrs;
		int j = 0;

		printf("%lli\t", (long long)entries[i].count);
		while (j < n) {
			uint64_t s = pp[j];

			if (j > 0)
				printf(", ");
			printf("%s", cached_size_t_to_symbol(s));

			j++;
		}
		printf("\n");

		i++;
	}
}


void add_count(uint64_t *icpointers, int n)
{
	int i = 0;

	/*  Scan all existing entries.  */
	while (i < n_entries) {
		if (memcmp(icpointers, entries[i].ptrs,
		    sizeof(uint64_t) * n) == 0) {
			entries[i].count ++;
			return;
		}
		i++;
	}

	/*  Add new entry:  */
	n_entries ++;
	entries = realloc(entries, sizeof(struct entry) * n_entries);
	entries[n_entries-1].ptrs = malloc(sizeof(void *) * n);
	memcpy(entries[n_entries-1].ptrs, &icpointers[0], n * sizeof(uint64_t));
	entries[n_entries-1].count = 1;
}


void try_len(FILE *f, int len)
{
	uint64_t *icpointers;
	off_t off, n_read = 0;

	icpointers = malloc(sizeof(uint64_t) * len);

	fseek(f, 0, SEEK_END);
	off = ftello(f);

	fseek(f, 0, SEEK_SET);

	while (!feof(f)) {
		static long long yo = 0;
		char buf[100];

		yo ++;
		if ((yo & 0xfffff) == 0) {
			fprintf(stderr, "[ len=%i, %i%% done ]\n",
			    len, 100 * yo * sizeof(void *) / off);
		}

		/*  Make room for next icpointer value:  */
		if (len > 1)
			memmove(&icpointers[0], &icpointers[1],
			    (len-1) * sizeof(uint64_t));

		/*  Read one value into icpointers[len-1]:  */
		fgets(buf, sizeof(buf), f);
		icpointers[len-1] = strtoull(buf, NULL, 0);

		n_read ++;

		if (n_read >= len)
			add_count(&icpointers[0], len);
	}

	free(icpointers);
}


int main(int argc, char *argv[])
{
	FILE *f;
	int len = 1;

	if (argc < 3) {
		fprintf(stderr, "usage: %s input.log n\n", argv[0]);
		exit(1);
	}

	f = fopen(argv[1], "r");
	if (f == NULL) {
		perror(argv[1]);
		exit(1);
	}

	len = atoi(argv[2]);

	if (len < 1) {
		fprintf(stderr, "bad len\n");
		exit(1);
	}

	try_len(f, len);
	print_all(len);

	return 0;
}

