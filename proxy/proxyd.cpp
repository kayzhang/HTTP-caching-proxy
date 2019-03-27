#include "proxyd.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include <sstream>

#define BUF_SIZE 8192

// define a mutex for all the output
pthread_mutex_t output_lock = PTHREAD_MUTEX_INITIALIZER;

proxyd::proxyd(const char * port) {
  snprintf(port_num, sizeof(port_num), "%s", port);
  client_id = 0;
}

void proxyd::run() {
  std::ofstream * log_stream_p = new std::ofstream("/var/log/erss/proxy.log");

  listen_fd = socket_and_bind(port_num);

  if (listen(listen_fd, BACKLOG) == -1) {
    std::perror("listen");
    // std::cout << errno << std::endl;
    std::exit(EXIT_FAILURE);
  }

  while (true) {
    struct sockaddr_storage client_addr;
    socklen_t client_addrLen = sizeof(client_addr);

    // Accept a new connection from the client
    int client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_addrLen);
    if (client_fd == -1) {
      std::perror("accept");
      std::exit(EXIT_FAILURE);
    }

    // Set argument for new thread
    thr_arg * arg = (thr_arg *)malloc(sizeof(*arg));
    arg->id = client_id;
    client_id++;
    arg->client_addr = client_addr;
    arg->client_fd = client_fd;
    arg->log_stream_p = log_stream_p;

    pthread_t thread;
    pthread_create(&thread, NULL, handle_request, arg);

    // Need to join the threads
  }

  close(listen_fd);
}

char * proxyd::get_time(char * curr_time) {
  time_t rawtime;
  struct tm * timeinfo;

  time(&rawtime);
  timeinfo = localtime(&rawtime);
  snprintf(curr_time, 26, "%s", asctime(timeinfo));
  return curr_time;
}

