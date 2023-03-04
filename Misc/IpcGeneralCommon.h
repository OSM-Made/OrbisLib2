#pragma once

// Genreall IPC ADDRS should follow the GeneralIPC-<ProcessName> pattern.
#define GENERAL_IPC_ADDR "/data/Orbis Suite/IPC/IpcGeneral-%d"

enum GeneralIPCCommands
{
	GIPC_HELLO,
	GIPC_LIB_LOAD,
	GIPC_LIB_UNLOAD,
	GIPC_JAILBREAK,
	GIPC_JAIL,
};

enum GeneralIPCResult
{
	GIPC_FAIL,
	GIPC_OK,
};

struct PRXPacket
{
	uint32_t Handle;
	char Path[256];
};