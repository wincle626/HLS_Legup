using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Threading;
using System.Diagnostics;

namespace Mips_Communication_Server
{
    class BoardDevice : Device
    {
        private JTAGSerialStream2 DeviceStream = null;
        private BinaryReader Reader;
        private BinaryWriter Writer;
        private Thread IOThread = null;
        private string CableName = null;
        private int DeviceNumber = 0;
        
        public void Connect(string Cable, int Device, int Instance)
        {
            DeviceStream = new JTAGSerialStream2(Cable, Device, Instance);
            Reader = new BinaryReader(DeviceStream);
            Writer = new BinaryWriter(DeviceStream);
            CableName = Cable;
            DeviceNumber = Device;
        }

        public void Disconnect()
        {
            /*lock (this)
            {
                if (IOThread != null)
                {
                    IOThread.Abort();
                    IOThread.Join();
                }
            }

            if(DeviceStream != null)
                DeviceStream.Close();*/

            if(DeviceStream != null)
                DeviceStream.Close();
        }

        public byte HaltReason()
        {
            byte Reason = 0;
            DoJTAGIO(delegate()
            {
                Writer.Write(PacketConstants.HaltReason);

                Writer.Flush();

                Reason = Reader.ReadByte();
            });

            return Reason;
        }

        public void Interrupt()
        {
            Process STPInterruptProcess = new Process();

            STPInterruptProcess.StartInfo.FileName = "quartus_stp";
            STPInterruptProcess.StartInfo.Arguments = String.Format("-t interrupt.tcl \"{0}\" {1}", CableName, DeviceNumber);
            STPInterruptProcess.StartInfo.CreateNoWindow = true;

            STPInterruptProcess.Start();
            STPInterruptProcess.WaitForExit();

            DoJTAGIO(delegate()
            { 
                Writer.Write(PacketConstants.Interrupt);
                Writer.Flush();
            });
        }

        public void SetRegs(uint[] Regs)
        {
            DoJTAGIO(delegate()
            {   
                Writer.Write(PacketConstants.SetRegs);

                foreach (uint i in Regs)
                {
                    Writer.Write(i);
                }

                Writer.Flush();
            });
        }

        public uint[] GetRegs()
        {
            uint[] Regs = new uint[34];

            DoJTAGIO(delegate()
            {
                Writer.Write(PacketConstants.GetRegs);

                Writer.Flush();

                for (int i = 0; i < 34; ++i)
                {
                    Regs[i] = Reader.ReadUInt32();
                }
            });

            return Regs;
        }

        public MemError WriteMemory(byte[] Data, uint Addr, uint Length)
        {
            uint Result = 1;

            DoJTAGIO(delegate()
            {
                Writer.Write(PacketConstants.WriteMem);
                Writer.Write(Length);
                Writer.Write(Addr);

                foreach (byte b in Data)
                {
                    Writer.Write(b);
                }

                Writer.Flush();

                Result = Reader.ReadUInt32();
            });

            if (Result == 0)
                return MemError.OK;
            else
                return MemError.Error;
        }

        public MemError ReadMemory(byte[] Data, uint Addr, uint Length)
        {
            uint Result = 1;

            DoJTAGIO(delegate()
            {
                Writer.Write(PacketConstants.ReadMem);
                Writer.Write(Length);
                Writer.Write(Addr);

                Writer.Flush();

                for (int i = 0; i < Length; ++i)
                {
                    Data[i] = Reader.ReadByte();
                }

                Result = Reader.ReadUInt32();
            });

            if (Result == 0)
                return MemError.OK;
            else
                return MemError.Error;
        }

        public byte Continue(uint Addr)
        {
            byte Reason = 0;

            DoJTAGIO(delegate()
            {
                Writer.Write(PacketConstants.Continue);
                Writer.Write(Addr);

                Writer.Flush();

                Reason = Reader.ReadByte();
            });

            return Reason;
        }

        public byte Step(uint Addr)
        {
            byte Reason = 0;

            DoJTAGIO(delegate()
            {
                Writer.Write(PacketConstants.Step);
                Writer.Write(Addr);

                Writer.Flush();

                Reason = Reader.ReadByte();
            });

            return Reason;
        }

        public void Run(uint Addr)
        {
            DoJTAGIO(delegate()
            {
                Writer.Write(PacketConstants.ControlRun);
                Writer.Write(Addr);

                Writer.Flush();
            });
        }

        private void DoJTAGIO(ThreadStart IODelegate)
        {
            bool Aborted = false;
            JTAGException IOException = null;

            lock (this)
            {
                IOThread = new Thread(delegate()
                {
                    try
                    {
                        IODelegate();
                    }
                    catch (JTAGException e)
                    {
                        IOException = e;
                    }
                    catch (ThreadAbortException)
                    {
                        Aborted = true;
                    }
                });
            }

            IOThread.Start();
            IOThread.Join();

            lock (this)
            {
                IOThread = null;
            }

            if (IOException != null)
                throw IOException;

            if (Aborted)
                throw new JTAGException("JTAG exception closed during operation");
        }
    }
}
