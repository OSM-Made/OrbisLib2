#pragma once

bool LoadModules();
void klog(const char* fmt, ...);
void Notify(const char* MessageFMT, ...);
void NotifyCustom(const char* IconURI, const char* MessageFMT, ...);
bool LoadSymbol(SceKernelModule handle, const char* symbol, void** funcOut);
void ExitGraceful();
bool CopySflash();