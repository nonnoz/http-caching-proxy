#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

class Client {
public:
    static int id;
private:
    int client_fd;
    std::string ip;

public:
    Client();
    void setID(int myid) { id = myid; }
    int getID() { return id; }
    void setFd(int my_client_fd) { client_fd = my_client_fd; }
    int getFd() { return client_fd; }
    void setIP(std::string myip) { ip = myip; }
    std::string getIP() { return ip; }
};