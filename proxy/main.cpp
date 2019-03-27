#include <cstdlib>

#include "proxyd.h"

int main(void) {
  // Construct the proxy daemon
  proxyd proxy("12345");

  // Run the proxy daemon
  proxy.run();
}
