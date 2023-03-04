#pragma once

typedef struct SceNetSockaddrUn
{
	uint8_t sun_len;
	SceNetSaFamily_t sun_family;
	char sun_path[104];
} SceNetSockaddrUn;

#define SCE_NET_AF_LOCAL 1
#define SUN_LEN(s) (2+strlen((s)->sun_path))

class Sockets
{
public:
	static SceNetId ConnectLocal(char* address, int timeOutSeconds);
	static SceNetId Connect(SceNetInAddr_t address, uint16_t port, int timeOutSeconds);
	static bool SendInt(SceNetId Sock, int val);
	static bool RecvInt(SceNetId Sock, int* val);
	static bool SendLargeData(SceNetId Sock, unsigned char* data, size_t dataLen);
	static bool RecvLargeData(SceNetId Sock, unsigned char* data, size_t dataLen);
	template<class T> static std::shared_ptr<T> RecieveType(SceNetId sock)
	{
		auto packet = std::make_shared<T>();
		return RecvLargeData(sock, (unsigned char*)packet.get(), sizeof(T)) ? packet : nullptr;
	}

private:

};
