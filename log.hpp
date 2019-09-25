#ifndef _LOG_HPP_
#define _LOG_HPP_

#include <iostream>
#include <sstream>
#include <unistd.h>
#include <sys/time.h>
#include <algorithm>

class Str {
private:
  std::stringstream ss;
public:
  Str() : ss() {}
  template<typename T>
  Str& operator<<(const T & t) {
    ss << t;
    return *this;
  }

  Str& operator<<(std::ostream& (*fun)(std::ostream&)) {
    ss << fun;
    return *this;
  }

  std::string str() const {
    return ss.str();
  }
};

std::ostream &  operator<<(std::ostream & o, const Str & s) {
  o << s.str();
  return o;
}

class Logger {
private:
  std::string name;
  std::ostream& out;
  int FLAGS = 0xFFFF;
public:
  Logger(const std::string& name,
         std::ostream& out = std::cerr):
    name(name),
    out(out) {}

  std::ostream& log(const std::string& level, const std::string& msg) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    std::string str(msg);
    str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());

    std::stringstream stream;
    stream <<"[" << tv.tv_sec << " sec " << tv.tv_usec << " usec] "
           << "(" << getpid() << ") "
           << level << " "
           << name << " "
           << str
           << std::endl;


    out << stream.str();
    return out;
  }

  void info(const std::string& msg) {
    log("INFO", msg);
  }

  void debug(const std::string& msg) {
    if(FLAGS & 0x1)
      log("DEBUG", msg);
  }

  void error(const std::string& msg) {
    log("ERROR", msg);
  }

  void info(const Str& msg) {
    log("INFO", msg.str());
  }

  void debug(const Str& msg) {
    if(FLAGS & 0x1)
      log("DEBUG", msg.str());
  }

  void error(const Str& msg) {
    log("ERROR", msg.str());
  }
};

Logger root("ROOT");

#endif
