#include "lib/mars.h"

#include <string>

namespace mars {
bool parse_port(const std::string &arg, uint16_t *port, std::string *error) {
  int p;
  try {
    p = std::stoi(arg);
  } catch (std::exception &e) {
    *error = "Unknown port: " + arg;
    return false;
  }
  if (p >= 64*1024) {
    *error = "Port too large: " + arg;
    return false;
  }
  *port = p;
  return true;
}
} // namespace mars
