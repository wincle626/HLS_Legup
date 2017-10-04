#include "uart.h"
#include "stdarg.h"

#ifdef SIM
	#include "dev_cons.h"

	#define	PHYSADDR_OFFSET		((signed int)0xa0000000)
	#define	PUTCHAR_ADDRESS		(PHYSADDR_OFFSET + DEV_CONS_ADDRESS + DEV_CONS_PUTGETCHAR)
	#define	HALT_ADDRESS		(PHYSADDR_OFFSET + DEV_CONS_ADDRESS + DEV_CONS_HALT)
	
	void printc_uart(const unsigned char ch) { *((volatile unsigned char *) PUTCHAR_ADDRESS) = ch; }

#else
	void printc_uart(const unsigned char c) {
		unsigned int uport;
		#define WRITE_BUSY 0x20

		do {
			uport = *(volatile unsigned char*) (UART+8);
		} while (!(uport&WRITE_BUSY));

		*(volatile unsigned char*) (UART+4)=c;
	}
#endif

void print_uart(const unsigned char* ptr) {
	while (*ptr) printc_uart(*(ptr++));
}

char _i2h(int i) {
	if (i < 10) return ('0'+i);
	else		return ('a'+i-10);
}

char _i2H(int i) {
	if (i < 10) return ('0'+i);
	else		return ('A'+i-10);
}

char* i2h(unsigned int i, int caps) {
	char thex[9];
	static char hex[9];
	int c = 0;

	// see which function to use
	char (*__i2h)(int);
	if (caps) __i2h = &_i2H;
	else	  __i2h = &_i2h;

	if (i == 0) return "0";

	while (i > 0) {
		thex[c++] = __i2h(i % 16);
		i /= 16;
	}

	i = 0;
	hex[c] = 0;
	while (c-- > 0) hex[i++] = thex[c];

	return hex;
}

char* l2h(unsigned long long i, int caps) {
	char thex[17];
	static char hex[17];
	int c = 0;

	// see which function to use
	char (*__i2h)(int);
	if (caps) __i2h = &_i2H;
	else	  __i2h = &_i2h;

	if (i == 0) return "0";

	while (i > 0) {
		thex[c++] = __i2h(i % 16);
		i /= 16;
	}

	i = 0;
	hex[c] = 0;
	while (c-- > 0) hex[i++] = thex[c];

	return hex;
}

char* itoa(int num) {
	static char buf[11];
	static char out[11];
	int i,j;
	int neg = 0;

	if (num < 0) {
		neg = 1;
		num *= -1;
	} else if (num == 0) {
		return "0";
	}

	buf[10]=0;
	for(i=9;i>=0;i--) {
		buf[i]=(char)((num%10)+'0');
		num/=10;
	}

	// find first non-zero value
	for (i=0; i<10; i++) {
		if (buf[i] != '0') break;
	}

	// remove leading 0's, take into account -ve sign
	if (neg) out[0] = '-';
	for (j=0; j<(10-i); j++) out[j+neg] = buf[i+j];
	out[j+neg] = '\0';

	return out;
}

char* utoa(unsigned int num) {
	static char buf[11];
	static char out[11];
	int i,j;

	if (num == 0) {
		return "0";
	}

	buf[10]=0;
	for(i=9;i>=0;i--) {
		buf[i]=(char)((num%10)+'0');
		num/=10;
	}

	// find first non-zero value
	for (i=0; i<10; i++) {
		if (buf[i] != '0') break;
	}

	// remove leading 0's
	for (j=0; j<(10-i); j++) out[j] = buf[i+j];
	out[j] = '\0';

	return out;
}

/*char* ltoa(long long num) {
	static char buf[25];
	static char out[25];
	int i,j;
	int neg = 0;

	if (num < 0) {
		neg = 1;
		num *= -1;
	} else if (num == 0) {
		return "0";
	}

	buf[24]=0;
	for(i=23;i>=0;i--) {
		buf[i]=(char)((num%10)+'0');
		num/=10;
	}

	// find first non-zero value
	for (i=0; i<24; i++) {
		if (buf[i] != '0') break;
	}

	// remove leading 0's, take into account -ve sign
	if (neg) out[0] = '-';
	for (j=0; j<(24-i); j++) out[j+neg] = buf[i+j];
	out[j+neg] = '\0';

	return out;
}*/

