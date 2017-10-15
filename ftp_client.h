#pragma once

#include "constant.h"
#include "socket.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <string.h>
#include <cerrno>

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>


class CFTPClient
{
public:
    CFTPClient();
    ~CFTPClient();

    void login_server(const std::string& host);
    void input_username(const std::string& username);
    void input_password(const std::string& password);
    void quit_server(void); 

    bool set_pasv_mode();
    bool download(const std::string& filename);
    bool store(const std::string& filename);
    bool print_work_directory();
    bool change_work_directory(const std::string& dirname);
    bool get_filesize(const std::string& filename);
    bool list_file(const std::string& dirname); 
private:
    std::string parse_command(int comCode, const std::string& comArg);
    bool send_recv_message(const std::string& control);

    bool send_command(const std::string& command);
    bool recv_response(std::string& response);
    
    bool CloseSocket();
private:
    CSocket m_control_socket;
    CSocket m_data_socket;

    struct sockaddr_in m_servAddr;
    
};