void * proxyd::handle_request(void * varg) {
  thr_arg * arg = (thr_arg *)varg;

  size_t id = arg->id;
  struct sockaddr_storage client_addr = arg->client_addr;
  int client_fd = arg->client_fd;
  std::ofstream * log_stream_f = arg->log_stream_p;

  free(arg);

  char curr_time[26];

  // Get ip address
  char client_ip[INET6_ADDRSTRLEN];
  if (((struct sockaddr *)&client_addr)->sa_family == AF_INET) {
    struct sockaddr_in * addr = (struct sockaddr_in *)&client_addr;
    inet_ntop(AF_INET, &addr->sin_addr, client_ip, sizeof(client_ip));
  }
  else {
    struct sockaddr_in6 * addr = (struct sockaddr_in6 *)&client_addr;
    inet_ntop(AF_INET6, &addr->sin6_addr, client_ip, sizeof(client_ip));
  }

  // Receive request from the client
  char req_buf[BUF_SIZE] = {0};
  ssize_t req_len = recv(client_fd, req_buf, sizeof(req_buf), 0);
  if (req_len == -1) {
    std::perror("recv request from client");
    return NULL;
  }
  if (req_len == 0) {
    pthread_mutex_lock(&output_lock);
    *log_stream_f << id << ": WARNING empty request from " << client_ip << " @ "
                  << get_time(curr_time);
    pthread_mutex_unlock(&output_lock);
    return NULL;
  }

  // Parse the request
  Request req;
  RequestParser::parse_request(req_buf, req_len, req);

  // Output - ID: "REQUEST" from IPFROM @ TIME
  pthread_mutex_lock(&output_lock);
  if (req.method == "GET" || req.method == "POST" || req.method == "CONNECT") {
    *log_stream_f << id << ": \"" << req.method << "\" from " << client_ip << " @ "
                  << get_time(curr_time);
  }
  else {
    *log_stream_f << id << ": invalid method or not supported by our proxy" << std::endl;
  }
  pthread_mutex_unlock(&output_lock);

  /* Debug info
  std::cout << "Request from client\n\n";
  std::cout << req_buf << "\n\n";
  std::cout << "method: " << req.method << std::endl;
  std::cout << "host: " << req.host << std::endl;
  std::cout << "port: " << req.port << std::endl;
  std::cout << "After edit:\n" << req.str << "\n\n";
  */

  // Connect to origin server
  int server_fd = socket_and_connect(req.host.c_str(), req.port.c_str());
  if (server_fd == -1) {
    // std::perror("connect to the server");
    return NULL;
  }

  // Handle different type of request
  if (req.method == "GET" || req.method == "POST") {  // GET or POST
    // Output- ID: Requesting "REQUEST" from SERVER
    pthread_mutex_lock(&output_lock);
    *log_stream_f << id << ": Requesting \"" << req.req_line << "\" from " << req.host << std::endl;
    pthread_mutex_unlock(&output_lock);

    if (send(server_fd, req.str.c_str(), req.str.size(), MSG_NOSIGNAL) == -1) {
      std::perror("GET or POST: send request to server");
      return NULL;
    }

    int mes_len = 0;
    char res_buf[BUF_SIZE] = {0};

    // Receive first response
    mes_len = recv(server_fd, res_buf, sizeof(res_buf), 0);
    send(client_fd, res_buf, mes_len, MSG_NOSIGNAL);

    // Output - ID: Received "RESPONSE" from SERVER
    char * res_line_end = strstr(res_buf, "\r\n");
    *res_line_end = '\0';
    pthread_mutex_lock(&output_lock);
    *log_stream_f << id << ": Received \"" << res_buf << "\" from " << req.host << std::endl;
    pthread_mutex_unlock(&output_lock);

    // Output - ID: Responding "RESPONSE"
    pthread_mutex_lock(&output_lock);
    *log_stream_f << id << ": Responding \"" << res_buf << "\"" << std::endl;
    pthread_mutex_unlock(&output_lock);

    // Receive the following possible data from the server
    while ((mes_len = recv(server_fd, res_buf, sizeof(res_buf), 0)) > 0) {
      send(client_fd, res_buf, mes_len, MSG_NOSIGNAL);
    }

    // Close file descriptors
    if (close(client_fd) == -1 || close(server_fd) == -1) {
      std::perror("close");
      return NULL;
    }

    return NULL;
  }
  else if (req.method == "CONNECT") {  // CONNECT
    // Send "200 OK" to client
    if (send(client_fd, "HTTP/1.1 200 OK\r\n\r\n", 19, 0) == -1) {
      std::perror("CONNECT: sent 200 OK to client");
      return NULL;
    }

    char mes_buf[BUF_SIZE] = {0};  // message buffer

    // Select the readable socket fd
    fd_set readfds;
    FD_ZERO(&readfds);
    int maxfdp1 = 0;  // max fd + 1
    int ready;        // return value of select
    int ready_fd;     // fd of the ready one

    if (server_fd >= FD_SETSIZE || client_fd >= FD_SETSIZE) {
      std::perror("File descriptor exceeds limit FD_SETSIZE");
      return NULL;
    }

    if (server_fd >= maxfdp1) {
      maxfdp1 = server_fd + 1;
    }
    FD_SET(server_fd, &readfds);

    if (client_fd >= maxfdp1) {
      maxfdp1 = client_fd + 1;
    }
    FD_SET(client_fd, &readfds);

    while (true) {
      // Note, if the other end of the socket is closed, it will
      // be counted as valid for select - read return 0 (EOF) immediately.
      fd_set new_set = readfds;
      ready = select(maxfdp1, &new_set, NULL, NULL, NULL);
      if (ready == -1) {
        std::perror("select() failed");
        return NULL;
      }

      // Get the fd that is ready
      if (FD_ISSET(server_fd, &new_set)) {  // first priority (terminating signal)
        ready_fd = server_fd;
      }
      else if (FD_ISSET(client_fd, &new_set)) {
        ready_fd = client_fd;
      }
      else {
        std::perror("Cannot get the ready fd");
        return NULL;
      }

      int other = (ready_fd == client_fd) ? server_fd : client_fd;

      // Receive data from one side
      int mes_len = 0;
      mes_len = recv(ready_fd, mes_buf, sizeof(mes_buf), 0);
      if (mes_len == -1) {
        std::perror("CONNECT: receive data from client or server");
        return NULL;
      }
      if (mes_len == 0) {  // server or client has closed its socket
        break;
      }

      // Send date to the other side
      if (send(other, mes_buf, mes_len, MSG_NOSIGNAL) == -1) {
        std::perror("CONNECT: sent data to client or server");
        return NULL;
      }
    }

    // Output - ID: Tunnel closed
    pthread_mutex_lock(&output_lock);
    *log_stream_f << id << ": Tunnel closed" << std::endl;
    pthread_mutex_unlock(&output_lock);

    // Close file descriptors
    if (close(client_fd) == -1 || close(server_fd) == -1) {
      std::perror("close");
      return NULL;
    }

    return NULL;
  }
  else {
    // Close file descriptors
    if (close(client_fd) == -1) {
      std::perror("close");
      return NULL;
    }

    return NULL;
  }
}

