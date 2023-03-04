#pragma once

#define	MNT_UPDATE	0x00010000
#define ARRAY_COUNT(arry) sizeof(arry) / sizeof(arry[0])

bool LoadModules();
bool Jailbreak();
void klog(const char* fmt, ...);
void Notify(const char* MessageFMT, ...);
void ExitGraceful();

void build_iovec(iovec** iov, int* iovlen, const char* name, const void* val, size_t len);
int nmount(struct iovec* iov, uint32_t niov, int flags);
int unmount(const char* dir, int flags);
int mount_large_fs(const char* device, const char* mountpoint, const char* fstype, const char* mode, unsigned int flags);
void CopyFile(const char* File, const char* Destination);
int MakeDir(const char* Dir, ...);

void InstallDaemon(const char* Daemon, const char* libs[], int libCount);
void InstallOrbisToolbox();
void InstallOrbisSuite();