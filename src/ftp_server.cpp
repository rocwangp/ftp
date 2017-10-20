#include "../inc/ftp_server.h"


CFTPServer::CFTPServer() :
    m_control_listen_fd(-1),
    m_data_listen_fd(-1),
    m_epoll(),
    m_current_workdir(""),
    m_log_filepath(""),
    m_client_map(),
    m_address_map(),
    m_pthread_pool()
{
   
     m_pthread_mutex = PTHREAD_MUTEX_INITIALIZER;
    /* pthread_mutex_init(&m_pthread_mutex, NULL);
     * pthread_cond_init(&m_pthread_cond, NULL);
     */
    create_control_listen_socket();
    create_epoll();
    init_log_filepath();
    init_current_workdir();
}

CFTPServer::~CFTPServer()
{
    /* pthread_mutex_destroy(&m_pthread_mutex);
     */
    if(m_control_listen_fd != -1)
        close(m_control_listen_fd);
    if(m_data_listen_fd != -1)
        close(m_data_listen_fd);

    close_epoll();
}

void CFTPServer::init_log_filepath()
{
    char wd[FTP_DEFAULT_BUFFER];
    bzero(wd, sizeof(wd));
    getcwd(wd, sizeof(wd));
    m_log_filepath = wd;
    m_log_filepath += "/" + FTP_LOG_FILENAME;
    
}

void CFTPServer::init_current_workdir()
{
    char current_workdir[FTP_DEFAULT_BUFFER];
    bzero(current_workdir, sizeof(current_workdir));
    getcwd(current_workdir, sizeof(current_workdir));
    m_current_workdir = current_workdir;
}

