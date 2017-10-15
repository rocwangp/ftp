#include "ftp_client.h"


CFTPClient::CFTPClient()
{

}

CFTPClient::~CFTPClient()
{

}


void CFTPClient::login_server(const std::string& host)
{
    m_control_socket.create_socket();
    std::cout << CONTROL_PORT << std::endl; 
    if(!m_control_socket.connect_socket(host, CONTROL_PORT))
    {
        perror("LoginServer : control port connect error\n");
        m_control_socket.close_socket();
        return;
    }
    std::string response;
    int recv_ret = m_control_socket.recv_message(response);
    if(recv_ret <= 0)
    {
        perror("login_server : recv welcome message error or disconnect from server\n");
        return;
    }
    else
    {
        std::cout << response << std::endl;
    }
}

void CFTPClient::input_username(const std::string& username)
{
    std::string control = parse_command(FTP_COMMAND_USERNAME, username);
    send_recv_message(control);
}

void CFTPClient::input_password(const std::string& password)
{
    std::string control = parse_command(FTP_COMMAND_PASSWORD, password);
    send_recv_message(control);
}

void CFTPClient::quit_server(void)
{
    std::string control = parse_command(FTP_COMMAND_QUIT, "");
    send_recv_message(control);
    if(m_data_socket.get_fd() != -1)
    {
        m_data_socket.close_socket();
    }

    if(m_control_socket.get_fd() != -1)
    {
        m_control_socket.close_socket();
    }
}



bool CFTPClient::list_file(const std::string& dirname)
{
    std::string control = parse_command(FTP_COMMAND_LIST, dirname);
    return send_recv_message(control);
}


bool CFTPClient::set_pasv_mode()
{
    std::string control = parse_command(FTP_COMMAND_PASV, "");
    if(send_command(control) == false)
        return false;

    std::string response;
    if(recv_response(response) == false)
        return false;

    std::cout << "response from server is " << response << std::endl;

   
    std::stringstream oss(response);
    int h1, h2, h3, h4, p1, p2;
    char ch;
    oss >> ch >> h1 >> ch >> h2 >> ch >> h3 >> ch >> h4 >> ch >> p1 >> ch >> p2 >> ch;
   
    oss.str("");
    oss.clear();
    oss << h1 << "." << h2 << "." << h3 << "." << h4;

    m_data_socket.close_socket();
    m_data_socket.create_socket();
    if(!m_data_socket.connect_socket(oss.str(), p1 * 256 + p2))
    {
        perror("SetPasv : data port connect error...");
        return false;
    }
    return true;
}


bool CFTPClient::download(const std::string& filename)
{ 
    std::string control = parse_command(FTP_COMMAND_SIZE, filename);
    send_command(control);
    std::string response;
    recv_response(response);

    std::stringstream oss(response);
    long long int file_size;
    oss >> file_size;
    std::cout << "file size is" << file_size << std::endl;

    control = parse_command(FTP_COMMAND_RETR, filename);
    if(send_recv_message(control) == false)
        return false;
    
    char buffer[FTP_DEFAULT_BUFFER];
    bzero(buffer, FTP_DEFAULT_BUFFER);

    std::ofstream out;
    out.open(filename.c_str(), std::ios_base::binary | std::ios_base::out);
    
    if(!out.is_open())
    {
        std::cout << "can't open file : " << filename << std::endl;
        return false;
    }

    long long int recv_size = 0;
    while(true)
    {
        int n = recv(m_data_socket.get_fd(), buffer, sizeof(buffer), 0);
        if(n < 0)
        {
            break;
        }
        else if(n == 0)
        {
            m_data_socket.close_socket();
            break;
        }
        else
        {
            out.write(buffer, n);
            recv_size += n;
            std::cout << recv_size << " " << file_size << std::endl;
            if(recv_size >= file_size)
                break;
        }
    }
    out.close();

    std::cout << "download over" << std::endl;
    return true;
}


bool CFTPClient::store(const std::string& filename)
{
    std::string control = parse_command(FTP_COMMAND_STOR, filename);
    if(send_recv_message(control) == false)
        return false;

    std::ifstream in;
    in.open(filename.c_str(), std::ios::binary | std::ios::in);
    if(!in.is_open())
    {
        std::cout << "can't open file : " << filename << std::endl;
        return false;
    }
    char buffer[FTP_DEFAULT_BUFFER];
    while(!in.eof())
    {
        bzero(buffer, sizeof(buffer));
        in.read(buffer, FTP_DEFAULT_BUFFER);
        m_data_socket.send_message(buffer);
    }

    in.close();
    return true;
}


bool CFTPClient::print_work_directory()
{
    std::string control = parse_command(FTP_COMMAND_PWD, "");
    return send_recv_message(control);
}

bool CFTPClient::change_work_directory(const std::string& dirname)
{
    std::string control = parse_command(FTP_COMMAND_CWD, dirname);
    return send_recv_message(control);
}

bool CFTPClient::get_filesize(const std::string& filename)
{
    std::string control = parse_command(FTP_COMMAND_SIZE, filename);
    return send_recv_message(control);
}



bool CFTPClient::send_recv_message(const std::string& control)
{
    if(send_command(control) == false)
    {
        std::cerr << "send command to server error" << std::endl;
        return false;
    }

    std::string response;
    if(recv_response(response) == false)
    {
        std::cerr << "recv response from server error" << std::endl;
        return false;
    }

    std::cout << "response from server : " << response << std::endl;
    return true;
}

std::string CFTPClient::parse_command(int control, const std::string& argument)
{
    std::string command("");

    switch(control)
    {
    case FTP_COMMAND_USERNAME:
        command = "USER " + argument + "\r\n";
        break;
    case FTP_COMMAND_PASSWORD:
        command = "PASS " + argument + "\r\n";
        break;
    case FTP_COMMAND_PASV:
        command = "PASV\r\n";
        break;
    case FTP_COMMAND_LIST:
        command = "LIST " + argument + "\r\n";
        break;
    case FTP_COMMAND_PWD:
        command = "PWD\r\n";
        break;
    case FTP_COMMAND_CWD:
        command = "CWD " + argument + "\r\n";
        break;
    case FTP_COMMAND_SIZE:
        command = "SIZE " + argument + "\r\n";
        break;
    case FTP_COMMAND_RETR:
        command = "RETR " + argument + "\r\n";
        break;
    case FTP_COMMAND_STOR:
        command = "STOR " + argument + "\r\n";
        break;
    case FTP_COMMAND_QUIT:
        command = "QUIT\r\n";
        break;
    default:
        break;
    }
    return command;
}

bool CFTPClient::send_command(const std::string& command)
{
    std::cout << "command is " << command << std::endl;
    if(m_control_socket.send_message(command) > 0)
        return true;
    else
        return false;
 
}

bool CFTPClient::recv_response(std::string& response)
{
    if(m_control_socket.recv_message(response) > 0)
        return true;
    else
        return false;
   
}
