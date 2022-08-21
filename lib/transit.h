#ifndef MEBRIDGE_LIB_TRANSIT_H_
#define MEBRIDGE_LIB_TRANSIT_H_

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <semaphore.h>
#include <string>
#include <time.h>

namespace mars {

class Transit final {
public:
  static std::shared_ptr<Transit> make(time_t (*arrival)(), std::string *error);
  void send(const std::string &message);
  std::string receive();

  ~Transit(){};
  Transit(const Transit &) = delete;
  Transit &operator=(const Transit &) = delete;
  Transit(Transit &&) = delete;
  Transit &operator=(Transit &&) = delete;

private:
  Transit(){};
  struct Transmitting {
    Transmitting() = default;
    Transmitting(const std::string message, time_t eta)
        : message(message), eta(eta){};
    std::string message;
    time_t eta;
  };
  std::queue<Transmitting> transmitting_;
  std::mutex lock_;
  std::condition_variable condition_;
  time_t (*arrival_)();
};
} // namespace mars

#endif // MEBRIDGE_LIB_TRANSIT_H_
