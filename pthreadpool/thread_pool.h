#pragma once

#include "task.h"

#include <pthread.h>

#include <iostream>
#include <queue>
#include <vector>
class CThreadPool
{
public:
    CThreadPool();
    ~CThreadPool();

    void run(int thread_number = 10);
    void stop();
    void add_task(CTask* task);
private:

    bool is_stop() const;
    CTask *get_task();
    static void* process_thread(void *arg);

private:
    std::vector<pthread_t> m_thread_ids;
    std::queue<CTask*> m_tasks;

    pthread_mutex_t m_thread_mutex;
    pthread_cond_t m_thread_cond;

    bool m_done;
};
