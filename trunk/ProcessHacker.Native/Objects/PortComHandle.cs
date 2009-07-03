﻿/*
 * Process Hacker - 
 *   port communication handle
 *
 * Copyright (C) 2009 wj32
 * 
 * This file is part of Process Hacker.
 * 
 * Process Hacker is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Process Hacker is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Process Hacker.  If not, see <http://www.gnu.org/licenses/>.
 */

using System;
using System.Collections.Generic;
using System.Text;
using ProcessHacker.Native.Api;
using ProcessHacker.Native.Security;
using System.Runtime.InteropServices;

namespace ProcessHacker.Native.Objects
{
    public class PortComHandle : NativeHandle<PortAccess>
    {
        public static PortComHandle Connect(string portName)
        {
            NtStatus status;
            UnicodeString portNameStr = new UnicodeString(portName);
            SecurityQualityOfService securityQos = 
                new SecurityQualityOfService(SecurityImpersonationLevel.SecurityImpersonation, true, false);
            IntPtr handle;

            try
            {
                if ((status = Win32.NtConnectPort(
                    out handle,
                    ref portNameStr,
                    ref securityQos,
                    IntPtr.Zero,
                    IntPtr.Zero,
                    IntPtr.Zero,
                    IntPtr.Zero,
                    IntPtr.Zero
                    )) >= NtStatus.Error)
                    Win32.ThrowLastError(status);
            }
            finally
            {
                portNameStr.Dispose();
            }

            return new PortComHandle(handle, true);
        }

        internal PortComHandle(IntPtr handle, bool owned)
            : base(handle, owned)
        { }
    }
}
