#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

class ThreadPool
{
public:
    ThreadPool(const size_t aThreadCount);
    ~ThreadPool();

    void Enqueue(const std::function<void()>& aTask);
    void Await();
private:
    void Work();

    std::vector<std::jthread> myThreads;

    std::queue<std::function<void()>> myTasks;

    std::mutex myQueueMutex;
    std::condition_variable myQueueVariable;

    int myWorkingCount{ 0 };
    bool myCancel{ false };
};