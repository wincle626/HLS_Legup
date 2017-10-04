using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Net.Sockets;
using System.Net;

namespace Mips_Communication_Server
{   
    class NetworkController
    {
        private const int DebugPort = 1337;
        private const int ControlPort = 1338;

        private bool DeviceInUse = false;
        private bool Disconnecting = false;

        private Socket DebugListenSocket = null;
        private Socket ControlListenSocket = null;
        private Socket CurrentNetworkSocket = null;

        private byte[] networkBuf = new byte[1024];

        private DebugCommandProcessor debugProcessor;
        private ControlCommandProcessor controlProcessor;
        
        public event EventNotify Error;
        public event EventNotify ConnectionSuccess;
        
        public delegate void EventNotify(object sender);

        private delegate byte[] ProcessData(byte[] buf, int length);

        private enum SocketPurpose
        {
            Debug,
            Control
        };

        private string SocketName(SocketPurpose Purpose)
        {
            if(Purpose == SocketPurpose.Debug)
                return "Debug";
            if(Purpose == SocketPurpose.Control)
                return "Control";

            return "";
        }

        public NetworkController(Device DebugDevice)
        {
            debugProcessor = new DebugCommandProcessor(DebugDevice);
            controlProcessor = new ControlCommandProcessor(DebugDevice);
        }

        public void Connect()
        {
            Disconnecting = false;
            StartListen();
        }
        
        private void StartListen()
        {
            lock (this)
            {
                if (Disconnecting)
                    return;

                try
                {
                    debugProcessor.Reset();
                    controlProcessor.Reset();

                    if (DebugListenSocket == null)
                    {
                        DebugListenSocket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
                        IPEndPoint DebugEP = new IPEndPoint(IPAddress.Any, DebugPort);
                        DebugListenSocket.Bind(DebugEP);
                        DebugListenSocket.Listen(1);

                        Socket ListenSocket = DebugListenSocket;

                        DebugListenSocket.BeginAccept(
                            delegate(IAsyncResult ar) { ConnectionAccept(SocketPurpose.Debug, ListenSocket, ar); },
                            null);
                    }

                    if (ControlListenSocket == null)
                    {
                        ControlListenSocket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
                        IPEndPoint ControlEP = new IPEndPoint(IPAddress.Any, ControlPort);
                        ControlListenSocket.Bind(ControlEP);
                        ControlListenSocket.Listen(1);

                        Socket ListenSocket = ControlListenSocket;

                        ControlListenSocket.BeginAccept(
                            delegate(IAsyncResult ar) { ConnectionAccept(SocketPurpose.Control, ListenSocket, ar); },
                            null);
                    }

                    OnConnectionSuccess();
                }
                catch (SocketException e)
                {
                    Console.WriteLine("Error setting up listening network sockets, (errcode: {0})", e.SocketErrorCode);

                    OnError();
                }
                catch (System.Security.SecurityException)
                {
                    Console.WriteLine("Error setting up listening network sockets, operation disallowed by security");

                    OnError();
                }
                catch (ObjectDisposedException)
                {
                    Console.WriteLine("Error setting up listening network sockets, socket was closed");

                    OnError();
                }
            }
        }

        public void Disconnect()
        {
            lock (this)
            {
                if (Disconnecting)
                    return;
                
                Disconnecting = true;

                CloseDebug();
                CloseControl();

                if (CurrentNetworkSocket != null)
                {
                    CurrentNetworkSocket.Close();
                    CurrentNetworkSocket = null;
                }
            }
        }

        private void OnError()
        {
            if (Error != null)
                Error(this);
        }

        private void OnConnectionSuccess()
        {
            if (ConnectionSuccess != null)
                ConnectionSuccess(this);
        }

        private void CloseDebug()
        {
            if (DebugListenSocket != null)
                DebugListenSocket.Close();

            DebugListenSocket = null;
        }

        private void CloseControl()
        {
            if (ControlListenSocket != null)
                ControlListenSocket.Close();

            ControlListenSocket = null;
        }

        private void ConnectionAccept(SocketPurpose Purpose, Socket ListenSocket, IAsyncResult ar)
        {
            bool SetDeviceInUse = false;
            Socket NetworkSocket = null;
            
            try
            {
                NetworkSocket = ListenSocket.EndAccept(ar);
                IPEndPoint RemoteEP = (IPEndPoint)NetworkSocket.RemoteEndPoint;
                System.Console.WriteLine("{0} connected from {1}:{2}", SocketName(Purpose),
                    RemoteEP.Address, RemoteEP.Port);
                
                lock (this)
                {
                    if (DeviceInUse) //Something already using the device
                    {
                        System.Console.WriteLine("Device in use, closing {0} connection", SocketName(Purpose));

                        NetworkSocket.Close();
                        NetworkSocket = null;
                        
                        CloseListenSocket(Purpose);

                        return;
                    }
                    else //Otherwise set the flag as we're using the device
                    {
                        DeviceInUse = true;
                        SetDeviceInUse = true;
                        
                        CurrentNetworkSocket = NetworkSocket;

                        CloseListenSocket(Purpose);
                    }
                }
                
                NetworkSocket.BeginReceive(networkBuf, 0, 1024, SocketFlags.None, 
                    delegate(IAsyncResult AsR){NetworkReceive(Purpose, NetworkSocket, AsR);}, 
                    null); 
            }
            catch (SocketException e)
            {
                Console.WriteLine("Error accepting connection from {0} (errcode: {1})", SocketName(Purpose), e.SocketErrorCode);
                CloseListenSocket(Purpose);

                if (NetworkSocket != null)
                    NetworkSocket.Close();

                if (SetDeviceInUse)
                    DeviceInUse = false;
                
                StartListen();
            }
            catch (ObjectDisposedException)
            {                
                Console.WriteLine("Error accepting connection from {0}, the socket was closed", SocketName(Purpose));
                CloseListenSocket(Purpose);

                if (NetworkSocket != null)
                    NetworkSocket.Close();

                if (SetDeviceInUse)
                    DeviceInUse = false;

                StartListen();
            }
        }

