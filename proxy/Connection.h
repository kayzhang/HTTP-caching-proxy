#ifndef CONNECTION_H
#define CONNECTION_H

#include <cstdio>
#include <string>
#include <sys/socket.h>
#include <iostream>

#include "Request.h"
#include "RequestHandler.h"
#include "Response.h"

class Connection
{
  explicit Connection(int fd, RequestHandler& rh) : fd_socket(fd), request_handler(rh) {}

 private:
  void do_read();
  void do_write();

  int fd_socket;
  RequestHandler request_handler;
  Request request;
  // RequestParser request_parser;
  Response response;
};

#endif
