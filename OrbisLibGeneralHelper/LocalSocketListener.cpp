#include "Common.h"
#include "LocalSocketListener.h"
#include "Sockets.h"

void LocalSocketListener::DoWork()
{
	SceNetSockaddrUn addr = { 0 };
	addr.sun_family = SCE_NET_AF_LOCAL;
	strncpy(addr.sun_path, this->ServerAddress, sizeof(addr.sun_path));

	// Make new local Socket
	this->Socket = sceNetSocket("Local Listener Socket", SCE_NET_AF_LOCAL, SCE_NET_SOCK_STREAM, 0);

	// Set Sending and reciving time out to 2s.
	int sock_timeout = 2 * 1000000;
	sceNetSetsockopt(this->Socket, SCE_NET_SOL_SOCKET, SCE_NET_SO_SNDTIMEO, &sock_timeout, sizeof(sock_timeout));
	sceNetSetsockopt(this->Socket, SCE_NET_SOL_SOCKET, SCE_NET_SO_RCVTIMEO, &sock_timeout, sizeof(sock_timeout));

	// Make sure every time we can rebind to the port.
	int reusePort = 1;
	sceNetSetsockopt(this->Socket, SCE_NET_SOL_SOCKET, SCE_NET_SO_REUSEPORT, &reusePort, sizeof(reusePort));

	auto bindError = sceNetBind(this->Socket, (SceNetSockaddr*)&addr, SUN_LEN(&addr));
	if (bindError != 0)
	{
		klog("Failed to bind Listener to address %s\nError: %X", this->ServerAddress, bindError);

		goto Cleanup;
	}

	if (sceNetListen(this->Socket, 100) != 0)
	{
		klog("Failed to start listening on Socket.\n");

		goto Cleanup;
	}

	while (this->ServerRunning)
	{
		fd_set set;
		struct timeval timeout;
		FD_ZERO(&set); /* clear the set */
		FD_SET(this->Socket, &set); /* add our file descriptor to the set */

		timeout.tv_sec = 2;
		timeout.tv_usec = 0;

		// Wait for incoming connections.
		auto rv = select((int)this->Socket + 1, &set, NULL, NULL, &timeout);
		if (rv == -1)
			goto Cleanup;
		else if (rv == 0)
		{
			if (!this->ServerRunning)
				goto Cleanup;
			continue;
		}
		else
		{
			if (!this->ServerRunning)
				goto Cleanup;

			SceNetSockaddrIn ClientAddr = { 0 };
			SceNetSocklen_t addrlen = sizeof(SceNetSockaddrIn);
			auto ClientSocket = sceNetAccept(this->Socket, (SceNetSockaddr*)&ClientAddr, &addrlen);

			if (ClientSocket != -1)
			{
				// Do call back.
				ClientCallBack(tdParam, ClientSocket);

				// Reset ClientSocket.
				sceNetSocketClose(ClientSocket);
				ClientSocket = -1;
			}
		}
	}

Cleanup:
	klog("Listener Thread Exiting!\n");

	// Clean up.
	sceNetSocketClose(this->Socket);
	sceKernelUnlink(this->ServerAddress);
}

LocalSocketListener::LocalSocketListener(void(*ClientCallBack)(void* tdParam, SceNetId Sock), void* tdParam, char* ServerAddress)
{
	this->ClientCallBack = ClientCallBack;
	this->tdParam = tdParam;
	this->ServerRunning = true;
	strcpy(this->ServerAddress, ServerAddress);

	scePthreadCreate(&ListenThreadHandle, NULL, [](void* arg) -> void* 
		{
			((LocalSocketListener*)arg)->DoWork();

			// Kill our thread and exit.
			scePthreadExit(NULL);
			return nullptr;
		}, this, "Local Listen Thread");
	scePthreadDetach(ListenThreadHandle);
}

LocalSocketListener::~LocalSocketListener()
{
	this->ServerRunning = false;
	scePthreadJoin(ListenThreadHandle, nullptr);
}