bool CFTPServer::create_control_listen_socket()
{
    m_control_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(m_control_listen_fd < 0)
    {
        write_log(CREATE_SOCKET_ERROR_LOG, 0);
        return false;
    }

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(CONTROL_PORT);
    inet_pton(AF_INET, SERVER_BIND_ADDRESS.c_str(), &servaddr.sin_addr);
    
    if(bind(m_control_listen_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
    {
        write_log(BIND_ADDRESS_ERROR_LOG, 0);
        close(m_control_listen_fd);
        return false;
    }

    if(listen(m_control_listen_fd, MAX_LISTEN_NUMBER) < 0)
    {
        write_log(LISTEN_SOCKET_ERROR_LOG, 0);
        close(m_control_listen_fd);
        return false;
    }
   
    return true;
}

bool CFTPServer::create_data_listen_socket()
{
    m_data_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(m_data_listen_fd < 0)
    {
        write_log(CREATE_SOCKET_ERROR_LOG, 0);
        return false;
    }

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(DATA_PORT);
    inet_pton(AF_INET, SERVER_BIND_ADDRESS.c_str(), &servaddr.sin_addr);
    
    if(bind(m_data_listen_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
    {
        write_log(BIND_ADDRESS_ERROR_LOG, 0);
        close(m_data_listen_fd);
        return false;
    }

    if(listen(m_data_listen_fd, MAX_LISTEN_NUMBER) < 0)
    {
        write_log(LISTEN_SOCKET_ERROR_LOG, 0);
        close(m_data_listen_fd);
        return false;
    }

    write_log(CREATE_LISTEN_SOCKET_SUCCESS_LOG, 0);
    m_epoll.add_event(m_data_listen_fd, EPOLLIN | EPOLLET);
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



void CFTPServer::write_log(const std::string& caption, int fd)
{
    pthread_mutex_lock(&m_pthread_mutex);
    std::ofstream log_file(m_log_filepath.c_str(), std::ios_base::out | std::ios_base::app);
    if(!log_file.is_open())
    {
        std::cout << "cannot open file " << FTP_LOG_FILENAME << std::endl;
        return;
    }
    std::string message;
    std::stringstream oss;
    if(fd != 0)
    {
        oss << caption << " " << fd << "\n";
        message = oss.str();
    }
    else
    {
        message = caption + "\n";
    }

    time_t timer;
    time(&timer);
    std::string time_str = ctime(&timer);

    std::string::size_type idx = time_str.find_first_of("\n", 0);
    if(idx != std::string::npos)
        time_str[idx] = ' ';

    message = time_str + message;

    log_file.write(message.c_str(), message.size());
    log_file.close();

    pthread_mutex_unlock(&m_pthread_mutex);
}

void CFTPServer::clear_log()
{
    std::ofstream log_file(FTP_LOG_FILENAME.c_str(), std::ios_base::out);
    log_file.close();
}
void CFTPServer::handle(int)
{
    exit(0);
}

/* 
 * FTP服务器的主循环，永远io复用事件监听，分成三种
 *  监听到控制命令的连接请求（通常是刚启动客户端），服务器接收
 *  监听到数据传输的连接请求（通常是转换到被动模式后），服务器接收
 *  其他命令请求，放入线程池中，绑定回调函数
 */
void CFTPServer::run()
{
    clear_log();
    write_log(SERVER_START_RUN_LOG, 0);
    
    struct sigaction act;
    act.sa_handler = CFTPServer::handle;
    if(sigaction(SIGINT, &act, NULL) < 0)
    {
        write_log(SET_SIGINT_HANDLE_ERROR_LOG, 0);
        return;
    }
    else
    {
        write_log(SET_SIGINT_HANDLE_SUCCESS_LOG, 0);
    }

    m_epoll.add_event(m_control_listen_fd, EPOLLIN | EPOLLET);

    m_pthread_pool.run(FTP_PTHREAD_NUMBER);

    std::stringstream oss;
    while(true)
    {
        int n = m_epoll.epoll_wait(-1);
        if(n <= 0)
            break;
        for(int i = 0; i < n; ++i)
        {
            int fd = m_epoll.get_fd(i);
            unsigned int events = m_epoll.get_events(i);


            write_log(FIND_ACTIVE_CLIENT_FD, fd);

            if((events & EPOLLHUP) || (events & EPOLLERR) || !(events & EPOLLIN))
            {
                write_log(DISCONNECT_WITH_CLIENT_LOG, fd);
                m_epoll.delete_event(fd, events);
                close(fd);
                continue;
            }

            if(fd == m_control_listen_fd)
            {

                struct sockaddr_in clientaddr;
                socklen_t len = sizeof(clientaddr);
                int fd = accept(m_control_listen_fd, (struct sockaddr*)&clientaddr, &len);

                m_epoll.add_event(fd, EPOLLIN | EPOLLET);

                /*
                 * ftp_client_t中包含
                 * 客户端控制套接字：用于接收命令
                 * 数据传输套接字：用于上传，下载
                 * 当前工作目录：客户端在服务器中设置的当前工作目录，不能真正改变服务器的工作目录，因为
                 *      如果有多个客户端请求，工作目录会乱掉，所以只是记录每个客户端的工作目录
                 * 命令参数：客户端发送命令时带有的参数
                 * 偏移量：用于断点续传，客户端发送REST时传入的参数
                 */
                ftp_client_t ftp_client;
                ftp_client.control_fd = fd;
                ftp_client.current_workdir = m_current_workdir;
                ftp_client.control_argument = "";
                ftp_client.file_offset = 0;
                m_client_map[fd] = ftp_client;

                /*
                 * 为了可以同时满足多个客户端，而又因为客户端的控制请求和数据传输请求不是同时发送的
                 * 为了找到当前数据传输请求属于哪个客户端，需要存储客户端的地址
                 * 根据地址找到控制请求
                 * 这也就造成了一台机器中只能运行一个FTP客户端
                 */
                std::string ip_string = parse_ip_address(clientaddr);
                m_address_map[ip_string] = fd;

                send(fd, RESPONSE_WELCOME.c_str(), RESPONSE_WELCOME.size(), MSG_NOSIGNAL);
                write_log(ACCEPT_CONTROL_CONNECTION_LOG, fd);
            }
            else if(fd == m_data_listen_fd)
            {
                struct sockaddr_in clientaddr;
                socklen_t len = sizeof(clientaddr);
                int fd = accept(m_data_listen_fd, (struct sockaddr*)&clientaddr, &len);

                std::string ip_string = parse_ip_address(clientaddr);
                int control_fd = m_address_map[ip_string];
                m_client_map[control_fd].data_fd = fd;

                write_log(ACCEPT_DATA_CONNECTION_LOG, fd);
            }
            else
            {
                write_log(PROCESS_CLIENT_COMMAND_LOG, fd);
                
                /*
                 * 创建线程池任务，添加到线程池中，参数
                 * void (*process_command)(std::vector<void*>);回调函数地址
                 * std::vector<void*>;给回调函数的参数
                 */
                CTask *task = new CTask(&CFTPServer::process_command, {static_cast<void*>(this), static_cast<void*>(&fd)});
                m_pthread_pool.add_task(task);

            }
        }
    }
}

std::string CFTPServer::parse_ip_address(struct sockaddr_in& addr)
{
    char ip_address[FTP_DEFAULT_BUFFER];
    bzero(ip_address, sizeof(ip_address));
    inet_ntop(addr.sin_family, &addr.sin_addr, ip_address, sizeof(ip_address));
    return ip_address;
}

void CFTPServer::process_command(std::vector<void*> args)
{
    CFTPServer *ftp_server = static_cast<CFTPServer *>(args[0]);
    int fd = *static_cast<int*>(args[1]);

    std::string message = ftp_server->recv_client_command(fd);
    std::string log_msg = RECV_CLIENT_COMMAND_LOG + message;
    ftp_server->write_log(log_msg, 0);
    if(message == "")
    {
        ftp_server->m_epoll.delete_event(fd, EPOLLIN | EPOLLET);
        close(fd);

        ftp_server->write_log(PARSE_COMMAND_ERROR_LOG, fd);
        return;
    }
    std::string::size_type back_idx = message.find_first_of("\r\n", 0);
    if(back_idx == std::string::npos)
    {
        ftp_server->write_log(PARSE_COMMAND_ERROR_LOG, fd);
        return;
    }
    message = message.substr(0, back_idx);
    ftp_server->write_log(PARSE_COMMAND_SUCCESS_LOG, 0);

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
    
    ftp_server->m_client_map[fd].control_argument = argument;

    /* 分发任务 */
    if(command == "USER")
        ftp_server->process_user_command(fd);
    else if(command == "PASS")
        ftp_server->process_pass_command(fd);
    else if(command == "CWD")
        ftp_server->process_cwd_command(fd);
    else if(command == "PWD")
        ftp_server->process_pwd_command(fd);
    else if(command == "PASV")
        ftp_server->process_pasv_command(fd);
    else if(command == "PORT")
        ftp_server->process_port_command(fd);
    else if(command == "SIZE")
        ftp_server->process_size_command(fd);
    else if(command == "RETR")
    {
        //struct pthread_argument_t* p_arg = new struct pthread_argument_t;
        //p_arg->fd = fd;
        //p_arg->ftp_server = ftp_server;

        ftp_server->process_retr_command(fd);
       // pthread_t tid;
       // pthread_create(&tid, NULL, process_retr_command, static_cast<void*>(p_arg));
       // pthread_detach(tid);
    }
    else if(command == "STOR")
        ftp_server->process_stor_command(fd);
    else if(command == "QUIT")
        ftp_server->process_quit_command(fd);
    else if(command == "LIST")
        ftp_server->process_list_command(fd);
    else if(command == "REST")
        ftp_server->process_rest_command(fd);
    else
        ftp_server->process_other_command(fd);
 
}


std::string CFTPServer::recv_client_command(int fd)
{
    char message[FTP_DEFAULT_BUFFER];
    int recv_ret = recv(fd, message, sizeof(message), 0);
    if(recv_ret <= 0)
        return "";
    else
    {
        message[recv_ret] = '\0';
        return message;
    }
}

void CFTPServer::process_other_command(int fd)
{
    std::string response = "cannot parse command, please enter corrent command";
    send(fd, response.c_str(), response.size(), MSG_NOSIGNAL);
    write_log(response, fd);
}

/*
 * 断电续传命令只是将偏移量简单记录在ftp_client_t中
 * 当客户端使用RETR下载时再偏移
 */
void CFTPServer::process_rest_command(int fd)
{
    std::stringstream oss(m_client_map[fd].control_argument);
    oss >> m_client_map[fd].file_offset;
    std::string response = "350 Restarting at <" + m_client_map[fd].control_argument + ">. Send STORE or RETRIEVE to initiate transfer.";
    send(fd, response.c_str(), response.size(), MSG_NOSIGNAL);
    write_log(response, fd);
}

void CFTPServer::process_user_command(int fd)
{
    /* find username's password */
    send(fd, RESPONSE_USER_SUCCESS.c_str(), RESPONSE_USER_SUCCESS.size(), MSG_NOSIGNAL);
}

void CFTPServer::process_pass_command(int fd)
{
  
    /* check password */

    send(fd, RESPONSE_PASS_SUCCESS.c_str(), RESPONSE_PASS_SUCCESS.size(), MSG_NOSIGNAL);
}

/*
 * 被动模式，服务器发送地址和端口给客户端，客户端链接
 */
void CFTPServer::process_pasv_command(int fd)
{
    std::string response;
    if(m_data_listen_fd == -1)
    {
        if(!create_data_listen_socket())
        {
            response = "fail to convert to pasv mode, please retry";
            send(fd, response.c_str(), response.size(), MSG_NOSIGNAL);
            write_log(response, fd);
            return;
        }
    }

    std::stringstream oss;
    int p1, p2;
    p1 = DATA_PORT / 256;
    p2 = DATA_PORT % 256;
    oss << "(" << "127,0,0,1," << p1 << "," << p2 << ")";
    response = oss.str();

    std::string log_msg = "send response to client : " + response;
    write_log(log_msg, fd);
    send(fd, response.c_str(), response.size(), MSG_NOSIGNAL);
}

/* 
 * 主动模式，需要服务器链接客户端地址和端口
 */
void CFTPServer::process_port_command(int fd)
{ 
    int h1, h2, h3, h4, p1, p2;
    char ch;

    std::stringstream oss(m_client_map[fd].control_argument);
    oss >> h1 >> ch >> h2 >> ch >> h3 >> ch >> h4 >> ch >> p1 >> ch >> p2 >> ch;
    int port = p1 * 256 + p2;
    oss.str("");
    oss.clear();
    oss << h1 << "." << h2 << "." << h3 << "." << h4;
    std::string ip_address = oss.str();

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    inet_pton(AF_INET, ip_address.c_str(), &servaddr.sin_addr);

    oss.str("");
    oss.clear();
    oss << "client ip and port is " << ip_address << " " << port;
    write_log(oss.str(), fd);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        std::string response = "fail to convert to port pattern, create data socket error";
        write_log(response, fd);
        send(fd, response.c_str(), response.size(), MSG_NOSIGNAL);
        return;
    }

    if(connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
    {
        close(sockfd);
        std::string response = "fail to convert to port pattern, connect to client error";
        write_log(response, fd);
        send(fd, response.c_str(), response.size(), MSG_NOSIGNAL);
        return;
    }

    if(m_client_map[fd].data_fd != -1)
    {
        close(m_client_map[fd].data_fd);
        write_log("close pasv pattern", fd);
    }
    m_client_map[fd].data_fd = sockfd;

    std::string response = "convert port pattern success";
    send(fd, response.c_str(), response.size(), MSG_NOSIGNAL);
    write_log(response, fd);
}

/* 
 * 改变当前工作目录，没有实际改变，只是将工作目录存在ftp_client_t中
 * 解决多个客户端的问题，因为每个客户端都可能改变工作目录，如果直接改变服务器的，会乱掉
 */
void CFTPServer::process_cwd_command(int fd)
{
    std::string change_dir = m_client_map[fd].control_argument;
    struct stat statinfo;
    if(lstat(change_dir.c_str(), &statinfo) < 0 || !S_ISDIR(statinfo.st_mode))
    {
        std::string response = "change work dir error, current workdir is " + m_client_map[fd].current_workdir;
        send(fd, response.c_str(), response.size(), MSG_NOSIGNAL);
        write_log(response, fd);
    }
    else
    {
        m_client_map[fd].current_workdir = change_dir;
        std::string response = "change workdir success, current workdir is " + change_dir;
        send(fd, response.c_str(), response.size(), MSG_NOSIGNAL);
        write_log(response, fd);
    }
}

/*
 * 打印当前工作目录，直接输出ftp_client_t中记录的工作目录
 */
void CFTPServer::process_pwd_command(int fd)
{
    std::string response = "current workdir is " + m_client_map[fd].current_workdir;
    send(fd, response.c_str(), response.size(), MSG_NOSIGNAL);
    write_log(response, fd);
}

/*
 * 获得文件大小
 */
void CFTPServer::process_size_command(int fd)
{
    std::string filepath = m_client_map[fd].current_workdir + "/" + m_client_map[fd].control_argument;
    struct stat fileinfo;
    if(lstat(filepath.c_str(), &fileinfo) < 0 || !S_ISREG(fileinfo.st_mode))
    {
        std::string response = "-1";
        send(fd, response.c_str(), response.size(), MSG_NOSIGNAL);
        write_log(STAT_FILE_ERROR_LOG, fd);
    }
    else
    {
        std::stringstream oss;
        oss << fileinfo.st_size;
        std::string response = oss.str();
        send(fd, response.c_str(), response.size(), MSG_NOSIGNAL);
        write_log(response, fd);
    }
}

/*
 * 列出当前目录下的所有文件/目录等
 */
void CFTPServer::process_list_command(int fd)
{
    std::string dirname = m_client_map[fd].control_argument;
    
    if(dirname.size() == 0)
    {
        dirname = m_client_map[fd].current_workdir;
        std::string log_msg = "dirname is null, default dirname is current workdirectory : " + dirname;
        write_log(log_msg, fd);
    }

    std::string response;

    struct stat statinfo;
    if(lstat(dirname.c_str(), &statinfo) < 0)
    {
        response = "fail to parse LIST command, please check argument";
        send(fd, response.c_str(), response.size(), MSG_NOSIGNAL);
        write_log(STAT_FILE_ERROR_LOG, fd);
        return;
    }

    
    if(!S_ISDIR(statinfo.st_mode))
    {
        std::stringstream oss;
        oss << dirname << '\t' << statinfo.st_size;
        response = oss.str();
    }
    else
    {
        DIR *dp;
        if((dp = opendir(dirname.c_str())) == NULL)
        {
            write_log(OPEN_DIR_ERROR_LOG, fd);
            response = "fail to parse LIST command, please check argument";
        }
        else
        {
            write_log(OPEN_DIR_SUCCESS_LOG, fd);
            struct dirent* entry;
            while((entry = readdir(dp)) != NULL)
            {
                response += entry->d_name;
                response += '\t';
            }
            
            closedir(dp);
        }
    }
   
    send(fd, response.c_str(), response.size(), MSG_NOSIGNAL);
}


/*
 * 下载文件，使用sendfile领拷贝传文件到客户端
 */
void CFTPServer::process_retr_command(int fd)
{
    //pthread_argument_t *p_arg = static_cast<pthread_argument_t*>(arg);
    //CFTPServer* ftp_server = p_arg->ftp_server;
    //int fd = p_arg->fd;

    write_log("open a thread to send file", fd);

    std::string filename = m_client_map[fd].control_argument;
    std::string filepath = m_client_map[fd].current_workdir + "/" + filename;
    std::string log_msg = "filepath is : " + filepath;
    write_log(log_msg, fd);

    struct stat statinfo;
    if(lstat(filepath.c_str(), &statinfo) < 0)
    {
        write_log(STAT_FILE_ERROR_LOG, fd);
        std::string response = "RETR error, please check argument";
        send(fd, response.c_str(), response.size(), MSG_NOSIGNAL);

        m_client_map[fd].file_offset = 0;
    }
    int filefd = open(filepath.c_str(), O_RDONLY);

    if(filefd < 0)
    {
        write_log(OPEN_FILE_ERROR_LOG, fd);
        std::string response = "RETR error, cannot open file";
        send(fd, response.c_str(), response.size(), MSG_NOSIGNAL);

        m_client_map[fd].file_offset = 0;

    }

    send(fd, RESPONSE_RETR_SUCCESS.c_str(), RESPONSE_RETR_SUCCESS.size(), MSG_NOSIGNAL);
    sendfile(m_client_map[fd].data_fd, filefd, 
             &m_client_map[fd].file_offset, 
             statinfo.st_size - m_client_map[fd].file_offset);
    write_log(SEND_FILE_OVER_LOG, fd);
    close(filefd);

    m_client_map[fd].file_offset = 0;
}


/*
 * 上传文件，服务器接受数据
 */
void CFTPServer::process_stor_command(int fd)
{
    std::string response = "recv command success, start store file";
    send(m_client_map[fd].control_fd, response.c_str(), response.size(), MSG_NOSIGNAL);

    std::string filename_with_size = m_client_map[fd].control_argument;
    std::string::size_type front_idx = filename_with_size.find_first_of("<", 0);
    std::string::size_type back_idx = filename_with_size.find_first_of(">", 0);
    if(front_idx == std::string::npos || back_idx == std::string::npos)
    {
        write_log(PARSE_COMMAND_ERROR_LOG, fd);
        return;
    }
    std::string filename = filename_with_size.substr(0, front_idx);
    std::stringstream oss;
    off_t filesize;
    oss << filename_with_size.substr(front_idx + 1, back_idx - front_idx - 1);
    oss >> filesize;
    
    std::string filepath = m_client_map[fd].current_workdir + "/" + filename;
    std::ofstream out(filepath.c_str(), std::ios_base::out | std::ios_base::binary);
    if(!out.is_open())
    {
        write_log(OPEN_FILE_ERROR_LOG, fd);
        return;
    }
    write_log(OPEN_FILE_SUCCESS_LOG, fd);
    
    char message[FTP_DEFAULT_BUFFER];
    off_t recvsize = 0;
    while(true)
    {
        int n = recv(m_client_map[fd].data_fd, message, sizeof(message), 0);
        if(n < 0)
        {
            write_log(RECV_ERROR_LOG, fd);
            break;
        }
        else if(n == 0)
        {
            write_log(DISCONNECT_WITH_CLIENT_LOG, fd);
            close(m_client_map[fd].data_fd);
            m_client_map[fd].data_fd = -1;
        }
        else
        {
            out.write(message, n);
            recvsize += n;
            if(recvsize >= filesize)
                break;
        }
    }
    
    out.close();
    write_log(STOR_FILE_OVER_LOG, fd);
    
}


void CFTPServer::process_quit_command(int fd)
{
    close(fd);
    if(m_client_map[fd].data_fd != -1)
        close(m_client_map[fd].data_fd);
    
    send(fd, RESPONSE_QUIT_SUCCESS.c_str(), RESPONSE_QUIT_SUCCESS.size(), MSG_NOSIGNAL);
    write_log(CLIENT_QUIT_LOG, fd);
}
