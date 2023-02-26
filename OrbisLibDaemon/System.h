#pragma once

enum NewSystemState
{
	Suspend = 1,
	Shutdown = 2,
	Reboot = 3,
};

enum ConsoleLEDColours
{
	white,
	white_Blinking,
	Blue_Blinking,
};

enum ConsoleTypes
{
	CT_UNK,
	CT_DIAG,	//0x80
	CT_DEVKIT,	//0x81
	CT_TESTKIT, //0x82
	CT_RETAIL,	//0x83 -> 0x8F
	CT_KRATOS,	//0xA0 IMPOSSIBLE??
};

int ChangeSystemState(NewSystemState State);
void SetConsoleLED(ConsoleLEDColours Colour);
void SetControllerLED();
void RingBuzzer(BuzzerType Type);
int32_t GetCPUTemp();
int32_t GetSOCTemp();
int GetSDKVersion();
int GetUpdateVersion();
bool GetConsoleName(char* Out, size_t len);
int GetIDPS(char* Out);
int GetPSID(char* Out);
ConsoleTypes GetConsoleType();