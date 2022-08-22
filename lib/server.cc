#include "lib/server.h"

#include <arpa/inet.h>
#include <cstdint>
#include <errno.h>
#include <iostream>
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

#include "lib/misc.h"

#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff
#endif

namespace mars {

namespace {

bool allowed_address(const std::string &allowed_hostname,
                     std::vector<struct sockaddr> *allowed_clients,
                     std::string *error) {
  struct addrinfo hints;
  struct addrinfo *res;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  int status = getaddrinfo(allowed_hostname.c_str(), NULL, &hints, &res);
  if (status != 0) {
    *error = std::string(gai_strerror(status));
    return false;
  }
  for (struct addrinfo *ai = res; ai != nullptr; ai = ai->ai_next) {
    struct sockaddr sa;
    memcpy(&sa, ai->ai_addr, ai->ai_addrlen);
    allowed_clients->emplace_back(sa);
  }
  freeaddrinfo(res);
  return true;
}

bool determine_allowed_clients(
    const std::vector<std::string> &allowed_hostnames,
    std::vector<struct sockaddr> *allowed_clients, std::string *error) {
  if (!allowed_address("127.0.0.1", allowed_clients, error))
    return false;
  char buffer[256];
  if (gethostname(buffer, sizeof(buffer)) == -1) {
    *error = error_description();
    return false;
  }
  std::string this_machine = std::string(buffer);
  if (!allowed_address(this_machine, allowed_clients, error))
    return false;
  for (auto &allowed_hostname : allowed_hostnames)
    if (!allowed_address(allowed_hostname, allowed_clients, error))
      return false;
  return true;
}

} // namespace

std::shared_ptr<Server>
Server::make(uint16_t port, const std::vector<std::string> &allowed_hostnames,
             std::string *error) {
  int32_t s;
  struct sockaddr_in addr;
  if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    *error = error_description();
    return nullptr;
  }
  memset(&addr, '\0', sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(port);
  if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    *error = error_description();
    return nullptr;
  }
  if (listen(s, 5) == -1) {
    *error = error_description();
    return nullptr;
  }
  std::vector<struct sockaddr> allowed_clients;
  if (!determine_allowed_clients(allowed_hostnames, &allowed_clients, error))
    return nullptr;
  std::shared_ptr<Server> server = std::shared_ptr<Server>(new Server());
  server->s_ = s;
  server->allowed_clients_ = allowed_clients;
  return server;
}

int Server::client(std::string *error) {
  int32_t s = -1;
  socklen_t addrlen;
  struct sockaddr_in addr;
  while (s == -1) {
    addrlen = sizeof(struct sockaddr_in);
    while ((s = accept(s_, (struct sockaddr *)&addr, &addrlen)) == -1)
      if (errno != EINTR) {
        *error = error_description();
        return -1;
      }
    bool okay = false;
    for (auto &ac : allowed_clients_)
      if (memcmp(&addr.sin_addr.s_addr,
                 &(*((struct sockaddr_in *)&ac)).sin_addr.s_addr,
                 sizeof(addr.sin_addr.s_addr)) == 0) {
        okay = true;
        break;
      }
    if (!okay) {
      close(s);
      s = -1;
    }
  }
  return s;
}

Server::~Server() { close(s_); }

int connect2server(const std::string &hostname, uint16_t port,
                   std::string *error) {
  int s;
  struct sockaddr_in addr;
  memset(&addr, '\0', sizeof(addr));
  if ((addr.sin_addr.s_addr = inet_addr(hostname.c_str())) == INADDR_NONE) {
    struct hostent *host;
    if ((host = gethostbyname(hostname.c_str())) == NULL) {
      *error = std::string("Invalid host name");
      return -1;
    }
    addr.sin_family = host->h_addrtype;
    memcpy(&addr.sin_addr.s_addr, host->h_addr, host->h_length);
  } else {
    addr.sin_family = AF_INET;
  }
  addr.sin_port = htons(port);
  if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    *error = error_description();
    return -1;
  }
  if (connect(s, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    *error = error_description();
    return -1;
  }
  return s;
}

} // namespace mars
