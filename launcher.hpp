#ifndef __LAUNCHER_HPP__
#define __LAUNCHER_HPP__
#include <stdio.h>
#include <stdlib.h>
class Launcher {
private:
  int childrenCount = 0;
  int (*process)();
  bool isMain = true;
  pid_t* childrenPIDs = NULL;
public:
  Launcher(int childrenCount,
           int (*process)()) {
    this-> childrenCount = childrenCount;
    this-> process = process;
    this-> childrenPIDs = new pid_t[childrenCount];
  }

  ~Launcher() {
    delete childrenPIDs;
  }

  void startup() {
    isMain = true;
    for (int i = 0; i < childrenCount && isMain; ++i) {
      pid_t pid = fork();
      if(pid == 0) {
        process();
        isMain = false;
        exit(0);
      } else {
        childrenPIDs[i]=pid;
      }
    }
  }

  void shutdown() {
    if(this->isMain) {
      for (int i = 0; i < childrenCount && isMain; ++i) {
        int status;
        waitpid(childrenPIDs[i], &status, 0);
      }
    }
  }
};

#endif
