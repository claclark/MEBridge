#ifndef MEBRIDGE_LIB_SERVER_H_
#define MEBRIDGE_LIB_SERVER_H_

#include <cstdint>
#include <memory>
#include <string>

namespace mars {

class Server final {
public:
  static std::shared_ptr<Server> make(uint16_t port, std::string *error);
  int client(std::string *error);

  ~Server();
  Server(const Server &) = delete;
  Server &operator=(const Server &) = delete;
  Server(Server &&) = delete;
  Server &operator=(Server &&) = delete;

private:
  Server(){};
  int32_t s_;
};

int connect2server(const std::string &hostname, uint16_t port,
                   std::string *error);

} // namespace mars
#endif // MEBRIDGE_LIB_SERVER_H_
