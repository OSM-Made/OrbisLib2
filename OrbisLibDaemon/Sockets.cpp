#include "stdafx.h"
#include "Sockets.h"

SceNetId Sockets::Connect(SceNetInAddr_t address, uint16_t port, int timeOutSeconds)
{
	// Set up socket params.
	SceNetSockaddrIn addr = { 0 };
	addr.sin_family = SCE_NET_AF_INET;
	addr.sin_addr.s_addr = address;
	addr.sin_port = sceNetHtons(port);

	// Create socket.
	auto sock = sceNetSocket("SendEventSock", SCE_NET_AF_INET, SCE_NET_SOCK_STREAM, SCE_NET_IPPROTO_TCP);
	if (sock < 0)
	{
		klog("Sockets::Connect() Failed to allocate sock: %llX %llX\n", sock, *sceNetErrnoLoc());
		return -1;
	}

	// Set connection/transmission time out.
	int sock_timeout = timeOutSeconds * 1000000;
	sceNetSetsockopt(sock, SCE_NET_SOL_SOCKET, SCE_NET_SO_CONNECTTIMEO, &sock_timeout, sizeof(sock_timeout));
	sceNetSetsockopt(sock, SCE_NET_SOL_SOCKET, SCE_NET_SO_SNDTIMEO, &sock_timeout, sizeof(sock_timeout));
	sceNetSetsockopt(sock, SCE_NET_SOL_SOCKET, SCE_NET_SO_RCVTIMEO, &sock_timeout, sizeof(sock_timeout));

	// Connect to the socket.
	auto res = sceNetConnect(sock, (SceNetSockaddr*)&addr, sizeof(addr));
	if (!res)
		return sock;
	else
	{
		klog("Sockets::Connect() sceNetConnect(): %llX %llX\n", res, *sceNetErrnoLoc());
		return -1;
	}
}

bool Sockets::SendInt(SceNetId Sock, int val)
{
	auto res = sceNetSend(Sock, &val, sizeof(int), 0);
	if (res <= 0)
	{
		klog("SendInt(): Failed to send %llX\n", res);

		return false;
	}

	return true;
}

bool Sockets::RecvInt(SceNetId Sock, int* val)
{
	auto res = sceNetRecv(Sock, val, sizeof(int), 0);
	if (res <= 0)
	{
		klog("RecvInt(): Failed to recv %llX\n", res);

		return false;
	}

	return true;
}

bool Sockets::SendLargeData(SceNetId Sock, unsigned char* data, size_t dataLen)
{
	unsigned char* CurrentPosition = data;
	size_t DataLeft = dataLen;
	int res = 0;

	while (DataLeft > 0)
	{
		size_t DataChunkSize = std::min((size_t)8192, DataLeft);

		res = sceNetSend(Sock, CurrentPosition, DataChunkSize, 0);

		if (res < 0)
		{
			klog("SendLargeData() Error: %llX(%llX)\n", res, *sceNetErrnoLoc());
			return false;
		}

		DataLeft -= res;
		CurrentPosition += res;
	}

	return true;
}

bool Sockets::RecvLargeData(SceNetId Sock, unsigned char* data, size_t dataLen)
{
	size_t DataLeft = dataLen;
	int Received = 0;
	int res = 0;

	while (DataLeft > 0)
	{
		size_t DataChunkSize = std::min((size_t)8192, DataLeft);
		res = sceNetRecv(Sock, data + Received, DataChunkSize, 0);

		if (res < 0)
		{
			klog("RecvLargeData() Error: %llX(%llX)\n", res, *sceNetErrnoLoc());
			return false;
		}

		Received += res;
		DataLeft -= res;
	}

	return true;
}