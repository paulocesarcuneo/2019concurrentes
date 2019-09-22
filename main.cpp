#include <stddef.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#include <iostream>
#include <vector>
#include <string>
#include <sstream>

#include "log.hpp"
#include "flower.hpp"
#include "shm.hpp"
#include "pipe.hpp"
#include "balancer.hpp"
#include "producer.hpp"
#include "distributor.hpp"
#include "sellpoint.hpp"


Mem<bool> stopFlag("/dev/null", 0, false);

void setStopFlag (int signo) {
  root.debug(Str() << "signal " << signo);
  *stopFlag = true;
};

int main(int argc, char** argv) {
  try{
    signal(SIGPIPE, setStopFlag);
    std::vector<pid_t> children;
    bool main = true;
    int producers = 3;
    int distributors = 3;
    int sellpoints = 3;
  // Create Producers;
    Pipe producerOut;
    for(int i = 0; i< producers; i++) {
      pid_t pid = fork();
      if(pid == 0) {
        Producer p(producerOut, stopFlag);
        p.run();
        producerOut.close();
        return 0;
      } else {
        children.push_back(pid);
      }
    }
    // Create DistributionCenters
    Pipe distributorOut;
    std::vector<Pipe> distributorsInPipes;
    for(int i = 0; i< distributors; i++) {
      Pipe distributorIn;
      pid_t pid = fork();
      if(pid == 0) {
        producerOut.close();
        Distributor d(distributorIn, distributorOut);
        d.run();
        distributorOut.close();
        distributorIn.close();
        return 0;
      } else {
        distributorsInPipes.push_back(distributorIn);
        children.push_back(pid);
      }
    }
    // Producers to distributors
    pid_t pid = fork();
    if(pid == 0) {
      distributorOut.close();
      Balancer distributorBalancer(producerOut, distributorsInPipes, "ProdToDistro");
      distributorBalancer.run();
      closeAll(distributorsInPipes);
      producerOut.close();
      return 0;
    }
    children.push_back(pid);
    closeAll(distributorsInPipes);
    producerOut.close();
    // Create Sellpoints
    std::vector<Pipe> sellpointInPipes;
    for(int i = 0; i < sellpoints; i++) {
      Pipe sellpointIn;
      pid_t pid = fork();
      if(pid == 0) {
        distributorOut.close();
        SellPoint s(sellpointIn, "/dev/null");
        s.run();
        sellpointIn.close();
        return 0;
      } else {
        children.push_back(pid);
        sellpointInPipes.push_back(sellpointIn);
      }
    }
    // distributor to sellpoints
    pid = fork();
    if(pid == 0) {
      Balancer sellpointsBalancer(distributorOut, sellpointInPipes, "DistroToSellPoint");
      sellpointsBalancer.run();
      closeAll(sellpointInPipes);
      distributorOut.close();
      return 0;
    }
    children.push_back(pid);
    closeAll(sellpointInPipes);
    distributorOut.close();
    
    // todo los pipes estan close.
    std::cout << "get" << std::endl;
    std::cin.get();
    *stopFlag = true;
    // Finish
    for(auto i: children) {
      waitpid(i, NULL, 0);
    }
  }catch(const char* str){
    root.error(str);
  }catch(const std::string & str) {
    root.error(str);
  }
  return 0;
}
