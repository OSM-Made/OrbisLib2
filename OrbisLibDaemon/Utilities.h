#pragma once

struct kinfo_proc {
	int structSize;				//0x00
	int layout;					//0x04
	void* args;					//0x08
	void* paddr;				//0x10
	void* addr;					//0x18
	void* tracep;				//0x20
	void* textvp;				//0x28
	void* fd;					//0x30
	void* vmspace;				//0x38
	void* wchan;				//0x40
	int pid;					//0x48
	char padding0[0x173];		//0x4C
	char name[0x20];			//0x1BF
	char padding1[0x268];		//0x1DF
}; // Size = 0x448

#define CTL_KERN 1
#define KERN_PROC 14
#define KERN_PROC_ALL 0
#define KERN_PROC_PROC 8
#define KERN_PROC_PID 1

bool LoadModules();
bool Jailbreak();
void klog(const char* fmt, ...);
void Notify(const char* MessageFMT, ...);
void NotifyCustom(const char* IconURI, const char* MessageFMT, ...);
bool LoadSymbol(SceKernelModule handle, const char* symbol, void** funcOut);
void ExitGraceful();
bool CopySflash();
int getMacAddress(int ifName_Num, char* strOut, size_t strlen);
int GetProcessList(std::vector<kinfo_proc>& ProcessList);
bool LinkDir(const char* Dir, const char* LinkedDir);

bool LoadToolbox();
bool LoadProcHelper(int pid);