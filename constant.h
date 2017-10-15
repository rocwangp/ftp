#pragma once

#include <string>

static const int FTP_DEFAULT_BUFFER = 4096;

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
    FTP_COMMAND_QUIT
};

static const int CONTROL_PORT = 5200;
static const int DATA_PORT = 5210;

static const int MAX_LISTEN_NUMBER = 10;
static const int MAX_EPOLL_NUMBER = 10;

static const std::string RESPONSE_WELCOME = "welcome to use ftp";
static const std::string RESPONSE_USER_SUCCESS = "331 User name okay, need password.";
static const std::string RESPONSE_PASS_SUCCESS = "230 User logged in, proceed.";
static const std::string RESPONSE_PASV_SUCCESS = "227 Entering passive mode";
static const std::string RESPONSE_CWD_SUCCESS = "250 Command okay.";

static const std::string RESPONSE_SIZE_SUCCESS = "213";
static const std::string RESPONSE_RETR_SUCCESS = "150 Opening data connection.";
static const std::string RESPONSE_QUIT_SUCCESS = "200 Closes connection.";

