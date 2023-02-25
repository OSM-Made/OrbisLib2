using OrbisLib2.Common.API;
using OrbisLib2.Common.Helpers;
using System.Net.Sockets;
using System.Runtime.InteropServices;

namespace OrbisLib2.Targets
{
    public record ProcInfo(int AppId, int ProcessId, string Name, string TitleId);

    public class Process
    {
        private Target Target;

        public Process(Target Target)
        {
            this.Target = Target;
        }

        public List<ProcInfo> GetList()
        {
            var list = new List<ProcInfo>();

            API.SendCommand(Target, 4, APICommands.API_PROC_GET_LIST, (Socket Sock, APIResults Result) =>
            {
                var processCount = Sock.RecvInt32();

                for(int i = 0; i < processCount; i++)
                {
                    var Packet = new ProcPacket();
                    if(!API.RecieveNextPacket(Sock, ref Packet))
                    {
                        continue;
                    }

                    list.Add(new ProcInfo(Packet.AppId, Packet.ProcessId, Packet.Name, Packet.TitleId));
                }
            });

            return list;
        }
    }
}
