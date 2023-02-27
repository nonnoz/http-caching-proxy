#include "client.h"

int Client::id = -1;

Client::Client(){
    id++;
    Client::setFd(0);
}

