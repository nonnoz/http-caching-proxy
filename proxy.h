#include "tcp.h"
#include "client.h"
#include "request.h"

#include "httprequestparser.h"
#include "urlparser.h"
#include "response.h"
#include "httpresponseparser.h"

// #include <stdio.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

// #include <exception>
#include <mutex>
// #include <netdb.h>
// #include <netinet/in.h>
// #include <sys/select.h>
#include <pthread.h>
// #include <unistd.h>
// #include <string.h>
// #include <iostream>
// #include <cstring>
// #include <arpa/inet.h>
#include <fstream>
// #include <ctime>
#include <unordered_map>


void *proxy_begin(Client *myclient);
std::string getTime();

void get_cache
