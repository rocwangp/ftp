#pragma once

#include "socket.h"

#include <string>

class CFTPServer;

struct ftp_client_t
{
    int control_fd;
    int data_fd;
    int data_listen_fd;
    off_t file_offset;
    std::string current_workdir;
    std::string control_argument;
};


struct pthread_argument_t
{
    int fd;
    CFTPServer* ftp_server;
};
