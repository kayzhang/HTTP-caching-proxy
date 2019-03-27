#ifndef HEADER_H
#define HEADER_H

#include <cstdio>
#include <string>

struct Header_t {
  std::string name;
  std::string value;
};
typedef struct Header_t Header;

#endif
