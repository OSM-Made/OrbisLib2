#pragma once

class ProcessMonitor
{
public:
	ProcessMonitor(int pid);
	~ProcessMonitor();

	std::function<void()> OnExit;

private:
	bool ShouldRun;

	void WatchThread(int pid);
};
