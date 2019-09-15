#ifndef _LOG_HPP_
#define _LOG_HPP_

#include <iostream>
#include <sstream>
#include <unistd.h>
#include <sys/time.h>



class Logger {
private:
  std::string name;
  std::ostream& out;
  int FLAGS = 0xF;
public:
  Logger(const std::string& name,
         std::ostream& out = std::cerr):
    name(name),
    out(out) {}

  std::ostream& operator<<(const std::string& msg) {
    return log("INFO", msg);
  }

  std::ostream& log(const std::string& level, const std::string& msg) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    std::stringstream stream;
    stream << "(" << getpid() << ") ["<< tv.tv_sec << " sec "<< tv.tv_usec << " usec] " << level << " " << name << " : " << msg << std::endl;
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
};

Logger root("ROOT");

#endif
