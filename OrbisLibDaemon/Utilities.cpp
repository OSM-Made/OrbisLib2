#include "stdafx.h"
#include <GoldHEN.h>
#include <NetExt.h>
#include "Utilities.h"
#include <KernelInterface.h>

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
	
	// Start up networking interface
	res = sceNetInit();
	if (res != 0)
	{
		klog("LoadModules(): sceNetInit failed\n");
		return false;
	}

	// Start up user service.
	SceUserServiceInitializeParams userParam = { SCE_KERNEL_PRIO_FIFO_HIGHEST };
	res = sceUserServiceInitialize(&userParam);
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

int getMacAddress(int ifName_Num, char* strOut, size_t len)
{
	if (len < 18)
	{
		klog("getMacAddress(): Output len must be >= 18.\n");
		return -1;
	}

	SceNetIfEntry ifEntry;
	auto res = sceNetGetIfList((SceNetIfName)ifName_Num, &ifEntry, 1);
	if (res < 0)
	{
		klog("getMacAddress(): failed to get IfList for %i\n", ifName_Num);
		return res;
	}

	return sceNetEtherNtostr((SceNetEtherAddr*)ifEntry.MacAddress, strOut, len);
}

int GetProcessList(std::vector<kinfo_proc>& ProcessList)
{
	size_t length;

	static int name[] = { CTL_KERN, KERN_PROC, KERN_PROC_PROC, 0 };

	// Get the size of buffer needed.
	if (sysctl(name, 3, nullptr, &length, nullptr, 0) < 0)
		return -1;

	// Resize our vector to accommodate.
	try
	{
		ProcessList.resize(length / sizeof(kinfo_proc));
	}
	catch (const std::bad_alloc&)
	{
		return -1;
	}

	// Retrive the processes.
	if (sysctl(name, 3, ProcessList.data(), &length, nullptr, 0) < 0)
		return -1;

	// Fix names.
	for (auto& proc : ProcessList)
	{
		sceKernelGetProcessName(proc.pid, (char*)proc.name);
	}

	return 0;
}

static void build_iovec(iovec** iov, int* iovlen, const char* name, const void* val, size_t len) {
	int i;

	if (*iovlen < 0)
		return;

	i = *iovlen;
	*iov = (iovec*)realloc(*iov, sizeof * *iov * (i + 2));
	if (*iov == NULL) {
		*iovlen = -1;
		return;
	}

	(*iov)[i].iov_base = strdup(name);
	(*iov)[i].iov_len = strlen(name) + 1;
	++i;

	(*iov)[i].iov_base = (void*)val;
	if (len == (size_t)-1) {
		if (val != NULL)
			len = strlen((const char*)val) + 1;
		else
			len = 0;
	}
	(*iov)[i].iov_len = (int)len;

	*iovlen = ++i;
}

int nmount(struct iovec* iov, unsigned int niov, int flags)
{
	return syscall(378, iov, niov, flags);
}

bool LinkDir(const char* Dir, const char* LinkedDir)
{
	auto res = sceKernelMkdir(LinkedDir, 0777);
	if (res != 0 && res != 0x80020011)
	{
		klog("Failed to make dir '%s' err: %llX\n", LinkedDir, res);
		return false;
	}

	struct iovec* iov = NULL;
	int iovlen = 0;

	build_iovec(&iov, &iovlen, "fstype", "nullfs", -1);
	build_iovec(&iov, &iovlen, "fspath", LinkedDir, -1);
	build_iovec(&iov, &iovlen, "target", Dir, -1);

	if (nmount(iov, iovlen, 0))
	{
		klog("nmount failed\n");
		sceKernelRmdir(LinkedDir);
		return false;
	}

	return true;
}

bool LoadToolbox()
{
	// Mount data & hostapp into ShellUI sandbox
	LinkDir("/data/", "/mnt/sandbox/NPXS20001_000/data");
	LinkDir("/hostapp/", "/mnt/sandbox/NPXS20001_000/hostapp");

	auto handle = sys_sdk_proc_prx_load((char*)"SceShellUI", (char*)"/user/data/Orbis Toolbox/OrbisToolbox-2.0.sprx");
	if (handle > 0) {
		klog("Orbis Toolbox loaded! %d\n", handle);
		return true;
	}
	else
	{
		klog("error: %d\n", handle);
		Notify("Failed to load Orbis Toolbox!");
		return false;
	}
}

bool LoadProcHelper(int pid)
{
	// Get the proc name.
	char processName[32];
	sceKernelGetProcessName(pid, processName);

	// Get the library list.
	OrbisLiraryInfo libraries[256];
	int actualCount = GetLibraries(pid, &libraries[0], 256);

	// Unload if it is already loaded.
	for (int i = 0; i < actualCount; i++)
	{
		if (strstr(libraries[i].Path, "OrbisLibGeneralHelper"))
		{
			sys_sdk_proc_prx_unload(processName, libraries[i].Handle);
			break;
		}
	}

	// Load the helper library.
	int handle = sys_sdk_proc_prx_load(processName, (char*)HelperPrxPath);
	
	if (handle < 0)
	{
		klog("Failed to load Helper PRX! %llX\n", handle);
		return false;
	}

	klog("Helper PRX has loaded with the handle of %d\n", handle);
	return true;
}