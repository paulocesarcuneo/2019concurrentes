#ifndef __LAUNCHER_HPP__
#define __LAUNCHER_HPP__
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <string>

class Process {
private:
  pid_t pid;
  pid_t parentpid;
  void (*exitFn)(int);
public:
  Process(void (*exitFn)(int) = ::exit):
    parentpid(getpid()),
    pid(getpid()),
    exitFn(exitFn) {}

  Process& fork() {
    pid = ::fork();
    if(-1 == pid)
      throw std::string("fork") + std::string(strerror(errno));

    if(0 == pid) {
      try {
        run();
        exitFn(0);
      } catch (const std::string & msg) {
        std::cerr << msg;
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
void forkAll(std::vector<T> & ps) {
  for(auto &p: ps) {
    p.fork();
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
