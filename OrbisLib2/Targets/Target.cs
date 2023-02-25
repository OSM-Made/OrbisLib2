using OrbisLib2.Common.API;
using OrbisLib2.Common.Database;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;

namespace OrbisLib2.Targets
{
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
        public Process Process;
        public FTP FTP;
        public Application Application;

        public Target(SavedTarget SavedTarget)
        {
            _SavedTargetId = SavedTarget.Id;

            Debug = new Debug(this);
            Payload = new Payload(this);
            Process = new Process(this);
            FTP = new FTP(this);
            Application = new Application(this);
        }

        public Target Clone()
        {
            return new Target(SavedTarget.Clone());
        }

        public bool Shutdown()
        {
            return API.SendCommand(this, 5, APICommands.API_TARGET_SHUTDOWN) == APIResults.API_OK;
        }

        public bool Reboot()
        {
            return API.SendCommand(this, 5, APICommands.API_TARGET_REBOOT) == APIResults.API_OK;
        }

        public bool Suspend()
        {
            return API.SendCommand(this, 5, APICommands.API_TARGET_RESTMODE) == APIResults.API_OK;
        }

        public bool Notify(string Message)
        {
            var result = API.SendCommand(this, 5, APICommands.API_TARGET_NOTIFY, (Socket Sock, APIResults Result) =>
            {
                Console.WriteLine($"Message: {Message}");
                Result = API.SendNextPacket(Sock, new TargetNotifyPacket() { Message = Message });
            });

            return result == APIResults.API_OK;
        }

        public bool Notify(string IconURI, string Message)
        {
            var result = API.SendCommand(this, 5, APICommands.API_TARGET_NOTIFY, (Socket Sock, APIResults Result) =>
            {
                Result = API.SendNextPacket(Sock, new TargetNotifyPacket() { IconURI = IconURI, Message = Message });
            });

            return result == APIResults.API_OK;
        }

        public bool Buzzer(BuzzerType Type)
        {
            var result = API.SendCommand(this, 5, APICommands.API_TARGET_BUZZER, (Socket Sock, APIResults Result) =>
            {
                Result = API.SendInt32(Sock, (int)Type);
            });

            return result == APIResults.API_OK;
        }

        public bool SetLED(ConsoleLEDColours Colour)
        {
            var result = API.SendCommand(this, 5, APICommands.API_TARGET_SET_LED, (Socket Sock, APIResults Result) =>
            {
                Result = API.SendInt32(Sock, (int)Colour);
            });

            return result == APIResults.API_OK;
        }

        public bool SetSettings(bool ShowDebugTitleIdLabel, bool ShowDevkitPanel, bool ShowDebugSettings, bool ShowAppHome)
        {
            var result = API.SendCommand(this, 5, APICommands.API_TARGET_SET_SETTINGS, (Socket Sock, APIResults Result) =>
            {
                Result = API.SendNextPacket(Sock, new TargetSettingsPacket()
                {
                    ShowDebugTitleIdLabel = Convert.ToInt32(ShowDebugTitleIdLabel),
                    ShowDevkitPanel = Convert.ToInt32(ShowDevkitPanel),
                    ShowDebugSettings = Convert.ToInt32(ShowDebugSettings),
                    ShowAppHome = Convert.ToInt32(ShowAppHome)
                });
            });

            return result == APIResults.API_OK;
        }
    }
}
