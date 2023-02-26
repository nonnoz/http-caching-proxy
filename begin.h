#include <exception>
#include <mutex>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <pthread>
#include <unistd.h>
#include <string>
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <fstream>
#include<ctime>

#include "tcp.hpp"
#include "client.hpp"
#include <request.h>
#include <httprequestparser.h>
#include <urlparser.h>
#include <response.h>
#include <httpresponseparser.h>

void proxy_begin(Client myclient){
    char myrequest[512];
        int myrequest_len = recv(myclient.getFd(), myrequest, sizeof(response), 0);
        Request request;
        HttpRequestParser parser;
        HttpRequestParser::ParseResult res = parser.parse(request, myrequest, myrequest + sizeof(buffer));
        if( res == HttpRequestParser::ParsingCompleted ){
            if (request.method.compare("POST") == 0 || request.method.compare("CONNECT") == 0 || request.method.compare("GET") == 0){
                pthread_mutex_lock(&mutex);
                logFile << myclient_info->getID() << ": \"" << request.line << "\" from "
                    << myclient_info->getIP() << " @ " << getTime().append("\0");
                pthread_mutex_unlock(&mutex);
                const char * url = request.uri.c_str();
                UrlParser parser_u;
                if( parser_u.parse(url) ){
                    host = parser.hostname();
                    port = parser.port();
                    //need test
                    //default 80
                    if (port == ""){
                        port = "80";
                    }
                }
                else{
                    std::cerr << "Can't parse url: " << request.uri << std::endl;
                }
                int server_fd = build_client(host, port);   
                //post
                if ( request.method.compare("POST") == 0){
                    pthread_mutex_lock(&mutex);
                    logFile << myclient_info->getID() << ": "<< "Requesting \"" << request.line() << "\" from " << host() << std::endl;
                    pthread_mutex_unlock(&mutex);
                    send(server_fd, myrequest, myrequest_len, MSG_NOSIGNAL);
                    char myresponse[512];
                    int myresponse_len = recv(server_fd,myresponse,sizeof(myresponse),MSG_WAITALL);
                    if (myresponse_len > 0){
                        Response response;
                        HttpResponseParser parser;
                        parser.parse(response, myresponse, myresponse + sizeof(myresponse));
                        pthread_mutex_lock(&mutex);
                        logFile << myclient_info->getID() << ": Received \"" << response.line() << "\" from " << host() << std::endl;
                        pthread_mutex_unlock(&mutex);
                        send(myclient.getFd(), myresponse, myresponse_len, MSG_NOSIGNAL);
                        pthread_mutex_lock(&mutex);
                        logFile << myclient_info->getID() << ": Responding \"" << response.line() << std::endl;
                        pthread_mutex_unlock(&mutex);
                    }
                    else{
                        std::cout << "server socket closed!\n";
                    }
                }    
                //connect
                else if ( request.method.compare("CONNECT") == 0){

                }
                //get
                else{

                } 
            }
            else{
                //400 code
                pthread_mutex_lock(&mutex);
                logFile << client_info->getID() << ": Responding \"HTTP/1.1 400 Bad Request\"" << std::endl;
                pthread_mutex_unlock(&mutex);
            }  
        }
        else{
            //400 code
            pthread_mutex_lock(&mutex);
            logFile << client_info->getID() << ": Responding \"HTTP/1.1 400 Bad Request\"" << std::endl;
            pthread_mutex_unlock(&mutex);
        }
        close(server_fd);
        close(myclient.getFd());
}