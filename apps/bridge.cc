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

void handle_client(int client, std::shared_ptr<mars::Temps> temps) {
  std::string temp = temps->temp();
  int fd = open(temp.c_str(), O_WRONLY | O_CREAT, 0666);
  assert(fd != -1);
  size_t size = 64 * 1024;
  std::unique_ptr<char[]> buffer = std::unique_ptr<char[]>(new char[size]);
  int n;
  while ((n = read(client, buffer.get(), size)) > 0)
    assert(write(fd, buffer.get(), n) == n);
  assert(n == 0);
  close(fd);
  close(client);
}

int main(int argc, char **argv) {
  std::string error;
  std::shared_ptr<mars::Temps> temps = mars::Temps::make("./tmp/", &error);
  if (temps == nullptr) {
    std::cerr << error << "\n";
    return 1;
  }
  std::shared_ptr<mars::Server> server =
      mars::Server::make(mars::mars_port, &error);
  if (server == nullptr) {
    std::cerr << error << "\n";
    return 1;
  }
  int client;
  while ((client = server->client(&error)) != -1) {
    std::thread t(handle_client, client, temps);
    t.detach();
  }
  return 0;
}
