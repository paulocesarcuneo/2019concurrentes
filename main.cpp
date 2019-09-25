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
#include "inventory.hpp"


bool stopFlag = false;

void setStopFlag (int signo) {
  root.debug(Str() << "Signal " << strsignal(signo) << ".");
  stopFlag = true;
};
struct Args {
  int producers;
  int distributors;
  int sellpoints;
  std::vector<std::string> requestFiles;
  std::vector<std::string> distributorsStorageFiles;
  std::vector<std::string> sellpointsStorageFiles;
  std::string inventoryFileName;
  Args(const std::string & configFile) {
    std::fstream config;
    config.open(configFile);
    config >> producers >> distributors >> sellpoints;
    for(int i=0; i < distributors; ++i) {
      std::string fileName;
      config >> fileName;
      distributorsStorageFiles.push_back(fileName);
    }

    for(int i=0; i < sellpoints; ++i) {
      std::string storageFileName, requestFileName;
      config >> storageFileName >> requestFileName;
      sellpointsStorageFiles.push_back(storageFileName);
      requestFiles.push_back(requestFileName);
    }

    config >> inventoryFileName;
    config.close();
  }
};

int main(int argc, char** argv) {
  try{
    Args args(argv[1]);
    signal(SIGINT, setStopFlag);
    std::vector<pid_t> children;
    // Create Producers;
    Pipe producerOut;
    for(int i = 0; i< args.producers; i++) {
      pid_t pid = fork();
      if(pid == 0) {
        Producer p(i,
                   producerOut,
                   stopFlag);
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
    for(int i = 0; i< args.distributors; i++) {
      Pipe distributorIn;
      pid_t pid = fork();
      if(pid == 0) {
        producerOut.close();
        Distributor d(
                      distributorIn,
                      distributorOut,
                      args.distributorsStorageFiles[i]);
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
    Pipe inventoryPipe;
    std::vector<Pipe> sellpointInPipes;
    for(int i = 0; i < args.sellpoints; i++) {
      Pipe sellpointIn;
      pid_t pid = fork();
      if(pid == 0) {
        distributorOut.close();
        SellPoint s(
                    sellpointIn,
                    inventoryPipe,
                    args.requestFiles[i],
                    args.sellpointsStorageFiles[i]);
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
      inventoryPipe.close();
      Balancer sellpointsBalancer(distributorOut, sellpointInPipes, "DistroToSellPoint");
      sellpointsBalancer.run();
      closeAll(sellpointInPipes);
      distributorOut.close();
      return 0;
    }
    children.push_back(pid);
    closeAll(sellpointInPipes);
    distributorOut.close();

    // Inventory
    pid=fork();
    if(pid == 0){
      Inventory inventory(inventoryPipe,
                          args.inventoryFileName);
      inventory.run();
      return 0;
    }
    inventoryPipe.close();

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
