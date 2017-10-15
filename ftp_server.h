#pragma once

#include "constant.h"
#include "epoll.h"
#include "socket.h"


#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <cstdlib>
#include <cerrno>
#include <iostream>
#include <sstream>
#include <fstream>
#include <queue>

class CFTPServer
{
public:
    CFTPServer();
    ~CFTPServer();

    void run();
private:
    bool create_socket();
    bool close_socket();

    bool create_epoll();
    bool close_epoll();

    void process_command(int fd);

    std::string recv_client_command(CSocket* client_socket);

    static void handle(int);
private:

    void process_quit_command();
    void process_pasv_command();
    void process_list_command(std::string& dirname);
    void process_pwd_command();
    void process_user_command(const std::string& username);
    void process_pass_command(const std::string& password);
    void process_size_command(const std::string& filename); 
    void process_cwd_command(const std::string& dirname);
    void process_port_command(const std::string& addr_and_port);
    void process_retr_command(const std::string& filename);
    void process_stor_command(const std::string& filename);
   
    
private:
    CSocket m_control_socket;
    CSocket m_data_socket;

    CSocket m_control_listen_socket;
    CSocket m_data_listen_socket;

    CSocket m_client_socket;

    CEpoll m_epoll; 

    std::queue<int> m_fd_queue;
};

