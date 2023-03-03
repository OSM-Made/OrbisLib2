#pragma once
#include "ProcessMonitor.h"

class Debug
{
public:
	static void Attach(SceNetId sock);
	static void Detach(SceNetId Sock);
	static void Current(SceNetId sock);

private:
	static std::mutex DebugMtx;
	static bool IsDebugging;
	static int CurrentPID;
	static std::shared_ptr<ProcessMonitor> DebuggeeMonitor;

	static bool TryDetach(int pid);
};
