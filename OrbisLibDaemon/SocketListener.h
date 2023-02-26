#pragma once

class SocketListener
{
private:
	void(*ClientCallBack)(void* tdParam, SceNetId Sock, SceNetInAddr sin_addr);
	bool ServerRunning;
	void* tdParam;

	void ListenThread(uint64_t port);

public:
	SocketListener(void(*ClientCallBack)(void* tdParam, SceNetId Sock, SceNetInAddr sin_addr), void* tdParam, unsigned short Port);
	~SocketListener();
};