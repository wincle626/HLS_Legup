#define REASON_INITIAL 0
#define REASON_BREAKPOINT 1
#define REASON_INTERRUPT 2

#include "uart_io.h"
#include "io.h"

#define SIGNAL_FIRST 0x0
#define SIGNAL_TRAP 0x5
#define SIGNAL_INT 0x2

#define BREAK_INSTRUCTION 0x0005000d

#define DEBUG_CONTROL 0xf0000860

void flushICache(void);

unsigned int savedStepIns;
unsigned int savedStepPC;
unsigned int savedStepIns2;
unsigned int savedStepPC2;

unsigned int wantsHaltReason = 0;
unsigned int stepping = 0;
unsigned int ignoreInterrupt = 0;

void setup_step(unsigned int BD, unsigned int* registers);
void teardown_step();
unsigned int determine_branch_address(unsigned int ins, unsigned int branchPC, unsigned int* registers);

void debug_stub(unsigned int* registers, unsigned int reason, unsigned int BD)
{			
	//print_uart("In debug stub\r\n");
	
	if(reason == REASON_BREAKPOINT)	{
		teardown_step();
		if (wantsHaltReason) uart_writec(DEBUG_UART, SIGNAL_TRAP);
		
	} else if(reason == REASON_INTERRUPT) {
		//if (wantsHaltReason) uart_writec(DEBUG_UART, SIGNAL_INT);
		
		IO_WR_WORD(DEBUG_CONTROL, 0, 1);
		
		if(ignoreInterrupt) {
			//uart_writeStr(PROG_UART, "Ignoring interrupt\r\n");
			//ignoreInterrupt = 0;
			return;
		}
	}
	
	//print_uart("a\r\n");
	
	//ignoreInterrupt = 0;
	wantsHaltReason = 0;
	
	while(1) {
		char op;
		//print_uart("before uart_readc\r\n");
		//op = uart_readc(DEBUG_UART);
		op = 'r';
		//print_uart("op = ");
		//print_uart(op);
		//print_uart("\r\n");
		switch(op) {
			case '?':			
				if(reason == REASON_INITIAL) {
					uart_writec(DEBUG_UART, SIGNAL_FIRST);
				} else if(reason == REASON_BREAKPOINT) {
					uart_writec(DEBUG_UART, SIGNAL_TRAP);
				} else if(reason == REASON_INTERRUPT) {
					uart_writec(DEBUG_UART, SIGNAL_INT);
				} else {
					uart_writec(DEBUG_UART, SIGNAL_TRAP);
				}
				break;
			case 'G':
				{				
					int i;
					for(i = 0;i < 34; ++i) registers[i] = uart_readUInt(DEBUG_UART);
					break;
				}
			case 'g':
				{			
					int i;
					for(i = 0;i < 34; ++i) uart_writeUInt(DEBUG_UART, registers[i]);
				}
				break;
			case 'M':
				{
					unsigned int Length = uart_readUInt(DEBUG_UART);
					volatile unsigned char* Addr = (volatile unsigned char*)uart_readUInt(DEBUG_UART);
					
					//uart_writeStr(PROG_UART, "Writing memory @ ");
					//uart_writeStrHex(PROG_UART, (unsigned int)Addr);
					//uart_writeStr(PROG_UART, "\r\n");
					
					int i;
					for(i = 0;i < Length; ++i) {
						*Addr = uart_readc(DEBUG_UART);
						++Addr;
					}
					
					uart_writeUInt(DEBUG_UART, 0);
					//uart_writeStr(PROG_UART, "Flushing ICache\r\n");
					flushICache();
				}
				break;
			case 'm':
				{
					unsigned int Length = uart_readUInt(DEBUG_UART);
					volatile unsigned char* Addr = (volatile unsigned char*)uart_readUInt(DEBUG_UART);
										
					int i;
					for(i = 0;i < Length; ++i) {
						uart_writec(DEBUG_UART, *Addr);
						++Addr;	
					}	
					
					uart_writeUInt(DEBUG_UART, 0);
				}
				break;
			case 'c':
				{
					//It sends a address at which we should continue
					unsigned int Addr = uart_readUInt(DEBUG_UART);
					
					if(Addr != 1) registers[33] = Addr;
					
					//uart_writeStr(PROG_UART, "Continuing, resuming at: ");
					//uart_writeStrHex(PROG_UART, registers[33]);
					//uart_writeStr(PROG_UART, "\r\n");
						
					wantsHaltReason = 1;
					return;
				}
				break;
			case 's':
				{
					//As for continue it sends an address
					unsigned int Addr = uart_readUInt(DEBUG_UART);
					
					if(Addr != 1) registers[33] = Addr;
					
					setup_step(BD, registers);
					wantsHaltReason = 1;
					//uart_writeStr(PROG_UART, "Stepping, resuming at: ");
					//uart_writeStrHex(PROG_UART, registers[33]);
					//uart_writeStr(PROG_UART, "\r\n");
					return;
				}
				break;
			case 'r':
				{
					//unsigned int Addr = uart_readUInt(DEBUG_UART);
					unsigned int Addr = 0x800000;
										
					if(Addr != 1) registers[33] = Addr;
						
					print_uart("Running, resuming at: \r\n");
					//print_uart(registers[33]);
										
					return;
				}
				break;
			case 0x1:
				//if(reason != REASON_INTERRUPT)
				//{
					//uart_writeStr(PROG_UART, "Setting ignore interrupt\r\n");
					IO_WR_WORD(DEBUG_CONTROL, 0, 1);
					//ignoreInterrupt = 1;
				//}
				break;
		}
	}
	
	print_uart("End of stub, resuming at: \r\n");
		
	return;
}

