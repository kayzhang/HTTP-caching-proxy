#ifndef RESPONSE_H
#define RESPONSE_H
#include <cstdio>
#include <string>
#include <vector>

#include "Header.h"

struct Response_t {
  std::string version;
  std::string status_code;
  std::string status_phrase;
  std::vector<Header> headers;
  std::string content;
};
typedef struct Response_t Response;

#endif
