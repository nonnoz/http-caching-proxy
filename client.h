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
    int getID() { return id; }
    int getFd() { return client_fd; }
    void setID(int myid) { id = myid; }
    void setFd(int my_client_fd) { client_fd = my_client_fd; }
    void setIP(std::string myip) { ip = myip; }
    std::string getIP() { return ip; }
};