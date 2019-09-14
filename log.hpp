#ifndef _LOG_HPP_
#define _LOG_HPP_

#include <iostream>
#include <sstream>
#include <unistd.h>

class Logger {
private:
  std::string name;
  std::ostream& out;
public:
  Logger(const std::string& name,
         std::ostream& out = std::cerr):
    name(name),
    out(out) {}

  std::ostream& operator<<(const std::string& msg) {
    std::stringstream stream;
    stream << "(" << getpid() << ") " << name << " : " << msg << std::endl;
    out << stream.str();
    return out;
  }
};

Logger root("");

#endif
