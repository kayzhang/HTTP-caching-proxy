#include "RequestParser.h"

#include <cstring>
#include <sstream>

#include "Header.h"

// Parser a request string to a Request object
void RequestParser::parse_request(const char * raw_req, int buf_len, Request & req) {
  req.str = std::string(raw_req, buf_len);

  // get req_line
  req.req_line = req.str.substr(0, req.str.find("\r\n"));

  // get method
  req.method = req.str.substr(0, req.str.find(" "));

  // get host
  size_t host_first = req.str.find("Host: ") + 6;
  size_t host_next = req.str.find_first_of(":\r", host_first);
  req.host = req.str.substr(host_first, host_next - host_first);

  // get port
  if (req.str[host_next] == ':') {
    size_t port_next = req.str.find("\r\n", host_next);
    req.port = req.str.substr(host_next + 1, port_next - host_next - 1);
  }
  else {
    req.port = "80";
  }

  // modify str, here we just change "Connection: keep-alive"
  // to "Connection: close"
  // Note, for CONNECT we do not need to do this.
  if (req.method != "CONNECT") {
    size_t conn_first = req.str.find("Connection: keep-alive\r\n");
    if (conn_first != std::string::npos) {
      req.str.replace(conn_first + 12, 10, "close");
    }
  }
}
