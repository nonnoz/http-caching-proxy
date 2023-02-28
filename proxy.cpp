#include "begin.h"

void *proxy_begin(Client *myclient)
{
    std::unordered_map<std::string, Response> Cache;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    /*
     TODO: log file path
    */
    std::ofstream logFile("/log/proxy.log");

    char myrequest[65536] = {0};
    int myrequest_len = recv(myclient->getFd(), myrequest, sizeof(myrequest), 0);
    if (myrequest_len <= 0)
    {
        pthread_mutex_lock(&mutex);
        logFile << myclient->getId() << ": Error Invalid Request." << std::endl;
        pthread_mutex_unlock(&mutex);
        return NULL;
    }
    Request request;
    HttpRequestParser parser;
    HttpRequestParser::ParseResult res = parser.parse(request, myrequest, myrequest + sizeof(myrequest));
    if (res == HttpRequestParser::ParsingCompleted)
    {
        pthread_mutex_lock(&mutex);
        logFile << myclient->getId() << ": \"" << request.getLine() << "\" from " << myclient->getIP() << " @ " << getTime() << std::endl;
        pthread_mutex_unlock(&mutex);

        const char *url = request.uri.c_str();
        UrlParser parser_u;
        if (parser_u.parse(url))
        {
            host = parser.hostname();
            port = parser.port();
            // need test
            // default 80
            if (port == "")
            {
                port = "80";
            }
        }
        else
        {
            std::cerr << "Can't parse url: " << request.uri << std::endl;
        }

        int server_fd = create_client(host, port);
        if (request.method.compare("POST") == 0 || request.method.compare("CONNECT") == 0 || request.method.compare("GET") == 0)
        {
            // post
            if (request.method.compare("POST") == 0)
            {
                pthread_mutex_lock(&mutex);
                logFile << myclient->getId() << ": "
                        << "Requesting \"" << request.getLine() << "\" from " << host() << std::endl;
                pthread_mutex_unlock(&mutex);
                send(server_fd, myrequest, myrequest_len, MSG_NOSIGNAL);
                char myresponse[10000] = {0};
                int myresponse_len = recv(server_fd, myresponse, sizeof(myresponse), 0);
                if (myresponse_len > 0)
                {
                    Response response;
                    HttpResponseParser parser;

                    parser.parse(response, myresponse, myresponse + sizeof(myresponse));
                    pthread_mutex_lock(&mutex);
                    logFile << myclient->getId() << ": Received \"" << response.getLine() << "\" from " << host() << std::endl;
                    pthread_mutex_unlock(&mutex);
                    send(myclient->getFd(), myresponse, myresponse_len, MSG_NOSIGNAL);
                    pthread_mutex_lock(&mutex);
                    logFile << myclient->getId() << ": Responding \"" << response.getLine() << std::endl;
                    pthread_mutex_unlock(&mutex);
                }
            }
            // connect
            else if (request.method.compare("CONNECT") == 0)
            {
                pthread_mutex_lock(&mutex);
                logFile << myclient_info->getID() << ": "
                        << "Requesting \"" << request.line() << "\" from " << host() << std::endl;
                pthread_mutex_unlock(&mutex);

                send(myclient->getFd(), "HTTP/1.1 200 OK\r\n\r\n", 19, 0);
                fd_set readfds;

                int nfds = max_element(myclient->getFd(), server_fd) + 1;

                while (true)
                {
                    FD_ZERO(&readfds);
                    FD_SET(myclient->getFd(), &readfds);
                    FD_SET(server_fd, &readfds);

                    int select_result = select(nfds, &readfds, NULL, NULL, NULL);
                    if (select_result == -1)
                    {
                        std::cerr << "Selection failed" << std::endl;
                        return;
                    }

                    int recv_size = 0;
                    int connect_fds[2] = {myclient->getFd(), server_fd};
                    for (int i = 0; i < 2; i++)
                    {
                        char msg[65536] = {0};
                        if (FD_ISSET(connect_fds[i], &readfds))
                        {
                            recv_size = recv(connect_fds[i], msg, sizeof(msg), 0);
                            if (recv_size == 0)
                            {
                                return;
                            }
                            else if (recv_size == -1)
                            {
                                std::cerr << "Receive failed" << std::endl;
                                return;
                            }
                            else
                            {
                                if (send(connect_fds[(i + 1) % 2], msg, recv_size, 0) <= 0)
                                {
                                    return;
                                }
                            }
                        }
                    }

                    pthread_mutex_lock(&mutex);
                    logFile << myclient_info->getID() << ": Tunnel closed" << std::endl;
                    pthread_mutex_unlock(&mutex);
                }
                // get
                else
                {
                    if (Cache.find(request.line) != Cache.end())
                    {
                        // in cache
                        /*revalidate:
                            Here, the response indicates the response you find in the cache
                                    for(std::vector<Response::HeaderItem>::const_iterator it = response.headers.begin();it != response.headers.end(); ++it){
                                        if (it->name.compare("Cache-Control") == 0){
                                            if (it->value.compare("no-cache") == 0){
                                                resend request to the server
                                            }
                                            else if (it->value.find("max-age") != std::string::npos){
                                                if (it->value.compare("max-age=0") == 0){resend request to the server}
                                                else{send the response in the cache}
                                            }
                                        }
                                        if (it->name.compare("ETag") == 0){
                                            add If-None-Match header in the request and send it
                                        }
                                        if (it->name.compare("Last-Modified") == 0){
                                            add If-Modified-Since header in the request and send it
                                        }
                                        if (it->name.compare("Expires") == 0){
                                            compare the it->value with the 'now' time, if < ,not fresh, send to server
                                            else, fresh, send the response in the cache
                                        }
                                    }
                        */
                    }
                    else
                    {
                        // not in cache
                        // or in cache but invalid
                        pthread_mutex_lock(&mutex);
                        logFile << myclient->getId() << ": not in cache" << std::endl;
                        pthread_mutex_unlock(&mutex);
                        pthread_mutex_lock(&mutex);
                        logFile << myclient->getId() << ": "
                                << "Requesting \"" << request.line() << "\" from " << host << std::endl;
                        pthread_mutex_unlock(&mutex);
                        send(server_fd, myrequest, myrequest_len, MSG_NOSIGNAL);
                        char myresponse[10000] = {0};
                        int myresponse_len = recv(server_fd, myresponse, sizeof(myresponse), 0);
                        if (myresponse_len > 0)
                        {
                            Response response;
                            HttpResponseParser parser;
                            parser.parse(response, myresponse, myresponse + sizeof(myresponse));
                            string Line = response.line;
                            if (Line.find(502) != std::string::npos)
                            {
                                string bad_gateway = "HTTP/1.1 502 Bad Gateway";
                                send(client_fd, bad_gateway.c_str(), bad_gateway.size(), 0);
                            }
                            else
                            {
                                // send message
                                std::string str_myresponse = myresponse;
                                if (str_myresponse.find("chunked") != std::string::npos)
                                {
                                    pthread_mutex_lock(&mutex);
                                    logFile << myclient->getId() << ": not cacheable because chunk" << std::endl;
                                    pthread_mutex_unlock(&mutex);
                                    send(myclient->getFD(), myresponse, myresponse_len, 0);
                                    char chunked_msg[10000] = {0};
                                    while (1)
                                    {
                                        int len = recv(server_fd, chunked_msg, sizeof(chunked_msg), 0);
                                        if (len <= 0)
                                        {
                                            std::cout << "chunked break\n";
                                            break;
                                        }
                                        send(myclient->getFD(), chunked_msg, len, 0);
                                    }
                                }
                                else
                                {
                                    send(myclient->getFd(), myresponse, myresponse_len, 0);

                                    pthread_mutex_lock(&mutex);
                                    logFile << myclient->getId() << ": Received \"" << response.line() << "\" from " << host << std::endl;
                                    pthread_mutex_unlock(&mutex);
                                    int ifcache = 1;
                                    for (std::vector<Response::HeaderItem>::const_iterator it = response.headers.begin(); it != response.headers.end(); ++it)
                                    {
                                        if (it->name.compare("Cache-Control") == 0)
                                        {
                                            if (it->value.compare("no-store"))
                                            {
                                                pthread_mutex_lock(&mutex);
                                                logFile << myclient->getId() << ": not cacheable because no-store" << std::endl;
                                                pthread_mutex_unlock(&mutex);
                                                ifcache = 0;
                                            }
                                            else if (it->value.compare("private") == 0)
                                            {
                                                pthread_mutex_lock(&mutex);
                                                logFile << myclient->getId() << ": not cacheable because private" << std::endl;
                                                pthread_mutex_unlock(&mutex);
                                                ifcache = 0;
                                            }
                                            else if (it->value.compare("no-cache") == 0)
                                            {
                                                pthread_mutex_lock(&mutex);
                                                logFile << myclient->getId() << ":  NOTE Cache-Control: must-revalidate" << std::endl;
                                                pthread_mutex_unlock(&mutex);
                                                pthread_mutex_lock(&mutex);
                                                logFile << myclient->getId() << ":  Cached, but requires re-validation" << std::endl;
                                                pthread_mutex_unlock(&mutex);
                                            }
                                            else if (it->value.find("max-age") != std::string::npos)
                                            {
                                                pthread_mutex_lock(&mutex);
                                                logFile << myclient->getId() << ":  NOTE Cache-Control: must-revalidate" << std::endl;
                                                pthread_mutex_unlock(&mutex);
                                            }
                                        }
                                        if (it->name.compare("ETag") == 0)
                                        {
                                            pthread_mutex_lock(&mutex);
                                            logFile << myclient->getId() << ":  NOTE ETag: " << it->value << std::endl;
                                            pthread_mutex_unlock(&mutex);
                                        }
                                        if (it->name.compare("Last-Modified") == 0)
                                        {
                                            pthread_mutex_lock(&mutex);
                                            logFile << myclient->getId() << ":  NOTE Last-Modified: " << it->value << std::endl;
                                            pthread_mutex_unlock(&mutex);
                                        }
                                        if (it->name.compare("Expires") == 0)
                                        {
                                            pthread_mutex_lock(&mutex);
                                            logFile << myclient->getId() << ": Cached, expires at" << it->value << std::endl;
                                            pthread_mutex_unlock(&mutex);
                                        }
                                    }
                                    if (iscache == 1)
                                    {
                                        Cache[request.line] = response;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                // 400 code
                string bad = "HTTP/1.1 400 Bad Request";
                send(client_fd, bad.c_str(), bad.size(), MSG_NOSIGNAL);
                pthread_mutex_lock(&mutex);
                logFile << myclient->getId() << ": Responding \"HTTP/1.1 400 Bad Request\"" << std::endl;
                pthread_mutex_unlock(&mutex);
                return NULL;
            }
        }
        else
        {
            // 400 code
            string bad = "HTTP/1.1 400 Bad Request";
            send(client_fd, bad.c_str(), bad.size(), MSG_NOSIGNAL);
            pthread_mutex_lock(&mutex);
            logFile << myclient->getId() << ": Responding \"HTTP/1.1 400 Bad Request\"" << std::endl;
            pthread_mutex_unlock(&mutex);
            return NULL;
        }
        close(server_fd);
        close(myclient->getFd());
    }
}

std::string getTime()
{
    time_t now = time(0);
    char *dt = ctime(&now);
    tm *gmtm = gmtime(&now);
    dt = asctime(gmtm);
    return std::string(dt);
}