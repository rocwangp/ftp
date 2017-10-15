#include "ftp_server.h"

CFTPServer::CFTPServer()
{
    create_socket();
    create_epoll();
}

CFTPServer::~CFTPServer()
{
    close_socket();
    close_epoll();
}


bool CFTPServer::create_socket()
{
    if(!m_control_listen_socket.create_socket())
    {
        std::cerr << "create_socket error";
        return false;
    }

    if(!m_control_listen_socket.bind_socket("127.0.0.1", CONTROL_PORT))
    {
        std::cerr << "bind_socket error";
        return false;
    }


    if(!m_control_listen_socket.listen_socket(MAX_LISTEN_NUMBER))
    {
        std::cerr << "listen error";
        return false;
    }
    
    return true;
}

bool CFTPServer::close_socket()
{
    if(!m_control_listen_socket.close_socket() || !m_data_listen_socket.close_socket())
        return false;
    else
        return true;
}

bool CFTPServer::create_epoll()
{
    return m_epoll.create_epoll();
}

bool CFTPServer::close_epoll()
{
    return m_epoll.close_epoll();
}

void CFTPServer::handle(int)
{
    exit(0);
}

void CFTPServer::run()
{
    struct sigaction act;
    act.sa_handler = CFTPServer::handle;
    sigaction(SIGINT, &act, NULL);

    m_epoll.add_event(m_control_listen_socket.get_fd(), EPOLLIN | EPOLLET);

    while(true)
    {
        int n = m_epoll.epoll_wait(-1);
        if(n <= 0)
            break;
        for(int i = 0; i < n; ++i)
        {
            int fd = m_epoll.get_fd(i);
            unsigned int events = m_epoll.get_events(i);

            std::cout << "active fd is " << fd << std::endl;
            if((events & EPOLLHUP) || (events & EPOLLERR) || !(events & EPOLLIN))
            {
                std::cout << "disconnect from client " << fd << std::endl;
                m_epoll.delete_event(fd, events);
                close(fd);
                continue;
            }

            if(fd == m_control_listen_socket.get_fd())
            {
                int fd = m_control_listen_socket.accept_socket();
                m_control_socket.set_fd(fd);
                m_epoll.add_event(fd, EPOLLIN | EPOLLET);
                m_control_socket.send_message(RESPONSE_WELCOME);
                 
                std::cout << "accept command connection from client " << fd << std::endl;
            }
            else if(fd == m_data_listen_socket.get_fd())
            {
                std::cout << "accept data connection from client" <<fd << std::endl;
                int fd = m_data_listen_socket.accept_socket();
                m_data_socket.set_fd(fd);
            }
            else
            {
                process_command(fd);
            }
        }
    }
}

void CFTPServer::process_command(int fd)
{
    m_client_socket.set_fd(fd);

    std::string message = recv_client_command(&m_client_socket);
    if(message == "")
        return;
    message = message.substr(0, message.size() - 2);
    std::cout << "receive command from client is " << message << std::endl;

    std::string command;
    std::string argument;

    std::string::size_type split_idx = message.find_first_of(" ", 0);
    if(split_idx == std::string::npos)
    {
        command = message;
        argument = "";
    }
    else
    {
        command = message.substr(0, split_idx);
        argument = message.substr(split_idx + 1);
    }
    
    if(command == "USER")
        process_user_command(argument);
    else if(command == "PASS")
        process_pass_command(argument);
    else if(command == "CWD")
        process_cwd_command(argument);
    else if(command == "PWD")
        process_pwd_command();
    else if(command == "PASV")
        process_pasv_command();
    else if(command == "PORT")
        process_port_command(argument);
    else if(command == "SIZE")
        process_size_command(argument);
    else if(command == "RETR")
        process_retr_command(argument);
    else if(command == "STOR")
        process_stor_command(argument);
    else if(command == "QUIT")
        process_quit_command();
    else if(command == "LIST")
        process_list_command(argument);
    else
        ;
 
}


std::string CFTPServer::recv_client_command(CSocket *client_socket)
{
    std::string message;
    if(client_socket->recv_message(message) == 0)
    {
        m_epoll.delete_event(client_socket->get_fd(), EPOLLIN | EPOLLET);
        client_socket->close_socket();
        return "";
    }
    return message;
}

void CFTPServer::process_user_command(const std::string& username)
{
    /* find username's password */
    m_control_socket.send_message(RESPONSE_USER_SUCCESS); 
}

void CFTPServer::process_pass_command(const std::string& password)
{
    /* check password */
    m_control_socket.send_message(RESPONSE_PASS_SUCCESS);
}

