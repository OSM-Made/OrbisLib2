#include "stdafx.h"
#include "Library.h"
#include "Debug.h"
#include <KernelInterface.h>
#include "LibraryPackets.h"
#include "IpcGeneral.h"

void Library::LoadLibrary(SceNetId s)
{
	if (!Debug::IsDebugging || Debug::CurrentPID == -1)
	{
		return;
	}

	auto packet = Sockets::RecieveType<SPRXPacket>(s);

	// Load the library.
	int handle = 0;
	IpcGeneral::LoadLibrary(Debug::CurrentPID, packet->Path, &handle);

	// Send the result.
	Sockets::SendInt(s, handle);
}

void Library::UnloadLibrary(SceNetId s)
{
	if (!Debug::IsDebugging || Debug::CurrentPID == -1)
	{
		return;
	}

	auto packet = Sockets::RecieveType<SPRXPacket>(s);

	// Unload the library.
	auto result = IpcGeneral::UnLoadLibrary(Debug::CurrentPID, packet->Handle);

	// Send the result.
	Sockets::SendInt(s, result);
}

void Library::ReloadLibrary(SceNetId s)
{
	if (!Debug::IsDebugging || Debug::CurrentPID == -1)
	{
		return;
	}

	auto packet = Sockets::RecieveType<SPRXPacket>(s);

	// Unload the library.
	auto result = IpcGeneral::UnLoadLibrary(Debug::CurrentPID, packet->Handle);
	if (result != 0)
	{
		klog("Failed to unload %d\n", packet->Handle);

		Sockets::SendInt(s, result);

		return;
	}

	// Load the library.
	int handle = 0;
	IpcGeneral::LoadLibrary(Debug::CurrentPID, packet->Path, &handle);

	// Send the result.
	Sockets::SendInt(s, handle);
}

void Library::GetLibraryList(SceNetId s)
{
	if (!Debug::IsDebugging || Debug::CurrentPID == -1)
	{
		return;
	}

	OrbisLibraryInfo libraries[256];
	int actualCount = GetLibraries(Debug::CurrentPID, &libraries[0], 256);

	// Send the data size.
	Sockets::SendInt(s, actualCount);

	// Send the list to host.
	Sockets::SendLargeData(s, (unsigned char*)&libraries[0], actualCount * sizeof(OrbisLibraryInfo));
}