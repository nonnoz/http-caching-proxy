#include <exception>
#include <mutex>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <thread>
#include <unistd.h>
#include <string>
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <fstream>

#include "tcp.hpp"
#include "client.hpp"

using namespace std;

std::ofstream log("/var/log/erss/proxy.log");

int main(){
    const char * port = "12345";
    Client myclient = create_server(NULL, port);
    while(1){
        struct sockaddr_storage socket_addr;
        socklen_t socket_addr_len = sizeof(socket_addr);
        Client myclient;
        int temp = accept(server_proxy_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
        myclient.setFd(temp);
        if (myclient.getFd() == -1) {
            cerr << "Error: cannot accept connection on socket" << endl;
            //return -1;
            continue;
        } //if
        struct sockaddr_in * addr = (struct sockaddr_in *)&socket_addr;
        myclient.setIP(inet_ntoa(addr->sin_addr));
        //handle everything
        //post

        //connect

        //get
    }
    
    freeaddrinfo(host_info_list);
    close(server_proxy_fd);
    return -1;
}