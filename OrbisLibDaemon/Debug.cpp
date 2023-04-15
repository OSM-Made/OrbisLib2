#include "stdafx.h"
#include "ProcessMonitor.h"
#include "Debug.h"
#include "DebugPackets.h"
#include "Events.h"
#include "PtraceDefs.h"
#include "SignalDefs.h"
#include <KernelInterface.h>
#include "IpcGeneral.h"

std::mutex Debug::DebugMtx;
bool Debug::IsDebugging;
int Debug::CurrentPID;
std::shared_ptr<ProcessMonitor> Debug::DebuggeeMonitor;

bool Debug::CheckDebug(SceNetId s)
{
	if (!IsDebugging || CurrentPID == -1)
	{
		Sockets::SendInt(s, 0);
		return false;
	}

	Sockets::SendInt(s, 1);
	return true;
}

void Debug::Attach(SceNetId sock)
{
	auto pid = 0;
	if (!Sockets::RecvInt(sock, &pid))
	{
		klog("Attach(): Failed to recieve the pid\n");
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
			klog("Attach(): ptrace(PT_ATTACH) failed with error %llX %s\n", __error(), strerror(errno));
			Sockets::SendInt(sock, 0);
			return;
		}

		// Wait till the process haults.
		waitpid(pid, NULL, 0);

		// Attaching by default will stop execution of the remote process. Lets continue it now.
		res = ptrace(PT_CONTINUE, pid, (void*)1, 0);
		if (res != 0)
		{
			klog("Attach(): ptrace(PT_CONTINUE) failed with error %llX %s\n", __error(), strerror(errno));
			Sockets::SendInt(sock, 0);
			return;
		}

		// Set current debugging state.
		IsDebugging = true;
		CurrentPID = pid;

		// Set up proc monitor.
		DebuggeeMonitor = std::make_shared<ProcessMonitor>(pid);
		DebuggeeMonitor->OnExit = OnExit; // Fired when a process dies.
		DebuggeeMonitor->OnException = OnException; // Fired when the process being debugged encounters an excepton.
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
		}
	}
}

void Debug::Current(SceNetId sock)
{
	if (!IsDebugging)
	{
		Sockets::SendInt(sock, -1);
	}
	else
	{
		Sockets::SendInt(sock, CurrentPID);
	}
}

void Debug::RWMemory(SceNetId s, bool write)
{
	if (!CheckDebug(s))
		return;

	auto packet = Sockets::RecieveType<RWPacket>(s);
	auto buffer = std::make_unique<unsigned char[]>(packet->Length);

	// TODO: Might be a good idea to make sure we are landing in the right memory regions. Should be good to check the vmmap and the library list.
	//		 Pretty sure we can use the syscall from the kernel context and specify the debug proc to achieve the same. 
	//		 (syscall 572) sceKernelVirtualQuery(const void* address, int flags, SceKernelVirtualQueryInfo* info, size_t infoSize)

	if (write)
	{
		if (!Sockets::RecvLargeData(s, buffer.get(), packet->Length))
		{
			klog("Debug::RWMemory(): Failed to recieve memory to write\n");

			return;
		}

		if (!ReadWriteMemory(CurrentPID, (void*)packet->Address, (void*)buffer.get(), packet->Length, true))
		{
			klog("Debug::RWMemory(): Failed to write memory to process %i at %llX\n", CurrentPID, packet->Address);

			Sockets::SendInt(s, 0);

			return;
		}

		Sockets::SendInt(s, 1);
	}
	else
	{
		if (!ReadWriteMemory(CurrentPID, (void*)packet->Address, (void*)buffer.get(), packet->Length, false))
		{
			klog("Debug::RWMemory(): Failed to read memory to process %i at %llX\n", CurrentPID, packet->Address);

			Sockets::SendInt(s, 0);

			return;
		}

		Sockets::SendInt(s, 1);

		if (!Sockets::SendLargeData(s, buffer.get(), packet->Length))
		{
			klog("Failed to send memory\n");
			return;
		}
	}
}

void Debug::OnExit()
{
	klog("Process %d has died!\n", CurrentPID);

	// Send the event to the host that the process has died.
	Events::SendEvent(Events::EVENT_DIE, CurrentPID);

	// For now just detach.
	if (!TryDetach(CurrentPID))
	{
		klog("OnExit(): TryDetach Failed. :(\n");
		return;
	}

	Events::SendEvent(Events::EVENT_DETACH);
}

void Debug::OnException(int status)
{
	int signal = WSTOPSIG(status);

	switch (signal)
	{
	case SIGSTOP:
		klog("SIGSTOP\n");
		break;
	}

	// For now just detach.
	if (!TryDetach(CurrentPID))
	{
		klog("OnException(): TryDetach Failed. :(\n");
		return;
	}

	Events::SendEvent(Events::EVENT_DETACH);
}

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
		std::vector<kinfo_proc> procList;
		GetProcessList(procList);

		if (std::find_if(procList.begin(), procList.end(), [=](const kinfo_proc& arg) { return arg.pid == pid; }) == procList.end())
		{
			// Reset vars.
			IsDebugging = false;
			CurrentPID = -1;

			return true;
		}

		klog("DetachProcess(): ptrace(PT_DETACH) failed with error %llX %s\n", __error(), strerror(errno));
		return false;
	}

	// Reset vars.
	IsDebugging = false;
	CurrentPID = -1;

	// Kill the current proc monitor.
	DebuggeeMonitor.reset();

	return true;
}