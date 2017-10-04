using System;
using System.Collections.Generic;
using System.Text;

namespace Mips_Communication_Server
{
    class PacketProcessor
    {
        private byte[] EmptyPacket = {};
        private Device DebugDevice;

        public PacketProcessor(Device D)
        {
            DebugDevice = D;
        }

        public byte[] ProcessPacket(byte[] Packet)
        {            
            if(Packet.Length == 0)
                return EmptyPacket;

            byte[] Response = EmptyPacket;

            switch (Packet[0])
            {
                case PacketConstants.HaltReason:
                    {
                        Response = new byte[3];

                        byte Reason = DebugDevice.HaltReason();
                        PacketUtils.ToHex(Reason, Response);

                        Response[0] = PacketConstants.ReplySignal;
                    }
                    break;
                case PacketConstants.SetRegs:
                    byte[] RegsHex = new byte[Packet.Length - 1];

                    Array.Copy(Packet, 1, RegsHex, 0, RegsHex.Length);

                    DebugDevice.SetRegs(PacketUtils.FromHexRegs(RegsHex));

                    Response = new byte[2];
                    Response[0] = PacketConstants.ReplyOK1;
                    Response[1] = PacketConstants.ReplyOK2;
                    break;
                case PacketConstants.GetRegs:
                    Response = PacketUtils.ToHexRegs(DebugDevice.GetRegs());
                    break;
                case PacketConstants.ReadMem:
                    {
                        int AddrLengthDividerPosition = Array.IndexOf(Packet, PacketConstants.AddrLengthDivider);

                        byte[] AddrHex = new Byte[AddrLengthDividerPosition - 1];
                        byte[] LengthHex = new Byte[Packet.Length - AddrLengthDividerPosition - 1];

                        Array.Copy(Packet, 1, AddrHex, 0, AddrHex.Length);
                        Array.Copy(Packet, AddrLengthDividerPosition + 1, LengthHex, 0, LengthHex.Length);

                        uint Addr = PacketUtils.FromHex(AddrHex);
                        uint Length = PacketUtils.FromHex(LengthHex);

                        byte[] Data = new byte[Length];

                        MemError Error = DebugDevice.ReadMemory(Data, Addr, Length);
                        if (Error != MemError.OK)
                        {
                            Response = new byte[3];
                            Response[0] = PacketConstants.ReplyError;
                            Response[1] = PacketConstants.Zero;
                            Response[2] = PacketConstants.Zero + 1;
                        }
                        else
                        {
                            Response = new byte[2 + Data.Length * 2];
                            Response[0] = PacketConstants.ReplyOK1;
                            Response[1] = PacketConstants.ReplyOK2;
                            byte[] HexData = PacketUtils.ToHexBytes(Data);
                            Array.Copy(HexData, 0, Response, 0, HexData.Length);
                        }
                    }
                    break;
                case PacketConstants.WriteMem:
                    {
                        int AddrLengthDividerPosition = Array.IndexOf(Packet, PacketConstants.AddrLengthDivider);
                        int HeaderDataDividerPosition = Array.IndexOf(Packet, PacketConstants.HeaderDataDivider);

                        byte[] AddrHex = new Byte[AddrLengthDividerPosition - 1];
                        byte[] LengthHex = new Byte[HeaderDataDividerPosition - AddrLengthDividerPosition - 1];
                        byte[] HexData = new Byte[Packet.Length - HeaderDataDividerPosition - 1];

                        Array.Copy(Packet, 1, AddrHex, 0, AddrHex.Length);
                        Array.Copy(Packet, AddrLengthDividerPosition + 1, LengthHex, 0, LengthHex.Length);
                        Array.Copy(Packet, HeaderDataDividerPosition + 1, HexData, 0, HexData.Length);

                        uint Addr = PacketUtils.FromHex(AddrHex);
                        uint Length = PacketUtils.FromHex(LengthHex);

                        MemError Error = DebugDevice.WriteMemory(PacketUtils.FromHexBytes(HexData), Addr, Length);
                        if (Error != MemError.OK)
                        {
                            Response = new byte[3];
                            Response[0] = PacketConstants.ReplyError;
                            Response[1] = PacketConstants.Zero;
                            Response[2] = PacketConstants.Zero + 1;
                        }
                        else
                        {
                            Response = new byte[2];
                            Response[0] = PacketConstants.ReplyOK1;
                            Response[1] = PacketConstants.ReplyOK2;
                        }
                    }
                    break;
                case PacketConstants.Continue:
                    {
                        Response = new byte[3];

                        byte Reason = DebugDevice.Continue(1);
                        PacketUtils.ToHex(Reason, Response);

                        Response[0] = PacketConstants.ReplySignal;
                    }
                    break;
                case PacketConstants.Step:
                    {
                        Response = new byte[3];

                        byte Reason = DebugDevice.Step(1);
                        PacketUtils.ToHex(Reason, Response);

                        Response[0] = PacketConstants.ReplySignal;
                    }
                    break;
            }

            return Response;
        }
    }
}