int puts(const char *fmt) {
	int k;
	int prev_r = 0;

	k = 0;
	while (fmt[k] != '\0') {

        // Turn '\n' into '\r\n' if not simulating
        #ifndef SIM
            if (fmt[k] == '\r') prev_r = 1;
            if (fmt[k] == '\n' && !prev_r) printc_uart('\r');
        #endif
        printc_uart(fmt[k]);
        k++;
	}

    // puts expect a new line at the end
    printc_uart('\n');    
	return 0;
}

int printf (const char *fmt, ...) {
	va_list arg;
	int k;
	int x;
	int prev_r = 0;
	long long ll;

	va_start(arg, fmt);

	k = 0;
	while (fmt[k] != '\0') {
		if (fmt[k] == '%') {
			k++;
			// ignore formatting
			while (fmt[k] >= '0' && fmt[k] <= '9') k++;
			// ld is same as d on 32-bit MIPS
			if (fmt[k] == 'l') k++;
			switch(fmt[k]) {
				case 'd':
					print_uart( itoa(va_arg(arg, int)) );
					break;
				case 'u':
					print_uart( utoa(va_arg(arg, unsigned int)) );
					break;
				case 's':
					print_uart( va_arg(arg, char*) );
					break;
				case 'c':
					x = va_arg (arg, int);
					printc_uart((char)x);
					break;
				/*case 'f':
					print_uart( ftoa(va_arg (arg, float), &x) );
					break;*/
				case 'x':
					print_uart( i2h(va_arg(arg, unsigned int), 0) );
					break;
				case 'X':
					print_uart( i2h(va_arg(arg, unsigned int), 1) );
					break;
				case 'l':
					switch(fmt[k+1]) {
						case 'd':
							//print_uart( ltoa(va_arg(arg, long long)) );
							x = va_arg(arg, long);
							x = va_arg(arg, long);
							print_uart( itoa(x) );
							k++;
							break;
						case 'x':
							x = va_arg(arg, unsigned long);
							ll = (unsigned long long)x & 0xffffffff;
							x = va_arg(arg, unsigned long);
							ll |= ((unsigned long long)x) << 32;
							print_uart( l2h(ll, 0) );
							k++;
							break;
						case 'X':
							x = va_arg(arg, unsigned long);
							ll = (unsigned long long)x & 0xffffffff;
							x = va_arg(arg, unsigned long);
							ll |= ((unsigned long long)x) << 32;
							print_uart( l2h(ll, 1) );
							k++;
							break;
					}
					break;
				case 'h':
					x = va_arg (arg, int);
					if (fmt[k+1] == 'h') {
						k++;
						x &= 0xFF;
					} else
						x &= 0xFFFF;
					k++;
					switch(fmt[k]) {
						case 'd':
							print_uart( itoa(x) );
							break;
						case 'x':
							print_uart( i2h((unsigned int)x, 0) );
							break;
						case 'X':
							print_uart( i2h((unsigned int)x, 1) );
							break;
					}
					break;
				case '%':
					printc_uart('%');
					break;
				default:
					printc_uart('%');
					printc_uart(fmt[k]);
					break;
			}
			k++;

		} else {
			// Turn '\n' into '\r\n' if not simulating
			#ifndef SIM
				if (fmt[k] == '\r') prev_r = 1;
				if (fmt[k] == '\n' && !prev_r) printc_uart('\r');
			#endif
			printc_uart(fmt[k]);
			k++;
		}
	}
	va_end(arg);
	// should return # of characters printed, negative number on failure
	// just return success for now
	return 0;
}

