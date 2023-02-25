using OrbisLib2.Common.API;
using OrbisLib2.Common.Database.App;
using OrbisLib2.Common.Helpers;
using System.Data.Entity.Core.Metadata.Edm;
using System.Drawing;
using System.IO;
using System.Net.Sockets;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.RegularExpressions;
using System.Windows.Controls;
using static SQLite.SQLite3;

namespace OrbisLib2.Targets
{
    public class Application
    {
        public enum VisibilityType : int
        {
            VT_NONE,
            VT_VISIBLE,
            VT_INVISIBLE,
        };

        private Target Target;

        public Application(Target Target)
        {
            this.Target = Target;
        }

        public string GetAppDBPath()
        {
            var foregroundAccountId = Target.Info.ForegroundAccountId;
            if (foregroundAccountId <= 0)
                return string.Empty;

            // Create the db cache folder if it does not exist.
            var dbCachePath = @$"{Environment.GetFolderPath(Environment.SpecialFolder.CommonApplicationData)}\Orbis Suite\DBCache";
            if (!Directory.Exists(dbCachePath))
            {
                Directory.CreateDirectory(dbCachePath);
            }

            // create a folder for this target if it does not exist yet.
            var targetFolder = @$"{dbCachePath}\{Target.Info.MACAddressLAN.Replace(":", "-")}";
            if (!Directory.Exists(targetFolder))
            {
                Directory.CreateDirectory(targetFolder);
            }

            return @$"{targetFolder}\app.db";
        }

        public bool IsOutOfDate()
        {
            var databasePath = GetAppDBPath();

            if (!File.Exists(databasePath))
            {
                return true;
            }

            var currentAppVersion = AppBrowseVersion.GetAppBrowseVersion(databasePath);

            bool result = false;
            API.SendCommand(Target, 5, APICommands.API_APPS_CHECK_VER, (Socket Sock, APIResults Result) =>
            {
                // Send the current app version.
                Sock.SendInt32(currentAppVersion);

                // Get the state from API.
                result = Sock.RecvInt32() == 1;
            });

            return result;
        }

        public void UpdateLocalDB()
        {
            if(IsOutOfDate())
            {
                API.SendCommand(Target, 5, APICommands.API_APPS_GET_DB, (Socket Sock, APIResults Result) =>
                {
                    var fileSize = Sock.RecvInt32();
                    var newDatabaseBytes = new byte[fileSize];
                    if (Sock.RecvLarge(newDatabaseBytes) < fileSize)
                        return;

                    var databasePath = GetAppDBPath();
                    var oldDatabasePath = @$"{databasePath}.old";

                    // If we already have a db back it up.
                    if (File.Exists(databasePath))
                    {
                        File.Copy(databasePath, oldDatabasePath, true);

                        // Remove the last db
                        File.Delete(databasePath);
                    }

                    // Write the new DB.
                    File.WriteAllBytes(databasePath, newDatabaseBytes);
                });
            }
        }

        public List<AppBrowse> GetAppList()
        {
            var databasePath = GetAppDBPath();

            // Update the DB if needed.
            UpdateLocalDB();

            // Make sure we actually have a DB now.
            if (!File.Exists(databasePath))
            {
                return new List<AppBrowse>();
            }

            return AppBrowse.GetAppBrowseList(databasePath, Target.Info.ForegroundAccountId);
        }

        public string GetAppInfoString(string TitleId, string Key)
        {
            var databasePath = GetAppDBPath();

            if (!File.Exists(databasePath))
            {
                return string.Empty;
            }

            return AppInfo.GetStringFromAppInfo(databasePath, TitleId, Key);
        }

        public AppState GetAppState(string TitleId)
        {
            if (!Regex.IsMatch(TitleId, @"[a-zA-Z]{4}\d{5}"))
            {
                Console.WriteLine($"Invaild titleId format {TitleId}");
                return AppState.STATE_ERROR;
            }

            AppState result = AppState.STATE_ERROR;
            API.SendCommand(Target, 5, APICommands.API_APPS_STATUS, (Socket Sock, APIResults Result) => 
            {
                // Send the titleId of the app.
                var bytes = Encoding.ASCII.GetBytes(TitleId.PadRight(10, '\0')).Take(10).ToArray();
                Sock.Send(bytes);

                // Get the state from API.
                result = (AppState)Sock.RecvInt32();
            });

            return result;
        }

        public bool Start(string TitleId)
        {
            if (!Regex.IsMatch(TitleId, @"[a-zA-Z]{4}\d{5}"))
            {
                Console.WriteLine($"Invaild titleId format {TitleId}");
                return false;
            }

            int result = 0;
            API.SendCommand(Target, 5, APICommands.API_APPS_START, (Socket Sock, APIResults Result) => 
            {
                // Send the titleId of the app.
                var bytes = Encoding.ASCII.GetBytes(TitleId.PadRight(10, '\0')).Take(10).ToArray();
                Sock.Send(bytes);

                // Get the state from API.
                result = Sock.RecvInt32();
            });

            return (result == 1);
        }

