using System;
using System.Collections.Generic;
using System.Text;

namespace Mips_Communication_Server
{
    class DummyDevice : Device
    {
        private static uint[] Regs = { 
            0xABCD1234,
            0xDEADBEEF,
            0xBAADF00D,
            0xBADC0DED,
            0x0,
            0x0,
            0x0,
            0x0,
            0x0,
            0x0,
            0x0,
            0x0,
            0x0,
            0x0,
            0x0,
            0x0,
            0x0,
            0x0,
            0x0,
            0x0,
            0x0,
            0x0,
            0x0,
            0x0,
            0x0,
            0x0,
            0x0,
            0x0,
            0x0,
            0xFFFFCCCC,
            0x0,
            0x0,
            0xD0D0FEAD,
            0xFACEBEEC
        };
        
        public byte HaltReason()
        {
            Console.WriteLine("Halt Reason");
            return PacketConstants.SignalTrap;
        }

        public void SetRegs(uint[] Regs)
        {
            Console.WriteLine("Setting registers: ");
            for(int i = 0;i < 31; ++i)
            {
                if (i != 0 && (i % 4) == 0)
                    Console.Write("\n");

                Console.Write("${0,2:D2} = {1,8:X8} ", i + 1, Regs[i]);
            }

            Console.Write("\n");

            Console.WriteLine("$LO = {0,8:X8} $HI = {1,8:X8} $PC = {2,8:X8}", Regs[31], Regs[32], Regs[33]);
        }

        public uint[] GetRegs()
        {
            Console.WriteLine("Getting registers");
            return Regs;
        }

        public MemError WriteMemory(byte[] Data, uint Addr, uint Length)
        {
            Console.WriteLine("Writing memory @ 0x{0,8:X8}", Addr);
            for (int i = 0; i < Length; ++i)
            {
                if (i != 0 && (i % 4) == 16)
                    Console.Write("\n");
                Console.Write("0x{0,8:X8} ", Data[i]);
            }

            return MemError.OK;
        }

        public MemError ReadMemory(byte[] Data, uint Addr, uint Length)
        {
            Console.WriteLine("Reading {0} bytes of memory from 0x{1,8:X8}", Length, Addr);

            Array.Clear(Data, 0, (int)Length);

            return MemError.OK;
        }

        public byte Continue(uint Addr)
        {
            Console.WriteLine("Continue at address 0x{0,8:X8}", Addr);

            return PacketConstants.SignalTrap;
        }

        public byte Step(uint Addr)
        {
            Console.WriteLine("Step at address 0x{0,8:X8}", Addr);

            return PacketConstants.SignalTrap;
        }

        public void Interrupt()
        {
            Console.WriteLine("Interrupt!");
        }

        public void Run(uint Addr)
        {
            Console.WriteLine("Run at address 0x{0,8:X8}", Addr);
        }
    }
}
