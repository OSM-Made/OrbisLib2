#include "stdafx.h"
#include <UserServiceExt.h>

int main(int argc, char** arg)
{
	// Jailbreak our current process.
	if (!Jailbreak())
	{
		Notify("Failed to jailbreak Process...");
		ExitGraceful();
		return 0;
	}

	// Load internal system modules.
	if (!LoadModules())
	{
		Notify("Failed to Load Modules...");
		ExitGraceful();
		return 0;
	}

	//MakeDirs
	MakeDir("/data/Orbis Suite");
	MakeDir("/data/Orbis Suite/IPC");

	// Set RW on the system directory.
	mount_large_fs("/dev/da0x4.crypt", "/system", "exfatfs", "511", MNT_UPDATE);

	// Install all the things! :D
	const char* LibList[] = { "libGoldHEN.sprx", "libKernelInterface.sprx" };
	InstallDaemon("ORBS30000", LibList, 2); // Orbis Lib
	InstallOrbisToolbox();
	InstallOrbisSuite();

	// Start the API.
	auto appId = sceLncUtilGetAppId("ORBS30000");
	if (appId)
	{
#ifdef DEBUG
		sceLncUtilKillApp(appId);
#else
		ExitGraceful();
		return 0;
#endif
	}

	LaunchAppParam appParam;
	appParam.size = sizeof(LaunchAppParam);
	sceUserServiceGetForegroundUser(&appParam.userId);
	appParam.enableCrashReport = 0;
	appParam.checkFlag = 0;
	appParam.appAttr = 0;

#ifdef DEBUG
	auto res = sceLncUtilLaunchApp("ORBS30000", nullptr, &appParam);

	if (res != 0)
	{
		klog("Failed to load Daemon.\n");
	}
#else
	// sceLncUtilLaunchApp("ORBS30000", nullptr, &appParam);
#endif // DEBUG

	ExitGraceful();
	return 0;
}