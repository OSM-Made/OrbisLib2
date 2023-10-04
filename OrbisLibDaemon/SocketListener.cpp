#include "stdafx.h"
#include "ThreadPool.h"
#include "SocketListener.h"
#include <sys/select.h>
#include <libnetctl.h>

void SocketListener::ListenThread(uint64_t port)
{
	SceNetSockaddrIn addr = { 0 };
	addr.sin_family = SCE_NET_AF_INET;
	addr.sin_addr.s_addr = SCE_NET_INADDR_ANY; // Any incoming address
	addr.sin_port = sceNetHtons(port); // Our desired listen port

	// Make new TCP Socket
	ServerSocket = sceNetSocket("Listener Socket", SCE_NET_AF_INET, SCE_NET_SOCK_STREAM, SCE_NET_IPPROTO_TCP);

	// Set Sending and reciving time out to 2s
	int sock_timeout = 2000000;
	sceNetSetsockopt(ServerSocket, SCE_NET_SOL_SOCKET, SCE_NET_SO_SNDTIMEO, &sock_timeout, sizeof(sock_timeout));
	sceNetSetsockopt(ServerSocket, SCE_NET_SOL_SOCKET, SCE_NET_SO_RCVTIMEO, &sock_timeout, sizeof(sock_timeout));

	// Make sure every time we can rebind to the port.
	int reusePort = 1;
	sceNetSetsockopt(ServerSocket, SCE_NET_SOL_SOCKET, SCE_NET_SO_REUSEPORT, &reusePort, sizeof(reusePort));

	auto bindError = sceNetBind(ServerSocket, (SceNetSockaddr*)&addr, sizeof(addr));
	if (bindError != 0)
	{
		klog("Failed to bind Listener to port %i\nError: %X", port, bindError);

		goto Cleanup;
	}

	if (sceNetListen(ServerSocket, 100) != 0)
	{
		klog("Failed to start listening on Socket.\n");

		goto Cleanup;
	}

	while (ServerRunning)
	{
		fd_set set;
		struct timeval timeout;
		FD_ZERO(&set); /* clear the set */
		FD_SET(ServerSocket, &set); /* add our file descriptor to the set */

		timeout.tv_sec = 2;
		timeout.tv_usec = 0;

		// Wait for incoming connections.
		auto rv = select((int)ServerSocket + 1, &set, NULL, NULL, &timeout);
		if (rv == -1)
		{
			goto Cleanup;
		}
		else if (rv == 0)
		{
			if (!ServerRunning)
				goto Cleanup;

			continue;
		}
		else
		{
			if (!ServerRunning)
				goto Cleanup;

			SceNetSockaddrIn ClientAddr = { 0 };
			SceNetSocklen_t addrlen = sizeof(SceNetSockaddrIn);
			auto ClientSocket = sceNetAccept(ServerSocket, (SceNetSockaddr*)&ClientAddr, &addrlen);

			if (ClientSocket != -1)
			{
				//klog("New Connection from %i.%i.%i.%i!\n", ClientAddr.sin_addr.s_addr & 0xFF, (ClientAddr.sin_addr.s_addr >> 8) & 0xFF, (ClientAddr.sin_addr.s_addr >> 16) & 0xFF, (ClientAddr.sin_addr.s_addr >> 24) & 0xFF);

				ThreadPool::QueueJob([=]
					{
						auto sock = ClientSocket;
						auto addr = ClientAddr.sin_addr;
				
						ClientCallBack(tdParam, sock, addr);
						sceNetSocketClose(sock);
					});

				// Reset ClientSocket.
				ClientSocket = -1;
			}
		}
	}

Cleanup:
	klog("Listener Thread Exiting!\n");

	// Clean up.
	if (ServerSocket != -1)
		sceNetSocketClose(ServerSocket);
}

SocketListener::SocketListener(void(*ClientCallBack)(void* tdParam, SceNetId Sock, SceNetInAddr sin_addr), void* tdParam, unsigned short Port)
{
	this->ClientCallBack = ClientCallBack;
	this->tdParam = tdParam;
	ServerRunning = true;

	sceNetCtlRegisterCallback([](int eventType, void* arg)
		{
			auto listener = (SocketListener*)arg;
			if (!listener->ServerRunning)
			{
				klog("Listener: Network Connection Lost.\n");

				listener->ServerRunning = false;
				sceNetSocketClose(listener->ServerSocket);
				listener->ServerSocket = -1;
			}
		}, this, &CallbackId);

	ThreadPool::QueueJob([=] { ListenThread(Port); });

	ThreadPool::QueueJob([=]
		{
			while (true)
			{
				int state = 0;
				sceNetCtlGetState(&state);

				switch (state)
				{
				default:
				case SCE_NET_CTL_STATE_DISCONNECTED:

					if (ServerRunning)
					{
						klog("Listener: Network Connection Lost.\n");
						ServerRunning = false;
						sceNetSocketClose(ServerSocket);
						ServerSocket = -1;
					}

					break;

				case SCE_NET_CTL_STATE_IPOBTAINED:

					if (!ServerRunning)
					{
						klog("Listener: Network Connection Online.\n");
						ServerRunning = true;
						ThreadPool::QueueJob([=] { ListenThread(Port); });
					}

					break;
				}

				sceKernelSleep(1);
			}
		});
}

SocketListener::~SocketListener()
{
	ServerRunning = false;
	sceNetCtlUnregisterCallback(CallbackId);
}