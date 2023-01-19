/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license. 
 * You should have received a copy of the license along with this program. 
 * If not, see <https://opensource.org/licenses/MIT>.
 */

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace Loader
{
    public struct STARTUPINFO
    {
        public uint cb;
        public string lpReserved;
        public string lpDesktop;
        public string lpTitle;
        public uint dwX;
        public uint dwY;
        public uint dwXSize;
        public uint dwYSize;
        public uint dwXCountChars;
        public uint dwYCountChars;
        public uint dwFillAttribute;
        public uint dwFlags;
        public short wShowWindow;
        public short cbReserved2;
        public IntPtr lpReserved2;
        public IntPtr hStdInput;
        public IntPtr hStdOutput;
        public IntPtr hStdError;
    }

    public struct PROCESS_INFORMATION
    {
        public IntPtr hProcess;
        public IntPtr hThread;
        public uint dwProcessId;
        public uint dwThreadId;
    }

    [Flags]
    public enum ProcessCreationFlags : uint
    {
        ZERO_FLAG = 0x00000000,
        CREATE_BREAKAWAY_FROM_JOB = 0x01000000,
        CREATE_DEFAULT_ERROR_MODE = 0x04000000,
        CREATE_NEW_CONSOLE = 0x00000010,
        CREATE_NEW_PROCESS_GROUP = 0x00000200,
        CREATE_NO_WINDOW = 0x08000000,
        CREATE_PROTECTED_PROCESS = 0x00040000,
        CREATE_PRESERVE_CODE_AUTHZ_LEVEL = 0x02000000,
        CREATE_SEPARATE_WOW_VDM = 0x00001000,
        CREATE_SHARED_WOW_VDM = 0x00001000,
        CREATE_SUSPENDED = 0x00000004,
        CREATE_UNICODE_ENVIRONMENT = 0x00000400,
        DEBUG_ONLY_THIS_PROCESS = 0x00000002,
        DEBUG_PROCESS = 0x00000001,
        DETACHED_PROCESS = 0x00000008,
        EXTENDED_STARTUPINFO_PRESENT = 0x00080000,
        INHERIT_PARENT_AFFINITY = 0x00010000
    }

    public enum ProcessExitCodes : uint
    {
        STILL_ACTIVE = 259
    }

    public enum ObjectInformationClass : int
    {
        ObjectBasicInformation = 0,
        ObjectNameInformation = 1,
        ObjectTypeInformation = 2,
        ObjectAllTypesInformation = 3,
        ObjectHandleInformation = 4
    }

    [Flags]
    public enum ProcessAccessFlags : uint
    {
        All = 0x001F0FFF,
        Terminate = 0x00000001,
        CreateThread = 0x00000002,
        VMOperation = 0x00000008,
        VMRead = 0x00000010,
        VMWrite = 0x00000020,
        DupHandle = 0x00000040,
        SetInformation = 0x00000200,
        QueryInformation = 0x00000400,
        Synchronize = 0x00100000
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct OBJECT_BASIC_INFORMATION
    {
        public int Attributes;
        public int GrantedAccess;
        public int HandleCount;
        public int PointerCount;
        public int PagedPoolUsage;
        public int NonPagedPoolUsage;
        public int Reserved1;
        public int Reserved2;
        public int Reserved3;
        public int NameInformationLength;
        public int TypeInformationLength;
        public int SecurityDescriptorLength;
        public System.Runtime.InteropServices.ComTypes.FILETIME CreateTime;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct OBJECT_TYPE_INFORMATION
    {
        public UNICODE_STRING Name;
        public int ObjectCount;
        public int HandleCount;
        public int Reserved1;
        public int Reserved2;
        public int Reserved3;
        public int Reserved4;
        public int PeakObjectCount;
        public int PeakHandleCount;
        public int Reserved5;
        public int Reserved6;
        public int Reserved7;
        public int Reserved8;
        public int InvalidAttributes;
        public GENERIC_MAPPING GenericMapping;
        public int ValidAccess;
        public byte Unknown;
        public byte MaintainHandleDatabase;
        public int PoolType;
        public int PagedPoolUsage;
        public int NonPagedPoolUsage;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct OBJECT_NAME_INFORMATION
    {
        public UNICODE_STRING Name;
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    public struct UNICODE_STRING
    {
        public readonly ushort Length;
        public readonly ushort MaximumLength;
        [MarshalAs(UnmanagedType.LPWStr)]
        public readonly string Buffer;

        public UNICODE_STRING(string s)
        {
            Length = (ushort)(s.Length * 2);
            MaximumLength = (ushort)(Length + 2);
            Buffer = s;
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct GENERIC_MAPPING
    {
        public int GenericRead;
        public int GenericWrite;
        public int GenericExecute;
        public int GenericAll;
    }

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct SYSTEM_HANDLE_INFORMATION
    {
        public int ProcessID;
        public byte ObjectTypeNumber;
        public byte Flags; // 0x01 = PROTECT_FROM_CLOSE, 0x02 = INHERIT
        public ushort Handle;
        public int Object_Pointer;
        public UInt32 GrantedAccess;
    }

    public enum MemoryProtectionMode : uint
    {
        PAGE_EXECUTE = 0x10,
        PAGE_EXECUTE_READ = 0x20,
        PAGE_EXECUTE_READWRITE = 0x40,
        PAGE_EXECUTE_WRITECOPY = 0x80,
        PAGE_NOACCESS = 0x01,
        PAGE_READONLY = 0x02,
        PAGE_READWRITE = 0x04,
        PAGE_WRITECOPY = 0x08,
    }
    
    public enum DwFilterFlag : uint
    {
        LIST_MODULES_DEFAULT = 0x0,
        LIST_MODULES_32BIT = 0x01,
        LIST_MODULES_64BIT = 0x02,
        LIST_MODULES_ALL = (LIST_MODULES_32BIT | LIST_MODULES_64BIT)
    }
    
    [Flags]
    public enum AllocationType
    {
         Commit = 0x1000,
         Reserve = 0x2000,
         Decommit = 0x4000,
         Release = 0x8000,
         Reset = 0x80000,
         Physical = 0x400000,
         TopDown = 0x100000,
         WriteWatch = 0x200000,
         LargePages = 0x20000000
    }

    [Flags]
    public enum MemoryProtection
    {
         Execute = 0x10,
         ExecuteRead = 0x20,
         ExecuteReadWrite = 0x40,
         ExecuteWriteCopy = 0x80,
         NoAccess = 0x01,
         ReadOnly = 0x02,
         ReadWrite = 0x04,
         WriteCopy = 0x08,
         GuardModifierflag = 0x100,
         NoCacheModifierflag = 0x200,
         WriteCombineModifierflag = 0x400
    }


    [StructLayout(LayoutKind.Sequential)]
    public struct MODULEINFO
    {
        public IntPtr lpBaseOfDll;
        public uint SizeOfImage;
        public IntPtr EntryPoint;
    }

    public static class WinAPI
    {
        [DllImport("kernel32.dll")]
        public static extern bool CreateProcess(string lpApplicationName,
                string lpCommandLine, IntPtr lpProcessAttributes,
                IntPtr lpThreadAttributes,
                bool bInheritHandles, ProcessCreationFlags dwCreationFlags,
                IntPtr lpEnvironment, string lpCurrentDirectory,
                ref STARTUPINFO lpStartupInfo,
                out PROCESS_INFORMATION lpProcessInformation);
                
        [DllImport("psapi.dll", SetLastError = true)]
        public static extern bool EnumProcessModulesEx(
            IntPtr hProcess,
            [Out] IntPtr lphModule,
            UInt32 cb,
            [MarshalAs(UnmanagedType.U4)] out UInt32 lpcbNeeded,
            DwFilterFlag dwff);

        [DllImport("kernel32.dll")]
        public static extern uint ResumeThread(IntPtr hThread);

        [DllImport("kernel32.dll")]
        public static extern uint SuspendThread(IntPtr hThread);
        
        [DllImport("kernel32.dll")]
        public static extern int GetLastError();

        [DllImport("kernel32.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool GetExitCodeProcess(IntPtr hProcess, out uint lpExitCode);

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern bool WriteProcessMemory(IntPtr hProcess, IntPtr lpBaseAddress, byte[] lpBuffer, uint nSize, out int lpNumberOfBytesWritten);

        [DllImport("ntdll.dll")]
        public static extern int NtQueryObject(IntPtr ObjectHandle, int
            ObjectInformationClass, IntPtr ObjectInformation, int ObjectInformationLength,
            ref int returnLength);

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern uint QueryDosDevice(string lpDeviceName, StringBuilder lpTargetPath, int ucchMax);

        [DllImport("ntdll.dll")]
        public static extern uint NtQuerySystemInformation(int
            SystemInformationClass, IntPtr SystemInformation, int SystemInformationLength,
            ref int returnLength);

        [DllImport("kernel32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        public static extern IntPtr OpenMutex(UInt32 desiredAccess, bool inheritHandle, string name);

        [DllImport("kernel32.dll")]
        public static extern IntPtr OpenProcess(ProcessAccessFlags dwDesiredAccess, [MarshalAs(UnmanagedType.Bool)] bool bInheritHandle, int dwProcessId);

        [DllImport("kernel32.dll")]
        public static extern int CloseHandle(IntPtr hObject);

        [DllImport("kernel32.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool DuplicateHandle(IntPtr hSourceProcessHandle,
           ushort hSourceHandle, IntPtr hTargetProcessHandle, out IntPtr lpTargetHandle,
           uint dwDesiredAccess, [MarshalAs(UnmanagedType.Bool)] bool bInheritHandle, uint dwOptions);

        [DllImport("kernel32.dll")]
        public static extern IntPtr GetCurrentProcess();

        [DllImport("kernel32", CharSet=CharSet.Ansi, ExactSpelling=true, SetLastError=true)]
        public static extern IntPtr GetProcAddress(IntPtr hModule, string procName);

        [DllImport("kernel32.dll", EntryPoint="GetModuleHandleA", SetLastError=true)]
        public static extern IntPtr GetModuleHandle(string moduleName);
        
        [DllImport("kernel32.dll", SetLastError=true, ExactSpelling=true)]
        public static extern IntPtr VirtualAllocEx(IntPtr hProcess, IntPtr lpAddress, uint dwSize, uint flAllocationType, uint flProtect);

        [DllImport("psapi.dll", SetLastError = true)]
        static extern bool GetModuleInformation(IntPtr hProcess, IntPtr hModule, out MODULEINFO lpmodinfo, uint cb);

        [DllImport("kernel32.dll")]
        public static extern IntPtr CreateRemoteThread(IntPtr hProcess, IntPtr lpThreadAttributes, uint dwStackSize, IntPtr lpStartAddress, IntPtr lpParameter, uint dwCreationFlags, IntPtr lpThreadId);

        public static IntPtr GetProcessModuleBaseAddress(IntPtr hProcess)
        {
            List<String> moduleNames = new List<String>();

            IntPtr[] hMods = new IntPtr[1024];

            GCHandle hPinnedModules = GCHandle.Alloc(hMods, GCHandleType.Pinned); 
            IntPtr pModules = hPinnedModules.AddrOfPinnedObject();

            uint uiSize = (uint)(Marshal.SizeOf(typeof(IntPtr)) * hMods.Length);
            uint cbNeeded = 0;

            if (EnumProcessModulesEx(hProcess, pModules, uiSize, out cbNeeded, DwFilterFlag.LIST_MODULES_ALL) == true)
            {
                Int32 uiTotalNumberofModules = (Int32)(cbNeeded / (Marshal.SizeOf(typeof(IntPtr))));

                for (int i = 0; i < (int)uiTotalNumberofModules; i++)
                {
                    MODULEINFO modInfo;
                    uint modInfoSize = (uint)Marshal.SizeOf(typeof(MODULEINFO));
                    if (GetModuleInformation(hProcess, hMods[i], out modInfo, modInfoSize))
                    {
                        // First module is the one we want.
                        return modInfo.lpBaseOfDll;
                    }
                }
            }

            int error = GetLastError();

            hPinnedModules.Free();

            return (IntPtr)0;
        }
    }

    // Based on the code from SO here, but fixed up so it actually works:
    // https://stackoverflow.com/questions/6808831/delete-a-mutex-from-another-process
    // This badly needs cleaning up, its a total mess.

    public class WinAPIProcesses
    {
        public const int MAX_PATH = 260;
        public const uint STATUS_INFO_LENGTH_MISMATCH = 0xC0000004;
        public const int DUPLICATE_SAME_ACCESS = 0x2;
        public const int DUPLICATE_CLOSE_SOURCE = 0x1;
        public const int CNST_SYSTEM_HANDLE_INFORMATION = 16;

        public static string getObjectTypeName(SYSTEM_HANDLE_INFORMATION shHandle, Process process)
        {
            IntPtr m_ipProcessHwnd = WinAPI.OpenProcess(ProcessAccessFlags.All, false, process.Id);
            IntPtr ipHandle = IntPtr.Zero;
            var objBasic = new OBJECT_BASIC_INFORMATION();
            IntPtr ipBasic = IntPtr.Zero;
            var objObjectType = new OBJECT_TYPE_INFORMATION();
            IntPtr ipObjectType = IntPtr.Zero;
            IntPtr ipObjectName = IntPtr.Zero;
            string strObjectTypeName = "";
            int nLength = 0;
            int nReturn = 0;
            IntPtr ipTemp = IntPtr.Zero;

            if (!WinAPI.DuplicateHandle(
                m_ipProcessHwnd, 
                shHandle.Handle,
                WinAPI.GetCurrentProcess(), 
                out ipHandle,
                0, 
                false,
                DUPLICATE_SAME_ACCESS))
            {
                return null;
            }

            ipBasic = Marshal.AllocHGlobal(Marshal.SizeOf(objBasic));
            WinAPI.NtQueryObject(
                ipHandle, 
                (int)ObjectInformationClass.ObjectBasicInformation,
                ipBasic, 
                Marshal.SizeOf(objBasic), 
                ref nLength);
            objBasic = (OBJECT_BASIC_INFORMATION)Marshal.PtrToStructure(ipBasic, objBasic.GetType());
            Marshal.FreeHGlobal(ipBasic);

            ipObjectType = Marshal.AllocHGlobal(objBasic.TypeInformationLength);
            nLength = objBasic.TypeInformationLength;
            while ((uint)(nReturn = WinAPI.NtQueryObject(
                            ipHandle, 
                            (int)ObjectInformationClass.ObjectTypeInformation, 
                            ipObjectType,
                            nLength, 
                            ref nLength)) == STATUS_INFO_LENGTH_MISMATCH)
            {
                Marshal.FreeHGlobal(ipObjectType);
                ipObjectType = Marshal.AllocHGlobal(nLength);
            }

            objObjectType = Marshal.PtrToStructure<OBJECT_TYPE_INFORMATION>(ipObjectType);
            strObjectTypeName = objObjectType.Name.Buffer;

            Marshal.FreeHGlobal(ipObjectType);
            WinAPI.CloseHandle(ipHandle);

            return strObjectTypeName;
        }

        public static string getObjectName(SYSTEM_HANDLE_INFORMATION shHandle, Process process)
        {
            IntPtr m_ipProcessHwnd = WinAPI.OpenProcess(ProcessAccessFlags.All, false, process.Id);
            IntPtr ipHandle = IntPtr.Zero;
            var objBasic = new OBJECT_BASIC_INFORMATION();
            IntPtr ipBasic = IntPtr.Zero;
            IntPtr ipObjectType = IntPtr.Zero;
            var objObjectName = new OBJECT_NAME_INFORMATION();
            IntPtr ipObjectName = IntPtr.Zero;
            int nLength = 0;
            int nReturn = 0;
            IntPtr ipTemp = IntPtr.Zero;

            if (!WinAPI.DuplicateHandle(
                    m_ipProcessHwnd,
                    shHandle.Handle,
                    WinAPI.GetCurrentProcess(),
                    out ipHandle,
                    0,
                    false,
                    DUPLICATE_SAME_ACCESS))
            {
                return null;
            }

            ipBasic = Marshal.AllocHGlobal(Marshal.SizeOf(objBasic));
            WinAPI.NtQueryObject(
                ipHandle, 
                (int)ObjectInformationClass.ObjectBasicInformation,
                ipBasic, 
                Marshal.SizeOf(objBasic), 
                ref nLength);
            objBasic = (OBJECT_BASIC_INFORMATION)Marshal.PtrToStructure(ipBasic, objBasic.GetType());
            Marshal.FreeHGlobal(ipBasic);

            nLength = objBasic.NameInformationLength;

            ipObjectName = Marshal.AllocHGlobal(nLength);
            while ((uint)(nReturn = WinAPI.NtQueryObject(
                     ipHandle, 
                     (int)ObjectInformationClass.ObjectNameInformation,
                     ipObjectName, 
                     nLength, 
                     ref nLength)) == STATUS_INFO_LENGTH_MISMATCH)
            {
                Marshal.FreeHGlobal(ipObjectName);
                ipObjectName = Marshal.AllocHGlobal(nLength);
            }
            objObjectName = (OBJECT_NAME_INFORMATION)Marshal.PtrToStructure(ipObjectName, objObjectName.GetType());
            
            string result = objObjectName.Name.Buffer;

            Marshal.FreeHGlobal(ipObjectName);
            WinAPI.CloseHandle(ipHandle);

            return result;
        }

        public static List<SYSTEM_HANDLE_INFORMATION> GetHandles(Process process = null, string IN_strObjectTypeName = null, string IN_strObjectName = null)
        {
            uint nStatus;
            int nHandleInfoSize = 0x10000;
            IntPtr ipHandlePointer = Marshal.AllocHGlobal(nHandleInfoSize);
            int nLength = 0;
            IntPtr ipHandle = IntPtr.Zero;

            while ((nStatus = WinAPI.NtQuerySystemInformation(
                                    CNST_SYSTEM_HANDLE_INFORMATION, 
                                    ipHandlePointer,
                                    nHandleInfoSize, 
                                    ref nLength)) == STATUS_INFO_LENGTH_MISMATCH)
            {
                nHandleInfoSize = nLength;
                Marshal.FreeHGlobal(ipHandlePointer);
                ipHandlePointer = Marshal.AllocHGlobal(nLength);
            }

            byte[] baTemp = new byte[nLength];
            Marshal.Copy(ipHandlePointer, baTemp, 0, nLength);

            long lHandleCount = 0;
            if (Is64Bits())
            {
                lHandleCount = Marshal.ReadInt64(ipHandlePointer);
                ipHandle = new IntPtr(ipHandlePointer.ToInt64() + 8);
            }
            else
            {
                lHandleCount = Marshal.ReadInt32(ipHandlePointer);
                ipHandle = new IntPtr(ipHandlePointer.ToInt32() + 4);
            }

            SYSTEM_HANDLE_INFORMATION shHandle;
            List<SYSTEM_HANDLE_INFORMATION> lstHandles = new List<SYSTEM_HANDLE_INFORMATION>();

            for (long lIndex = 0; lIndex < lHandleCount; lIndex++)
            {
                shHandle = new SYSTEM_HANDLE_INFORMATION();
                if (Is64Bits())
                {
                    shHandle = (SYSTEM_HANDLE_INFORMATION)Marshal.PtrToStructure(ipHandle, shHandle.GetType());
                    ipHandle = new IntPtr(ipHandle.ToInt64() + Marshal.SizeOf(shHandle) + 8);
                }
                else
                {
                    ipHandle = new IntPtr(ipHandle.ToInt64() + Marshal.SizeOf(shHandle));
                    shHandle = (SYSTEM_HANDLE_INFORMATION)Marshal.PtrToStructure(ipHandle, shHandle.GetType());
                }

                if (process != null)
                {
                    if (shHandle.ProcessID != process.Id) continue;
                }

                string strObjectTypeName = "";
                if (IN_strObjectTypeName != null)
                {
                    strObjectTypeName = getObjectTypeName(shHandle, Process.GetProcessById(shHandle.ProcessID));
                    if (strObjectTypeName != IN_strObjectTypeName) continue;
                }

                string strObjectName = "";
                if (IN_strObjectName != null)
                {
                    strObjectName = getObjectName(shHandle, Process.GetProcessById(shHandle.ProcessID));
                    if (strObjectName != IN_strObjectName) continue;
                }

                string strObjectTypeName2 = getObjectTypeName(shHandle, Process.GetProcessById(shHandle.ProcessID));
                string strObjectName2 = getObjectName(shHandle, Process.GetProcessById(shHandle.ProcessID));

                lstHandles.Add(shHandle);
            }

            return lstHandles;
        }

        public static bool Is64Bits()
        {
            return Marshal.SizeOf(typeof(IntPtr)) == 8 ? true : false;
        }

        public static bool KillMutex(Process Proc, string Name)
        {
            try
            {
                var handles = WinAPIProcesses.GetHandles(Proc, "Mutant", Name);
                if (handles.Count == 0) 
                {
                    throw new System.ArgumentException("NoMutex", "original");
                }

                foreach (var handle in handles)
                {
                    IntPtr ipHandle = IntPtr.Zero;
                    if (WinAPI.DuplicateHandle(
                            Process.GetProcessById(handle.ProcessID).Handle,
                            handle.Handle,
                            WinAPI.GetCurrentProcess(),
                            out ipHandle,
                            0,
                            false,
                            DUPLICATE_CLOSE_SOURCE))
                    {
                        WinAPI.CloseHandle(ipHandle);
                        return true;
                    }
                }
            }
            catch (IndexOutOfRangeException)
            {
                return false;
            }
            catch (ArgumentException)
            {
                return false;
            }

            return true;
        }
    }
}
