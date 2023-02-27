#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

class Client
{
public:
    static int id;

private:
    int client_fd;
    std::string ip;

public:
    Client();
    ~Client();
    int id { return id; }
    int getFd() { return client_fd; }
    void setFd(int my_client_fd) { client_fd = my_client_fd; }
    void setIP(std::string myip) { ip = myip; }
    std::string getIP() { return ip; }
};