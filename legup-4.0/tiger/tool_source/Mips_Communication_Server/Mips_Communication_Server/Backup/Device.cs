using System;
using System.Collections.Generic;
using System.Text;

namespace Mips_Communication_Server
{
    interface Device
    {
        byte HaltReason();
        void SetRegs(uint[] Regs);
        uint[] GetRegs();
        MemError WriteMemory(byte[] Data, uint Addr, uint Length);
        MemError ReadMemory(byte[] Data, uint Addr, uint Length);
        byte Continue(uint Addr);
        byte Step(uint Addr);
        void Interrupt();
        void Run(uint Addr);
    }

    enum MemError
    {
        OK,
        Error
    }
}