/*char* ftoa(float num) {
	int i, j;
	int whole, dec;
	char *bufWhole;
	static char bufDec[12];
	static char out[22];

	int num2 =  (int)num;
	int *ptr = &num;

	whole = (int)num;
	dec = (num - whole) * 100000000;	// 18 decimal places

	print_uart("\r\n--------------------\r\n");
	print_uart(itoa(*ptr));
	print_uart("\r\n---\r\n");
	print_uart(itoa((*ptr & 0x0000000F) >> 0));
	print_uart("\r\n");
	print_uart(itoa((*ptr & 0x000000F0) >> 4));
	print_uart("\r\n");
	print_uart(itoa((*ptr & 0x00000F00) >> 8));
	print_uart("\r\n");
	print_uart(itoa((*ptr & 0x0000F000) >> 12));
	print_uart("\r\n");
	print_uart(itoa((*ptr & 0x000F0000) >> 16));
	print_uart("\r\n");
	print_uart(itoa((*ptr & 0x00F00000) >> 20));
	print_uart("\r\n");
	print_uart(itoa((*ptr & 0x0F000000) >> 24));
	print_uart("\r\n");
	print_uart(itoa((*ptr & 0xF0000000) >> 28));
	print_uart("\r\n");

	// find char for whole
	bufWhole = itoa(whole);

	// find char for dec
	bufDec[10]=0;
	for(i=9; i>=0; i--) {
		bufDec[i]=(char)((dec%10)+'0');
		dec/=10;
	}

	// trim decimal places
	for (i=9; i>=0; i--) {
		if (bufDec[i] != '0') break;
	}

	// create output buffer
	for (j=0; bufDec[j]!=0; j++) out[j] = bufWhole[j];	// add whole
	out[j++] = '.';
	for ( ; i>0; i--,j++)		 out[j] = bufDec[j];
	out[j] = 0;

	return ""; //out;
}*/

/*http://www.edaboard.com/ftopic41714.html#160029
typedef union {
	long	L;
	float	F;
} LF_t;

char* ftoa(float f, int *status) {
	long mantissa, int_part, frac_part;
	short exp2;
	LF_t x;
	char *p;
	static char outbuf[15];

	*status = 0;
	if (f == 0) {
		outbuf[0] = '0';
		outbuf[1] = '.';
		outbuf[2] = '0';
		outbuf[3] = 0;
		return outbuf;
	}
	x.F = f;

	exp2 = (unsigned char)(x.L >> 23) - 127;
	mantissa = (x.L & 0xFFFFFF) | 0x800000;
	frac_part = 0;
	int_part = 0;

	if (exp2 >= 31) {
		// *status = _FTOA_TOO_LARGE;
		return "+FTOA_TOO_LARGE";
	} else if (exp2 < -23) {
		// *status = _FTOA_TOO_SMALL;
		return "_FTOA_TOO_SMALL";
	} else if (exp2 >= 23) {
		int_part = mantissa << (exp2 - 23);
	} else if (exp2 >= 0) {
		int_part = mantissa >> (23 - exp2);
		frac_part = (mantissa << (exp2 + 1)) & 0xFFFFFF;
	} else { //if (exp2 < 0)
		frac_part = (mantissa & 0xFFFFFF) >> -(exp2 + 1);
	}

	p = outbuf;

	if (x.L < 0) *p++ = '-';

	if (int_part == 0) {
		*p++ = '0';
	} else {
		//ltoa(p, int_part, 10);
		p = itoa(int_part);
		while (*p) p++;
	}
	*p++ = '.';

	if (frac_part == 0) {
		*p++ = '0';
	} else {
		char m, max;

		max = sizeof (outbuf) - (p - outbuf) - 1;
		if (max > 7) max = 7;

		// print BCD
		for (m = 0; m < max; m++) {
			// frac_part *= 10;
			frac_part = (frac_part << 3) + (frac_part << 1);

			*p++ = (frac_part >> 24) + '0';
			frac_part &= 0xFFFFFF;
		}

		// delete ending zeroes
		for (--p; p[0] == '0' && p[-1] != '.'; --p);
		++p;
	}
	*p = 0;

	return outbuf;
}*/

//void mexit_spin(int a) {
void exit(int a) {
   printf("program returned %d exit status\n", a);

   // hang
   while (1);
}
