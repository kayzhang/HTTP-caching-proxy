#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <cstdio>
#include <string>

#include "Request.h"
#include "Response.h"

class RequestHandler
{
  // Process a request and return a reply
  void handle_request(const Request & req, Response & rep);
};

#endif
