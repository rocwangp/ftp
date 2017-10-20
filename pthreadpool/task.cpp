#include "task.h"

void(*CTask::process_task)(std::vector<void*>);

CTask::CTask(void (*p_task)(std::vector<void*>), std::vector<void*> args):
    m_args(args)
{
    process_task = p_task;
}

CTask::~CTask()
{
}

void CTask::run()
{
     process_task(m_args);
}
