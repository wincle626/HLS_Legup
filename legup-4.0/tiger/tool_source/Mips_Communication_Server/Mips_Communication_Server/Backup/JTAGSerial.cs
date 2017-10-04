using System;
using System.Collections.Generic;
using System.Text;
using System.ComponentModel;
using System.Runtime.InteropServices;
using System.IO;

namespace Mips_Communication_Server
{
    class JTAGSerialStream : Stream
    {
        internal enum JATL_ERROR : int
        {
            E_NO_ERROR = 0,

            E_UNEXPECTED_SERVER_ERROR = -3,
            
            [Description("The board is either: (1) not connected, or (2) not powered on")]
            E_BOARD_NOT_CONNECTED = -5,

            E_BOARD_IN_USE = -6,

            [Description("The bootloader can't be contacted; try power cycling the board")]
            E_BOARD_NOT_LISTENING = -8
        }

        //  struct JTAGATLANTIC * __cdecl jtagatlantic_open(char const *,int,int,char const *)
        [DllImport("jtag_atlantic.dll", EntryPoint = "?jtagatlantic_open@@YAPAUJTAGATLANTIC@@PBDHH0@Z", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr jtagatlantic_open(string cable, int device, int instance, string application);

        //  enum JATL_ERROR __cdecl jtagatlantic_get_error(char const * *)
        [DllImport("jtag_atlantic.dll", EntryPoint = "?jtagatlantic_get_error@@YA?AW4JATL_ERROR@@PAPBD@Z", CallingConvention = CallingConvention.Cdecl)]
        private static extern JATL_ERROR _jtagatlantic_get_error(ref string empty);

        //  void __cdecl jtagatlantic_get_info(struct JTAGATLANTIC *,char const * *,int *,int *)
        [DllImport("jtag_atlantic.dll", EntryPoint = "?jtagatlantic_get_info@@YAXPAUJTAGATLANTIC@@PAPBDPAH2@Z", CallingConvention = CallingConvention.Cdecl)]
        private static extern void jtagatlantic_get_info(IntPtr handle, out string cable, out int device, out int instance);

        //  int __cdecl jtagatlantic_read(struct JTAGATLANTIC *,char *,unsigned int)
        [DllImport("jtag_atlantic.dll", EntryPoint = "?jtagatlantic_read@@YAHPAUJTAGATLANTIC@@PADI@Z", CallingConvention = CallingConvention.Cdecl)]
        private static extern int jtagatlantic_read(IntPtr handle, byte[] data, uint length);

        //  int __cdecl jtagatlantic_write(struct JTAGATLANTIC *,char const *,unsigned int)
        [DllImport("jtag_atlantic.dll", EntryPoint = "?jtagatlantic_write@@YAHPAUJTAGATLANTIC@@PBDI@Z", CallingConvention = CallingConvention.Cdecl)]
        private static extern int jtagatlantic_write(IntPtr handle, byte[] data, uint length);

        //  void __cdecl jtagatlantic_close(struct JTAGATLANTIC *)
        [DllImport("jtag_atlantic.dll", EntryPoint = "?jtagatlantic_close@@YAXPAUJTAGATLANTIC@@@Z", CallingConvention = CallingConvention.Cdecl)]
        private static extern void jtagatlantic_close(IntPtr handle);

        //  int __cdecl jtagatlantic_flush(struct JTAGATLANTIC *)
        [DllImport("jtag_atlantic.dll", EntryPoint = "?jtagatlantic_flush@@YAHPAUJTAGATLANTIC@@@Z", CallingConvention = CallingConvention.Cdecl)]
        private static extern int jtagatlantic_flush(IntPtr handle);

        IntPtr JTAGHandle = IntPtr.Zero;

        private static JATL_ERROR jtagatlantic_get_error()
        {
            string empty = null;
            return _jtagatlantic_get_error(ref empty);
        }

        public JTAGSerialStream(string Cable, int Device, int Instance)
        {
            JTAGHandle = jtagatlantic_open(Cable, Device, Instance, "MIPS Communication Server");
            if (JTAGHandle == IntPtr.Zero)
            {
                throw new JTAGException("Error connecting: " + jtagatlantic_get_error().ToString());
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
            /*if (jtagatlantic_flush(JTAGHandle) == -1)
            {
                throw new JTAGException("Error during flush");
            }*/
        }
        
        public override int Read(byte[] buffer, int offset, int count)
        {
            byte[] ReadBuf = new byte[count];

            int AmountRead;
            do
            {
                AmountRead = jtagatlantic_read(JTAGHandle, ReadBuf, (uint)count);

                if (AmountRead == -1)
                {
                    throw new JTAGException("Error during read");
                }
            } while (AmountRead == 0);
            
            Array.Copy(ReadBuf, 0, buffer, offset, AmountRead);

            return AmountRead;
        }

        public override int ReadByte()
        {
            byte[] ReadBuf = new byte[1];

            int AmountRead;
            do
            {
                AmountRead = jtagatlantic_read(JTAGHandle, ReadBuf, 1);

                if (AmountRead == -1)
                {
                    throw new JTAGException("Error during read");
                }

            } while (AmountRead != 1);

            return ReadBuf[0];
        }

        public override void Write(byte[] buffer, int offset, int count)
        {
            int AmountWritten = 0;
            do
            {
                byte[] WriteBuf = new byte[count - AmountWritten];
                Array.Copy(buffer, offset + AmountWritten, WriteBuf, 0, count - AmountWritten);
                
                int AmountThisWrite = jtagatlantic_write(JTAGHandle, WriteBuf, (uint)count);

                if(AmountThisWrite == -1)
                {
                    throw new JTAGException("Error during write");
                }

                AmountWritten += AmountThisWrite;
            } while (AmountWritten < count);
        }

        protected override void  Dispose(bool disposing)
        {
            if (JTAGHandle != IntPtr.Zero)
            {
                jtagatlantic_close(JTAGHandle);
                JTAGHandle = IntPtr.Zero;
            }

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

    class JTAGException : Exception
    {
        public JTAGException(string Error) : base(Error) {}
    }
}
