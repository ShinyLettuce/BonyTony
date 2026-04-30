#include "ThreadPool.h"

ThreadPool::ThreadPool(const size_t aThreadCount)
{
    for (size_t i = 0; i < aThreadCount; ++i)
    {
        myThreads.emplace_back(&ThreadPool::Work, this);
    }
}

ThreadPool::~ThreadPool()
{
    {
        std::lock_guard lock{ myQueueMutex };
        myCancel = true;
    }

    myQueueVariable.notify_all();
}

void ThreadPool::Enqueue(const std::function<void()>& aTask)
{
    {
        std::lock_guard lock{ myQueueMutex };
        myTasks.push(aTask);
        ++myWorkingCount;
    }

    myQueueVariable.notify_one();
}

void ThreadPool::Await()
{
    std::unique_lock lock{ myQueueMutex };
    myQueueVariable.wait(lock, [this] { return myWorkingCount == 0; });
}

void ThreadPool::Work()
{
    for (;;)
    {
        std::unique_lock lock{ myQueueMutex };
        myQueueVariable.wait(lock, [this] { return myCancel || !myTasks.empty(); });

        if (myCancel)
        {
            return;
        }

        const std::function<void()> task = std::move(myTasks.front());
        myTasks.pop();

        lock.unlock();

        task();

        {
            std::lock_guard lock_guard{ myQueueMutex };
            --myWorkingCount;
            if (myWorkingCount == 0)
            {
                myQueueVariable.notify_all();
            }
        }
    }
}
