#pragma once

struct MemoryInfo
{
	int Used;
	int Free;
	int Total;
	float Percentage;
};

struct CurrentBigApp
{
	int Pid;
	char Name[32];
	char TitleId[10];
};

struct TargetInfoPacket
{
	int SDKVersion;
	int SoftwareVersion;
	int FactorySoftwareVersion;
	CurrentBigApp BigApp;
	char ConsoleName[100];
	char MotherboardSerial[14];
	char Serial[10];
	char Model[14];
	char MACAdressLAN[18];
	char MACAdressWIFI[18];
	int UART;
	int IDUMode;
	char IDPS[16];
	char PSID[16];
	int ConsoleType;
	int Attached;
	int AttachedPid;
	int ForegroundAccountId;

	// Storage Stats
	uint64_t FreeSpace;
	uint64_t TotalSpace;

	// System Stats
	int CPUTemp;
	int SOCTemp;
	int ThreadCount;
	float AverageCPUUsage;
	int BusyCore;
	MemoryInfo Ram;
	MemoryInfo VRam;
};

struct TargetNotifyPacket
{
	char IconURI[1024];
	char Message[1024];
};

struct ProcPacket
{
	int AppId;
	int ProcessId;
	char Name[32];
	char TitleId[10];
};