        public bool Stop(string TitleId)
        {
            if (!Regex.IsMatch(TitleId, @"[a-zA-Z]{4}\d{5}"))
            {
                Console.WriteLine($"Invaild titleId format {TitleId}");
                return false;
            }

            int result = 0;
            API.SendCommand(Target, 5, APICommands.API_APPS_STOP, (Socket Sock, APIResults Result) =>
            {
                // Send the titleId of the app.
                var bytes = Encoding.ASCII.GetBytes(TitleId.PadRight(10, '\0')).Take(10).ToArray();
                Sock.Send(bytes);

                // Get the state from API.
                result = Sock.RecvInt32();
            });

            return (result == 1);
        }

        public bool Suspend(string TitleId)
        {
            if (!Regex.IsMatch(TitleId, @"[a-zA-Z]{4}\d{5}"))
            {
                Console.WriteLine($"Invaild titleId format {TitleId}");
                return false;
            }

            int result = 0;
            API.SendCommand(Target, 5, APICommands.API_APPS_SUSPEND, (Socket Sock, APIResults Result) =>
            {
                // Send the titleId of the app.
                var bytes = Encoding.ASCII.GetBytes(TitleId.PadRight(10, '\0')).Take(10).ToArray();
                Sock.Send(bytes);

                // Get the state from API.
                result = Sock.RecvInt32();
            });

            return (result == 1);
        }

        public bool Resume(string TitleId)
        {
            if (!Regex.IsMatch(TitleId, @"[a-zA-Z]{4}\d{5}"))
            {
                Console.WriteLine($"Invaild titleId format {TitleId}");
                return false;
            }

            int result = 0;
            API.SendCommand(Target, 5, APICommands.API_APPS_RESUME, (Socket Sock, APIResults Result) =>
            {
                // Send the titleId of the app.
                var bytes = Encoding.ASCII.GetBytes(TitleId.PadRight(10, '\0')).Take(10).ToArray();
                Sock.Send(bytes);

                // Get the state from API.
                result = Sock.RecvInt32();
            });

            return (result == 1);
        }

        public bool Delete(string TitleId)
        {
            if (!Regex.IsMatch(TitleId, @"[a-zA-Z]{4}\d{5}"))
            {
                Console.WriteLine($"Invaild titleId format {TitleId}");
                return false;
            }

            int result = 0;
            API.SendCommand(Target, 5, APICommands.API_APPS_DELETE, (Socket Sock, APIResults Result) =>
            {
                // Send the titleId of the app.
                var bytes = Encoding.ASCII.GetBytes(TitleId.PadRight(10, '\0')).Take(10).ToArray();
                Sock.Send(bytes);

                // Get the state from API.
                result = Sock.RecvInt32();
            });

            return (result == 1);
        }

        public bool SetVisibility(string TitleId, VisibilityType Visibility)
        {
            if (!Regex.IsMatch(TitleId, @"[a-zA-Z]{4}\d{5}"))
            {
                Console.WriteLine($"Invaild titleId format {TitleId}");
                return false;
            }

            int result = 0;
            API.SendCommand(Target, 5, APICommands.API_APPS_SET_VISIBILITY, (Socket Sock, APIResults Result) =>
            {
                // Send the titleId of the app.
                var bytes = Encoding.ASCII.GetBytes(TitleId.PadRight(10, '\0')).Take(10).ToArray();
                Sock.Send(bytes);

                // Send the visibility state.
                Sock.SendInt32((int)Visibility);

                // Get the state from API.
                result = Sock.RecvInt32();
            });

            return (result == 1);
        }

        public VisibilityType GetVisibility(string TitleId)
        {
            if (!Regex.IsMatch(TitleId, @"[a-zA-Z]{4}\d{5}"))
            {
                Console.WriteLine($"Invaild titleId format {TitleId}");
                return VisibilityType.VT_NONE;
            }

            VisibilityType result = VisibilityType.VT_NONE;
            API.SendCommand(Target, 5, APICommands.API_APPS_GET_VISIBILITY, (Socket Sock, APIResults Result) =>
            {
                // Send the titleId of the app.
                var bytes = Encoding.ASCII.GetBytes(TitleId.PadRight(10, '\0')).Take(10).ToArray();
                Sock.Send(bytes);

                // Get the state from API.
                result = (VisibilityType)Sock.RecvInt32();
            });

            return result;
        }
    }
}
