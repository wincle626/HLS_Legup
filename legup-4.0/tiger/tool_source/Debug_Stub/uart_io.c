#include "uart_io.h"
#include "io.h"

#define UART_DATA_REGISTER 0
#define UART_CONTROL_REGISTER 1

#define UART_DATA_RVALID 0x8000
#define UART_CONTROL_WRITESPACE 0xFFFF0000


#define UART		0xF0000880

void printc_uart(const unsigned char c) {
	unsigned int uport;
	#define WRITE_BUSY 0x20

	do {
		uport = *(volatile unsigned char*) (UART+8);
	} while (!(uport&WRITE_BUSY));

	*(volatile unsigned char*) (UART+4)=c;
}


void print_uart(const unsigned char* ptr) {
	while (*ptr) {
		printc_uart(*(ptr++));
	}
}


char uart_readc(unsigned int addr)
{
	unsigned int readData;
	
	//Wait for some data to appear
	do
	{
		readData = IO_RD_WORD(addr, UART_DATA_REGISTER);
	} while(!(readData & UART_DATA_RVALID));
	
	return (char)(readData & 0xFF);
}

unsigned int uart_readUInt(unsigned int addr)
{
	unsigned int ret = 0;
	unsigned int c;
	
	int i;
	for(i = 0;i < 4; ++i)
	{
		ret >>= 8;
		c = uart_readc(addr);
		ret |= (c << 24); 
	} 
	
	return ret;
}

void uart_writeStr(unsigned int addr, char* str)
{
	while(*str)
	{
		uart_writec(addr, *str);
		++str;
	}
}

void uart_writeStrHex(unsigned int addr, unsigned int Num)
{
	int i;
	for(i = 0;i < 8; ++i)
	{
		unsigned int Digit = Num & 0xF0000000;
		char c;
		Digit >>= 28;
		
		if(Digit >=0 && Digit <= 9)
			c = '0' + Digit;
		else
			c = 'A' + (Digit - 10);
			
		uart_writec(addr, c);
		
		Num <<= 4;
	}	
}

void uart_writec(unsigned int addr, char c)
{
	unsigned int control;
	do
	{
		control = IO_RD_WORD(addr, UART_CONTROL_REGISTER);	
	} while(!(control & UART_CONTROL_WRITESPACE));
	
	IO_WR_WORD(addr, UART_DATA_REGISTER, c);
}

void uart_writeUInt(unsigned int addr, unsigned int i)
{
	int j;
	for(j = 0;j < 4; ++j)
	{
		uart_writec(addr, (char)(i & 0xFF));
		i >>= 8;	
	}
}

int uart_init(unsigned int addr)
{
	IO_WR_WORD(addr, UART_CONTROL_REGISTER, 0);
	
	return 0;
}
