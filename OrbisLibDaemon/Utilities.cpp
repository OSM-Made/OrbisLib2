#include "stdafx.h"
#include "GoldHEN.h"
#include "Utilities.h"

bool LoadModules()
{
	auto res = sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_SYSTEM_SERVICE);
	if (res != 0)
	{
		klog("LoadModules(): Failed to load SCE_SYSMODULE_INTERNAL_SYSTEM_SERVICE (%llX)\n", res);
		return false;
	}
	
	res = sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_APPINSTUTIL);
	if (res != 0)
	{
		klog("LoadModules(): Failed to load SCE_SYSMODULE_INTERNAL_APPINSTUTIL (%llX)\n", res);
		return false;
	}
	
	res = sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_USER_SERVICE);
	if (res != 0)
	{
		klog("LoadModules(): Failed to load SCE_SYSMODULE_INTERNAL_USER_SERVICE (%llX)\n", res);
		return false;
	}
	
	res = sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_SYS_CORE);
	if (res != 0)
	{
		klog("LoadModules(): Failed to load SCE_SYSMODULE_INTERNAL_SYS_CORE (%llX)\n", res);
		return false;
	}
	
	res = sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_NETCTL);
	if (res != 0)
	{
		klog("LoadModules(): Failed to load SCE_SYSMODULE_INTERNAL_NETCTL (%llX)\n", res);
		return false;
	}
	
	res = sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_NET);
	if (res != 0)
	{
		klog("LoadModules(): Failed to load SCE_SYSMODULE_INTERNAL_NET (%llX)\n", res);
		return false;
	}
	
	res = sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_HTTP);
	if (res != 0)
	{
		klog("LoadModules(): Failed to load SCE_SYSMODULE_INTERNAL_HTTP (%llX)\n", res);
		return false;
	}
	
	res = sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_BGFT);
	if (res != 0)
	{
		klog("LoadModules(): Failed to load SCE_SYSMODULE_INTERNAL_BGFT (%llX)\n", res);
		return false;
	}
	
	sceSysmoduleLoadModuleInternal(0x8000000D);

	// Start up networking interface
	res = sceNetInit();
	if (res != 0)
	{
		klog("LoadModules(): sceNetInit failed\n");
		return false;
	}

	// Start up user service.
	res = sceUserServiceInitialize(nullptr);
	if (res != 0)
	{
		klog("LoadModules(): sceUserServiceInitialize failed (%llX)\n", res);
		return false;
	}

	// Init temporary wrapper for lncutils.
	res = sceLncUtilInitialize();
	if (res != 0)
	{
		klog("LoadModules(): sceLncUtilInitialize failed (%llX)\n", res);
		return false;
	}

	// Init SysCoreUtils.
	res = sceApplicationInitialize();
	if (res != 0)
	{
		klog("LoadModules(): sceApplicationInitialize failed (%llX)\n", res);
		return false;
	}

	// Init App install utils.
	res = sceAppInstUtilInitialize();
	if (res != 0)
	{
		klog("LoadModules(): sceAppInstUtilInitialize failed (%llX)\n", res);
		return false;
	}

	klog("LoadModules(): Success!\n");
	return true;
}

bool Jailbreak()
{
	// Load the prx.
	sceKernelLoadStartModule("/app0/libGoldHEN.sprx", 0, 0, 0, 0, 0);

	// Jailbreak the prx.
	jailbreak_backup bk;
	return (sys_sdk_jailbreak(&bk) == 0);
}

void klog(const char* fmt, ...)
{
	char Buffer[0x200];

	//Create full string from va list.
	va_list args;
	va_start(args, fmt);
	vsprintf(Buffer, fmt, args);
	va_end(args);

	sceKernelDebugOutText(0, Buffer);
}

void Notify(const char* MessageFMT, ...)
{
	SceNotificationRequest Buffer;

	//Create full string from va list.
	va_list args;
	va_start(args, MessageFMT);
	vsprintf(Buffer.message, MessageFMT, args);
	va_end(args);

	//Populate the notify buffer.
	Buffer.type = SceNotificationRequestType::NotificationRequest; //this one is just a standard one and will print what ever is stored at the buffer.Message.
	Buffer.unk3 = 0;
	Buffer.useIconImageUri = 1; //Bool to use a custom uri.
	Buffer.targetId = -1; //Not sure if name is correct but is always set to -1.
	strcpy(Buffer.iconUri, "https://i.imgur.com/SJPIBGG.png"); //Copy the uri to the buffer.

	//From user land we can call int64_t sceKernelSendNotificationRequest(int64_t unk1, char* Buffer, size_t size, int64_t unk2) which is a libkernel import.
	sceKernelSendNotificationRequest(0, &Buffer, 3120, 0);
}

void NotifyCustom(const char* IconURI, const char* MessageFMT, ...)
{
	SceNotificationRequest Buffer;

	//Create full string from va list.
	va_list args;
	va_start(args, MessageFMT);
	vsprintf(Buffer.message, MessageFMT, args);
	va_end(args);

	//Populate the notify buffer.
	Buffer.type = SceNotificationRequestType::NotificationRequest; //this one is just a standard one and will print what ever is stored at the buffer.Message.
	Buffer.unk3 = 0;
	Buffer.useIconImageUri = 1; //Bool to use a custom uri.
	Buffer.targetId = -1; //Not sure if name is correct but is always set to -1.
	strcpy(Buffer.iconUri, IconURI); //Copy the uri to the buffer.

	//From user land we can call int64_t sceKernelSendNotificationRequest(int64_t unk1, char* Buffer, size_t size, int64_t unk2) which is a libkernel import.
	sceKernelSendNotificationRequest(0, &Buffer, 3120, 0);
}

bool LoadSymbol(SceKernelModule handle, const char* symbol, void** funcOut)
{
	if (sceKernelDlsym(handle, symbol, funcOut) != 0 || *funcOut == 0)
	{
		klog("Failed to load %s.\n", symbol);
		return false;
	}

	return true;
}

void ExitGraceful()
{
	SceAppInfo info;
	sceKernelGetAppInfo(getpid(), &info);

	sceLncUtilKillApp(info.AppId);
}

bool CopySflash()
{
	int sflashFd = sceKernelOpen("/dev/sflash0", SCE_KERNEL_O_RDONLY, 0);
	int backupFd = sceKernelOpen("/data/Orbis Suite/sflash0", SCE_KERNEL_O_CREAT | SCE_KERNEL_O_WRONLY | SCE_KERNEL_O_APPEND, 0777);
	if (sflashFd && backupFd)
	{
		auto buffer = (unsigned char*)malloc(4 * 1024 * 1024);
		if (buffer == nullptr)
		{
			klog("failled to allocate memory for sflash read.\n");
			return false;
		}

		size_t bytesRead = 0;
		while ((bytesRead = sceKernelRead(sflashFd, buffer, 4 * 1024 * 1024)) > 0)
		{
			sceKernelWrite(backupFd, buffer, bytesRead);
		}

		free(buffer);
		sceKernelClose(sflashFd);
		sceKernelClose(backupFd);
		return true;
	}

	return false;
}