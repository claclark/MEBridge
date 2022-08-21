#include <cassert>
#include <fcntl.h>
#include <iostream>
#include <memory>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

#include "lib/mars.h"

void handle_client(int client) {
  size_t size = 64 * 1024;
  std::unique_ptr<char[]> buffer = std::unique_ptr<char[]>(new char[size]);
  int n;
  while ((n = read(client, buffer.get(), size)) > 0)
    assert(write(1, buffer.get(), n) == n);
  assert(n == 0);
  close(client);
}

int main(int argc, char **argv) {
  std::string error;
  std::shared_ptr<mars::Server> server =
      mars::Server::make(mars::mars_port, &error);
  if (server == nullptr) {
    std::cerr << error << "\n";
    return 1;
  }
  int client;
  while ((client = server->client(&error)) != -1) {
    std::thread t(handle_client, client);
    t.detach();
  }
  return 0;
}
