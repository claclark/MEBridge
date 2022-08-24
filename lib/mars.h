#ifndef MEBRIDGE_LIB_MARS_H_
#define MEBRIDGE_LIB_MARS_H_

#include "lib/misc.h"
#include "lib/server.h"
#include "lib/temps.h"
#include "lib/transit.h"

namespace mars {
constexpr uint16_t em_port = 3860;
constexpr uint16_t mars_port = 3861;
constexpr uint16_t me_port = 3862;
constexpr uint16_t earth_port = 3863;

bool parse_port(const std::string &arg, uint16_t *port, std::string *error);

} // namespace mars

#endif // MEBRIDGE_LIB_MARS_H_
