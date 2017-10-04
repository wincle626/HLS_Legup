using System;
using System.Collections.Generic;
using System.Text;

namespace Mips_Communication_Server
{
    static class PacketConstants
    {
        public const byte Stop = 0x23;
        public const byte Start = 0x24;
        public const byte Escape = 0x7D;
        public const byte Resend = 0x2D;
        public const byte RecvOK = 0x2B;

        public const byte Zero = 0x30;
        public const byte A = 0x61;

        public const byte AddrLengthDivider = 0x2C;
        public const byte HeaderDataDivider = 0x3A;

        public const byte HaltReason = 0x3F;
        public const byte SetRegs = 0x47;
        public const byte GetRegs = 0x67;
        public const byte WriteMem = 0x4D;
        public const byte ReadMem = 0x6D;
        public const byte Continue = 0x63;
        public const byte Step = 0x73;
        public const byte SetReg = 0x50;
        public const byte Interrupt = 0x1;

        public const byte ControlInterrupt = 0x49;
        public const byte ControlRun = 0x72;

        public const byte ReplySignal = 0x53;
        public const byte ReplyOK1 = 0x4F;
        public const byte ReplyOK2 = 0x4B;
        public const byte ReplyError = 0x45;

        public const byte SignalFirst = 0x0;
        public const byte SignalQuit = 0x3;
        public const byte SignalTrap = 0x5;
    }
}
