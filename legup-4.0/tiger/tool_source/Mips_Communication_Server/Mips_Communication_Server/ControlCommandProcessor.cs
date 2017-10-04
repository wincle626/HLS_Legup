using System;
using System.Collections.Generic;
using System.Text;

namespace Mips_Communication_Server
{
    class ControlCommandProcessor
    {
        private List<byte> recv = new List<byte>();
        private Device debugDevice;
        private int amountWanted;

        private byte PacketHeader;
        private uint MemLength;
        private uint MemAddr;
        private byte RegNum;

        private byte[] Send = null;

        private ControlState state;

        enum ControlState
        {
            Searching,
            GettingLength,
            GettingAddr,
            GettingData,
            GettingRegNum,
            GettingRegValue,
        }

        public ControlCommandProcessor(Device D)
        {
            debugDevice = D;
        }
        
        public byte[] Process(byte[] buf, int length)
        {
            try
            {
                for (int i = 0; i < length; ++i)
                {
                    if (recv.Count < amountWanted)
                        recv.Add(buf[i]);

                    if (recv.Count == amountWanted)
                        OnDataFound();
                }
            }
            catch (JTAGException e)
            {
                throw e;
            }

            return Send;
        }

        public void Reset()
        {
            recv.Clear();
            state = ControlState.Searching;
            amountWanted = 1;
        }

        private void OnDataFound()
        {
            Send = null;
            
            switch (state)
            {
                case ControlState.Searching:
                    HandleHeader(recv[0]);
                    break;
                case ControlState.GettingAddr:
                    MemAddr = ((uint)recv[3] << 24) | ((uint)recv[2] << 16) | ((uint)recv[1] << 8) | (uint)recv[0];

                    if (PacketHeader == PacketConstants.ControlRun)
                    {
                        debugDevice.Run(MemAddr);

                        Send = new byte[1];
                        Send[0] = 0x1;

                        state = ControlState.Searching;
                        amountWanted = 1;
                    }
                    else
                    {
                        state = ControlState.GettingLength;
                    }

                    break;
                case ControlState.GettingLength:
                    MemLength = ((uint)recv[3] << 24) | ((uint)recv[2] << 16) | ((uint)recv[1] << 8) | (uint)recv[0];

                    if (PacketHeader == PacketConstants.WriteMem)
                    {
                        state = ControlState.GettingData;
                        amountWanted = (int)MemLength;
                    }
                    else if (PacketHeader == PacketConstants.ReadMem)
                    {
                        Send = new byte[MemLength];
                        debugDevice.ReadMemory(Send, MemAddr, MemLength);
                        state = ControlState.Searching;
                        amountWanted = 1;
                    }
                    else
                    {
                        throw new Exception("Invalid header while GettingLength in ControlCommandProcessor.OnDataFound");
                    }

                    break;
                case ControlState.GettingData:
                    Console.WriteLine("Writing {1} bytes of memory at address {0,8:X8}", MemAddr, MemLength);
                    debugDevice.WriteMemory(recv.ToArray(), MemAddr, MemLength);
                    amountWanted = 1;
                    state = ControlState.Searching;
                    break;
                case ControlState.GettingRegNum:
                    RegNum = recv[0];
                    state = ControlState.GettingRegValue;
                    amountWanted = 4;
                    break;
                case ControlState.GettingRegValue:
                    {
                        uint Value = ((uint)recv[3] << 24) | ((uint)recv[2] << 16) | ((uint)recv[1] << 8) | (uint)recv[0];

                        Console.WriteLine("Setting register {0} to {1,8:X8}", RegNum, Value);

                        if (RegNum > 33)
                        {
                            Console.WriteLine("Invalid register number, not doing set");
                        }
                        else
                        {
                            uint[] Regs = debugDevice.GetRegs();

                            Regs[RegNum] = Value;

                            debugDevice.SetRegs(Regs);
                        }

                        Send = new byte[1];
                        Send[0] = 0x1;

                        amountWanted = 1;
                        state = ControlState.Searching;

                    }
                    break;
            }

            recv.Clear();
        }

        private void HandleHeader(byte Header)
        {
            PacketHeader = Header;
            switch (Header)
            {
                case PacketConstants.WriteMem:
                case PacketConstants.ReadMem:
                case PacketConstants.ControlRun:
                    state = ControlState.GettingAddr;
                    amountWanted = 4;
                    break;
                case PacketConstants.ControlInterrupt:
                    debugDevice.Interrupt();
                    break;
                case PacketConstants.SetReg:
                    state = ControlState.GettingRegNum;
                    amountWanted = 1;
                    break;
                default:
                    Console.WriteLine("Invalid control header byte: {0,2:X2}", Header);
                    break;
            }
        }
    }
}
