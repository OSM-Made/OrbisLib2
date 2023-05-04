#include "stdafx.h"
#include "Apps.h"
#include "AppDatabase.h"
#include <SystemServiceExt.h>
#include <UserServiceExt.h>
#include "AppPackets.h"

#define APP_DB_PATH "/system_data/priv/mms/app.db"

void Apps::GetDB(SceNetId Sock)
{
	//Open file descriptors 
	auto fd = sceKernelOpen(APP_DB_PATH, SCE_KERNEL_O_RDONLY, 0);
	if (fd <= 0)
	{
		klog("Failed to open app database file.\n");
		return;
	}

	//Get File size
	SceKernelStat stats;
	sceKernelFstat(fd, &stats);

	if (stats.st_size == 0)
	{
		klog("Failed to get size of app database.\n");
		return;
	}

	//Allocate space to read data.
	auto FileData = (unsigned char*)malloc(stats.st_size);

	//ReadFile.
	sceKernelRead(fd, FileData, stats.st_size);
	sceKernelClose(fd);

	Sockets::SendInt(Sock, stats.st_size);
	Sockets::SendLargeData(Sock, FileData, stats.st_size);

	free(FileData);
}

void Apps::CheckVersion(SceNetId Sock)
{
	auto currentVersion = 0;
	if (!Sockets::RecvInt(Sock, &currentVersion))
	{
		klog("CheckVersion: Failed to recieve the current app version.\n");
		return;
	}

	Sockets::SendInt(Sock, AppDatabase::GetVersion() > currentVersion ? 1 : 0);
}

void Apps::GetAppsList(SceNetId Sock)
{
	std::vector<AppDatabase::AppInfo> AppList;
	if (!AppDatabase::GetApps(AppList))
	{
		Sockets::SendInt(Sock, 0);
		return;
	}

	Sockets::SendInt(Sock, AppList.size());

	Sockets::SendLargeData(Sock, (unsigned char*)AppList.data(), AppList.size() * sizeof(AppInfoPacket));
}

void Apps::GetAppInfoString(SceNetId Sock)
{
	char titleId[10];
	memset(titleId, 0, sizeof(titleId));
	sceNetRecv(Sock, titleId, sizeof(titleId), 0);

	// Get the key we are interested in.
	char KeyValue[50];
	sceNetRecv(Sock, KeyValue, sizeof(KeyValue), 0);

	// Look up the key for that titleId in the app.db.
	char OutStr[200];
	memset(OutStr, 0, sizeof(OutStr));
	AppDatabase::GetAppInfoString(titleId, OutStr, sizeof(OutStr), KeyValue);

	// Send back the result.
	sceNetSend(Sock, OutStr, sizeof(OutStr), 0);
}

// TODO: Currently cant get the appId of child processes like the Web Browser since it is a child of the ShellUI.
int Apps::GetAppId(const char* TitleId)
{
	int appId = 0;

	// Get the list of running processes.
	std::vector<kinfo_proc> processList;
	GetProcessList(processList);

	for (const auto& i : processList)
	{
		// Get the app info using the pid.
		SceAppInfo appInfo;
		sceKernelGetAppInfo(i.pid, &appInfo);

		// Using the titleId match our desired app and return the appId from the appinfo.
		if (!strcmp(appInfo.TitleId, TitleId))
		{
			appId = appInfo.AppId;

			break;
		}
	}

	return appId;
}

void Apps::SendAppStatus(SceNetId Sock)
{
	char titleId[10];
	memset(titleId, 0, sizeof(titleId));
	sceNetRecv(Sock, titleId, sizeof(titleId), 0);

	auto appId = GetAppId(titleId);

	// If we have no appId that means the process is not running. 
	if (appId <= 0)
	{
		Sockets::SendInt(Sock, STATE_NOT_RUNNING);
	}
	else
	{
		int state = 0;
		auto res = sceSystemServiceIsAppSuspended(appId, &state);
		if (res == 0 && state)
		{
			Sockets::SendInt(Sock, STATE_SUSPENDED);
		}
		else
		{
			Sockets::SendInt(Sock, STATE_RUNNING);
		}
	}
}

