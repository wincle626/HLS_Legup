#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {

	int DEBUG_MSG = 1 - argc % 2 ;
	unsigned int ini = strtoul (argv[1], NULL, 0);
	unsigned int val = strtoul (argv[1], NULL, 0);
	unsigned int V   = strtoul (argv[2], NULL, 0);
	unsigned int A1  = atoi(argv[3]);
	unsigned int A2  = atoi(argv[4]);
	unsigned int B1  = atoi(argv[5]);
	unsigned int B2  = atoi(argv[6]);

	unsigned int a, b, rsl;
	
	char debug_message[999], buffer[999];

	sprintf(debug_message, "|%08x|%02x|%02x|%02x|%02x", V, A1, A2, B1, B2);

	val += V;
		sprintf(buffer, "| %08x ", val);	strcat (debug_message, buffer);
	val += (val<<8);
		sprintf(buffer, "| %08x ", val);	strcat (debug_message, buffer);
	val ^= (val>>4);
		sprintf(buffer, "| %08x ", val);	strcat (debug_message, buffer);

	if (B1 == 32)	b = 0;
	else	b = (val >> B1) & B2;
		sprintf(buffer, "|%02d", b);	strcat (debug_message, buffer);

	if (A1 == 32)	a = 0;
	else	a = val<<A1;
	a = (val + a);
		sprintf(buffer, "| %08x |  %02x |", a, atoi(argv[b+7]));	strcat (debug_message, buffer);
	
	if (A2 == 32) a = 0;
	else	a = a >> A2;
	rsl= (a^atoi(argv[b+7]) );
		sprintf(buffer, "| %08x |  %02x ", ini, rsl);	strcat (buffer, debug_message);
	
	if (DEBUG_MSG) printf ("%s\n", buffer);
	else printf ("%x", rsl );
	return rsl;
}


