#include "client.hpp"

int Client::id = -1;

Client::Client(){
    id++;
    client_fd = 0;
}
