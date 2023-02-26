#include "stdafx.h"
#include "System.h"
#include <SystemStateMgr.h>

int ChangeSystemState(NewSystemState state)
{
	int ret = 0;

	switch (state)
	{
	case Suspend:
		ret = sceSystemStateMgrEnterStandby();
		break;

	case Shutdown:
		ret = sceSystemStateMgrTurnOff();
		break;

	case Reboot:
		ret = sceSystemStateMgrReboot();
		break;
	}

	return ret;
}

void SetConsoleLED(ConsoleLEDColours Colour)
{
	switch (Colour)
	{
	default:
	case white:
		sceKernelIccIndicatorBootDone();
		break;

	case white_Blinking:
		sceKernelIccIndicatorShutdown();
		break;

	case Blue_Blinking:
		sceKernelIccIndicatorStandbyBoot();
		break;
	}
}

void SetControllerLED()
{

}

void RingBuzzer(BuzzerType Type)
{
	if (Type < 6)
		sceKernelIccSetBuzzer(Type);
}

int32_t GetCPUTemp()
{
	int32_t CPUTemp = 0;
	if (sceKernelGetCpuTemperature(&CPUTemp) == 0)
		return CPUTemp;
	else
		return -1;
}

int32_t GetSOCTemp()
{
	int32_t SOCTemp = 0;
	if (sceKernelGetSocSensorTemperature(0, &SOCTemp) == 0)
		return SOCTemp;
	else
		return -1;
}

int GetSDKVersion()
{
	int sdk_version = 0;
	size_t sdk_versionlen = 4;

	sysctlbyname("kern.sdk_version", (char*)&sdk_version, &sdk_versionlen, nullptr, 0);

	return sdk_version;
}

int GetUpdateVersion()
{
	int sdk_version = 0;
	size_t sdk_versionlen = 4;

	sysctlbyname("machdep.upd_version", (char*)&sdk_version, &sdk_versionlen, nullptr, 0);

	return sdk_version;
}

bool GetConsoleName(char* Out, size_t len)
{
	return sceSystemServiceParamGetString(SCE_SYSTEM_SERVICE_PARAM_ID_SYSTEM_NAME, Out, len) == 0;
}

int GetIDPS(char* Out)
{
	size_t IDPSlen = 16;
	return sysctlbyname("machdep.idps", Out, &IDPSlen, nullptr, 0);
}

int GetPSID(char* Out)
{
	size_t PSID = 16;
	return sysctlbyname("machdep.openpsid", Out, &PSID, nullptr, 0);
}

ConsoleTypes GetConsoleType()
{
	char IDPS[16] = { 0 };
	size_t IDPSlen = 16;
	sysctlbyname("machdep.idps", (char*)&IDPS, &IDPSlen, nullptr, 0);

	switch (IDPS[5] & 0xffU)
	{
	case 0x80:
		return CT_DIAG;
		break;

	case 0x81:
		return CT_DEVKIT;
		break;

	case 0x82:
		return CT_TESTKIT;
		break;

	case 0x83 ... 0x8F:
		return CT_RETAIL;
		break;

	case 0xA0:
		return CT_KRATOS;
		break;
	}

	return CT_UNK;
}