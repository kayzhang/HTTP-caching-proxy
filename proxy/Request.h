#ifndef REQUEST_H
#define REQUEST_H
#include <cstdio>
#include <string>
#include <vector>

#include "Header.h"

struct Request_t {
  std::string req_line;  // GET http://www.google.com/index.html HTTP/1.1
  std::string method;    // GET/POST/CONNECT
  std::string host;      // google.com
  std::string port;      // 80/443
  std::string str;       // the whole request which should be modified

  /*********************************************
   * Not using the following fields in this project
   * std::string protocal;         // Http
   * std::string path;             // /index.com
   * std::string version;          // 1.0/1.1
   * std::vector<Header> headers;  // name: value...
   * std::string content;          // bala bala...
  */
};
typedef struct Request_t Request;

#endif
