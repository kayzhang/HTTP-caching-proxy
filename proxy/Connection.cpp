#include "Connection.h"

#include <iostream>

void Connection::do_read() {
  char buffer[1000];
  if (recv(fd_socket, buffer, sizeof(buffer), 0) != -1) {
    std::cout << buffer << std::endl;
  }
}
