#ifndef __UART_IO_H__
#define __UART_IO_H__

#define DEBUG_UART 0xF0000850
#define PROG_UART 0xF0000840

char uart_readc(unsigned int addr);
unsigned int uart_readUInt(unsigned int addr);

void uart_writeStr(unsigned int addr, char* str);
void uart_writeStrHex(unsigned int addr, unsigned int Num);
void uart_writec(unsigned int addr, char c);
void uart_writeUInt(unsigned int addr, unsigned int i);

int uart_init(unsigned int addr);

#endif //__UART_IO_H__
