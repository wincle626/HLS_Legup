using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

namespace Mips_Communication_Server
{
    class JTAGClient
    {
        private enum AJI_ERROR : int
        {
            E_NONE = 0,
            E_IN_USE = 36,
            E_GOT_NUM = 47
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
        public struct AJI_HARDWARE
        {
            public IntPtr chain;
            public uint u1;
            public string cableName;
            public string cablePort;
            public uint u2;
            public uint u3;
            public uint u4;
            public uint u5;

            public override string ToString()
            {
                return String.Format("{0} [{1}]", cableName, cablePort);
            }
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
        public struct AJI_DEVICE
        {
            public uint deviceID;
            public IntPtr chain;
            public uint u1;
            public uint u2;
            public string deviceName;
        }
        
        [DllImport("jtag_client.dll", EntryPoint = "?aji_get_hardware@@YA?AW4AJI_ERROR@@PAKPAUAJI_HARDWARE@@K@Z", CallingConvention = CallingConvention.Cdecl)]
        private static extern AJI_ERROR aji_get_hardware(ref uint hardwareAmount, [Out] AJI_HARDWARE[] hardware, uint timeout);

        [DllImport("jtag_client.dll", EntryPoint = "?aji_lock_chain@@YA?AW4AJI_ERROR@@PAVAJI_CHAIN@@K@Z", CallingConvention = CallingConvention.Cdecl)]
        private static extern AJI_ERROR aji_lock_chain(IntPtr chain, uint timeout);

        [DllImport("jtag_client.dll", EntryPoint = "?aji_scan_device_chain@@YA?AW4AJI_ERROR@@PAVAJI_CHAIN@@@Z", CallingConvention = CallingConvention.Cdecl)]
        private static extern AJI_ERROR aji_scan_device_chain(IntPtr chain);

        [DllImport("jtag_client.dll", EntryPoint = "?aji_read_device_chain@@YA?AW4AJI_ERROR@@PAVAJI_CHAIN@@PAKPAUAJI_DEVICE@@_N@Z", CallingConvention = CallingConvention.Cdecl)]
        private static extern AJI_ERROR aji_read_device_chain(IntPtr chain, ref uint numDevices, [Out] AJI_DEVICE[] devices, bool unknown);

        [DllImport("jtag_client.dll", EntryPoint = "?aji_unlock_chain@@YA?AW4AJI_ERROR@@PAVAJI_CHAIN@@@Z", CallingConvention = CallingConvention.Cdecl)]
        private static extern AJI_ERROR aji_unlock_chain(IntPtr chain);

        public static AJI_HARDWARE[] GetHardware()
        {
            uint hardwareAmount = 0;
            if (aji_get_hardware(ref hardwareAmount, null, 1000) != AJI_ERROR.E_GOT_NUM)
            {
                throw new JTAGClientException("Error getting cable info");
            }

            AJI_HARDWARE[] hardware = new AJI_HARDWARE[hardwareAmount];

            if (aji_get_hardware(ref hardwareAmount, hardware, 1000) != AJI_ERROR.E_NONE)
            {
                throw new JTAGClientException("Error getting cable info");
            }

            return hardware;
        }

        public static AJI_DEVICE[] GetDevices(IntPtr Chain)
        {
            try
            {
                if (aji_lock_chain(Chain, 1000) != AJI_ERROR.E_NONE)
                {
                    throw new JTAGClientException("Error locking chain");
                }

                //If we have E_IN_USE, device chain scan still works fine
                AJI_ERROR err = aji_scan_device_chain(Chain);
                if (err != AJI_ERROR.E_NONE && err != AJI_ERROR.E_IN_USE)
                {
                    throw new JTAGClientException("Error scanning chain");
                }

                uint numDevices = 0;
                if (aji_read_device_chain(Chain, ref numDevices, null, true) != AJI_ERROR.E_GOT_NUM)
                {
                    throw new JTAGClientException("Error reading devices");
                }

                AJI_DEVICE[] devices = new AJI_DEVICE[numDevices];

                if (aji_read_device_chain(Chain, ref numDevices, devices, true) != AJI_ERROR.E_NONE)
                {
                    throw new JTAGClientException("Error reading devices");
                }

                return devices;
            }
            finally
            {
                aji_unlock_chain(Chain);
            }
        }
    }

    class JTAGClientException : Exception
    {
        public JTAGClientException(string Error) : base(Error) { }
    }
}
