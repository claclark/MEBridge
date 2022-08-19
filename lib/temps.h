#ifndef MEBRIDGE_LIB_TEMPS_H_
#define MEBRIDGE_LIB_TEMPS_H_

#include <memory>
#include <mutex>
#include <string>

namespace mars {

class Temps final {
public:
  static std::shared_ptr<Temps> make(const std::string &directory,
                                     std::string *error);
  std::string temp();
  void remove(const std::string &temp);

  ~Temps();
  Temps(const Temps &) = delete;
  Temps &operator=(const Temps &) = delete;
  Temps(Temps &&) = delete;
  Temps &operator=(Temps &&) = delete;

private:
  Temps(){};
  std::mutex lock_;
  std::string directory_;
  unsigned counter_;
};

} // namespace mars

#endif // MEBRIDGE_LIB_TEMPS_H_
