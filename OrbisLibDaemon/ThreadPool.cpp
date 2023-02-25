#include "stdafx.h"
#include "ThreadPool.h"

bool ThreadPool::ShouldRun;
std::mutex ThreadPool::JobQueueMtx;
std::condition_variable ThreadPool::MtxCondition;
std::vector<ScePthread> ThreadPool::ThreadsPool;
std::queue<std::function<void()>> ThreadPool::JobQueue;

void ThreadPool::WorkingLoop()
{
	while (true)
	{
		try
		{
			std::function<void()> job;
			{
				std::unique_lock<std::mutex> lock(JobQueueMtx);
				MtxCondition.wait(lock,
					[]
					{
						return !JobQueue.empty() || !ShouldRun;
					});

				if (!ShouldRun)
					return;

				job = JobQueue.front();
				JobQueue.pop();
			}

			job();
		}
		catch (const std::exception& ex)
		{
			klog("Std Error: %s\n", ex.what());
		}
		catch (...)
		{
			klog("Other Uknown Error Occured in Worker Thread.\n");
		}
	}
}

void ThreadPool::Init(int poolSize)
{
	ShouldRun = true;
	ThreadsPool.resize(poolSize);
	for (int i = 0; i < poolSize; i++)
	{
		char threadName[0x200];
		snprintf(threadName, sizeof(threadName), "WorkerThread%i", i);
		scePthreadCreate(&ThreadsPool.at(i), nullptr, [](void*) -> void*
			{
				ThreadPool::WorkingLoop();

				// Clean up the thread.
				scePthreadExit(nullptr);
				return nullptr;
			}, nullptr, threadName);

		scePthreadSetaffinity(ThreadsPool.at(i), SCE_KERNEL_CPUMASK_7CPU_ALL);
	}
}

void ThreadPool::Term()
{
	{
		std::unique_lock<std::mutex> lock(JobQueueMtx);
		ShouldRun = false;
	}

	MtxCondition.notify_all();

	for (auto& activeThread : ThreadsPool)
	{
		scePthreadJoin(activeThread, nullptr);
	}

	ThreadsPool.clear();
}

void ThreadPool::QueueJob(const std::function<void()>& job)
{
	{
		std::unique_lock<std::mutex> lock(JobQueueMtx);
		JobQueue.push(job);
	}
	MtxCondition.notify_one();
}