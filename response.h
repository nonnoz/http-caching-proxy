/*
 * Copyright (C) Alex Nekipelov (alex@nekipelov.net)
 * License: MIT
 */

#ifndef HTTPPARSER_RESPONSE_H
#define HTTPPARSER_RESPONSE_H

#include <string>
#include <vector>
#include <sstream>


struct Response {
    Response() : versionMajor(0), versionMinor(0), keepAlive(false), statusCode(0), revalidate(0){}
    
    struct HeaderItem
    {
        std::string name;
        std::string value;
    };

    int versionMajor;
    int versionMinor;
    std::vector<HeaderItem> headers;
    std::vector<char> content;
    bool keepAlive;
    int revalidate;
    
    unsigned int statusCode;
    std::string status;

    std::string getLine() const
    {
        std::stringstream stream;
        stream << "HTTP/" << versionMajor << "." << versionMinor
               << " " << statusCode << " " << status << "\n";
        return stream.str();
    /*    for(std::vector<Response::HeaderItem>::const_iterator it = headers.begin();
            it != headers.end(); ++it)
        {
            stream << it->name << ": " << it->value << "\n";
        }

        std::string data(content.begin(), content.end());
        stream << data << "\n";
        return stream.str();*/
    }

    std::string getResponse(){
        std::stringstream stream;
        stream << "HTTP/" << versionMajor << "." << versionMinor
               << " " << statusCode << " " << status << "\n";
        return stream.str();
        for(std::vector<Response::HeaderItem>::const_iterator it = headers.begin();
            it != headers.end(); ++it)
        {
            stream << it->name << ": " << it->value << "\n";
        }

        std::string data(content.begin(), content.end());
        stream << data << "\n";
        return stream.str();
    }
};

// }  namespace httpparser

#endif // HTTPPARSER_RESPONSE_H

