using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;
using System.IO;

namespace JTAGTerminal
{
    class Program
    {
        static JTAGSerialStream2 SerialStream = null;
        static byte[] SerialReadBuf = new byte[1024];
        static char[] ConsoleReadBuf = new char[1024];
        
        static void Main(string[] args)
        {
            if (args.Length != 1)
            {
                Console.WriteLine("Usage: JTAGTerminal cable_name");

                return;
            }

            try
            {
                SerialStream = new JTAGSerialStream2(args[0], 0, 0);
            }
            catch (JTAGException e)
            {
                Console.Error.WriteLine("Error starting terminal: {0}\nQuiting...", e.Message);
                return;
            }

            SerialStream.BeginRead(SerialReadBuf, 0, SerialReadBuf.Length, RecvData, null);

            while (true)
            {
                try
                {
                    int AmountRead = Console.In.Read(ConsoleReadBuf, 0, ConsoleReadBuf.Length);
                    byte[] ConsoleReadBufBytes = Encoding.ASCII.GetBytes(ConsoleReadBuf, 0, ConsoleReadBuf.Length);

                    SerialStream.Write(ConsoleReadBufBytes, 0, AmountRead);
                }
                catch (JTAGException e)
                {
                    Console.Error.WriteLine("Error during send: {0}\nQuitting...", e.Message);
                    return;
                }
                catch (IOException)
                {
                    Console.Error.WriteLine("Error during send: {0}\nQuitting...");
                    return;
                }
                catch (ObjectDisposedException)
                {
                    Console.Error.WriteLine("Error during send: {0}\nQuitting...");
                    return;
                }
            }
        }

        static void RecvData(IAsyncResult ar)
        {
            try
            {
                int AmountRead = SerialStream.EndRead(ar);
                Console.Write(Encoding.ASCII.GetString(SerialReadBuf, 0, AmountRead));

                SerialStream.BeginRead(SerialReadBuf, 0, SerialReadBuf.Length, RecvData, null);
            }
            catch (JTAGException e)
            {
                Console.Error.WriteLine("Error during receive: {0}\nQuiting...", e.Message);
                System.Diagnostics.Process.GetCurrentProcess().CloseMainWindow();
            }
            catch (IOException)
            {
                Console.Error.WriteLine("Error during receive\nQuitting...");
                System.Diagnostics.Process.GetCurrentProcess().CloseMainWindow();
            }
        }
    }
}
