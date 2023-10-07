using OrbisLib2.Common.API;
using OrbisLib2.Common.Database.App;
using OrbisLib2.Common.Helpers;
using System.IO;
using System.Net.Sockets;
using System.Text.RegularExpressions;

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

        public ResultState IsOutOfDate(out bool IsOutOfDate)
        {
            var databasePath = GetAppDBPath();

            if (!File.Exists(databasePath))
            {
                IsOutOfDate = true;
                return new ResultState { Succeeded = true };
            }

            var tempIsOutOfDate = false;
            var result = API.SendCommand(Target, 5, APICommand.ApiAppsCheckVer, (Socket Sock, ResultState Result) =>
            {
                // Send the current app version.
                Sock.SendInt32(AppBrowseVersion.GetAppBrowseVersion(databasePath));

                // Get the state from API.
                tempIsOutOfDate = Sock.RecvInt32() == 1;
            });

            IsOutOfDate = tempIsOutOfDate;
            return result;
        }

        public ResultState UpdateLocalDB()
        {
            bool isOutOfDate;
            var result = IsOutOfDate(out isOutOfDate);

            // If the out of date check failed we need to abort.
            if (!result.Succeeded)
                return result;

            // If the DB is up to date we have nothing to do here, lets get out of here!
            if (!isOutOfDate)
                return new ResultState { Succeeded = true };

            return API.SendCommand(Target, 5, APICommand.ApiAppsGetDb, (Socket Sock, ResultState Result) =>
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

        public ResultState GetAppState(string TitleId, out AppState State)
        {
            if (!Regex.IsMatch(TitleId, @"[a-zA-Z]{4}\d{5}"))
            {
                State = AppState.StateNotRunning;
                return new ResultState { Succeeded = false, ErrorMessage = $"Invaild titleId format {TitleId}" };
            }

            var tempAppState = AppState.StateNotRunning;
            var result = API.SendCommand(Target, 5, APICommand.ApiAppsStatus, (Socket Sock, ResultState Result) => 
            {
                Result = API.SendNextPacket(Sock, new AppPacket { TitleId = TitleId });

                // Get the state from API.
                if (Result.Succeeded)
                    tempAppState = (AppState)Sock.RecvInt32();
            });

            State = tempAppState;
            return result;
        }

        public ResultState Start(string TitleId)
        {
            if (!Regex.IsMatch(TitleId, @"[a-zA-Z]{4}\d{5}"))
                return new ResultState { Succeeded = false, ErrorMessage = $"Invaild titleId format {TitleId}" };

            return API.SendCommand(Target, 5, APICommand.ApiAppsStart, (Socket Sock, ResultState Result) => 
            {
                Result = API.SendNextPacket(Sock, new AppPacket { TitleId = TitleId });
            });
        }

        public ResultState Stop(string TitleId)
        {
            if (!Regex.IsMatch(TitleId, @"[a-zA-Z]{4}\d{5}"))
                return new ResultState { Succeeded = false, ErrorMessage = $"Invaild titleId format {TitleId}" };

            return API.SendCommand(Target, 5, APICommand.ApiAppsStop, (Socket Sock, ResultState Result) =>
            {
                Result = API.SendNextPacket(Sock, new AppPacket { TitleId = TitleId });
            });
        }

        public ResultState Suspend(string TitleId)
        {
            if (!Regex.IsMatch(TitleId, @"[a-zA-Z]{4}\d{5}"))
                return new ResultState { Succeeded = false, ErrorMessage = $"Invaild titleId format {TitleId}" };

            return API.SendCommand(Target, 5, APICommand.ApiAppsSuspend, (Socket Sock, ResultState Result) =>
            {
                Result = API.SendNextPacket(Sock, new AppPacket { TitleId = TitleId });
            });
        }

        public ResultState Resume(string TitleId)
        {
            if (!Regex.IsMatch(TitleId, @"[a-zA-Z]{4}\d{5}"))
                return new ResultState { Succeeded = false, ErrorMessage = $"Invaild titleId format {TitleId}" };

            return API.SendCommand(Target, 5, APICommand.ApiAppsResume, (Socket Sock, ResultState Result) =>
            {
                Result = API.SendNextPacket(Sock, new AppPacket { TitleId = TitleId });
            });
        }

        public ResultState Delete(string TitleId)
        {
            if (!Regex.IsMatch(TitleId, @"[a-zA-Z]{4}\d{5}"))
                return new ResultState { Succeeded = false, ErrorMessage = $"Invaild titleId format {TitleId}" };

            return API.SendCommand(Target, 5, APICommand.ApiAppsDelete, (Socket Sock, ResultState Result) =>
            {
                Result = API.SendNextPacket(Sock, new AppPacket { TitleId = TitleId });
            });
        }

        public ResultState SetVisibility(string TitleId, VisibilityType Visibility)
        {
            if (!Regex.IsMatch(TitleId, @"[a-zA-Z]{4}\d{5}"))
                return new ResultState { Succeeded = false, ErrorMessage = $"Invaild titleId format {TitleId}" };

            return API.SendCommand(Target, 5, APICommand.ApiAppsSetVisibility, (Socket Sock, ResultState Result) =>
            {
                Result = API.SendNextPacket(Sock, new AppPacket { TitleId = TitleId });

                if (Result.Succeeded)
                {
                    // Send the visibility state.
                    Sock.SendInt32((int)Visibility);

                    Result = API.GetState(Sock);
                }
            });
        }

        public ResultState GetVisibility(string TitleId, out VisibilityType Type)
        {
            if (!Regex.IsMatch(TitleId, @"[a-zA-Z]{4}\d{5}"))
            {
                Type = VisibilityType.VT_NONE;
                return new ResultState { Succeeded = false, ErrorMessage = $"Invaild titleId format {TitleId}" };
            }

            var tempType = VisibilityType.VT_NONE;
            var result = API.SendCommand(Target, 5, APICommand.ApiAppsGetVisibility, (Socket Sock, ResultState Result) =>
            {
                Result = API.SendNextPacket(Sock, new AppPacket { TitleId = TitleId });

                // Get the state from API.
                if (Result.Succeeded)
                    tempType = (VisibilityType)Sock.RecvInt32();
            });

            Type = tempType;
            return result;
        }
    }
}