void Apps::StartApp(SceNetId Sock)
{
	char titleId[10];
	memset(titleId, 0, sizeof(titleId));
	sceNetRecv(Sock, titleId, sizeof(titleId), 0);

	LaunchAppParam appParam;
	appParam.enableCrashReport = 0;
	appParam.checkFlag = 0;
	appParam.appAttr = 0;
	appParam.size = sizeof(LaunchAppParam);

	if (auto res = sceUserServiceGetForegroundUser(&appParam.userId) != 0)
	{
		klog("sceUserServiceGetForegroundUser(): Failed with error %llX\n", res);

		Sockets::SendInt(Sock, 0);
		return;
	}

	auto res = sceLncUtilLaunchApp(titleId, nullptr, &appParam);
	if (res <= 0)
	{
		klog("sceLncUtilLaunchApp() : Failed with error %llX\n", res);

		Sockets::SendInt(Sock, 0);
		return;
	}

	Sockets::SendInt(Sock, res);
}

void Apps::KillApp(SceNetId Sock)
{
	char titleId[10];
	memset(titleId, 0, sizeof(titleId));
	sceNetRecv(Sock, titleId, sizeof(titleId), 0);

	auto appId = GetAppId(titleId);

	if (appId > 0 && sceSystemServiceKillApp(appId, -1, 0, 0) == 0)
	{
		Sockets::SendInt(Sock, 1);
	}
	else
	{
		Sockets::SendInt(Sock, 0);
	}
}

void Apps::SuspendApp(SceNetId Sock)
{
	char titleId[10];
	memset(titleId, 0, sizeof(titleId));
	sceNetRecv(Sock, titleId, sizeof(titleId), 0);

	auto appId = GetAppId(titleId);

	if (appId > 0 && sceLncUtilSuspendApp(appId, 0) == 0)
	{
		Sockets::SendInt(Sock, 1);
	}
	else
	{
		Sockets::SendInt(Sock, 0);
	}
}

void Apps::ResumeApp(SceNetId Sock)
{
	char titleId[10];
	memset(titleId, 0, sizeof(titleId));
	sceNetRecv(Sock, titleId, sizeof(titleId), 0);

	auto appId = GetAppId(titleId);

	if (appId > 0 && sceLncUtilResumeApp(appId, 0) == 0 && sceApplicationSetApplicationFocus(appId) == 0)
	{
		Sockets::SendInt(Sock, 1);
	}
	else
	{
		Sockets::SendInt(Sock, 0);
	}
}

void Apps::DeleteApp(SceNetId Sock)
{
	char titleId[10];
	memset(titleId, 0, sizeof(titleId));
	sceNetRecv(Sock, titleId, sizeof(titleId), 0);

	auto result = sceAppInstUtilAppUnInstall(titleId);

	Sockets::SendInt(Sock, (result == 0) ? 1 : 0);
}

void Apps::SetVisibility(SceNetId Sock)
{
	char titleId[10];
	memset(titleId, 0, sizeof(titleId));
	sceNetRecv(Sock, titleId, sizeof(titleId), 0);

	auto value = 0;
	if (!Sockets::RecvInt(Sock, &value))
	{
		klog("SetVisibility(): Failed to recieve value.\n");
		return;
	}

	if (value >= AppDatabase::VisibilityType::VT_NONE && value <= AppDatabase::VisibilityType::VT_INVISIBLE)
	{
		auto result = AppDatabase::SetVisibility(titleId, (AppDatabase::VisibilityType)value);

		// TODO:
		//ShellUIIPC::RefreshContentArea();

		Sockets::SendInt(Sock, result ? 1 : 0);
	}

	Sockets::SendInt(Sock, 0);
}

void Apps::GetVisibility(SceNetId Sock)
{
	char titleId[10];
	memset(titleId, 0, sizeof(titleId));
	sceNetRecv(Sock, titleId, sizeof(titleId), 0);

	auto visibility = AppDatabase::GetVisibility(titleId);
	Sockets::SendInt(Sock, visibility);
}
