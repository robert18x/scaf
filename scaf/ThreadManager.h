#pragma once
#include <concepts>
#include <BS_thread_pool.hpp>

class ThreadManager {
public:
    void add(std::invocable auto task) {
        pool.detach_task(task);
    }

    void wait(){
        pool.wait();
    }

private:
    BS::thread_pool pool;
};
