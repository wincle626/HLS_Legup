using System;
using System.Collections.Generic;
using System.Text;

namespace Mips_Communication_Server
{
    class DebugCommandProcessor
    {
        PacketProcessor processor;

        private List<byte> recv = new List<byte>();
        private byte[] checksum = new byte[2];
        private byte[] packet = null;
        private byte[] sendData = null;
        private RecvState state = RecvState.Searching;
        private Device DebugDevice;

        enum RecvState
        {
            Searching,
            GettingPacket,
            GettingChecksum1,
            GettingChecksum2,
            SendingPacket,
        };

        public DebugCommandProcessor(Device D)
        {
            DebugDevice = D;
            processor = new PacketProcessor(DebugDevice);
        }

        public byte[] Process(byte[] buf, int length)
        {
            for (int i = 0; i < length; ++i)
            {
                switch (state)
                {
                    case RecvState.Searching:
                        if (buf[i] == PacketConstants.Start)
                        {
                            state = RecvState.GettingPacket;
                            recv.Clear();
                        }
                        else
                        {
                            Console.WriteLine("Unexpected byte while seaching");
                        }
                        break;
                    case RecvState.GettingPacket:
                        if (buf[i] == PacketConstants.Stop)
                        {
                            state = RecvState.GettingChecksum1;
                        }
                        else
                        {
                            recv.Add(buf[i]);
                        }
                        break;
                    case RecvState.GettingChecksum1:
                        checksum[0] = buf[i];
                        state = RecvState.GettingChecksum2;
                        break;
                    case RecvState.GettingChecksum2:
                        checksum[1] = buf[i];
                        packet = recv.ToArray();

                        Console.WriteLine("Done getting checksum, packet {0} checksum {1}", Encoding.ASCII.GetString(packet),
                            Encoding.ASCII.GetString(checksum));

                        if (PacketUtils.VerifyChecksum(packet, checksum))
                        {
                            byte[] SendPacket = processor.ProcessPacket(packet);

                            sendData = new byte[SendPacket.Length + 5];
                            sendData[0] = PacketConstants.RecvOK;
                            sendData[1] = PacketConstants.Start;

                            SendPacket.CopyTo(sendData, 2);
                            sendData[sendData.Length - 3] = PacketConstants.Stop;

                            byte[] PacketChecksum = PacketUtils.GenerateChecksum(SendPacket);
                            PacketChecksum.CopyTo(sendData, sendData.Length - 2);
                            
                            state = RecvState.SendingPacket;

                            Console.WriteLine("Sending {0}", Encoding.ASCII.GetString(sendData));

                            return sendData;
                        }
                        else
                        {
                            Console.WriteLine("Invalid checksum, requesting resend");

                            byte[] resendPacket = new byte[1];
                            resendPacket[0] = PacketConstants.Resend;

                            state = RecvState.Searching;

                            return resendPacket;
                        }
                        break;
                    case RecvState.SendingPacket:
                        if (buf[i] == PacketConstants.Resend)
                        {
                            return sendData;
                        }
                        else if (buf[i] == PacketConstants.RecvOK)
                        {
                            state = RecvState.Searching;
                        }
                        else
                        {
                            Console.WriteLine("Unexpected byte while sending");
                        }
                        break;
                }
            }

            return null;
        }

        public void Reset()
        {
            recv.Clear();
            state = RecvState.Searching;
        }
    }
}
