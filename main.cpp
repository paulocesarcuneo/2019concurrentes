#include <stddef.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <sys/wait.h>
#include "launcher.hpp"
#include "log.hpp"
#include "flower.hpp"
#include "queue.hpp"
#include "shm.hpp"
#include "pipe.hpp"
#include <string>
#include <sstream>


int main(int argc, char** argv) {
  std::vector<pid_t> children;
  bool main = true;
  int producers = 3;
  int distributors = 3
  int sellpoints = 3;
  // Create Producers;
  {
    Pipe producerPipe;
    for(int i=0; i< producers; i++) {
      pid_t pid = fork();
      if(pid == 0) {
        producerPipe.out();
        std::cout << "producer" << i << std::endl;
        producerPipe.close();
        return 0;
      } else {
        children.push_back(pid);
      }
    }
    producerPipe.close();
  }
  // Create DistributionCenters
  {
    Pipe distributorPipe;
    for(int i=0; i< distributors; i++) {
      pid_t pid = fork();
      if(pid == 0) {
        distributorPipe.out();
        std::cout << "disributor" << i << std::endl;
        distributorPipe.close();
        return 0;
      } else {
        children.push_back(pid);
      }
    }
    distributorPipe.close();
  }
  // Create Sellpoints
  {
    Pipe sellpointsPipe;
    for(int i=0; i < sellpoints; i++) {
      pid_t pid = fork();
      if(pid == 0) {
        sellpointsPipe.out();
        std::cout << "sellpoint" << i << std::endl;
        sellpointsPipe.close();
        return 0;
      } else {
        children.push_back(pid);
      }
    }
    sellpointsPipe.close();
  }

  for(auto i: children) {
    waitpid(i, NULL, 0);
  }

  return 0;
}
