#pragma once

#include <string>

static const int FTP_PTHREAD_NUMBER = 10;
static const int FTP_DEFAULT_BUFFER = 4096;
static const std::string FTP_LOG_FILENAME = "server_log.txt";

static std::string SERVER_BIND_ADDRESS = "127.0.0.1";

enum FTP_COMMAND
{
    FTP_COMMAND_USERNAME,
    FTP_COMMAND_PASSWORD,
    FTP_COMMAND_PASV,
    FTP_COMMAND_RETR,
    FTP_COMMAND_STOR,
    FTP_COMMAND_PWD,
    FTP_COMMAND_CWD,
    FTP_COMMAND_SIZE,
    FTP_COMMAND_LIST,
    FTP_COMMAND_QUIT,
    FTP_COMMAND_REST,
    FTP_COMMAND_PORT
};

static const int CONTROL_PORT = 5200;
static const int DATA_PORT = 5210;

static const int MAX_LISTEN_NUMBER = 10;
static const int MAX_EPOLL_NUMBER = 10;

static const std::string RESPONSE_WELCOME = "welcome to use ftp";
static const std::string RESPONSE_USER_SUCCESS = "User name okay, need password.";
static const std::string RESPONSE_PASS_SUCCESS = "User logged in, proceed.";

static const std::string RESPONSE_RETR_SUCCESS = "Opening data connection.";
static const std::string RESPONSE_QUIT_SUCCESS = "Closes connection.";

static const std::string FIND_ACTIVE_CLIENT_FD = "find active client fd is";
static const std::string DISCONNECT_WITH_CLIENT_LOG = "disconnect with client";
static const std::string ACCEPT_CONTROL_CONNECTION_LOG = "accept control connection from client";
static const std::string ACCEPT_DATA_CONNECTION_LOG = "accept data connection from client";
static const std::string PROCESS_CLIENT_COMMAND_LOG = "start process client command";
static const std::string RECV_CLIENT_COMMAND_LOG = "recv command from client, command is";
static const std::string PARSE_COMMAND_SUCCESS_LOG = "parse client command success";
static const std::string PARSE_COMMAND_ERROR_LOG = "parse client command error";

static const std::string CREATE_SOCKET_ERROR_LOG = "create socket error";
static const std::string BIND_ADDRESS_ERROR_LOG = "bind socket address error";
static const std::string LISTEN_SOCKET_ERROR_LOG = "listen socket error";
static const std::string CREATE_LISTEN_SOCKET_SUCCESS_LOG = "create listen listen success";


static const std::string SERVER_START_RUN_LOG = "ftp server start running";

static const std::string SET_SIGINT_HANDLE_SUCCESS_LOG = "set SIGINT signal handle success";
static const std::string SET_SIGINT_HANDLE_ERROR_LOG = "set SIGINT signal handle error";

static const std::string OPEN_FILE_ERROR_LOG = "fail to open file";
static const std::string OPEN_FILE_SUCCESS_LOG = "open file success";
static const std::string STAT_FILE_ERROR_LOG = "stat error";
static const std::string SEND_FILE_OVER_LOG = "send file over";
static const std::string STOR_FILE_OVER_LOG = "store file over";

static const std::string OPEN_DIR_ERROR_LOG = "fail to open dir";
static const std::string OPEN_DIR_SUCCESS_LOG = "open dir success";

static const std::string RECV_ERROR_LOG = "recv error";
static const std::string CLIENT_QUIT_LOG = "client quit";
