#define JTAG_UART_BASE 0xF0000010
#define JTAG_UART_DATA *(volatile unsigned int*)(JTAG_UART_BASE)
#define JTAG_UART_CONFIG *(volatile unsigned int*)(JTAG_UART_BASE + 0x4)

#define HEX_DISPLAY_BASE 0xF0000030
#define HEX_DISPLAY *(volatile unsigned int*)(HEX_DISPLAY_BASE)

void printString(char* string);

void main()
{
	unsigned int i = 0;
	
	while(1)
	{
		printString("Hello, World from C! :) :) :)\r\n");
		++i;
		
		HEX_DISPLAY = i;
	}
}

void printString(char* string)
{	
	while(*string)
	{
		while(!((JTAG_UART_CONFIG) & 0xFFFF0000));
		
		JTAG_UART_DATA = *string;
		++string;
	}
}