/* This function finds an avaiable address with port_num,
   and returns a socket fd which is bound to port_num */
int proxyd::socket_and_bind(const char * port_num) {
  /* Get the listenable address list with port_num */

  int status;
  int listen_fd;
  struct addrinfo hints;
  struct addrinfo * addr_info_list;  // head of the addr info list

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;      // SearchingIPv4 and IPv6
  hints.ai_socktype = SOCK_STREAM;  // TCP
  hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;

  status = getaddrinfo(NULL, port_num, &hints, &addr_info_list);
  if (status != 0) {
    return -1;
  }

  /* Iterate the returned address list to find one using which we
     can successfully create and bind a socket */

  struct addrinfo * curr;
  for (curr = addr_info_list; curr != NULL; curr = curr->ai_next) {
    listen_fd = socket(curr->ai_family, curr->ai_socktype, curr->ai_protocol);
    if (listen_fd == -1) {
      continue;  // Create socket failed, try next address
    }

    int yes = 1;
    status = setsockopt(listen_fd, SOL_SOCKET | SO_REUSEADDR, 0, &yes, sizeof(yes));
    if (yes == -1) {
      freeaddrinfo(addr_info_list);
      if (close(listen_fd) == -1) {
        std::perror("close");
        return -1;
      }
      return -1;
    }

    status = bind(listen_fd, curr->ai_addr, curr->ai_addrlen);
    if (status == 0) {
      break;  // bind() succeed
    }
    else {  // bind() failed, close the socket and try next address
      if (close(listen_fd) == -1) {
        std::perror("close");
        return -1;
      }
    }
  }
  freeaddrinfo(addr_info_list);  // Free dynamically allocated space

  if (curr == NULL) {
    return -1;
  }

  return listen_fd;
}

/* This function finds an avaiable server address with host and port_num,
   and returns a socket fd which connects to the address */
int proxyd::socket_and_connect(const char * host, const char * port) {
  /* Get the connectable address list with host & port */

  int status;
  int server_fd;
  struct addrinfo hints;
  struct addrinfo * addr_info_list;  // head of the addr info list

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;      // Searching IPv4 and IPv6
  hints.ai_socktype = SOCK_STREAM;  // TCP
  hints.ai_flags = AI_NUMERICSERV;

  status = getaddrinfo(host, port, &hints, &addr_info_list);
  if (status != 0) {
    return -1;
  }

  /* Iterate the returned address list to find one to which we                
    one successfully connect */

  struct addrinfo * curr;
  for (curr = addr_info_list; curr != NULL; curr = curr->ai_next) {
    server_fd = socket(curr->ai_family, curr->ai_socktype, curr->ai_protocol);
    if (server_fd == -1) {
      continue;  // Create socket failed, try next address
    }

    status = connect(server_fd, curr->ai_addr, curr->ai_addrlen);
    if (status == 0) {
      break;  // connect() succeed
    }
    else {  // connect() failed, close the socket and try next address
      if (close(server_fd) == -1) {
        std::perror("close");
        return -1;
      }
    }
  }
  freeaddrinfo(addr_info_list);  // Free dynamically allocated space

  if (curr == NULL) {
    return -1;
  }

  return server_fd;
}
