/*
 *  Copyright (C) 2006  Anders Gavare.  All rights reserved.
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
 *  $Id: udp_snoop.c,v 1.1 2006-09-07 11:44:42 debug Exp $
 *
 *  Dumps UDP packets in hex and ASCII that arrive at a specific port.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


void dump_buf(unsigned char *p, ssize_t len)
{
	while (len > 0) {
		ssize_t i, n = len>=16? 16 : len;
		for (i=0; i<n; i++)
			printf(" %02x", p[i]);
		for (i=n; i<16; i++)
			printf("   ");
		printf("  ");
		for (i=0; i<n; i++) {
			if (p[i]>=32 && p[i]<127)
				printf("%c", p[i]);
			else
				printf(".");
		}
		printf("\n");
		p += 16;
		len -= 16;
	}
}


int main(int argc, char *argv[])
{
	struct sockaddr_in si;
	int s;

	if (argc < 2) {
		fprintf(stderr, "usage: %s udp_portnr\n", argv[0]);
		exit(1);
	}

	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		exit(1);
	}

	memset((char *)&si, sizeof(si), 0);
	si.sin_family = AF_INET;
	si.sin_port = htons(atoi(argv[1]));
	si.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(s, (struct sockaddr *)&si, sizeof(si)) < 0) {
		perror("bind");
		exit(1);
	}

	for (;;) {
		unsigned char buf[2048];
		ssize_t r = recv(s, buf, sizeof(buf), 0);

		if (r < 0) {
			perror("recv");
			exit(1);
		}

		dump_buf(buf, r);
		printf("\n");
	}

	return 0;
}

