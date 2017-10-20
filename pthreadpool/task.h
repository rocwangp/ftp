#pragma once

#include <iostream>
#include <cstring>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <vector>
class CTask
{
public:
    CTask(void(*p_task)(std::vector<void*>), std::vector<void*> args);
    ~CTask();

    void run();
private:
    static void(*process_task)(std::vector<void*>);

    std::vector<void*> m_args;
};
