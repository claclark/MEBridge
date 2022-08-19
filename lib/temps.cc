#include "lib/temps.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>

namespace mars {

std::shared_ptr<Temps> Temps::make(const std::string &directory,
                                   std::string *error) {
  std::string mkdir_command = "/bin/mkdir -p " + directory;
  system(mkdir_command.c_str());
  std::string cleanup_command = "/bin/rm -f " + directory + "/temp.*";
  system(cleanup_command.c_str());
  static std::shared_ptr<Temps> temps = std::shared_ptr<Temps>(new Temps());
  temps->directory_ = directory;
  temps->counter_ = 0;
  return temps;
}

std::string Temps::temp() {
  std::stringstream ss;
  ss << directory_ << "/temp.";
  ss.fill('0');
  ss.width(8);
  lock_.lock();
  ss << counter_;
  counter_++;
  lock_.unlock();
  return ss.str();
}

void Temps::remove(const std::string &temp) { std::remove(temp.c_str()); }

} // namespace mars
