#include "uart.h"
#include "clang_stdarg.h"


#ifdef SIM
	void printc_uart(const char c) { *(volatile unsigned int*)(SIM_UART) = c; }
#else
	void printc_uart(const char c) {
#ifdef RS232_UART
		unsigned int uport;

		do {
			uport = *(volatile unsigned int*)(RS232_UART + 4);
		} while (!(uport & WRITE_BUSY));
#else
		unsigned int w_avail;

		do {
			w_avail = *(volatile unsigned int*)(JTAG_UART + 4);
		} while ((w_avail >> 16) == 0);
#endif

		*(volatile unsigned int*)(JTAG_UART) = c;
	}
#endif

void print_uart(const char* ptr) {
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

#ifndef MIPS
char* ltoa(long long num) {
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
}
#endif // ifndef MIPS

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
#ifdef ARM
							print_uart( ltoa(va_arg(arg, long long)) );
#endif
#ifdef MIPS
							x = va_arg(arg, long);
							x = va_arg(arg, long);
							print_uart( itoa(x) );
#endif
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

//void mexit_spin(int a) {
void exit(int a) {
   printf("program returned %d exit status\n", a);

   // hang
   while (1);
}

#ifdef ARM
void abort(int a) { exit(a); }
#endif

// EOF
