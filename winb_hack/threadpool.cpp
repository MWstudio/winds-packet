#include "stdafx.h"
#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <memory>
#include <atomic>
#include <future>
#include <sstream> // �߰���: std::ostringstream�� ���� �ʿ�
class ThreadPool {
public:
    ThreadPool(size_t numThreads) {
        for (size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back([this]() {
                while (true) {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(queueMutex);
                        condition.wait(lock, [this]() { return stop || !tasks.empty(); });

                        if (stop && tasks.empty())
                            return;

                        task = std::move(tasks.front());
                        tasks.pop();
                    }

                    printf("Worker thread %s is processing a task.\n", getThreadId().c_str());
                    task();
                    printf("Worker thread %s finished processing a task.\n", getThreadId().c_str());
                }
                });
        }
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread& worker : workers)
            worker.join();
    }

    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
        printf("enqueue");
        using returnType = typename std::result_of<F(Args...)>::type;

        auto task = std::make_shared<std::packaged_task<returnType()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<returnType> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queueMutex);

            if (stop)
                throw std::runtime_error("enqueue on stopped ThreadPool");

            tasks.emplace([task]() { (*task)(); });
        }
        condition.notify_one();
        return res;
    }

private:
    std::vector<std::thread> workers; // ������ ���
    std::queue<std::function<void()>> tasks; // �۾� ť

    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop = false;

    static std::string getThreadId() {
        std::ostringstream oss;
        oss << std::this_thread::get_id();
        return oss.str();
    }
};