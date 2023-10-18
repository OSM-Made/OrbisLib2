using OrbisLib2.Common.API;
using OrbisLib2.Common.Helpers;
using System.Net.Sockets;

namespace OrbisLib2.Targets
{
    public record LibraryInfo(long Handle, string Path, ulong MapBase, ulong TextSize, ulong MapSize, ulong DataBase, ulong dataSize);

    public class Debug
    {
        private Target Target;

        public Debug(Target Target)
        {
            this.Target = Target;
        }

        public bool IsDebugging
        {
            get 
            {
                int currentTarget;
                return GetCurrentProcessId(out currentTarget).Succeeded && currentTarget != -1; 
            }
        }

        public ResultState Attach(int pid)
        {
            return API.SendCommand(Target, 3, APICommand.ApiDbgAttach, (Socket Sock) =>
            {
                Sock.SendInt32(pid);

                return API.GetState(Sock);
            });
        }

        public ResultState Detach()
        {
            if(!IsDebugging)
                return new ResultState { Succeeded = false, ErrorMessage = $"The target {Target.Name} ({Target.IPAddress}) is not currently debugging any process." };

            return API.SendCommand(Target, 3, APICommand.ApiDbgDetach, (Socket Sock) =>
            {
                return API.GetState(Sock);
            });
        }

        public ResultState GetCurrentProcessId(out int ProcessId)
        {
            var tempProcessId = -1;
            var result = API.SendCommand(Target, 3, APICommand.ApiDbgGetCurrent, (Socket Sock) =>
            {
                tempProcessId = Sock.RecvInt32();

                return new ResultState { Succeeded = true };
            });

            ProcessId = tempProcessId;
            return result;
        }

        public ResultState LoadLibrary(string Path, out int Handle)
        {
            if (!IsDebugging)
            {
                Handle = -1;
                return new ResultState { Succeeded = false, ErrorMessage = $"The target {Target.Name} ({Target.IPAddress}) is not currently debugging any process." };
            }

            int tempHandle = -1;
            var result = API.SendCommand(Target, 3, APICommand.ApiDbgLoadLibrary, (Socket Sock) =>
            {
                if (Sock.RecvInt32() != 1)
                    return new ResultState { Succeeded = false, ErrorMessage = $"The target {Target.Name} ({Target.IPAddress}) is not currently debugging any process." };
                else
                {
                    var result = API.SendNextPacket(Sock, new SPRXPacket { Path = Path });

                    if (result.Succeeded)
                        tempHandle = Sock.RecvInt32();

                    return result;
                }
            });

            Handle = tempHandle;
            return result;
        }

        public ResultState UnloadLibrary(int Handle)
        {
            if (!IsDebugging)
                return new ResultState { Succeeded = false, ErrorMessage = $"The target {Target.Name} ({Target.IPAddress}) is not currently debugging any process." };

            return API.SendCommand(Target, 3, APICommand.ApiDbgUnloadLibrary, (Socket Sock) =>
            {
                if (Sock.RecvInt32() != 1)
                    return new ResultState { Succeeded = false, ErrorMessage = $"The target {Target.Name} ({Target.IPAddress}) is not currently debugging any process." };
                else
                    return API.SendNextPacket(Sock, new SPRXPacket { Handle = Handle });
            });
        }

        public ResultState ReloadLibrary(int Handle, string Path, out int NewHandle)
        {
            if (!IsDebugging)
            {
                NewHandle = -1;
                return new ResultState { Succeeded = false, ErrorMessage = $"The target {Target.Name} ({Target.IPAddress}) is not currently debugging any process." };
            }

            int tempHandle = -1;
            var result = API.SendCommand(Target, 3, APICommand.ApiDbgReloadLibrary, (Socket Sock) =>
            {
                if (Sock.RecvInt32() != 1)
                    return new ResultState { Succeeded = false, ErrorMessage = $"The target {Target.Name} ({Target.IPAddress}) is not currently debugging any process." };
                else
                {
                    var result = API.SendNextPacket(Sock, new SPRXPacket { Path = Path, Handle = Handle });
                    tempHandle = Sock.RecvInt32();

                    return result;
                }
            });

            NewHandle = tempHandle;
            return result;
        }

        public ResultState GetLibraries(out List<LibraryInfo> LibraryList)
        {
            var tempLibraryList = new List<LibraryInfo>();

            if (!IsDebugging)
            {
                LibraryList = tempLibraryList;
                return new ResultState { Succeeded = false, ErrorMessage = $"The target {Target.Name} ({Target.IPAddress}) is not currently debugging any process." };
            }

            var result = API.SendCommand(Target, 6, APICommand.ApiDbgLibraryList, (Socket Sock) =>
            {
                if (Sock.RecvInt32() != 1)
                    return new ResultState { Succeeded = false, ErrorMessage = $"The target {Target.Name} ({Target.IPAddress}) is not currently debugging any process." };
                else
                {
                    var rawPacket = Sock.ReceiveSize();
                    var Packet = LibraryListPacket.Parser.ParseFrom(rawPacket);

                    foreach(var library in Packet.Libraries)
                    {
                        tempLibraryList.Add(new LibraryInfo(library.Handle, library.Path, library.MapBase, library.MapSize, library.TextSize, library.DataBase, library.TextSize));
                    }

                    return new ResultState { Succeeded = true };
                }
            });

            LibraryList = tempLibraryList;
            return result;
        }


        public ResultState ReadMemory(ulong Address, out byte[] Data, ulong Length)
        {
            if (!IsDebugging)
            {
                Data = new byte[Length];
                return new ResultState { Succeeded = false, ErrorMessage = $"The target {Target.Name} ({Target.IPAddress}) is not currently debugging any process." };
            }

            var data = new byte[Length];
            var result = API.SendCommand(Target, 6, APICommand.ApiDbgRead, (Socket Sock) =>
            {
                if (Sock.RecvInt32() != 1)
                    return new ResultState { Succeeded = false, ErrorMessage = $"The target {Target.Name} ({Target.IPAddress}) is not currently debugging any process." };
                else
                {
                    var result = API.SendNextPacket(Sock, new RWPacket { Address = Address, Length = Length });

                    if (result.Succeeded)
                        Sock.RecvLarge(data);

                    return result;
                }
            });

            Data = data;
            return result;
        }

        public ResultState WriteMemory(ulong Address, byte[] Data)
        {
            if (!IsDebugging)
                return new ResultState { Succeeded = false, ErrorMessage = $"The target {Target.Name} ({Target.IPAddress}) is not currently debugging any process." };

            return API.SendCommand(Target, 6, APICommand.ApiDbgWrite, (Socket Sock) =>
            {
                if (Sock.RecvInt32() != 1)
                    return new ResultState { Succeeded = false, ErrorMessage = $"The target {Target.Name} ({Target.IPAddress}) is not currently debugging any process." };
                else
                {
                    var result = API.SendNextPacket(Sock, new RWPacket { Address = Address, Length = (ulong)Data.Length });

                    if (result.Succeeded)
                    {
                        Sock.SendLarge(Data);

                        result = API.GetState(Sock);
                    }

                    return result;
                }
            });
        }
    }
}
