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
 *  $Id: bintrans_eval.c,v 1.3 2005-01-09 01:55:27 debug Exp $
 *
 *  Bintrans algorithm evaluation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>


int64_t n_instrs = 0;
int64_t time_used = 0;

int64_t cost_of_interpreted_instruction = 500;
int64_t cost_of_native_instruction = 5;
int64_t cost_of_translation = 10000;


void print_stats(void)
{
	printf("%10lli instructions, time used: %lli\n",
	    (long long)n_instrs, (long long)time_used);
}


int main(int argc, char *argv[])
{
	FILE *f;
	int ticks = 0;
	uint64_t addr;
	char buf[40];

	if (argc < 2) {
		fprintf(stderr, "usage: %s inputfile\n", argv[0]);
		exit(1);
	}

	f = fopen(argv[1], "r");
	if (f == NULL) {
		perror(argv[1]);
		exit(1);
	}

	while (!feof(f)) {
		buf[2] = buf[sizeof(buf)-1] = 0;
		fgets(buf + 2, sizeof(buf) - 2, f);

		addr = strtoull(buf, NULL, 0);
		n_instrs ++;

/*  TODO  */

		if ((++ ticks) == 100000) {
			ticks = 0;
			print_stats();
		}
	}

	print_stats();

	return 0;
}

