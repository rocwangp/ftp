#include "../inc/socket.h"
#include <iostream>

CSocket::CSocket(int fd) :
    m_sockfd(fd)
{

}

CSocket::~CSocket()
{
    if(m_sockfd)
        close_socket();
}

bool CSocket::create_socket()
{
    m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(m_sockfd < 0)
        return false;
    else
        return true;
}

bool CSocket::close_socket()
{
    int close_ret = 0;
    if(m_sockfd)
    {
        close_ret = close(m_sockfd);
    }
    m_sockfd = -1;
    if(close_ret < 0)
        return false;
    else 
        return true;
}


bool CSocket::bind_socket(const std::string& ip, int port)
{
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &servaddr.sin_addr);
    if(bind(m_sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
        return false;
    else
        return true;
}


bool CSocket::listen_socket(int listen_number)
{
    if(listen(m_sockfd, listen_number) < 0)
        return false;
    else
        return true;
}

int CSocket::accept_socket()
{
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    socklen_t len = sizeof(addr);

    return accept(m_sockfd, (struct sockaddr*)&addr, &len);
}

bool CSocket::connect_socket(const std::string& ip, int port)
{
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &servaddr.sin_addr);

    if(connect(m_sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
        return false;
    else
        return true;
}


bool CSocket::set_socket_nonblocking()
{
    int flag = fcntl(m_sockfd, F_GETFL);
    flag |= O_NONBLOCK;
    if(fcntl(m_sockfd, F_SETFL, flag) < 0)
        return false;
    else
        return true;
}

bool CSocket::set_socket_blocking()
{
    int flag = fcntl(m_sockfd, F_GETFL);
    flag &= (~O_NONBLOCK);
    if(fcntl(m_sockfd, F_SETFL, flag) < 0)
        return false;
    else
        return true;
}


int CSocket::recv_message(std::string& message)
{
    char msg[FTP_DEFAULT_BUFFER];
    bzero(msg, sizeof(msg));
    
    int n = recv(m_sockfd, msg, sizeof(msg), 0);
    if(n < 0)
    { 
        return -1;
    }
    else if(n == 0)
    {
       return 0;
    }
    else
    {
        message.assign(msg, n); 

        return n;
    }
}

int CSocket::send_message(const std::string& message)
{
    return send(m_sockfd, message.c_str(), message.size(), MSG_NOSIGNAL);
}
