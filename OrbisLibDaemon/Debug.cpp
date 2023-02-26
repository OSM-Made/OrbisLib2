#include "stdafx.h"
#include "ProcessMonitor.h"
#include "Debug.h"
#include "Events.h"
#include "PtraceDefs.h"

std::mutex Debug::DebugMtx;
bool Debug::IsDebugging;
int Debug::CurrentPID;
std::shared_ptr<ProcessMonitor> Debug::DebuggeeMonitor;

bool Debug::TryDetach(int pid)
{
	// Check if we are even attached.
	if (!IsDebugging)
	{
		return true;
	}

	// Detach from the process.
	int res = ptrace(PT_DETACH, pid, nullptr, 0);
	if (res != 0)
	{
		// Check if proc is dead anyway and just detach.

		klog("DetachProcess(): ptrace(PT_DETACH) failed with error %llX\n", res);
		return false;
	}

	// Reset vars.
	IsDebugging = false;
	CurrentPID = -1;

	// Wait for the current proc thread to die.
	DebuggeeMonitor.reset();

	return true;
}

void Debug::Attach(SceNetId sock)
{
	auto pid = 0;
	if (!Sockets::RecvInt(sock, &pid))
	{

		return;
	}

	// Get Process name.
	char processName[32];
	sceKernelGetProcessName(pid, processName);

	{
		std::unique_lock<std::mutex> lock(DebugMtx);

		klog("Attempting to attach to %s (%d)\n", processName, pid);

		// If we are currently debugging another process lets detach from it.
		if (!TryDetach(pid))
		{
			klog("Attach(): TryDetach Failed. :(\n");
			Sockets::SendInt(sock, 0);
			return;
		}

		// Use ptrace to attach to begin debugging this pid.
		int res = ptrace(PT_ATTACH, pid, nullptr, 0);
		if (res != 0)
		{
			klog("Attach(): ptrace(PT_ATTACH) failed with error %llX\n", res);
			Sockets::SendInt(sock, 0);
			return;
		}

		sceKernelUsleep(500);

		// Attaching by default will stop execution of the remote process. Lets continue it now.
		res = ptrace(PT_CONTINUE, pid, (void*)1, 0);
		if (res != 0)
		{
			klog("Attach(): ptrace(PT_CONTINUE) failed with error %llX\n", res);
			Sockets::SendInt(sock, 0);
			return;
		}

		// Set current debugging state.
		IsDebugging = true;
		CurrentPID = pid;

		// Set up proc monitor.
		DebuggeeMonitor = std::make_shared<ProcessMonitor>(pid);
	}

	// Send attach event to host.
	Events::SendEvent(Events::EVENT_ATTACH, pid);

	klog("Attached to %s(%d)\n", processName, pid);

	Sockets::SendInt(sock, 1);

	// Mount /data/ in sandbox.
	if (strcmp(processName, "SceShellCore"))
	{
		// Get app info.
		SceAppInfo appInfo;
		sceKernelGetAppInfo(pid, &appInfo);

		// Get sandbox path.
		char sandBoxPath[PATH_MAX];
		snprintf(sandBoxPath, sizeof(sandBoxPath), "/mnt/sandbox/%s_000/data", appInfo.TitleId);

		// Mount data into sandbox
		LinkDir("/data/", sandBoxPath);
	}
}

void Debug::Detach(SceNetId sock)
{
	if (!IsDebugging)
		Sockets::SendInt(sock, 0);

	{
		std::unique_lock<std::mutex> lock(DebugMtx);

		if (TryDetach(CurrentPID))
		{
			Events::SendEvent(Events::EVENT_DETACH);
			Sockets::SendInt(sock, 1);
		}
		else
		{
			klog("Failed to detach from %d\n", CurrentPID);
			Sockets::SendInt(sock, 0);

			// TODO: if proc dead detach or release.
		}
	}
}

void Debug::Current(SceNetId sock)
{
	if (!IsDebugging)
		Sockets::SendInt(sock, -1);

	Sockets::SendInt(sock, CurrentPID);
}