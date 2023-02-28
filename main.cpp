#include <exception>
#include <mutex>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <pthread.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <fstream>

#include "tcp.h"
#include "client.h"
#include "proxy.h"

using namespace std;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
std::ofstream logFile("/var/log/erss/proxy.log");

int main(){
    const char * port = "12345";
    int socket_fd = create_server(NULL, port);
    while(1){
        struct sockaddr_storage socket_addr;
        socklen_t socket_addr_len = sizeof(socket_addr);
        Client client;
        int temp = accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
        client.setFd(temp);
        if (client.getFd() == -1) {
            cerr << "Error: cannot accept connection on socket" << endl;
            //return -1;
            continue;
        } //if
        struct sockaddr_in * addr = (struct sockaddr_in *)&socket_addr;
        client.setIP(inet_ntoa(addr->sin_addr));
        //thread to begin our proxy
        pthread_t thread;
        pthread_create(&thread, NULL, proxy_begin, &client);
    }
    
    close(socket_fd);
    return -1;
}