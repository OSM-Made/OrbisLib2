#include "stdafx.h"
#include <GoldHEN.h>
#include <NetExt.h>
#include "Utilities.h"

bool LoadModules()
{
	auto res = sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_SYSTEM_SERVICE);
	if (res != 0)
	{
		klog("LoadModules(): Failed to load SCE_SYSMODULE_INTERNAL_SYSTEM_SERVICE (%llX)\n", res);
		return false;
	}

	res = sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_USER_SERVICE);
	if (res != 0)
	{
		klog("LoadModules(): Failed to load SCE_SYSMODULE_INTERNAL_USER_SERVICE (%llX)\n", res);
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

void ExitGraceful()
{
	SceAppInfo info;
	sceKernelGetAppInfo(getpid(), &info);

	sceLncUtilKillApp(info.AppId);
}

void build_iovec(iovec** iov, int* iovlen, const char* name, const void* val, size_t len) 
{
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

int nmount(struct iovec* iov, uint32_t niov, int flags)
{
	return syscall(378, iov, niov, flags);
}

int unmount(const char* dir, int flags)
{
	return syscall(22, dir, flags);
}

int mount_large_fs(const char* device, const char* mountpoint, const char* fstype, const char* mode, unsigned int flags)
{
	struct iovec* iov = NULL;
	int iovlen = 0;

	build_iovec(&iov, &iovlen, "fstype", fstype, -1);
	build_iovec(&iov, &iovlen, "fspath", mountpoint, -1);
	build_iovec(&iov, &iovlen, "from", device, -1);
	build_iovec(&iov, &iovlen, "large", "yes", -1);
	build_iovec(&iov, &iovlen, "timezone", "static", -1);
	build_iovec(&iov, &iovlen, "async", "", -1);
	build_iovec(&iov, &iovlen, "ignoreacl", "", -1);

	if (mode) {
		build_iovec(&iov, &iovlen, "dirmask", mode, -1);
		build_iovec(&iov, &iovlen, "mask", mode, -1);
	}

	return nmount(iov, iovlen, flags);
}

void CopyFile(const char* File, const char* Destination)
{
	int src = 0, dst = 0;
	SceKernelStat Stats;

	//Open file descriptors 
	src = sceKernelOpen(File, SCE_KERNEL_O_RDONLY, 0);
	if (src <= 0)
	{
		klog("Failed to open Source File.\n");
		return;
	}

	dst = sceKernelOpen(Destination, SCE_KERNEL_O_CREAT | SCE_KERNEL_O_WRONLY, 0777);
	if (dst <= 0)
	{
		klog("Failed to open Destination File.\n");
		return;
	}

	//Get File size
	sceKernelFstat(src, &Stats);

	if (Stats.st_size == 0)
	{
		klog("Failed to get file size.\n");
		return;
	}

	//Allocate space to read data.
	char* FileData = (char*)malloc(Stats.st_size);

	//ReadFile.
	sceKernelRead(src, FileData, Stats.st_size);

	//Write the file data.
	sceKernelWrite(dst, FileData, Stats.st_size);

	//Close The handles.
	sceKernelClose(src);
	sceKernelClose(dst);
}

int MakeDir(const char* Dir, ...)
{
	char buffer[0x400] = { 0 };
	va_list args;
	va_start(args, Dir);
	vsprintf(buffer, Dir, args);
	va_end(args);

	klog("Creating Directory \"%s\"...\n", buffer);
	return sceKernelMkdir(buffer, 0777);
}

void InstallDaemon(const char* Daemon, const char* libs[], int libCount)
{
	klog("Installing Daemoin %s...\n[%s] Making Directories...\n", Daemon, Daemon);
	MakeDir("/system/vsh/app/%s", Daemon);
	MakeDir("/system/vsh/app/%s/sce_sys", Daemon);

	klog("[%s] Writing Files...\n", Daemon);
	char EbootFromPath[0x100], EbootPath[0x100];
	sprintf(EbootFromPath, "%s%s/eboot.bin", DAEMONGETPATH, Daemon);
	sprintf(EbootPath, "%s%s/eboot.bin", DAEMONPATH, Daemon);
	CopyFile(EbootFromPath, EbootPath);

	char ParamFromPath[0x100], ParamPath[0x100];
	sprintf(ParamFromPath, "%s%s/sce_sys/param", DAEMONGETPATH, Daemon);
	sprintf(ParamPath, "%s%s/sce_sys/param.sfo", DAEMONPATH, Daemon);
	CopyFile(ParamFromPath, ParamPath);

	char IconFromPath[0x100], IconPath[0x100];
	sprintf(IconFromPath, "%s%s/sce_sys/icon0.png", DAEMONGETPATH, Daemon);
	sprintf(IconPath, "%s%s/sce_sys/icon0.png", DAEMONPATH, Daemon);
	CopyFile(IconFromPath, IconPath);

	for (int i = 0; i < libCount; i++)
	{
		char LibraryFromPath[0x100], LibraryPath[0x100];
		sprintf(LibraryFromPath, "%s%s/%s.sprx", DAEMONGETPATH, Daemon, libs[i]);
		sprintf(LibraryPath, "%s%s/%s.sprx", DAEMONPATH, Daemon, libs[i]);
		CopyFile(LibraryFromPath, LibraryPath);
	}

	klog("[%s] Installation Success!\n", Daemon);
}

void InstallOrbisToolbox()
{
	const char* Icons[] = { "icon_daemon.png", "icon_payload.png", "icon_pkg.png", "icon_plugin.png", "icon_reboot.png", "icon_reload_ui.png", "icon_shutdown.png", "icon_suspend.png", "icon_system_settings.png", "icon_toolbox.png" };

	klog("[Orbis Toolbox] Making Directories...\n");
	MakeDir("/data/Orbis Toolbox");
	MakeDir("/data/Orbis Toolbox/Plugins");
	MakeDir("/data/Orbis Toolbox/Icons");
	MakeDir("/data/Orbis Toolbox/Payloads");

	klog("[Orbis Toolbox] Writing Files...\n");
	CopyFile("/mnt/sandbox/ORBS00000_000/app0/Orbis Toolbox/OrbisToolbox-2.0.sprx", "/data/Orbis Toolbox/OrbisToolbox-2.0.sprx");
	for (int i = 0; i < 10; i++)
	{
		char IconFromPath[0x100], IconPath[0x100];
		sprintf(IconFromPath, ICONGETPATH, Icons[i]);
		sprintf(IconPath, ICONPATH, Icons[i]);
		CopyFile(IconFromPath, IconPath);
	}

	klog("[Orbis Toolbox] Installation Success!\n");
}

void InstallOrbisSuite()
{
	klog("[Orbis Suite] Making Directories...\n");
	MakeDir("/data/Orbis Suite");
	MakeDir("/data/Orbis Suite/IPC");

	klog("[Orbis Suite] Writing Files...\n");
	CopyFile("/mnt/sandbox/ORBS00000_000/app0/Orbis Suite/OrbisLibGeneralHelper.sprx", "/data/Orbis Suite/OrbisLibGeneralHelper.sprx");
}