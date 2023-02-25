using System.Runtime.InteropServices;

namespace OrbisLib2.Common.API
{
    public enum APICommands : int
    {
        /* ####### Proc functions ####### */
        PROC_START = 1,

        API_PROC_GET_LIST,
        API_PROC_LOAD_ELF,
        API_PROC_CALL, /* RPC Call. */

        PROC_END,
        /* ############################## */

        /* ####### Apps functions ####### */
        APP_START,

        API_APPS_CHECK_VER,
        API_APPS_GET_DB,
        API_APPS_GET_INFO_STR,
        API_APPS_STATUS,
        API_APPS_START,
        API_APPS_STOP,
        API_APPS_SUSPEND,
        API_APPS_RESUME,
        API_APPS_DELETE,
        API_APPS_SET_VISIBILITY,
        API_APPS_GET_VISIBILITY,

        APP_END,
        /* ############################## */

        /* ##### Debugger functions ##### */
        DBG_START,

        API_DBG_ATTACH, /* Debugger attach to target */
        API_DBG_DETACH, /* Debugger detach from target */
        API_DBG_GET_CURRENT,
        API_DBG_READ,
        API_DBG_WRITE,
        API_DBG_KILL,
        API_DBG_BREAK,
        API_DBG_RESUME,
        API_DBG_SIGNAL,
        API_DBG_STEP,
        API_DBG_STEP_OVER,
        API_DBG_STEP_OUT,
        API_DBG_GET_CALLSTACK,
        API_DBG_GET_REG,
        API_DBG_SET_REG,
        API_DBG_GET_FREG,
        API_DBG_SET_FREG,
        API_DBG_GET_DBGREG,
        API_DBG_SET_DBGREG,

        /* Remote Library functions */
        API_DBG_LOAD_LIBRARY,
        API_DBG_UNLOAD_LIBRARY,
        API_DBG_RELOAD_LIBRARY,
        API_DBG_LIBRARY_LIST,

        /* Thread Management */
        API_DBG_THREAD_LIST,
        API_DBG_THREAD_STOP,
        API_DBG_THREAD_RESUME,

        /* Breakpoint functions */
        API_DBG_BREAKPOINT_GETFREE,
        API_DBG_BREAKPOINT_SET,
        API_DBG_BREAKPOINT_UPDATE,
        API_DBG_BREAKPOINT_REMOVE,
        API_DBG_BREAKPOINT_GETINFO,
        API_DBG_BREAKPOINT_LIST,

        /* Watchpoint functions */
        API_DBG_WATCHPOINT_SET,
        API_DBG_WATCHPOINT_UPDATE,
        API_DBG_WATCHPOINT_REMOVE,
        API_DBG_WATCHPOINT_GETINFO,
        API_DBG_WATCHPOINT_LIST,

        DBG_END,
        /* ############################## */

        /* ###### Kernel functions ###### */
        KERN_START,

        API_KERN_BASE,
        API_KERN_READ,
        API_KERN_WRITE,

        KERN_END,
        /* ############################## */

        /* ###### Target functions ###### */
        TARGET_START,

        API_TARGET_INFO,
        API_TARGET_RESTMODE,
        API_TARGET_SHUTDOWN,
        API_TARGET_REBOOT,
        API_TARGET_NOTIFY,
        API_TARGET_BUZZER,
        API_TARGET_SET_LED,
        API_TARGET_DUMP_PROC,
        API_TARGET_SET_SETTINGS,
        API_TARGET_GETFILE,

        TARGET_END,
        /* ############################## */
    }

    public enum APIResults : int
    {
        API_OK = 1,

        API_ERROR_COULDNT_CONNECT,
        API_ERROR_NOT_CONNECTED,
        API_ERROR_NOT_ATTACHED,
        API_ERROR_LOST_PROC,
        API_ERROR_GENERAL,
        API_ERROR_INVALID_ADDRESS,

        //Debugger
        API_ERROR_PROC_RUNNING,
        API_ERROR_DEBUG_TO_ATTACHED,
    }

