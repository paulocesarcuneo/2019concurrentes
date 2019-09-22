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
#include <signal.h>

class Balancer {
private:
  Pipe & inputPipe;
  std::vector<Pipe> & outputPipes;
  Logger logger;
public:
  Balancer(Pipe & in,
           std::vector<Pipe> & out,
           const std::string & name):
    inputPipe(in),
    outputPipes(out),
    logger(name) {
  }

  void close() {
    inputPipe.close();
    for(auto &o:outputPipes) {
      o.close();
    }
  }

  void run() {
    inputPipe.in().asStdIn();
    std::vector<Out> outs;
    for(auto&o: outputPipes) {
      outs.push_back(o.out());
    }
    while(true) {
      for(auto &o : outs) {
        logger.debug("eof");
        if(std::cin.peek() == -1) {
          logger.debug("closing");
          close();
          return;
        }
        std::string lineIn;
        logger.debug("reading.");
        std::getline(std::cin, lineIn);
        logger.debug("read:" + lineIn);
        logger.debug("writing.");
        std::string lineOut = lineIn + "\n";
        o.write(lineOut.c_str(), lineOut.size());
        logger.debug("write:" + lineOut);
      }
    }
  }
};

class Producer {
private:
  Mem<bool>& stopFlag;
  Pipe& output;
  Logger logger;
public:
  Producer(Pipe& output, Mem<bool>& stopFlag):
    output(output),
    stopFlag(stopFlag),
    logger("Producer") {
  }

  Box randomBox() {
    Box box;
    pid_t i = getpid();
    box.flowers[0] = Bouquet(0, ROSE);
    box.flowers[1] = Bouquet(0, ROSE);
    box.flowers[2] = Bouquet(0, ROSE);
    box.flowers[3] = Bouquet(0, ROSE);
    box.flowers[4] = Bouquet(0, ROSE);
    box.flowers[5] = Bouquet(0, TULIP);
    box.flowers[6] = Bouquet(0, TULIP);
    box.flowers[7] = Bouquet(0, TULIP);
    box.flowers[8] = Bouquet(0, TULIP);
    box.flowers[9] = Bouquet(0, TULIP);
    return box;
  }

  void run() {
    try {
      Out out = output.out();
      out.asStdOut();
      while(!(*stopFlag)) {
        Box box = randomBox();
        logger.info(Str() << box);
        std::cout << box;
        sleep(2);
      }
    } catch (...) {
      logger.error("Error");
    }
    output.close();
    
  }
};

class DistributionCenter  {
private:
  Pipe& packets;
  Pipe& boxes;
  Logger logger;
public:
  DistributionCenter(Pipe& boxes,
                     Pipe& packets):
    boxes(boxes),
    packets(packets),
    logger("DistroCenter") {
  }

  void run() {
    boxes.in().asStdIn();
    packets.out().asStdOut();
    while(std::cin.peek() != -1) {
      Box box = deserialize<Box>(std::cin);
      logger.info(Str() << box);
      std::cout << box;
      sleep(3);
    }
    packets.close();
    boxes.close();
  }
};

class Storage {
public:
  Bouquet * decrement(Request*request) {
    return NULL;
  }
  void add(Packet* packet){
  }
  bool canFullfill(Request* request){
    return true;
  }
};

class SellPoint  {
private:
  Pipe& packets;
  Storage& storage;
  Logger logger;
public:
  SellPoint(Pipe& packets,
            Storage& storage):
    packets(packets),
    storage(storage),
    logger("SellPoint") {
  }

  void run() {
    packets.in().asStdIn();
    while(std::cin.peek() != -1) {
      Box box = deserialize<Box>(std::cin);
      logger.info(Str() << box);
      sleep(3);
    }
    packets.close();
  }

  void iterate() {
    /*
      Request* request=requests.pull();
      if(storage.canFullfill(request)){
       Bouquet* flower = storage.decrement(request);
       if(request->getType() == INTERNET) {
         dispatchToBycicle(request, flower);
       } else{
         giveToClient(request, flower);
       }
      } else {
       Packet * packet = distributionCenterTransport.pull();
       storage.add(packet);
      }
    */
  }

  void giveToClient(Request* req, Bouquet * flower) {
  }

  void dispatchToBycicle(Request*  req, Bouquet * flower) {

  }
};


Mem<bool> stopFlag("/dev/null", 0, false);

void sig_handler (int signo) {
  root.debug(Str() << "signal " << signo);
  *stopFlag = true;
};

int main(int argc, char** argv) {
  try{
  signal(SIGPIPE, sig_handler);
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
      DistributionCenter d(distributorIn, distributorOut);
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
      Storage storage;
      SellPoint s(sellpointIn, storage);
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
