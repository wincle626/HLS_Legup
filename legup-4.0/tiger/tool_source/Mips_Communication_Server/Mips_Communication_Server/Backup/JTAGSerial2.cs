using System;
using System.Collections.Generic;
using System.Text;
using System.Diagnostics;
using System.IO;
using System.Net.Sockets;
using System.Net;

namespace Mips_Communication_Server
{
    //As the JTAG Atlantic stuff doesn't seem to work on the PWF, I've written
    //a replacement JTAG UART using the virtual JTAG stuff.  Currently the only way
    //to access this is via TCL scripts, so this stream opens up a process which
    //executes a tcl script which connects to a certain port on localhost, all terminal
    //data then goes over this socket.
    
    class JTAGSerialStream2 : Stream
    {
        System.Diagnostics.Process SerialProcess = null;
        Socket SerialSocket = null;

        public JTAGSerialStream2(string Cable, int Device, int Instance)
        {
            try
            {
                Socket SerialListenSocket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
                IPEndPoint SerialEP = new IPEndPoint(IPAddress.Any, 0);
                SerialListenSocket.Bind(SerialEP);
                SerialListenSocket.Listen(1);

                SerialEP = (IPEndPoint)SerialListenSocket.LocalEndPoint;
     
                SerialProcess = new Process();
                SerialProcess.StartInfo.FileName = "quartus_stp";
                SerialProcess.StartInfo.Arguments = String.Format("-t serial.tcl \"{0}\" {1} {2} {3}", Cable, Device, Instance, SerialEP.Port);
                SerialProcess.EnableRaisingEvents = true;
                SerialProcess.Exited +=
                    delegate(object Sender, EventArgs e)
                    {
                        if (SerialListenSocket != null)
                            SerialListenSocket.Close();
                    };
                
                SerialProcess.Start();
                SerialSocket = SerialListenSocket.Accept();

                SerialListenSocket.Close();
                SerialListenSocket = null;
            }
            catch (System.ComponentModel.Win32Exception e)
            {
                throw new JTAGException(String.Format("Error starting JTAG serial process: {0}", e.Message));
            }
        }

        public override bool CanRead
        {
            get { return true; }
        }

        public override bool CanSeek
        {
	        get { return false; }
        }

        public override bool CanWrite
        {
	        get { return true; }
        }

        public override void Flush()
        {
            
        }

        public override int Read(byte[] buffer, int offset, int count)
        {
            try
            {
                return SerialSocket.Receive(buffer, offset, count, SocketFlags.None);
            }
            catch (SocketException)
            {
                throw new JTAGException("Error during read");
            }
            catch (ObjectDisposedException)
            {
                throw new JTAGException("Error during read");
            }
        }

        public override int ReadByte()
        {
            byte[] byteBuf = new byte[1];

            int amountRead;

            do
            {
                amountRead = Read(byteBuf, 0, 1);
            } while (amountRead < 1);

            return (int)byteBuf[0];
        }

        public override void Write(byte[] buffer, int offset, int count)
        {
            try
            {
                SerialSocket.Send(buffer, offset, count, SocketFlags.None);
            }
            catch (SocketException)
            {
                throw new JTAGException("Error during read");
            }
            catch (ObjectDisposedException)
            {
                throw new JTAGException("Error during read");
            }
        }

        protected override void  Dispose(bool disposing)
        {
            if (SerialProcess != null && !SerialProcess.HasExited)
                SerialProcess.CloseMainWindow();

            base.Dispose(disposing);
        }

        public override long Length
        {
            get { throw new Exception("The method or operation is not implemented."); }
        }

        public override long Position
        {
            get
            {
                throw new Exception("The method or operation is not implemented.");
            }
            set
            {
                throw new Exception("The method or operation is not implemented.");
            }
        }

        public override long Seek(long offset, SeekOrigin origin)
        {
            throw new Exception("The method or operation is not implemented.");
        }

        public override void SetLength(long value)
        {
            throw new Exception("The method or operation is not implemented.");
        }
    }
}
