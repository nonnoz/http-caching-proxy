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
#include <unordered_map>

#include "tcp.hpp"
#include "client.hpp"
#include <request.h>
#include <httprequestparser.h>
#include <urlparser.h>
#include <response.h>
#include <httpresponseparser.h>

void proxy_begin(Client myclient){
    std::unordered_map<std::string, Response> Cache;
    char myrequest[512];
        int myrequest_len = recv(myclient.getFd(), myrequest, sizeof(response), 0);
        Request request;
        HttpRequestParser parser;
        HttpRequestParser::ParseResult res = parser.parse(request, myrequest, myrequest + sizeof(buffer));
        if( res == HttpRequestParser::ParsingCompleted ){
            if (request.method.compare("POST") == 0 || request.method.compare("CONNECT") == 0 || request.method.compare("GET") == 0){
                pthread_mutex_lock(&mutex);
                logFile << myclient.getID() << ": \"" << request.line << "\" from "
                    << myclient.getIP() << " @ " << getTime().append("\0");
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
                    logFile << myclient.getID() << ": "<< "Requesting \"" << request.line() << "\" from " << host() << std::endl;
                    pthread_mutex_unlock(&mutex);
                    send(server_fd, myrequest, myrequest_len, MSG_NOSIGNAL);
                    char myresponse[512];
                    int myresponse_len = recv(server_fd,myresponse,sizeof(myresponse),MSG_WAITALL);
                    if (myresponse_len > 0){
                        Response response;
                        HttpResponseParser parser;
                        parser.parse(response, myresponse, myresponse + sizeof(myresponse));
                        pthread_mutex_lock(&mutex);
                        logFile << myclient.getID() << ": Received \"" << response.line() << "\" from " << host() << std::endl;
                        pthread_mutex_unlock(&mutex);
                        send(myclient.getFd(), myresponse, myresponse_len, MSG_NOSIGNAL);
                        pthread_mutex_lock(&mutex);
                        logFile << myclient.getID() << ": Responding \"" << response.line() << std::endl;
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
                    if (auto search = Cache.find(request.line); search != example.end()){
                        //in cache

                    }
                    else{
                        //not in cache
                        pthread_mutex_lock(&mutex);
                        logFile << myclient.getID() << ": not in cache" << std::endl;
                        pthread_mutex_unlock(&mutex);
                        pthread_mutex_lock(&mutex);
                        logFile << myclient.getID() << ": "
                                << "Requesting \"" << request.line() << "\" from " << host << std::endl;
                        pthread_mutex_unlock(&mutex);
                        send(myclient.getFd(), myresponse, myresponse_len, MSG_NOSIGNAL);
                        send(server_fd, myrequest, myrequest_len, MSG_NOSIGNAL);
                        char myresponse[512];
                        int myresponse_len = recv(server_fd,myresponse,sizeof(myresponse),MSG_WAITALL);
                        if (myresponse_len > 0){
                            Response response;
                            HttpResponseParser parser;
                            parser.parse(response, myresponse, myresponse + sizeof(myresponse));
                            string Line = response.line;
                            if (Line.find(502) != std::string::npos ){
                                string bad_gateway = "HTTP/1.1 502 Bad Gateway";
                                send(client_fd,bad_gateway.c_str(),bad_gateway.size(),MSG_NOSIGNAL);
                            }
                            else{
                                //send message
                                std::string str_myresponse = myresponse;
                                if (str_myresponse.find(chunk)){
                                    pthread_mutex_lock(&mutex);
                                    logFile << myclient.getID() << ": not cacheable because chunk" << std::endl;
                                    pthread_mutex_unlock(&mutex);
                                    send(myclient.getFD(), myresponse, str_myresponse.len(), 0);   
                                    char chunked_msg[28000] = {0};
                                    while (1) {  //receive and send remaining message
                                    int len = recv(server_fd, chunked_msg, sizeof(chunked_msg), 0);
                                    if (len <= 0) {
                                        std::cout << "chunked break\n";
                                        break;
                                    }
                                    send(myclient.getFD(), chunked_msg, len, 0);
                                    }
                                }
                                else{
                                    send(myclient.getFd(), myresponse, myresponse_len, MSG_NOSIGNAL);

                                pthread_mutex_lock(&mutex);
                                logFile << myclient.getID() << ": Received \"" << response.line() << "\" from " << host() << std::endl;
                                pthread_mutex_unlock(&mutex);
                                int ifcache = 1;
                                for(std::vector<Response::HeaderItem>::const_iterator it = response.headers.begin();it != response.headers.end(); ++it){
                                    if (it->name.compare("Cache-Control") == 0){
                                        if (it->value.compare("no-store")){
                                            pthread_mutex_lock(&mutex);
                                            logFile << myclient.getID() << ": not cacheable because no-store" << std::endl;
                                            pthread_mutex_unlock(&mutex);
                                            ifcache = 0;
                                        }
                                        else if (it->value.compare("private") == 0){
                                            pthread_mutex_lock(&mutex);
                                            logFile << myclient.getID() << ": not cacheable because private" << std::endl;
                                            pthread_mutex_unlock(&mutex);
                                            ifcache = 0;
                                        }
                                        else if (it->value.compare("no-cache") == 0){
                                            pthread_mutex_lock(&mutex);
                                            logFile << myclient.getID() << ":  NOTE Cache-Control: must-revalidate" << std::endl;
                                            pthread_mutex_unlock(&mutex);
                                            response.revalidate = 1;
                                        }
                                        else if (it->value.find("max-age") != std::string::npos){
                                            pthread_mutex_lock(&mutex);
                                            logFile << myclient.getID() << ":  NOTE Cache-Control: must-revalidate" << std::endl;
                                            pthread_mutex_unlock(&mutex);
                                            response.revalidate = 1;
                                        }
                                    }
                                    if (it->name.compare("ETag") == 0){
                                        pthread_mutex_lock(&mutex);
                                        logFile << myclient.getID() << ":  NOTE ETag: W/" << it->value<<std::endl;
                                        pthread_mutex_unlock(&mutex);
                                        response.revalidate = 1;                         
                                }
                            }
                            if (iscache == 1){
                                Cache.push_back(request.line(),response);
                            }
                                }
                    }
                    }
                } 
            }
            else{
                //400 code
                string bad = "HTTP/1.1 400 Bad Request";
                send(client_fd,bad.c_str(),bad.size(),MSG_NOSIGNAL);
                pthread_mutex_lock(&mutex);
                logFile << myclient.getID() << ": Responding \"HTTP/1.1 400 Bad Request\"" << std::endl;
                pthread_mutex_unlock(&mutex);
            }  
        }
        else{
            //400 code
            string bad = "HTTP/1.1 400 Bad Request";
            send(client_fd,bad.c_str(),bad.size(),MSG_NOSIGNAL);
            pthread_mutex_lock(&mutex);
            logFile << myclient.getID() << ": Responding \"HTTP/1.1 400 Bad Request\"" << std::endl;
            pthread_mutex_unlock(&mutex);
        }
        close(server_fd);
        close(myclient.getFd());
}