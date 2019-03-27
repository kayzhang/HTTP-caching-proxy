#ifndef REQUEST_PARSER_H
#define REQUEST_PARSER_H

#include <cstdio>
#include <string>

#include "Request.h"

class RequestParser
{
 public:
  // Parser a request string to a Request object
  static void parse_request(const char * buf, int buf_len, Request & req);
};

#endif
