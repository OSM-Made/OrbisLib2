#include "stdafx.h"
#include "Monitor.h"
#include "API.h"
#include "ProcessMonitor.h"

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

	// Copy back up of sflash so we can read it and not break things :)
	CopySflash();

	// Set the Name of this process so it shows up as something other than eboot.bin.
	sceKernelSetProcessName("OrbisLibAPI");

	// Start up the thread pool.
	//ThreadPool::Init(10);

	// Log the loaded version string.
	klog("\n%s\n\n", ORBISLIB_BUILDSTRING);

	// Start up the API.
	//API::Init();

	// Blocking run the system monitor.
	//Monitor::Run();

	sceKernelLoadStartModule("/mnt/sandbox/ORBS30000_000/app0/libKernelInterface.sprx", 0, 0, 0, 0, 0);
	std::vector<kinfo_proc> ProcList;
	GetProcessList(ProcList);
	auto proc = std::find_if(ProcList.begin(), ProcList.end(), [](kinfo_proc a) -> bool { return !strcmp(a.name, "eboot.bin"); });
	if (proc != ProcList.end())
	{
		
	}

	//sceKernelSleep(5);

	//UnloadKDriver();

	ExitGraceful();
	return 0;
}