    [StructLayout(LayoutKind.Sequential, Pack = 4, CharSet = CharSet.Ansi, Size = 8), Serializable]
    public struct APIPacket
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 12)]
        public string PacketMagic;
        public int PacketVersion;
        public APICommands Command;
    }


    #region Process

    [StructLayout(LayoutKind.Sequential, Pack = 8, CharSet = CharSet.Ansi)]
    public struct ProcPacket
    {
        public int AppId;
        public int ProcessId;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
        public string Name;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 10)]
        public string TitleId;
    }

    #endregion

    #region Apps

    public enum AppState
    {
        STATE_ERROR = -1,
        STATE_NOT_RUNNING,
        STATE_RUNNING,
        STATE_SUSPENDED,
    };

    [StructLayout(LayoutKind.Sequential, Pack = 8, CharSet = CharSet.Ansi, Size = 652), Serializable]
    public struct AppInfoPacket
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 10)]
        public string TitleId;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 100)]
        public string ContentId;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 200)]
        public byte[] TitleName;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 100)]
        public string MetaDataPath;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 100)]
        public string LastAccessTime;
        public int Visible;
        public int SortPriority;
        public int DispLocation;
        public char CanRemove;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 10)]
        public string Category;
        public int ContentSize;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 100)]
        public string InstallDate;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 10)]
        public string UICategory;
    };

    #endregion

    #region Debug

    [StructLayout(LayoutKind.Sequential, Pack = 4, CharSet = CharSet.Ansi)]
    public struct SegmentInfo
    {
        public ulong baseAddr;
        public uint size;
        public int prot;
    }

    [StructLayout(LayoutKind.Sequential, Pack = 8, CharSet = CharSet.Ansi)]
    public struct LibraryPacket
    {
        public int Handle;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
        public string Path;
        public int SegmentCount;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public SegmentInfo[] Segments;
    }

    [StructLayout(LayoutKind.Sequential, Pack = 4, CharSet = CharSet.Ansi)]
    public struct DbgRWPacket
    {
        public UInt64 Address;
        public UInt64 Length;
    }

    [StructLayout(LayoutKind.Sequential, Pack = 4, CharSet = CharSet.Ansi)]
    public struct DbgSPRXPacket
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
        public string Path;
        public int Handle;
    }

    [StructLayout(LayoutKind.Sequential, Pack = 4, CharSet = CharSet.Ansi)]
    public struct DbgBreakpointPacket
    {
        public int Index;
        public UInt64 Address;
        public int Enable;
    }

    #endregion

    #region Kernel

    #endregion

    #region Target

    public enum ConsoleTypes
    {
        UNK,
        DIAG, //0x80
        DEVKIT, //0x81
        TESTKIT, //0x82
        RETAIL, //0x83 -> 0x8F
        KRATOS, //0xA0 IMPOSSIBLE??
    };

    public enum ConsoleLEDColours
    {
        white,
        white_Blinking,
        Blue_Blinking,
    };

    [StructLayout(LayoutKind.Sequential, Pack = 4, CharSet = CharSet.Ansi), Serializable]
    public struct MemoryInfo
    {
        public int Used;
        public int Free;
        public int Total;
        public float Percentage;
    };

    [StructLayout(LayoutKind.Sequential, Pack = 8, CharSet = CharSet.Ansi), Serializable]
    public struct CurrentBigApp
    {
        public int Pid;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
        public string Name;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 10)]
        public string TitleId;
    };

    [StructLayout(LayoutKind.Sequential, Pack = 8, CharSet = CharSet.Ansi), Serializable]
    public struct TargetInfoPacket
    {
        public int SDKVersion;
        public int SoftwareVersion;
        public int FactorySoftwareVersion;
        public CurrentBigApp BigApp;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 100)]
        public string ConsoleName;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 14)]
        public string MotherboardSerial;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 10)]
        public string Serial;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 14)]
        public string Model;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 18)]
        public string MACAddressLAN;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 18)]
        public string MACAddressWIFI;
        public int UART;
        public int IDUMode;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
        public byte[] IDPS;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
        public byte[] PSID;
        public int ConsoleType;
        public int Attached;
        public int AttachedPid;
        public int ForegroundAccountId;

        public ulong FreeSpace;
        public ulong TotalSpace;

        public int CPUTemp;
        public int SOCTemp;
        public int ThreadCount;
        public float AverageCPUUsage;
        public int BusyCore;
        public MemoryInfo Ram;
        public MemoryInfo VRam;
    }

    [StructLayout(LayoutKind.Sequential, Pack = 4, Size = 2048, CharSet = CharSet.Ansi)]
    public struct TargetNotifyPacket
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 1024)]
        public string IconURI;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 1024)]
        public string Message;
    }

    public enum BuzzerType
    {
        RingOnce = 1,
        RingThree,
        LongRing,
        ThreeLongRing,
        ThreeLongDoubleBeeps,
    }

    [StructLayout(LayoutKind.Sequential, Pack = 4, CharSet = CharSet.Ansi)]
    public struct TargetSettingsPacket
    {
        public int AutoLoadSettings;
        public int ShowDebugTitleIdLabel;
        public int ShowDevkitPanel;
        public int ShowDebugSettings;
        public int ShowAppHome;
        public int ShowBuildOverlay;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
        public string GameOverlayLocation;
        public int ShowCPUUsage;
        public int ShowThreadCount;
        public int Showram;
        public int Showvram;
        public int ShowCPUTemp;
        public int ShowSOCTemp;
    };

    #endregion
}
