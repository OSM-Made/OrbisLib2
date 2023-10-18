using OrbisLib2.Common.API;
using OrbisLib2.Common.Database;
using OrbisLib2.Common.Helpers;
using System.Data.Entity.Core.Metadata.Edm;
using System.Net.Sockets;
using System.Text;
using System.Windows.Controls.Primitives;
using System.Windows.Markup;
using static SQLite.SQLite3;

namespace OrbisLib2.Targets
{
    public record ProcInfo(int AppId, int ProcessId, string Name, string TitleId);

    public class Target
    {
        private int _SavedTargetId = 0;

        public SavedTarget SavedTarget
        {
            get
            {
                var savedTarget = SavedTarget.FindTarget(x => x.Id == _SavedTargetId);

                if (savedTarget == null)
                    savedTarget = new SavedTarget();

                return savedTarget;
            }
        }

        /// <summary>
        /// Weather or not this is our default target to be selected on start up.
        /// </summary>
        public bool IsDefault
        {
            get
            {
                return SavedTarget.IsDefault;
            }
        }

        /// <summary>
        /// The name given to the target.
        /// </summary>
        public string Name
        {
            get
            {
                return SavedTarget.Name;
            }
        }

        /// <summary>
        /// The IP Address as a string.
        /// </summary>
        public string IPAddress
        {
            get
            {
                return SavedTarget.IPAddress;
            }
        }

        /// <summary>
        /// The port used to send payloads to the saved IP Address.
        /// </summary>
        public int PayloadPort
        {
            get
            {
                return SavedTarget.PayloadPort;
            }
        }


        public TargetInfo Info
        {
            get
            {
                return SavedTarget.Info;
            }
        }

        public Debug Debug;
        public Payload Payload;
        public FTP FTP;
        public Application Application;

        public Target(SavedTarget SavedTarget)
        {
            _SavedTargetId = SavedTarget.Id;

            Debug = new Debug(this);
            Payload = new Payload(this);
            FTP = new FTP(this);
            Application = new Application(this);
        }

        public Target Clone()
        {
            return new Target(SavedTarget.Clone());
        }

        public ResultState Shutdown()
        {
            return API.SendCommand(this, 5, APICommand.ApiTargetShutdown);
        }

        public ResultState Reboot()
        {
            return API.SendCommand(this, 5, APICommand.ApiTargetReboot);
        }

        public ResultState Suspend()
        {
            return API.SendCommand(this, 5, APICommand.ApiAppsSuspend);
        }

        public ResultState Notify(string Message)
        {
            return API.SendCommand(this, 5, APICommand.ApiTargetNotify, (Socket Sock) =>
            {
                return API.SendNextPacket(Sock, new TargetNotifyPacket { Message = Message });
            });
        }

        public ResultState Notify(string IconURI, string Message)
        {
            return API.SendCommand(this, 5, APICommand.ApiTargetNotify, (Socket Sock) =>
            {
                return API.SendNextPacket(Sock, new TargetNotifyPacket { IconURI = IconURI, Message = Message });
            });
        }

        public ResultState Buzzer(BuzzerType Type)
        {
            return API.SendCommand(this, 5, APICommand.ApiTargetBuzzer, (Socket Sock) =>
            {
                Sock.SendInt32((int)Type);

                // Set the result state of the call.
                return API.GetState(Sock);
            });
        }

        public ResultState SetLED(ConsoleLEDColours Colour)
        {
            return API.SendCommand(this, 5, APICommand.ApiTargetSetLed, (Socket Sock) =>
            {
                Sock.SendInt32((int)Colour);

                // Set the result state of the call.
                return API.GetState(Sock);
            });
        }

        public bool SetSettings(bool ShowDebugTitleIdLabel, bool ShowDevkitPanel, bool ShowDebugSettings, bool ShowAppHome)
        {
            //var result = API.SendCommand(this, 5, APICommand.ApiTargetSetSettings, (Socket Sock) =>
            //{
            //    Result = API.SendNextPacket(Sock, new TargetSettingsPacket()
            //    {
            //        ShowDebugTitleIdLabel = Convert.ToInt32(ShowDebugTitleIdLabel),
            //        ShowDevkitPanel = Convert.ToInt32(ShowDevkitPanel),
            //        ShowDebugSettings = Convert.ToInt32(ShowDebugSettings),
            //        ShowAppHome = Convert.ToInt32(ShowAppHome)
            //    });
            //});
            //
            //return result == APIResults.API_OK;

            return false;
        }

        public ResultState GetProcList(out List<ProcInfo> List)
        {
            var result = new ResultState { Succeeded = true };
            var tempList = new List<ProcInfo>();

            try
            {
                result = API.SendCommand(this, 4, APICommand.ApiTargetGetProcList, (Socket Sock) =>
                {
                    var rawPacket = Sock.ReceiveSize();
                    var packet = ProcListPacket.Parser.ParseFrom(rawPacket);

                    foreach (var process in packet.Processes)
                    {
                        tempList.Add(new ProcInfo(process.AppId, process.ProcessId, process.Name, process.TitleId));
                    }

                    return new ResultState { Succeeded = true };
                });
            }
            catch (Exception ex)
            {
                result = new ResultState { Succeeded = false, ErrorMessage = ex.Message };
            }

            List = tempList;
            return result;
        }

        public byte[] GetFile(string filePath)
        {
            int bytesRecieved = 0;
            var file = new byte[0];
            API.SendCommand(this, 4, APICommand.ApiTargetSendFile, (Socket Sock) =>
            {
                var result = API.SendNextPacket(Sock, new FilePacket { FilePath = filePath });

                if (!result.Succeeded)
                    return result;

                var fileSize = Sock.RecvInt32();
                file = new byte[fileSize];
                bytesRecieved = Sock.RecvLarge(file);

                return result;
            });

            return bytesRecieved > 0 ? file : new byte[0];
        }

        public ResultState SendFile(byte[] data, string filePath)
        {
            return API.SendCommand(this, 4, APICommand.ApiTargetRecieveFile, (Socket Sock) =>
            {
                var result = API.SendNextPacket(Sock, new FilePacket { FilePath = filePath });

                if (!result.Succeeded)
                    return result;

                // Send the file.
                Sock.SendSize(data);

                return result;
            });
        }

        public ResultState DeleteFile(string filePath)
        {
            return API.SendCommand(this, 4, APICommand.ApiTargetDeleteFile, (Socket Sock) =>
            {
                return API.SendNextPacket(Sock, new FilePacket { FilePath = filePath });
            });
        }
    }
}
