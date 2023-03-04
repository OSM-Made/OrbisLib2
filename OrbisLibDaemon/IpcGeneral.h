#pragma once

#include <IpcGeneralCommon.h>

class IpcGeneral
{
private:
	static SceNetId Connect(int pid);
	static bool SendCommand(SceNetId Sock, int Command);

public:
	static void DeleteTempFile(int pid);
	static bool TestConnection(int pid);
	static bool LoadLibrary(int pid, const char* Path, int* HandleOut);
	static bool UnLoadLibrary(int pid, int Handle);
};