﻿using System;
using System.Runtime.InteropServices;

namespace DynSec_Wrapper
{
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct Callbacks
    {

    }
    public class DynSecAntiCheat
    {
        [DllImport("Client.dll", EntryPoint="InitializeClient", CallingConvention = CallingConvention.StdCall)]
        private static extern void _InitializeClient(ref Callbacks callbacks);

        public static void InitializeClient(Callbacks callbacks)
        {
            _InitializeClient(ref callbacks);
        }
    }
}
