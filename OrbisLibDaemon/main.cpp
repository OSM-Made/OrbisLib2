#include "stdafx.h"

int main(int argc, char** arg)
{
	// Jailbreak our current process.
	//if (!Jailbreak())
	//{
	//	Notify("Failed to jailbreak Process...");
	//	ExitGraceful();
	//	return 0;
	//}

	// Load internal system modules.
	if (!LoadModules())
	{
		Notify("Failed to Load Modules...");
		ExitGraceful();
		return 0;
	}

	// Call cleanup on exit.

	// Copy back up of sflash so we can read it and not break things :)
	//CopySflash();

	// Set the Name of this process so it shows up as something other than eboot.bin.
	sceKernelSetProcessName("OrbisLibAPI");

	// Log the loaded version string.
	klog("\n%s\n\n", ORBISLIB_BUILDSTRING);

	// Start up the thread pool.
	ThreadPool::Init(10);

	klog("Done\n");

	ExitGraceful();
	return 0;
}