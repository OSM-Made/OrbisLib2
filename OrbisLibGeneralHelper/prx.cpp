#include "Common.h"
#include "Sockets.h"
#include "LocalSocketListener.h"
#include <IpcGeneralCommon.h>
#include <KernelExt.h>

LocalSocketListener* LocalListener = nullptr;

void LoadUnloadLib(int Command, SceNetId Sock)
{
	auto Packet = (PRXPacket*)malloc(sizeof(PRXPacket));
	sceNetRecv(Sock, Packet, sizeof(PRXPacket), 0);

	if (Command == GIPC_LIB_LOAD)
	{
		int res = sceKernelLoadStartModule(Packet->Path, 0, 0, 0, 0, 0);

		// Send the result.
		Sockets::SendInt(Sock, res);
	}
	else
	{
		int res = sceKernelStopUnloadModule(Packet->Handle, 0, 0, 0, 0, 0);

		// Send the result.
		Sockets::SendInt(Sock, res);
	}

	free(Packet);
}

void ListenerClientThread(void* tdParam, SceNetId Sock)
{
	int Command = 0;

	if (!Sockets::RecvInt(Sock, &Command))
	{
		klog("Failed to recv Command.\n");
		return;
	}

	if (!Sockets::SendInt(Sock, 1))
	{
		klog("Failed to send confirm command.\n");
		return;
	}

	switch (Command)
	{
	default:
		klog("Invalid Command enum %i\n", Command);
		break;

	case GIPC_HELLO:

		Sockets::SendInt(Sock, 1);

		break;

	case GIPC_LIB_LOAD:
	case GIPC_LIB_UNLOAD:

		LoadUnloadLib(Command, Sock);

		break;
	}
}

extern "C"
{
    int __cdecl module_start(size_t argc, const void* args)
    {
		klog("Helping with %d\n", getpid());

		char serverAddress[0x200];
		snprintf(serverAddress, sizeof(serverAddress), GENERAL_IPC_ADDR, getpid());

		LocalListener = new LocalSocketListener(ListenerClientThread, nullptr, serverAddress);

		klog("Helper Init Complete.\n");

        return 0;
    }

    int __cdecl module_stop(size_t argc, const void* args)
    {
		delete LocalListener;

        return 0;
    }
}