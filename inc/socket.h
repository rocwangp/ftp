#pragma once

#include "constant.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <cerrno>
#include <iostream>

class CSocket
{
public:
    CSocket(int fd = -1);
    ~CSocket();

    bool create_socket();
    bool close_socket();

    bool bind_socket(const std::string& ip, int port);
    bool listen_socket(int listen_number);
    int accept_socket();

    bool connect_socket(const std::string& ip, int port);

    bool set_socket_nonblocking();
    bool set_socket_blocking();

    int get_fd() { return m_sockfd; }
    void set_fd(int fd) { m_sockfd = fd; }
    int recv_message(std::string& message);
    int send_message(const std::string& message);

private:
    int m_sockfd;
};
