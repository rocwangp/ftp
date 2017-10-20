#pragma once

#include "constant.h"
#include "socket.h"

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/sendfile.h>
#include <sys/stat.h>

#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <cerrno>

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <utility>
#include <map>
#include <vector>
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
    bool set_port_mode();
    bool download(const std::string& filename);
    bool store(const std::string& filename);
    bool continue_download(const std::string& offset);
    bool print_work_directory();
    bool change_work_directory(const std::string& dirname);
    bool get_filesize(const std::string& filename);
    bool list_file(const std::string& dirname); 
private:
    std::string parse_command(int comCode, const std::string& comArg);
    bool send_recv_message(const std::string& control);

    bool send_command(const std::string& command);
    bool recv_response(std::string& response);
    
    static void* process_download(void* arg);

private:
    bool is_continue_download();
private:
    CSocket m_control_socket;
    CSocket m_data_socket;

    CSocket m_data_listen_socket;
    
    std::string m_filename;
    long long int m_filesize;

    bool m_is_rest;
    off_t m_file_offset;
};
