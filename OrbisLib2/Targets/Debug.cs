using OrbisLib2.Common.API;
using OrbisLib2.Common.Helpers;
using System;
using System.Collections.Generic;
using System.Data.Entity.Core.Metadata.Edm;
using System.Linq;
using System.Net.Sockets;
using System.Reflection.Metadata;
using System.Runtime.InteropServices;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Markup;
using static SQLite.SQLite3;

namespace OrbisLib2.Targets
{
    public record SegmentInfo(ulong Address, uint Size, int Protection);

    public record LibraryInfo(long Handle, string Path, SegmentInfo[] Segments);

    public class Debug
    {
        private Target Target;

        public Debug(Target Target)
        {
            this.Target = Target;
        }

        public bool IsDebugging
        {
            get { return GetCurrentProcessId() != -1; }
        }

        public bool Attach(int pid)
        {
            var result = API.SendCommand(Target, 3, APICommands.API_DBG_ATTACH, (Socket Sock, APIResults Result) =>
            {
                Sock.SendInt32(pid);

                var res = Sock.RecvInt32();

                if (res == 1)
                    Result = APIResults.API_OK;
                else
                    Result = APIResults.API_ERROR_GENERAL;
            });

            return result == APIResults.API_OK;
        }

        // TODO: We probably want to detach when our program is exiting.
        public bool Detach()
        {
            if(!IsDebugging)
            {
                return false;
            }

            var result = API.SendCommand(Target, 3, APICommands.API_DBG_DETACH, (Socket Sock, APIResults Result) =>
            {
                var res = Sock.RecvInt32();

                if (res == 1)
                    Result = APIResults.API_OK;
                else
                    Result = APIResults.API_ERROR_GENERAL;
            });

            return result == APIResults.API_OK;
        }

        public int GetCurrentProcessId()
        {
            var currentPid = -1;
            var result = API.SendCommand(Target, 3, APICommands.API_DBG_GET_CURRENT, (Socket Sock, APIResults Result) =>
            {
                currentPid = Sock.RecvInt32();
            });

            return result == APIResults.API_OK ? currentPid : -1;
        }

        public int LoadLibrary(string Path)
        {
            int loadResult = 0;
            var result = API.SendCommand(Target, 3, APICommands.API_DBG_LOAD_LIBRARY, (Socket Sock, APIResults Result) =>
            {
                var isDebugging = Sock.RecvInt32();
                if(isDebugging == 1)
                {
                    var Packet = new DbgSPRXPacket();
                    Packet.Path = Path;
                    Sock.Send(Helper.StructToBytes(Packet));

                    loadResult = Sock.RecvInt32();
                }
            });

            return result == APIResults.API_OK ? loadResult : -1;
        }

        public bool UnloadLibrary(int Handle)
        {
            if (!IsDebugging)
            {
                return false;
            }

            int loadResult = 0;
            var result = API.SendCommand(Target, 3, APICommands.API_DBG_UNLOAD_LIBRARY, (Socket Sock, APIResults Result) =>
            {
                var isDebugging = Sock.RecvInt32();
                if (isDebugging == 1)
                {
                    var Packet = new DbgSPRXPacket();
                    Packet.Handle = Handle;
                    Sock.Send(Helper.StructToBytes(Packet));

                    loadResult = Sock.RecvInt32();
                }
            });

            return result == APIResults.API_OK ? (loadResult == 0) : false;
        }

        public int ReloadLibrary(int Handle, string Path)
        {
            if (!IsDebugging)
            {
                return -1;
            }

            int loadResult = 0;
            var result = API.SendCommand(Target, 3, APICommands.API_DBG_RELOAD_LIBRARY, (Socket Sock, APIResults Result) =>
            {
                var isDebugging = Sock.RecvInt32();
                if (isDebugging == 1)
                {
                    var Packet = new DbgSPRXPacket();
                    Packet.Handle = Handle;
                    Packet.Path = Path;
                    Sock.Send(Helper.StructToBytes(Packet));

                    loadResult = Sock.RecvInt32();
                }
            });

            return result == APIResults.API_OK ? loadResult : -1;
        }

        public List<LibraryInfo> GetLibraries()
        {
            var libraryList = new List<LibraryInfo>();

            if (!IsDebugging)
            {
                return libraryList;
            }

            var result = API.SendCommand(Target, 6, APICommands.API_DBG_LIBRARY_LIST, (Socket Sock, APIResults Result) =>
            {
                var isDebugging = Sock.RecvInt32();
                if (isDebugging == 1)
                {
                    var libraryCount = Sock.RecvInt32();

                    // Recieve all of the arrary as one large packet.
                    var dataSize = libraryCount * Marshal.SizeOf(typeof(LibraryPacket));
                    var data = new byte[dataSize];
                    Sock.RecvLarge(data);

                    // Allocate and copy the packet to begin marshaling it.
                    IntPtr ptr = Marshal.AllocHGlobal(dataSize);
                    Marshal.Copy(data, 0, ptr, dataSize);

                    for (int i = 0; i < libraryCount; i++)
                    {
                        // Marshal each part of the buffer to a struct.
                        var Packet = new LibraryPacket();
                        Packet = (LibraryPacket)Marshal.PtrToStructure(IntPtr.Add(ptr, i * Marshal.SizeOf(typeof(LibraryPacket))), typeof(LibraryPacket));

                        if (Packet.SegmentCount > 4 || Packet.SegmentCount < 0)
                            Packet.SegmentCount = 4;

                        var segments = new SegmentInfo[Packet.SegmentCount];
                        for (int j = 0; j < Packet.SegmentCount; j++)
                        {
                            segments[j] = new SegmentInfo(Packet.Segments[j].baseAddr, Packet.Segments[j].size, Packet.Segments[j].prot);
                        }

                        libraryList.Add(new LibraryInfo(Packet.Handle, Packet.Path, segments));
                    }

                    Marshal.FreeHGlobal(ptr);
                }
            });

            return libraryList;
        }


        public byte[] ReadMemory(ulong Address, ulong Length)
        {
            if (!IsDebugging)
            {
                return new byte[0];
            }

            int readResult = 0;
            var data = new byte[Length];
            var result = API.SendCommand(Target, 6, APICommands.API_DBG_READ, (Socket Sock, APIResults Result) =>
            {
                var isDebugging = Sock.RecvInt32();
                if (isDebugging == 1)
                {
                    var Packet = new DbgRWPacket();
                    Packet.Address = Address;
                    Packet.Length = Length;
                    Sock.Send(Helper.StructToBytes(Packet));

                    // Status
                    readResult = Sock.RecvInt32();

                    // Recv the memory if the address was valid.
                    if (readResult == 1)
                    {
                        Sock.RecvLarge(data);
                    }
                }
            });

            return readResult == 1 ? data : new byte[0];
        }

        public bool WriteMemory(ulong Address, byte[] Data)
        {
            if (!IsDebugging)
            {
                return false;
            }

            int writeResult = 0;
            var result = API.SendCommand(Target, 6, APICommands.API_DBG_WRITE, (Socket Sock, APIResults Result) =>
            {
                var isDebugging = Sock.RecvInt32();
                if (isDebugging == 1)
                {
                    var Packet = new DbgRWPacket();
                    Packet.Address = Address;
                    Packet.Length = (ulong)Data.Length;
                    Sock.Send(Helper.StructToBytes(Packet));

                    // Send the memory to write.
                    Sock.SendLarge(Data);

                    // Status
                    writeResult = Sock.RecvInt32();
                }
            });

            return writeResult == 1;
        }
    }
}
