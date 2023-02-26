#include "stdafx.h"
#include "Monitor.h"
#include <SystemStateMgr.h>

void Monitor::Run()
{
	while (true)
	{
		switch ((SystemState)sceSystemStateMgrGetCurrentState())
		{
		default:
			break;

			case SHUTDOWN_ON_GOING:
				klog("Console is shutting down! API exiting...\n");
				return;

			case POWER_SAVING:
			case SUSPEND_ON_GOING:
			case MAIN_ON_STANDBY:
				klog("Going to sleep!\n");
				break;

			case INITIALIZING:
				klog("Waking up!\n");
				break;
		}

		// Sleep for 5s before we check our state again.
		sceKernelSleep(5);
	}
}