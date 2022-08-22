#include "lib/misc.h"

#include <errno.h>
#include <locale.h>
#include <memory>
#include <stdio.h>
#include <string.h>
#include <string>

namespace mars {

std::string error_description() {
  if (errno == 0)
    return "";
  const size_t SIZE = 80;
  char buffer[SIZE + 1];
  strerror_r(errno, buffer, SIZE);
  return std::string(buffer);
}
} // namespace mars
