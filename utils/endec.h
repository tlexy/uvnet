#ifndef ENDEC_SSH_H
#define ENDEC_SSH_H

#include <string>
#include <cstdlib>
#include <cstring>

/*
* 基于openssl的功能库
*/

std::string base64Decode(const char *input, int length, bool newLine);
std::string base64Encode(const char* buff, int len);

std::string sha1(const std::string& text);

#endif