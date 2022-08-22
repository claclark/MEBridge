#include "lib/misc.h"

#include <errno.h>
#include <mutex>
#include <string>
#include <string.h>

namespace mars {

std::string error_description() {
  if (errno == 0)
    return "";
  static std::mutex m;
  const std::lock_guard<std::mutex> lock(m);
  std::string message = strerror(errno);
  return message;
}
} // namespace mars
