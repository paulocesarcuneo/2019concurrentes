#ifndef __LAUNCHER_HPP__
#define __LAUNCHER_HPP__
#include <stdio.h>
#include <stdlib.h>

class Process {
private:
  pid_t pid = 0;
public:
  pid_t fork() {
    pid = ::fork();
    if(pid == 0) {
      try {
        run();
        exit(0);
      } catch (...) {
        exit(1);
      }
    }
    return pid;
  }

  int wait() {
    int status;
    ::waitpid(pid, &status, 0);
    return status;
  }
protected:
  virtual void run() = 0;
};

template <class T>
void forkAll(std::vector<T> & ps) {
  for(auto &p: ps) {
    p.fork();
  }
};

template <class T>
void waitAll(std::vector<T> & ps) {
  for(auto &p: ps) {
    p.wait();
  }
};
#endif
