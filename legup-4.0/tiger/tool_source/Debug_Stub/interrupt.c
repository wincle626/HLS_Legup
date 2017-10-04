#define JTAG_UART_BASE 0xF0000010
#define JTAG_UART_DATA *(volatile unsigned int*)(JTAG_UART_BASE)
#define JTAG_UART_CONFIG *(volatile unsigned int*)(JTAG_UART_BASE + 0x4)

#define HEX_DISPLAY_BASE 0xF0000030
#define HEX_DISPLAY *(volatile unsigned int*)(HEX_DISPLAY_BASE)

unsigned int charsSent = 0;

void exception(void)
{
	print_uart("in exception\r\n");
	
	if(!(JTAG_UART_CONFIG & 0x100))
		return;
	
	unsigned int readData;
	unsigned int readAvailable;
	char c;
	
	while(1)
	{
		readData = JTAG_UART_DATA;
		
		if(!(readData & 0x8000))
			break;
			
		c = readData & 0xFF;
		
		while(!((JTAG_UART_CONFIG) & 0xFFFF0000));
		
		JTAG_UART_DATA = c;
		++charsSent;
	}
	
	HEX_DISPLAY = charsSent;
}