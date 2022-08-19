#include <iostream>
#include <memory>
#include <string>

#include "lib/mars.h"

int main(int argc, char **argv) {
  std::string error;

  std::shared_ptr<mars::Server> server =
      mars::Server::make(mars::mars_port, &error);
  if (server == nullptr) {
    std::cerr << error << "\n";
    return 1;
  }
  return 0;
}
