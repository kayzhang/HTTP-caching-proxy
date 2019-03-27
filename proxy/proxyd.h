#ifndef DAEMON_H
#define DAEMON_H

#include <netdb.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "RequestHandler.h"
#include "RequestParser.h"

#define BACKLOG 50

extern pthread_mutex_t output_lock;

class proxyd
{
 public:
  // Constructor with the host and service of the proxy
  explicit proxyd(const char * port);

  // Run the proxy daemon
  void run();

 private:
  int listen_fd;

  char port_num[NI_MAXSERV];

  size_t client_id;

  // static std::ofstream logfile;

  // The handler for imcoming requests
  RequestHandler request_handler;

  /* This function finds an avaiable address with port_num,
     and returns a socket fd which is bound to port_num */
  int socket_and_bind(const char * port_num);

  /* This function finds an avaiable server address with host and port_num,
     and returns a socket fd which connects to the address */
  static int socket_and_connect(const char * host, const char * port);

  // Accept a connection, spawn a new thread and then continue to accept
  static void * handle_request(void * varg);

  // Get current time
  static char * get_time(char * curr_time);
};

typedef struct _thr_arg {
  size_t id;
  struct sockaddr_storage client_addr;
  int client_fd;
  std::ofstream * log_stream_p;
} thr_arg;

#endif