        private void CloseListenSocket(SocketPurpose Purpose)
        {
            if (Purpose == SocketPurpose.Debug)
                CloseDebug();
            else if (Purpose == SocketPurpose.Control)
                CloseControl();
        }

        private void NetworkReceive(SocketPurpose Purpose, Socket NetworkSocket, IAsyncResult ar)
        {            
            try
            {
                int length = NetworkSocket.EndReceive(ar);
                if (length > 0)
                {
                    ProcessData Processor = null;

                    if (Purpose == SocketPurpose.Debug)
                        Processor = debugProcessor.Process;
                    else if (Purpose == SocketPurpose.Control)
                        Processor = controlProcessor.Process;

                    if (Processor != null)
                    {
                        Processor.BeginInvoke(networkBuf, length,
                            delegate(IAsyncResult AsR) { ProcessFinish(Processor, Purpose, NetworkSocket, AsR); },
                            null);
                    }
                    else
                    {
                        throw new Exception("In network receive with invalid purpose, this should not happen");
                    }

                }
                else 
                {
                    Console.WriteLine("{0} connection closed", SocketName(Purpose));

                    DeviceInUse = false;
                    StartListen();
                }
            }
            catch (SocketException e)
            {
                OnSocketException(Purpose, NetworkSocket, e);
            }
            catch (ObjectDisposedException)
            {
                OnObjectDisposedException(Purpose, NetworkSocket);
            }
        }

        private void NetworkWrite(ProcessData Processor, SocketPurpose Purpose, Socket NetworkSocket, byte[] Data, int offset, IAsyncResult ar)
        {
            try
            {
                int length = NetworkSocket.EndSend(ar);

                if (length + offset < Data.Length)
                {
                    NetworkSocket.BeginSend(Data, length, Data.Length - (length + offset), SocketFlags.None,
                        delegate(IAsyncResult AsR) { NetworkWrite(Processor, Purpose, NetworkSocket, Data, length + offset, AsR); },
                        null);
                }
                else
                {
                    NetworkSocket.BeginReceive(networkBuf, 0, 1024, SocketFlags.None,
                            delegate(IAsyncResult AsR) { NetworkReceive(Purpose, NetworkSocket, AsR); },
                            null);
                }
            }
            catch (SocketException e)
            {
                OnSocketException(Purpose, NetworkSocket, e);
            }
            catch (ObjectDisposedException)
            {
                OnObjectDisposedException(Purpose, NetworkSocket);
            }
        }

        private void ProcessFinish(ProcessData Processor, SocketPurpose Purpose, Socket NetworkSocket, IAsyncResult ar)
        {
            try
            {
                byte[] writeData = Processor.EndInvoke(ar);

                if (writeData != null)
                {
                    NetworkSocket.BeginSend(writeData, 0, writeData.Length, SocketFlags.None,
                        delegate(IAsyncResult AsR) { NetworkWrite(Processor, Purpose, NetworkSocket, writeData, 0, AsR); },
                        null);
                }
                else
                {
                    NetworkSocket.BeginReceive(networkBuf, 0, 1024, SocketFlags.None,
                            delegate(IAsyncResult AsR) { NetworkReceive(Purpose, NetworkSocket, AsR); },
                            null);
                }
            }
            catch (JTAGException e)
            {
                Console.WriteLine("JTAG Error while procesing.  {0}", e.Message);

                Disconnect();
                DeviceInUse = false;

                OnError();
            }
            catch (SocketException e)
            {
                OnSocketException(Purpose, NetworkSocket, e);
            }
            catch (ObjectDisposedException)
            {
                OnObjectDisposedException(Purpose, NetworkSocket);
            }
        }

        private void OnObjectDisposedException(SocketPurpose Purpose, Socket NetworkSocket)
        {
            Console.WriteLine("Error on {0} connection, the connection was closed", SocketName(Purpose));

            NetworkSocket.Close();

            DeviceInUse = false;
            StartListen();
        }

        private void OnSocketException(SocketPurpose Purpose, Socket NetworkSocket, SocketException e)
        {
            if (!NetworkSocket.Connected)
            {
                Console.WriteLine("{0} connection closed (errcode: {1})", SocketName(Purpose), e.SocketErrorCode);
            }
            else
            {
                Console.WriteLine("Error on {0} connection, closing (errcode: {1})", SocketName(Purpose), e.SocketErrorCode);
            }

            NetworkSocket.Close();

            DeviceInUse = false;
            StartListen();
        }
    }
}
