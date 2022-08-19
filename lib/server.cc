#include "lib/server.h"

#include <arpa/inet.h>
#include <cstdint>
#include <errno.h>
#include <memory>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff
#endif

namespace mars {

std::shared_ptr<Server> Server::make(uint16_t port, std::string *error) {
  int32_t s;
  struct sockaddr_in addr;

  if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    *error = std::string(strerror(errno));
    return nullptr;
  }
  memset(&addr, '\0', sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(port);
  if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    *error = std::string(strerror(errno));
    return nullptr;
  }
  if (listen(s, 5) == -1) {
    *error = std::string(strerror(errno));
    return nullptr;
  }
  static std::shared_ptr<Server> server = std::shared_ptr<Server>(new Server());
  server->s_ = s;
  return server;
}

Server::~Server() { close(s_); }
} // namespace mars
