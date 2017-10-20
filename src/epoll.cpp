#include "../inc/epoll.h"

CEpoll::CEpoll() :
    m_fd_number(0)
{
   
}

CEpoll::~CEpoll()
{
    if(m_epollfd)
        close_epoll();
}

bool CEpoll::create_epoll()
{
    m_epollfd = epoll_create(MAX_EPOLL_NUMBER);
    if(m_epollfd < 0)
        return false;
    else
        return true;
}

bool CEpoll::close_epoll()
{
    int ret = close(m_epollfd);
    m_epollfd = -1;
    if(ret < 0)
        return false;
    else
        return true;
}

bool CEpoll::add_event(int fd, unsigned int events)
{
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = events;

    if(epoll_ctl(m_epollfd, EPOLL_CTL_ADD, fd, &ev) < 0)
        return false;
    else
    {
        ++m_fd_number;
        return true;
    }
}

bool CEpoll::modify_event(int fd, unsigned int events)
{
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = events;

    if(epoll_ctl(m_epollfd, EPOLL_CTL_MOD, fd, &ev) < 0)
        return false;
    else
        return true;
}

bool CEpoll::delete_event(int fd, unsigned int events)
{
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = events;

    if(epoll_ctl(m_epollfd, EPOLL_CTL_DEL, fd, &ev) < 0)
        return false;
    else
    {
        --m_fd_number;
        return true;
    }
}

int CEpoll::epoll_wait(int timeout)
{
    return ::epoll_wait(m_epollfd, m_epoll_events, m_fd_number, timeout);
}

int CEpoll::get_fd(int idx)
{
    return m_epoll_events[idx].data.fd;
}

int CEpoll::get_events(int idx)
{
    return m_epoll_events[idx].events;
}
