#include "thread_pool.h"

CThreadPool::CThreadPool() :
    m_done(false)
{
    pthread_mutex_init(&m_thread_mutex, NULL);
    pthread_cond_init(&m_thread_cond, NULL);
}

CThreadPool::~CThreadPool()
{
    pthread_mutex_destroy(&m_thread_mutex);
    pthread_cond_destroy(&m_thread_cond);
    for(int thread_id : m_thread_ids)
    {
        pthread_cancel(thread_id);
    }
}


void CThreadPool::run(int thread_number)
{
 
    while(thread_number--)
    {
        pthread_t tid;
        pthread_create(&tid, NULL, process_thread, static_cast<void*>(this));
        pthread_detach(tid);
        m_thread_ids.push_back(tid);

    }
}


void* CThreadPool::process_thread(void *arg)
{
    CThreadPool* thread_pool = static_cast<CThreadPool*>(arg);
    while(!thread_pool->is_stop())
    {
        pthread_testcancel();

        pthread_mutex_lock(&thread_pool->m_thread_mutex);
        while(thread_pool->m_tasks.size() == 0)
        {
            pthread_cond_wait(&thread_pool->m_thread_cond, &thread_pool->m_thread_mutex);
            if(thread_pool->is_stop())
            {
                pthread_mutex_unlock(&thread_pool->m_thread_mutex);
                pthread_exit(NULL);
                return NULL;
            }
        }

        CTask* task = thread_pool->get_task();
        pthread_mutex_unlock(&thread_pool->m_thread_mutex);
        task->run();
        delete task;
    }

    pthread_exit(NULL);
}

bool CThreadPool::is_stop() const
{
    return m_done == true;
}

void CThreadPool::stop()
{
    m_done = true;
    pthread_cond_broadcast(&m_thread_cond);
}

void CThreadPool::add_task(CTask* task)
{
    pthread_mutex_lock(&m_thread_mutex);
    bool is_signal = false;
    if(m_tasks.size() == 0)
        is_signal = true;
    m_tasks.push(task);
   
    if(is_signal)
        pthread_cond_signal(&m_thread_cond);
    pthread_mutex_unlock(&m_thread_mutex);
}

CTask* CThreadPool::get_task()
{
    CTask* task = m_tasks.front();
    m_tasks.pop();
    return task;
}

