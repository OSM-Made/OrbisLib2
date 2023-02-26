#include "stdafx.h"
#include "Monitor.h"
#include "API.h"
#include <ctype.h>

void Terminate()
{
	klog("Good bye friends\n");
}

void hexdump(void* ptr, int buflen) {
	unsigned char* buf = (unsigned char*)ptr;
	int i, j;
	for (i = 0; i < buflen; i += 16) {
		klog("%06x: ", i);
		for (j = 0; j < 16; j++)
			if (i + j < buflen)
				klog("%02x ", buf[i + j]);
			else
				klog("   ");
		klog(" ");
		for (j = 0; j < 16; j++)
			if (i + j < buflen)
				klog("%c", isprint(buf[i + j]) ? buf[i + j] : '.');
		klog("\n");
	}
}

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

	// Call cleanup on exit.
	atexit(Terminate);

	// Copy back up of sflash so we can read it and not break things :)
	CopySflash();

	// Set the Name of this process so it shows up as something other than eboot.bin.
	sceKernelSetProcessName("OrbisLibAPI");

	// Start up the thread pool.
	ThreadPool::Init(10);

	// Log the loaded version string.
	klog("\n%s\n\n", ORBISLIB_BUILDSTRING);

	// Start up the API.
	//API::Init();

	// Blocking run the system monitor.
	//Monitor::Run();

	SceAppInfo info;
	sceKernelGetAppInfo(getpid(), &info);

	hexdump(&info, sizeof(SceAppInfo));

	ExitGraceful();
	return 0;
}