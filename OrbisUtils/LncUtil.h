#pragma once

extern "C"
{
	int IsAppLaunched(const char* titleId, bool* isLaunched);
	int IsAppSuspended(int appId, bool* isSuspended);
	int GetAppId(const char* titleId);
	int LaunchApp(const char* titleId, char** args, LaunchAppParam* appParam);
	int SuspendApp(int appId, int flags);
	int ResumeApp(int appId, int flags);
	int SetAppFocus(int appId, int flags);
}
