/*
 * Copyright (C) Alex Nekipelov (alex@nekipelov.net)
 * License: MIT
 */

#ifndef HTTPPARSER_RESPONSE_H
#define HTTPPARSER_RESPONSE_H

#include <string>
#include <unordered_map>
#include <sstream>

namespace httpparser
{

struct Response {
    Response()
        : versionMajor(0), versionMinor(0), keepAlive(false), statusCode(0)
    {}
    
    struct HeaderItem
    {
        std::string name;
        std::string value;
    };

    int versionMajor;
    int versionMinor;
    std::unordered_map<std::string, std::string> headers;
    //std::vector<char> content;
    std::string content;
    bool keepAlive;
    
    unsigned int statusCode;
    std::string status;

    std::string inspect() const
    {
        std::stringstream stream;
        stream << "HTTP/" << versionMajor << "." << versionMinor
               << " " << statusCode << " " << status << "\r\n";

        for(auto it = headers.begin();
            it != headers.end(); ++it)
        {
            stream << it->first << ": " << it->second << "\r\n";
        }
        stream << "Content-Length:" << std::to_string(content.size()) << "\r\n\r\n";
        //std::string data(content.begin(), content.end());
        //stream << data << "\n";
        stream << content << "\r\n";
        return stream.str();
    }
};

} // namespace httpparser

#endif // HTTPPARSER_RESPONSE_H

