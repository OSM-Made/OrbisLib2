#pragma once
#include <cstdint>

extern "C"
{
	int syscall(int num, ...);
	uint64_t kernelRdmsr(int Register);
	void cpu_enable_wp();
	void cpu_disable_wp();
}