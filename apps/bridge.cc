#include <iostream>
#include <memory>
#include <string>
#include <unistd.h>

#include "lib/mars.h"

int main(int argc, char **argv) {
  std::string error;

  std::shared_ptr<mars::Server> server =
      mars::Server::make(mars::mars_port, &error);
  if (server == nullptr) {
    std::cerr << error << "\n";
    return 1;
  }
  int client;
  while ((client = server->client()) != -1) {
    close(client);
    std::cout << "pong\n";
  }
  return 0;
}
