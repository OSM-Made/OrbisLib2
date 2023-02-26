#include "stdafx.h"
#include "Flash.h"
#include "TargetPackets.h"
#include "Target.h"
#include <NetExt.h>
#include <user_service.h>
#include "ShellCoreUtil.h"
#include "System.h"
#include "UserServiceExt.h"
#include "SystemServiceExt.h"
#include "APIPackets.h"

void Target::SendTargetInfo(SceNetId sock)
{
	auto Packet = std::make_unique<TargetInfoPacket>();

	Packet->SDKVersion = GetSDKVersion();
	Packet->SoftwareVersion = GetUpdateVersion();
	ReadFlash(FLASH_FACTORY_FW, &Packet->FactorySoftwareVersion, sizeof(int));
	Packet->CPUTemp = GetCPUTemp();
	Packet->SOCTemp = GetSOCTemp();

	// Current Big App.
	auto bigAppAppId = sceSystemServiceGetAppIdOfBigApp();
	if (bigAppAppId > 0)
	{
		// Get the list of running processes.
		std::vector<kinfo_proc> processList;
		GetProcessList(processList);

		for (const auto& i : processList)
		{
			// Get the app info using the pid.
			SceAppInfo appInfo;
			sceKernelGetAppInfo(i.pid, &appInfo);

			// Using the titleId match our desired app and return the appId from the appinfo.
			if (appInfo.AppId == bigAppAppId)
			{
				Packet->BigApp.Pid = i.pid;
				strcpy(Packet->BigApp.Name, i.name);
				strcpy(Packet->BigApp.TitleId, appInfo.TitleId);

				break;
			}
		}
	}
	else
	{
		strcpy(Packet->BigApp.TitleId, "N/A");
	}


	GetConsoleName(Packet->ConsoleName, 100);
	ReadFlash(FLASH_MB_SERIAL, &Packet->MotherboardSerial, 14);
	ReadFlash(FLASH_SERIAL, &Packet->Serial, 10);
	ReadFlash(FLASH_MODEL, &Packet->Model, 14);
	getMacAddress(SCE_NET_IF_NAME_ETH0, Packet->MACAdressLAN, 18);
	getMacAddress(SCE_NET_IF_NAME_WLAN0, Packet->MACAdressWIFI, 18);
	ReadFlash(FLASH_UART_FLAG, &Packet->UART, 1);
	ReadFlash(FLASH_IDU_MODE, &Packet->IDUMode, 1);
	GetIDPS(Packet->IDPS);
	GetPSID(Packet->PSID);
	Packet->ConsoleType = GetConsoleType();

	// Debugging.
	//Packet->AttachedPid = Debug->CurrentPID;
	//Packet->Attached = Debug->IsDebugging;

	// User.
	sceUserServiceGetForegroundUser(&Packet->ForegroundAccountId);

	// Storage Stats.
	sceShellCoreUtilGetFreeSizeOfUserPartition(&Packet->FreeSpace, &Packet->TotalSpace);

	// Perf Stats. TODO: Move from toolbox
	/*Packet->ThreadCount = SystemMonitor::Thread_Count;
	Packet->AverageCPUUsage = SystemMonitor::Average_Usage;
	Packet->BusyCore = SystemMonitor::Busy_Core;
	memcpy(&Packet->Ram, &SystemMonitor::RAM, sizeof(MemoryInfo));
	memcpy(&Packet->VRam, &SystemMonitor::VRAM, sizeof(MemoryInfo));*/

	sceNetSend(sock, Packet.get(), sizeof(TargetInfoPacket), 0);
}

void Target::DoNotify(SceNetId sock)
{
	auto packet = Sockets::RecieveType<TargetNotifyPacket>(sock);

	if (!strcmp(packet->IconURI, ""))
		Notify(packet->Message);
	else
		NotifyCustom(packet->IconURI, packet->Message);

	Sockets::SendInt(sock, API_OK);
}

void Target::DoBuzzer(SceNetId sock)
{
	BuzzerType buzzerType = RingOnce;
	if (Sockets::RecvInt(sock, (int*)&buzzerType))
	{
		RingBuzzer(buzzerType);
	}
}

void Target::SetConsoleLED(SceNetId sock)
{
	ConsoleLEDColours ledColour = white;
	if (Sockets::RecvInt(sock, (int*)&ledColour))
	{
		SetConsoleLED(ledColour);
	}
}

void Target::SetSettings(SceNetId sock)
{
	//TODO: IPC here...
	/*auto Packet = new TargetSettingsPacket();

	sceNetRecv(Sock, Packet, sizeof(TargetSettingsPacket), 0);

	Debug_Feature::DebugTitleIdLabel::ShowLabels = Config::Data->Show_DebugTitleIdLabel = Packet->ShowDebugTitleIdLabel;
	Debug_Feature::DevkitPanel::ShowPanel = Config::Data->Show_DevkitPanel = Packet->ShowDevkitPanel;
	Debug_Feature::Custom_Content::Show_Debug_Settings = Config::Data->Show_Debug_Settings = Packet->ShowDebugSettings;
	Debug_Feature::Custom_Content::Show_App_Home = Config::Data->Show_App_Home = Packet->ShowAppHome;

	SendStatus(Sock, APIResults::API_OK);

	Config::SetSettingsNow = true;*/
}

void Target::ProcList(SceNetId sock)
{
	// Get the list of running processes.
	std::vector<kinfo_proc> processList;
	GetProcessList(processList);

	Sockets::SendInt(sock, processList.size());

	for (const auto& i : processList)
	{
		// Get the app info using the pid.
		SceAppInfo appInfo;
		sceKernelGetAppInfo(i.pid, &appInfo);

		// Build packet.
		auto packet = std::make_unique<ProcPacket>();
		packet->AppId = appInfo.AppId;
		packet->ProcessId = i.pid;
		strcpy(packet->Name, i.name);
		strcpy(packet->TitleId, appInfo.TitleId);

		// send packet.
		sceNetSend(sock, packet.get(), sizeof(ProcPacket), 0);
	}
}