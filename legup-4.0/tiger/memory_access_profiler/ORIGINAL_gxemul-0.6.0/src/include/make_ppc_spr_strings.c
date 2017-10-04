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
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


int main(int argc, char *argv[])
{
	char *names[1024];
	unsigned int i;

	memset(names, 0, sizeof(names));

	while (!feof(stdin)) {
		char tmps[100];
		tmps[0] = '\0';
		fgets(tmps, sizeof(tmps), stdin);
		if (tmps[0] < ' ')
			break;
		for (i=0; i<strlen(tmps); i++) {
			if (tmps[i] >= 'A' && tmps[i] <= 'Z')
				tmps[i] += 32;
			if (tmps[i] == ' ' || tmps[i] == '\t') {
				int j = i, n;
				while (tmps[i]==' ' || tmps[i]=='\t')
					i++;
				n = strtol(tmps + i, NULL, 0);
				tmps[j] = '\0';
				names[n] = strdup(tmps);
				break;
			}
		}
	}

	printf("/*\n *  AUTOMATICALLY GENERATED from ppc_spr.h! Do "
	    "not edit.\n */\n\nstatic const char *ppc_spr_names[1024] = {\n");

	for (i=0; i<1024; i++)
		printf(" \"%s\"%s%s", names[i]? names[i] : "(unknown)",
		    i<1023? "," : "",
		    (i & 3) == 3? "\n" : "");

	printf("};\n\n");

	return 0;
}

