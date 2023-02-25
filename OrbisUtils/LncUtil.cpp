#include <cstdint>
#include <orbis/LncUtil.h>
#include "LncUtil.h"

int IsAppLaunched(const char* titleId, bool* isLaunched)
{
	return sceLncUtilIsAppLaunched(titleId, isLaunched);
}

int IsAppSuspended(int appId, bool* isSuspended)
{
	return sceLncUtilIsAppSuspended(appId, isSuspended);
}

int GetAppId(const char* titleId)
{
	return sceLncUtilGetAppId(titleId);
}

int LaunchApp(const char* titleId, char** args, LaunchAppParam* appParam)
{
	return sceLncUtilLaunchApp(titleId, args, appParam);
}

int SuspendApp(int appId, int flags)
{
	return sceLncUtilSuspendApp(appId, flags);
}

int ResumeApp(int appId, int flags)
{
	return sceLncUtilResumeApp(appId, flags);
}

int SetAppFocus(int appId, int flags)
{
	return sceLncUtilSetAppFocus(appId, flags);
}
