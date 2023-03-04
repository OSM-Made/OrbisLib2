#include "stdafx.h"
#include "IpcGeneral.h"
#include <KernelInterface.h>
#include <GoldHEN.h>

SceNetId IpcGeneral::Connect(int pid)
{
	char fullPath[0x200];
	snprintf(fullPath, sizeof(fullPath), GENERAL_IPC_ADDR, pid);

	// Try to connect to the IPC sock.
	return Sockets::ConnectLocal(fullPath, 5);
}

bool IpcGeneral::SendCommand(SceNetId Sock, int Command)
{
	if (!Sockets::SendInt(Sock, Command))
	{
		klog("[IpcGeneral] Failed to send Command\n");
		return false;
	}

	int Status;
	if (!Sockets::RecvInt(Sock, &Status))
	{
		klog("[IpcGeneral] Failed to recv status\n");
		return false;
	}

	return Status == GIPC_OK;
}

void IpcGeneral::DeleteTempFile(int pid)
{
	char fullPath[0x200];
	snprintf(fullPath, sizeof(fullPath), GENERAL_IPC_ADDR, pid);

	sceKernelUnlink(fullPath);
}

bool IpcGeneral::TestConnection(int pid)
{
	// Open a new local socket connection for the process.
	auto sock = Connect(pid);
	if (!sock)
	{
		klog("[IpcGeneral] Failed to connect to socket.\n");
		return false;
	}

	// Send the command.
	if (!SendCommand(sock, GIPC_HELLO))
	{
		// Close the socket.
		sceNetSocketClose(sock);

		klog("[IpcGeneral] Failed to send command.\n");
		return false;
	}

	// Get the library status.
	int status = 0;
	if (!Sockets::RecvInt(sock, &status))
	{
		// Close the socket.
		sceNetSocketClose(sock);

		klog("[IpcGeneral] Failed to recv status.\n");
		return false;
	}

	// Close the socket.
	sceNetSocketClose(sock);

	return status == GIPC_OK;
}

bool IpcGeneral::LoadLibrary(int pid, const char* Path, int* HandleOut)
{
	// Open a new local socket connection for the process.
	auto sock = Connect(pid);
	if (!sock)
	{
		klog("[IpcGeneral] Failed to connect to socket.\n");
		return false;
	}

	// Send the command.
	if (!SendCommand(sock, GIPC_LIB_LOAD))
	{
		// Close the socket.
		sceNetSocketClose(sock);

		klog("[IpcGeneral] Failed to send command.\n");
		return false;
	}

	// Create next packet.
	auto packet = std::make_unique<PRXPacket>();
	strcpy(packet->Path, Path);

	// Send the packet.
	if (sceNetSend(sock, packet.get(), sizeof(PRXPacket), 0) < 0)
	{
		// Close the socket.
		sceNetSocketClose(sock);

		klog("[IpcGeneral] Failed to send PRXPacket.\n");

		return false;
	}

	// Recieve the result.
	if (!Sockets::RecvInt(sock, HandleOut))
	{
		// Close the socket.
		sceNetSocketClose(sock);

		klog("[IpcGeneral] Failed to recv handle.\n");

		return false;
	}

	// Close the socket.
	sceNetSocketClose(sock);

	// Check to see if it was loaded successfully.
	if (*HandleOut <= 0)
	{
		klog("[IpcGeneral] Failed to load PRX '%s' (0x%llX).\n", Path, *HandleOut);

		return false;
	}

	return true;
}

bool IpcGeneral::UnLoadLibrary(int pid, int Handle)
{
	// Open a new local socket connection for the process.
	auto sock = Connect(pid);
	if (!sock)
	{
		klog("[IpcGeneral] Failed to connect to socket.\n");
		return false;
	}

	// Send the command.
	if (!SendCommand(sock, GIPC_LIB_UNLOAD))
	{
		// Close the socket.
		sceNetSocketClose(sock);

		klog("[IpcGeneral] Failed to send command.\n");
		return false;
	}

	// Create next packet.
	auto packet = std::make_unique<PRXPacket>();
	packet->Handle = Handle;

	// Send the packet.
	if (sceNetSend(sock, packet.get(), sizeof(PRXPacket), 0) < 0)
	{
		// Close the socket.
		sceNetSocketClose(sock);

		klog("[IpcGeneral] Failed to send PRXPacket.\n");

		return false;
	}

	// Recieve the result.
	int result = 0;
	if (!Sockets::RecvInt(sock, &result))
	{
		// Close the socket.
		sceNetSocketClose(sock);

		klog("[IpcGeneral] Failed to recv result.\n");
		return false;
	}

	// Check to see if it was unloaded successfully.
	if (result != 0)
	{
		// Close the socket.
		sceNetSocketClose(sock);

		klog("[IpcGeneral] Failed to un load PRX '%s' (0x%llX).\n", result);
		return false;
	}

	// Close the socket.
	sceNetSocketClose(sock);

	return true;
}
