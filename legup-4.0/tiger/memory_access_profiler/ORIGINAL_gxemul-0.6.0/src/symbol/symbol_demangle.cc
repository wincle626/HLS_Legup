/*
 *  Copyright (C) 2006-2009  Anders Gavare.  All rights reserved.
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
 *  C++ symbol name demangling.
 *
 *  For obvious performance reasons, the external c++filt utility cannot be
 *  used. Also, the host's version of this utility might be incompatible with
 *  the binary being emulated.
 *
 *  TODO: Constructors, destructors, and lots of other stuff. See
 *  http://www.codesourcery.com/cxx-abi/abi.html#mangling for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "symbol.h"


#define	MAXLEN		1000


static void add_string(char *p, size_t *curlenp, const char *to_add)
{
	size_t curlen = *curlenp;
	while (curlen < MAXLEN && *to_add)
		p[curlen++] = *to_add++;
	*curlenp = curlen;
}


/*
 *  symbol_demangle_cplusplus_nested():
 *
 *  Try to demangle a nested cplusplus name. name points to the first character
 *  after "_ZN".
 */
static char *symbol_demangle_cplusplus_nested(const char *name)
{
	char *result;
	size_t result_len = 0, len;
	int first = 1, type_added = 0, pointercounter, reference;

	CHECK_ALLOCATION(result = (char *) malloc(MAXLEN + 1));
	result[0] = '\0';

	while (name[0] && name[0] != 'E' && result_len < MAXLEN) {
		/*  Read length of the next part:  */
		len = 0;
		if (*name == '0') {
			name ++;
		} else {
			while (isdigit((int)*name)) {
				len *= 10;
				len += (*name - '0');
				name ++;
			}
		}

		/*  Add ::  */
		if (!first)
			add_string(result, &result_len, "::");

		/*  Read the part itself:  */
		while (len-- >= 1 && result_len < MAXLEN)
			result[result_len ++] = *name++;

		first = 0;
	}

	if (name[0] != 'E')
		goto fail;

	name ++;

	if (*name)
		add_string(result, &result_len, "{");

	/*  Type:  */
	pointercounter = reference = 0;
	while (*name) {
		int argument_done = 0;
		char t = *name++;
		switch (t) {
		case 'c':
			add_string(result, &result_len, "char");
			argument_done = 1;
			break;
		case 'a':
			add_string(result, &result_len, "signed char");
			argument_done = 1;
			break;
		case 'h':
			add_string(result, &result_len, "unsigned char");
			argument_done = 1;
			break;
		case 'i':
			add_string(result, &result_len, "int");
			argument_done = 1;
			break;
		case 'j':
			add_string(result, &result_len, "unsigned int");
			argument_done = 1;
			break;
		case 'w':
			add_string(result, &result_len, "wchar_t");
			argument_done = 1;
			break;
		case 'b':
			add_string(result, &result_len, "bool");
			argument_done = 1;
			break;
		case 's':
			add_string(result, &result_len, "short");
			argument_done = 1;
			break;
		case 't':
			add_string(result, &result_len, "unsigned short");
			argument_done = 1;
			break;
		case 'l':
			add_string(result, &result_len, "long");
			argument_done = 1;
			break;
		case 'm':
			add_string(result, &result_len, "unsigned long");
			argument_done = 1;
			break;
		case 'x':
			add_string(result, &result_len, "long long");
			argument_done = 1;
			break;
		case 'y':
			add_string(result, &result_len, "unsigned long long");
			argument_done = 1;
			break;
		case 'n':
			add_string(result, &result_len, "__int128");
			argument_done = 1;
			break;
		case 'o':
			add_string(result, &result_len, "unsigned __int128");
			argument_done = 1;
			break;
		case 'f':
			add_string(result, &result_len, "float");
			argument_done = 1;
			break;
		case 'd':
			add_string(result, &result_len, "double");
			argument_done = 1;
			break;
		case 'e':
			add_string(result, &result_len, "__float80");
			argument_done = 1;
			break;
		case 'g':
			add_string(result, &result_len, "__float128");
			argument_done = 1;
			break;
		case 'z':
			add_string(result, &result_len, "...");
			argument_done = 1;
			break;
		case 'P':
			pointercounter ++;
			break;
		case 'R':
			reference ++;
			break;
		case 'v':	/*  void  */
			break;
		default:/*  Unknown  */
			goto fail;
		}
		if (argument_done) {
			while (pointercounter-- > 0)
				add_string(result, &result_len, "*");
			while (reference-- > 0)
				add_string(result, &result_len, "&");
			if (*name)
				add_string(result, &result_len, ",");
		}
		type_added = 1;
	}

	if (type_added)
		add_string(result, &result_len, "}");

	if (result_len == MAXLEN)
		goto fail;

	result[result_len] = '\0';

	return result;

fail:
	free(result);
	return NULL;
}


/*
 *  symbol_demangle_cplusplus():
 *
 *  Try to demangle name. If name was not a valid/known C++ symbol, then NULL
 *  is returned. Otherwise, a newly allocated string is returned, containing
 *  the demangled name.
 */
char *symbol_demangle_cplusplus(const char *name)
{
	/*  Only support _Z-style mangled names, for now:  */
	if (strlen(name) < 2 || name[0] != '_' || name[1] != 'Z')
		return NULL;

	name += 2;

	switch (name[0]) {
	case 'N':
		return symbol_demangle_cplusplus_nested(name + 1);
		break;
	}

	return NULL;
}



#ifdef TEST

void test(char *mangled, char *result)
{
	char *p = symbol_demangle_cplusplus(mangled);
	if (p == NULL) {
		if (result == NULL) {
			return;
		} else {
			printf("FAILURE for %s!\n", mangled);
			exit(1);
		}
	}
	if (strcmp(p, result) == 0)
		return;
	printf("FAILURE for %s! (result = %s)\n", mangled, p);
	exit(1);
}

int main(int argc, char *argv[])
{
	test("monkey", NULL);
	test("_monkey", NULL);
	test("_zmonkey", NULL);
	test("_Zmonkey", NULL);
	test("_ZQ5abcde", NULL);
	test("_ZN3abc5defghE", "abc::defgh");
	test("_ZN05defghEv", "::defgh{}");
	test("_ZN5defghEv", "defgh{}");
	test("_ZN3abc5defghEv", "abc::defgh{}");
	test("_ZN3abc5defghEc", "abc::defgh{char}");
	test("_ZN1a2bcEjij", "a::bc{unsigned int,int,unsigned int}");
	printf("OK\n");
	return 0;
}

#endif

