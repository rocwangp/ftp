#pragma once

#include "constant.h"

#include <unistd.h>

#include <sys/epoll.h>
#include <sys/types.h>

class CEpoll
{
public:
    CEpoll();
    ~CEpoll();

    bool add_event(int fd,  unsigned int events);
    bool modify_event(int fd, unsigned int events);
    bool delete_event(int fd, unsigned int events);

    int epoll_wait(int timeout);

    int get_fd(int idx);
    int get_events(int idx);
    bool create_epoll();
    bool close_epoll();
private:
    int m_fd_number;
    int m_epollfd;
    struct epoll_event m_epoll_events[MAX_EPOLL_NUMBER];
};
