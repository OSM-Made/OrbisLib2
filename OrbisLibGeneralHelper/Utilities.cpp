#include "Utilities.h"
#include <cstdarg>
#include <stdio.h>
#include <KernelExt.h>

#define KLOG_FMT "[OrbisHelper] %s"

void klog(const char* fmt, ...)
{
	char Buffer[0x200];
	char Buffer2[0x200];

	//Create full string from va list.
	va_list args;
	va_start(args, fmt);
	vsprintf(Buffer, fmt, args);
	va_end(args);

	sprintf(Buffer2, KLOG_FMT, Buffer);

	sceKernelDebugOutText(0, Buffer2);
}