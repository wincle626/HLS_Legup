using System;
using System.Collections.Generic;
using System.Text;

namespace Mips_Communication_Server
{
    static class PacketUtils
    {
        public static bool VerifyChecksum(byte[] Packet, byte[] GivenChecksum)
        {
            byte[] PacketChecksum = GenerateChecksum(Packet);

            if(PacketChecksum.Length == 2 
                && PacketChecksum[0] == GivenChecksum[0]
                && PacketChecksum[1] == GivenChecksum[1])
                return true;
            else
                return false;
        }

        public static byte[] GenerateChecksum(byte[] Packet)
        {            
            byte Checksum = 0;

            foreach (byte b in Packet)
            {
                Checksum += b;
            }

            byte[] ChecksumEnc = new byte[2];

            ToHex((uint)Checksum, ChecksumEnc);

            return ChecksumEnc;
        }

        public static void ToHex(uint n, byte[] buf)
        {
            int pos = 0;

            while (n > 0 || pos < buf.Length)
            {
                uint digit = n % 16;
                if (digit < 10)
                {
                    buf[buf.Length - pos - 1] = (byte)(PacketConstants.Zero + digit);
                }
                else
                {
                    buf[buf.Length - pos - 1] = (byte)(PacketConstants.A + (digit - 10));
                }

                ++pos;
                n >>= 4;
            }
        }

        public static uint FromHex(byte[] buf)
        {
            uint n = 0;

            foreach (byte b in buf)
            {
                uint digit;
                if (b >= PacketConstants.Zero && b <= PacketConstants.Zero + 9)
                {
                    digit = (uint)(b - PacketConstants.Zero);
                }
                else if (b >= PacketConstants.A && b <= PacketConstants.A + 5)
                {
                    digit = (uint)((b - PacketConstants.A) + 10);
                }
                else
                {
                    return 0;
                }

                n <<= 4;
                n |= digit;
            }

            return n;
        }

        public static uint[] FromHexRegs(byte[] RegsHex)
        {
            byte[] Reg = new byte[8];
            uint[] Regs = new uint[34];

            uint CurrentReg = 0;

            //Skip first register as it's $zero
            //Next 31 match to the 31 general purpose registers
            for(int i = 8;i < 32 * 8; i += 8) 
            {
                Array.Copy(RegsHex, i, Reg, 0, 8);
                Regs[CurrentReg] = EndianSwap(FromHex(Reg));

                ++CurrentReg;
            }

            //Low and high are the 33rd and 34th registers given
            for (int i = 33 * 8; i < 35 * 8; i += 8)
            {
                Array.Copy(RegsHex, i, Reg, 0, 8);
                Regs[CurrentReg] = EndianSwap(FromHex(Reg));

                ++CurrentReg;
            }

            //PC is the 37th register given
            Array.Copy(RegsHex, 37 * 8, Reg, 0, 8);
            Regs[CurrentReg] = EndianSwap(FromHex(Reg));

            return Regs;
        }

        public static byte[] ToHexRegs(uint[] Regs)
        {
            byte[] RegsHex = new byte[73 * 8];

            byte[] Reg = new byte[8];
            
            //First register is $zero
            for (int i = 0; i < 8; ++i)
            {
                RegsHex[i] = PacketConstants.Zero;
            }

            uint CurrentReg = 0;

            //Next 31 are the general purpose registers
            for (int i = 8; i < 32 * 8; i += 8)
            {
                ToHex(EndianSwap(Regs[CurrentReg]), Reg);
                Array.Copy(Reg, 0, RegsHex, i, 8);

                ++CurrentReg;
            }

            //Next one is sr, we just give zeros
            for (int i = 32 * 8; i < 33 * 8; ++i)
            {
                RegsHex[i] = PacketConstants.Zero;
            }

            //Next two are high and low
            for (int i = 33 * 8; i < 35 * 8; i += 8)
            {
                ToHex(EndianSwap(Regs[CurrentReg]), Reg);
                Array.Copy(Reg, 0, RegsHex, i, 8);

                ++CurrentReg;
            }

            //Next two are bad and cause, we just give zeros
            for (int i = 35 * 8; i < 37 * 8; ++i)
            {
                RegsHex[i] = PacketConstants.Zero;
            }

            //Next one is PC
            ToHex(EndianSwap(Regs[CurrentReg]), Reg);
            Array.Copy(Reg, 0, RegsHex, 37 * 8, 8);
            ++CurrentReg;

            //Next 34 are fp registers we jut give zeros
            for (int i = 38 * 8; i < 72 * 8; ++i)
            {
                RegsHex[i] = PacketConstants.Zero;
            }

            //Next one is fp (already given as a general purpose register)
            ToHex(EndianSwap(Regs[29]), Reg);
            Array.Copy(Reg, 0, RegsHex, 72 * 8, 8);
            ++CurrentReg;

            return RegsHex;
        }

        public static byte[] ToHexBytes(byte[] Bytes)
        {
            byte[] HexBytes = new byte[Bytes.Length * 2];
            byte[] HexByte = new byte[2];

            uint CurrentByte = 0;
            for (int i = 0; i < Bytes.Length * 2; i += 2)
            {
                ToHex(Bytes[CurrentByte], HexByte);
                Array.Copy(HexByte, 0, HexBytes, i, 2);
                ++CurrentByte;
            }

            return HexBytes;
        }

        public static byte[] FromHexBytes(byte[] HexBytes)
        {
            byte[] Bytes = new byte[HexBytes.Length / 2];
            byte[] Byte = new byte[2];

            uint CurrentByte = 0;

            for (int i = 0; i < HexBytes.Length; i += 2)
            {
                Array.Copy(HexBytes, i, Byte, 0, 2);
                
                Bytes[CurrentByte] = (byte)FromHex(Byte);
                ++CurrentByte;
            }

            return Bytes;
        }

        public static uint EndianSwap(uint n)
        {
            return ((n & 0xFF000000) >> 24) | ((n & 0x00FF0000) >> 8) | ((n & 0x0000FF00) << 8) | ((n & 0x000000FF) << 24);
        }
    }
}
