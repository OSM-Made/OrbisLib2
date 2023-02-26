#include "stdafx.h"
#include "ProcessMonitor.h"
#include "SignalDefs.h"

ProcessMonitor::ProcessMonitor(int pid)
{
	ShouldRun = true;
	ThreadPool::QueueJob([=] { WatchThread(pid); });
}

ProcessMonitor::~ProcessMonitor()
{
	ShouldRun = false;
}

void ProcessMonitor::WatchThread(int pid)
{
	while (ShouldRun)
	{
		std::vector<kinfo_proc> procList;
		GetProcessList(procList);

		if (std::find_if(procList.begin(), procList.end(), [=](const kinfo_proc& arg) { return arg.pid == pid; }) == procList.end())
		{
			klog("Proc %d has died.\n", pid);

			if (OnExit != nullptr)
				OnExit();

			return;
		}

		int status;
		auto debuggeePid = wait4(pid, &status, WNOHANG, nullptr);
		if (debuggeePid == pid)
		{
			int signal = WSTOPSIG(status);
			klog("Process %d has recieved the signal %d\n", pid, signal);

			switch (signal)
			{
			case SIGSTOP:
				klog("SIGSTOP\n");
				break;
			}
		}

		sceKernelSleep(1);
	}
}