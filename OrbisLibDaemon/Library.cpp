#include "stdafx.h"
#include "Library.h"
#include "Debug.h"
#include <KernelInterface.h>
#include "LibraryPackets.h"
#include "IpcGeneral.h"
#include <GoldHEN.h>

void Library::LoadLibrary(SceNetId s)
{
	if (!Debug::CheckDebug(s))
		return;

	auto packet = Sockets::RecieveType<SPRXPacket>(s);

	// Get Process name.
	char processName[32];
	sceKernelGetProcessName(Debug::CurrentPID, processName);

	// Load the library.
	auto handle = sys_sdk_proc_prx_load(processName, packet->Path);

	// Send the result.
	Sockets::SendInt(s, handle);
}

void Library::UnloadLibrary(SceNetId s)
{
	if (!Debug::CheckDebug(s))
		return;

	auto packet = Sockets::RecieveType<SPRXPacket>(s);

	// Get Process name.
	char processName[32];
	sceKernelGetProcessName(Debug::CurrentPID, processName);

	// Unload the library.
	auto result = sys_sdk_proc_prx_unload(processName, packet->Handle);

	// Send the result.
	Sockets::SendInt(s, result);
}

void Library::ReloadLibrary(SceNetId s)
{
	if (!Debug::CheckDebug(s))
		return;

	auto packet = Sockets::RecieveType<SPRXPacket>(s);

	// Get Process name.
	char processName[32];
	sceKernelGetProcessName(Debug::CurrentPID, processName);

	// Unload the library.
	auto result = sys_sdk_proc_prx_unload(processName, packet->Handle);
	if (result != 0)
	{
		klog("Failed to unload %d\n", packet->Handle);

		Sockets::SendInt(s, result);

		return;
	}

	// Load the library.
	auto handle = sys_sdk_proc_prx_load(processName, packet->Path);

	// Send the result.
	Sockets::SendInt(s, handle);
}

void Library::GetLibraryList(SceNetId s)
{
	if (!Debug::CheckDebug(s))
		return;

	auto libraries = std::make_unique<OrbisLibraryInfo[]>(256);
	int actualCount = GetLibraries(Debug::CurrentPID, libraries.get(), 256);

	// Send the data size.
	Sockets::SendInt(s, actualCount);

	// Send the list to host.
	Sockets::SendLargeData(s, (unsigned char*)libraries.get(), actualCount * sizeof(OrbisLibraryInfo));
}