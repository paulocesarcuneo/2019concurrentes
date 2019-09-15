#ifndef _LOG_HPP_
#define _LOG_HPP_

#include <iostream>
#include <sstream>
#include <unistd.h>

/*
class LogStream : public std::stringstream {
 private:
  std::ostream& out;
 public:
  LogStream(std::ostream& out): out(out) {}

  LogStream(const LogStream& other):out(other.out) {}
  void flush() {
    out << this->str() << std::endl;
  }
};

LogStream& endl(LogStream& os) {
  os.flush();
  return os;
};

LogStream info() {
  LogStream o(out);
  o << "(" << getpid() << ") " << name << " : " ;
  return o;
}
*/
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
