#include <cassert>
#include <fcntl.h>
#include <iostream>
#include <memory>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include <time.h>
#include <unistd.h>

#include "lib/mars.h"

void handle_client(int client, std::shared_ptr<mars::Temps> temps,
                   std::shared_ptr<mars::Transit> transit) {
  std::string temp_filename = temps->temp();
  int fd = open(temp_filename.c_str(), O_WRONLY | O_CREAT, 0666);
  assert(fd != -1);
  size_t size = 64 * 1024;
  std::unique_ptr<char[]> buffer = std::unique_ptr<char[]>(new char[size]);
  int n;
  while ((n = read(client, buffer.get(), size)) > 0)
    assert(write(fd, buffer.get(), n) == n);
  assert(n == 0);
  close(fd);
  close(client);
  transit->send(temp_filename);
}

void deliver(int remote_server, std::shared_ptr<mars::Transit> transit) {
  size_t size = 64 * 1024;
  std::unique_ptr<char[]> buffer = std::unique_ptr<char[]>(new char[size]);
  for (;;) {
    std::string temp_filename = transit->receive();
    int fd = open(temp_filename.c_str(), O_RDONLY);
    int n;
    while ((n = read(fd, buffer.get(), size)) > 0)
      assert(write(remote_server, buffer.get(), n) == n);
    assert(n == 0);
    close(fd);
    std::remove(temp_filename.c_str());
  }
}

time_t ten_seconds() { return time(0) + 10; }

int main(int argc, char **argv) {
  std::string error;
  std::shared_ptr<mars::Temps> temps = mars::Temps::make("./tmp", &error);
  if (temps == nullptr) {
    std::cerr << error << "\n";
    return 1;
  }
  std::shared_ptr<mars::Transit> transit =
      mars::Transit::make(ten_seconds, &error);
  if (transit == nullptr) {
    std::cerr << error << "\n";
    return 1;
  }
  std::shared_ptr<mars::Server> server =
      mars::Server::make(mars::em_port, &error);
  if (server == nullptr) {
    std::cerr << error << "\n";
    return 1;
  }
  {
    int remote_server =
        mars::connect2server("127.0.0.1", mars::mars_port, &error);
    if (remote_server < 0) {
      std::cerr << error << "\n";
      return 1;
    }
    std::thread t(deliver, remote_server, transit);
    t.detach();
  }
  int client;
  while ((client = server->client(&error)) != -1) {
    std::thread t(handle_client, client, temps, transit);
    t.detach();
  }
  return 0;
}
