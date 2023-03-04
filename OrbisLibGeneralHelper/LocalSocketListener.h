#pragma once

class LocalSocketListener
{
private:
	ScePthread ListenThreadHandle;
	SceNetId Socket;
	/// Used to signal thread to shut down
	bool ServerRunning;
	char ServerAddress[0x100];

	void DoWork();
	void* tdParam;
	void(*ClientCallBack)(void* tdParam, SceNetId Sock);

public:
	LocalSocketListener(void(*ClientCallBack)(void* tdParam, SceNetId Sock), void* tdParam, char* ServerAddress);
	~LocalSocketListener();
};

struct ClientThreadParams
{
	LocalSocketListener* LocalSocketListener;
	SceNetId Sock;
};