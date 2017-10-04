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
 *  This file contains things that don't fit anywhere else, and fake/dummy
 *  implementations of libc functions that are missing on some systems.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "cpu.h"
#include "misc.h"


/*
 *  mystrtoull():
 *
 *  This function is used on OSes that don't have strtoull() in libc.
 */
unsigned long long mystrtoull(const char *s, char **endp, int base)
{
	unsigned long long res = 0;
	int minus_sign = 0;

	if (s == NULL)
		return 0;

	/*  TODO: Implement endp?  */
	if (endp != NULL) {
		fprintf(stderr, "mystrtoull(): endp isn't implemented\n");
		exit(1);
	}

	if (s[0] == '-') {
		minus_sign = 1;
		s++;
	}

	/*  Guess base:  */
	if (base == 0) {
		if (s[0] == '0') {
			/*  Just "0"? :-)  */
			if (!s[1])
				return 0;
			if (s[1] == 'x' || s[1] == 'X') {
				base = 16;
				s += 2;
			} else {
				base = 8;
				s ++;
			}
		} else if (s[0] >= '1' && s[0] <= '9')
			base = 10;
	}

	while (s[0]) {
		int c = s[0];
		if (c >= '0' && c <= '9')
			c -= '0';
		else if (c >= 'a' && c <= 'f')
			c = c - 'a' + 10;
		else if (c >= 'A' && c <= 'F')
			c = c - 'A' + 10;
		else
			break;
		switch (base) {
		case 8:	res = (res << 3) | c;
			break;
		case 16:res = (res << 4) | c;
			break;
		default:res = (res * base) + c;
		}
		s++;
	}

	if (minus_sign)
		res = (uint64_t) -(int64_t)res;
	return res;
}


/*
 *  mymkstemp():
 *
 *  mkstemp() replacement for systems that lack that function. This is NOT
 *  really safe, but should at least allow the emulator to build and run.
 */
int mymkstemp(char *templ)
{
	int h = 0;
	char *p = templ;

	while (*p) {
		if (*p == 'X')
			*p = 48 + random() % 10;
		p++;
	}

	h = open(templ, O_RDWR, 0600);
	return h;
}


#ifdef USE_STRLCPY_REPLACEMENTS
/*
 *  mystrlcpy():
 *
 *  Quick hack strlcpy() replacement for systems that lack that function.
 *  NOTE: No length checking is done.
 */
size_t mystrlcpy(char *dst, const char *src, size_t size)
{
	strcpy(dst, src);
	return strlen(src);
}


/*
 *  mystrlcat():
 *
 *  Quick hack strlcat() replacement for systems that lack that function.
 *  NOTE: No length checking is done.
 */
size_t mystrlcat(char *dst, const char *src, size_t size)
{
	size_t orig_dst_len = strlen(dst);
	strcat(dst, src);
	return strlen(src) + orig_dst_len;
}
#endif


/*
 *  print_separator_line():
 *
 *  Prints a line of "----------".
 */
void print_separator_line(void)
{
        int i = 79; 
        while (i-- > 0)
                debug("-");
        debug("\n");
}