void setup_step(unsigned int BD, unsigned int* registers)
{
	//uart_writeStr(PROG_UART, "Setup Step!\r\n");
	
	if(!BD) //Not in a branch delay slot
	{
		//grab instruction at PC which may be a branch instruction
		volatile unsigned int* branchIns = (volatile unsigned int*)registers[33];
		//and determine to which address it branches
		unsigned int branchAddr = determine_branch_address(*branchIns, registers[33], registers);
		
		//If it is a branch instruction
		if(branchAddr != -1)
		{
			//Put break at branch destination
			volatile unsigned int* ins = (volatile unsigned int*)branchAddr;
			savedStepPC2 = branchAddr;
			savedStepIns2 = *ins;
			*ins = BREAK_INSTRUCTION;
			
			ins = (volatile unsigned int*)(registers[33] + 8);
			savedStepPC = registers[33] + 8;
			savedStepIns = *ins;
			*ins = BREAK_INSTRUCTION;
			
			stepping = 2;
		}
		else
		{
			volatile unsigned int* ins = (volatile unsigned int*)(registers[33] + 4);
			savedStepPC = registers[33] + 4;
			savedStepIns = *ins;
			*ins = BREAK_INSTRUCTION;
			
			stepping = 1;
		}
		
		
	}
	else //In branch delay slot
	{
		//uart_writeStr(PROG_UART, "Branchd Delay\r\n");
		//so grab branch instruction
		volatile unsigned int* branchIns = (volatile unsigned int*)registers[33];
		//and determine to which address it branches
		unsigned int branchAddr = determine_branch_address(*branchIns, registers[33], registers);
				
		//put a break instrunction at the branch target
		volatile unsigned int* ins = (volatile unsigned int*)branchAddr;
		savedStepPC = branchAddr;
		savedStepIns = *ins;
		*ins = BREAK_INSTRUCTION;
		
		stepping = 1;
	}
	
	//uart_writeStrHex(PROG_UART, savedStepPC);
	//uart_writeStr(PROG_UART, "\r\n");
	
	flushICache();
}

void teardown_step()
{
	volatile unsigned int* ins;
	
	if(stepping == 1)
	{
		ins = (volatile unsigned int*)savedStepPC;
		*ins = savedStepIns;
	}
	else if(stepping == 2)
	{
		ins = (volatile unsigned int*)savedStepPC;
		*ins = savedStepIns;
		
		ins = (volatile unsigned int*)savedStepPC2;
		*ins = savedStepIns2;
	}
	
	flushICache();
	
	stepping = 0;
}

unsigned int determine_branch_address(unsigned int ins, unsigned int branchPC, unsigned int* registers)
{	
	int opCode = ins >> 26;
	if(opCode == 0x2 || opCode == 0x3) //J, JAL
	{
		//So address is given as lower 26-bits of instruction
		//its a word address relative to the 256 MB block
		//the delay slot is in
		unsigned int addr = ins & 0x3FFFFFF;
		addr <<= 2;
		addr |= ((branchPC + 4) & 0xF0000000);
				
		return addr;
	}
	//BLTZ, BGEZ, BEQ, BNE, BLEZ, BGTZ
	else if(opCode == 0x1 || opCode == 0x4 || opCode == 0x5 || opCode == 0x6 || opCode == 0x7)
	{
		//So address is 16-bit word offset from branch delay slot
		short wordOffset = (short)(ins & 0xFFFF);
		int byteOffset = wordOffset;
		byteOffset *= 4;
		return (branchPC + 4 + byteOffset);
	}
	//JR, JALR
	else if(opCode == 0x0)
	{
		unsigned int funct = ins & 0x3F;
		if(funct == 0x8 || funct == 0x9)
		{
			//So address is in register
			unsigned int reg = (ins & 0x3E00000);
			reg >>= 21;
						
			if(reg == 0)
				return 0;
			else
				return registers[reg - 1];
		}
		else
		{
			return -1; //not a branch instruction
		}
	}
	
	//not a branch instruction
	return -1;
}