void CFTPServer::process_pasv_command()
{
    if(!m_data_listen_socket.create_socket())
    {
        std::cerr << "data_listen_socket create_socket error";
        return;
    }
    
    if(!m_data_listen_socket.bind_socket("127.0.0.1", DATA_PORT))
    {
        std::cerr << "bind_socket error";
        m_data_listen_socket.close_socket();
    }
    if(!m_data_listen_socket.listen_socket(MAX_LISTEN_NUMBER))
    {
        std::cerr <<"listen error";
        m_data_listen_socket.close_socket();
    }

    std::cout << "data listen socket is " << m_data_listen_socket.get_fd() << std::endl;

    m_epoll.add_event(m_data_listen_socket.get_fd(), EPOLLIN | EPOLLET);

    int p1, p2;
    p1 = DATA_PORT / 256;
    p2 = DATA_PORT % 256;
    std::stringstream oss;
    oss << "(" << "127,0,0,1," << p1 << "," << p2 << ")";
   
    m_control_socket.send_message(oss.str());
}

void CFTPServer::process_port_command(const std::string& addr_and_port)
{
    /* no done */
    if(m_data_listen_socket.get_fd() != -1)
    {
        m_epoll.delete_event(m_data_listen_socket.get_fd(), EPOLLIN | EPOLLET);
        m_data_listen_socket.close_socket();
       
    }

    std::stringstream oss(addr_and_port);
    int h1, h2, h3, h4, p1, p2;
    char ch;
    oss >> h1 >> ch >> h2 >> ch >> h3 >> ch >> h4 >> ch >> p1 >> ch >> p2;
    oss.clear();
    oss << h1 << "." << h2 << "." << h3 << "." << h4;

    if(m_data_socket.get_fd() != -1)
    {    
        m_epoll.delete_event(m_data_socket.get_fd(), EPOLLIN | EPOLLET);
        m_data_socket.close_socket();
    }
    m_data_socket.create_socket();
    m_data_socket.connect_socket(oss.str(), p1 * 256 + p2);
  
    m_epoll.add_event(m_data_listen_socket.get_fd(), EPOLLIN | EPOLLET);
}

void CFTPServer::process_cwd_command(const std::string& dirname)
{
    if(chdir(dirname.c_str()) < 0)
        ;/* error */
    else
        m_control_socket.send_message("change workdirectory success");
}

void CFTPServer::process_pwd_command()
{
    char dirname[FTP_DEFAULT_BUFFER];
    bzero(dirname, sizeof(dirname));

    getcwd(dirname, sizeof(dirname));

    std::string response = "current workdirectory is ";
    response += dirname;
    m_control_socket.send_message(response);
}

void CFTPServer::process_size_command(const std::string& filename)
{
    struct stat fileinfo;
    if(lstat(filename.c_str(), &fileinfo) < 0)
        ;
    else
    {
        std::stringstream oss;
        oss << fileinfo.st_size;
        m_control_socket.send_message(oss.str());
    }
}

void CFTPServer::process_list_command(std::string& dirname)
{
    std::cout << dirname << std::endl;
 
    if(dirname.size() == 0)
    {
        char wd[FTP_DEFAULT_BUFFER];
        bzero(wd, sizeof(wd));
        getcwd(wd, sizeof(wd));
        dirname = wd;
        std::cout << dirname << std::endl;
    }

    struct stat statinfo;
    if(lstat(dirname.c_str(), &statinfo) < 0)
    {
        perror("lstat error");
        m_control_socket.send_message("");
        return;
    }

    std::string response;
    
    if(!S_ISDIR(statinfo.st_mode))
    {
        std::stringstream oss;
        oss << dirname << " " << statinfo.st_size;
        response = oss.str();
    }
    else
    {
        DIR *dp;
        if((dp = opendir(dirname.c_str())) == NULL)
        {
            std::cerr << "opendir error";
            m_control_socket.send_message("");
        }


        struct dirent* entry;
        while((entry = readdir(dp)) != NULL)
        {
            response += entry->d_name;
            response += '\n';
        }
        
        closedir(dp);
    }
   

    m_control_socket.send_message(response);
}


void CFTPServer::process_retr_command(const std::string& filename)
{
    m_control_socket.send_message(RESPONSE_RETR_SUCCESS);
   
    char current_work_dir[FTP_DEFAULT_BUFFER];
    bzero(current_work_dir, sizeof(current_work_dir));
    getcwd(current_work_dir, sizeof(current_work_dir));

    std::string filepath = current_work_dir;
    filepath += "/" + filename;

    std::ifstream in(filepath.c_str(), std::ios_base::in | std::ios_base::binary);
    if(!in.is_open())
    {
        std::cout << "open file " << filepath << "error " << std::endl;
        return;
    }
    char msg[FTP_DEFAULT_BUFFER];
    while(!in.eof())
    {
        bzero(msg, sizeof(msg));
        in.read(msg, sizeof(msg));
        int n = in.gcount();
        send(m_data_socket.get_fd(), msg, n, MSG_NOSIGNAL);
        
        
      
        //std::cerr << "send to client " << n << " bytes" << std::endl;

    }

    in.close();
    std::cout << "send message over " << std::endl;
}

void CFTPServer::process_stor_command(const std::string& filename)
{

}


void CFTPServer::process_quit_command()
{
    m_control_socket.send_message(RESPONSE_QUIT_SUCCESS);
    m_control_socket.close_socket();
}
