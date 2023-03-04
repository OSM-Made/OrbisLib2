#pragma once

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-security"

template<typename ... Args>
void klog(const char* fmt, Args ... args)
{
	int strSize = snprintf(nullptr, 0, fmt, args ...) + 1;
	if (strSize <= 0)
	{
		return;
	}

	auto buffer = std::make_unique<char[]>(strSize);
	snprintf(buffer.get(), strSize, fmt, args ...);
	sceKernelDebugOutText(0, buffer.get());
}

#pragma clang diagnostic pop

