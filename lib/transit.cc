#include "lib/transit.h"

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <time.h>
#include <unistd.h>

namespace mars {

std::shared_ptr<Transit> Transit::make(time_t (*arrival)(),
                                       std::string *error) {
  std::shared_ptr<Transit> transit = std::shared_ptr<Transit>(new Transit());
  transit->arrival_ = arrival;
  return transit;
}

void Transit::send(const std::string &message) {
  lock_.lock();
  transmitting_.emplace(Transmitting(message, arrival_()));
  lock_.unlock();
  condition_.notify_one();
}

std::string Transit::receive() {
  Transmitting t;
  {
    std::unique_lock<std::mutex> l(lock_);
    condition_.wait(l, [&] { return transmitting_.size() > 0; });
    t = transmitting_.front();
    transmitting_.pop();
  }
  condition_.notify_one();
  time_t now = time(0);
  if (now <= t.eta)
    std::this_thread::sleep_for(std::chrono::seconds(t.eta - now));
  return t.message;
}
} // namespace mars
