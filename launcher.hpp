#ifndef __LAUNCHER_HPP__
#define __LAUNCHER_HPP__
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <string>
#include <sstream>
#include "log.hpp"

class Init {
public:
  virtual void operator()() = 0;
};

class Process {
private:
  pid_t pid;
  pid_t parentpid;
  void (*exitFn)(int);
protected:
  Logger logger;
public:
  Process(const std::string& name, void (*exitFn)(int) = ::exit):
    logger(name),
    parentpid(getpid()),
    pid(getpid()),
    exitFn(exitFn) {}

  Process& fork(Init & init) {
    pid = ::fork();
    if(-1 == pid)
      throw std::string("fork") + std::string(strerror(errno));

    if(0 == pid) {
      try {
        init();
        run();
        logger.debug("exiting");
        exitFn(0);
      } catch (const std::string & msg) {
        logger.error(msg);
        exitFn(1);
      } catch (...) {
        exitFn(1);
      }
    }
    return *this;
  }

  int wait() {
    if(parentpid == pid)
      return 0;

    if(pid == 0)
      return 0;

    int status;
    if (-1 == ::waitpid(pid, &status, 0))
      throw std::string("wait") + std::string(strerror(errno));

    return status;
  }

  pid_t id() const {
    return pid;
  }

  pid_t parentId() const {
    return parentpid;
  }

  bool hasForked() const {
    return pid != parentpid;
  }

  bool isFork() const {
    return pid == 0;
  }

protected:
  virtual void run() = 0;
};

template <class T>
void forkAll(std::vector<T> & ps, Init & i) {
  for(auto &p: ps) {
    p.fork(i);
    if(p.isFork())
      break;
  }
};

template <class T>
void waitAll(std::vector<T> & ps) {
  for(auto &p: ps) {
    p.wait();
  }
};
#endif
