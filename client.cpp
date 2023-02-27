#include "client.h"

int Client::id = -1;

Client::Client(){
    id++;
    client_fd = 0;
}
Client::~Client(){
    close(client_fd);
}
