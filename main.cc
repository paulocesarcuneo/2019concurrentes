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

class Balancer : public Process , public Writeable {
private:
  std::vector<Pipe> outputPipes;
  Pipe pipe;
public:
  Balancer(const std::string & name):
    Process(name) {
  }

  Out out() {
    return pipe.out();
  }

  void close() {
    pipe.close();
    for(auto &o:outputPipes) {
      o.close();
    }
  }

  Readable& newReadable() {
     outputPipes.push_back(Pipe());
     return outputPipes.back();
  }

  void run() {
    pipe.in().asStdIn();
    std::vector<Out> outs;
    for(auto&o: outputPipes) {
      outs.push_back(o.out());
    }
    while(true){
      for(auto &o : outs) {
        std::string lineIn;
        logger.debug("reading");
        int test = pipe.in().read(NULL, 0);
        if(test == -1) {
          logger.debug("was -1");
        }
        if(std::cin.eof()) {
          logger.debug("closing");
          close();
          return;
        }
        std::getline(std::cin, lineIn);
        logger.debug("have read:" + lineIn);
        std::string lineOut = lineIn + "\n";
        o.write(lineOut.c_str(), lineOut.size());
        logger.debug("write done!");
      }
    }
  }
};
void log(Logger& logger, Box& box) {
  std::stringstream ss;
  serialize(ss, box);
  logger << ss.str();
};

class Producer : public Process {
private:
  Mem<bool> &stopFlag;
  Writeable& pipe;
public:
  Producer(Writeable& pipe, Mem<bool> &stopFlag) :
    pipe(pipe),
    stopFlag(stopFlag),
    Process("Producer") {
  }

  Box randomBox() {
    Box box;
    pid_t i = getpid();
    box.flowers[0] = {i, ROSE};
    box.flowers[1] = {i, ROSE};
    box.flowers[2] = {i, ROSE};
    box.flowers[3] = {i, ROSE};
    box.flowers[4] = {i, ROSE};
    box.flowers[5] = {i, TULIP};
    box.flowers[6] = {i, TULIP};
    box.flowers[7] = {i, TULIP};
    box.flowers[8] = {i, TULIP};
    box.flowers[9] = {i, TULIP};
    return box;
  }

  void run() {
    Out out = pipe.out();
    out.asStdOut();
    while(!(*stopFlag)) {
      Box box = randomBox();
      std::stringstream ss;
      log(logger, box);
      serialize(std::cout, box);
      sleep(3);
    }
    // sleep(10);
    pipe.close();
  }
};

class DistributionCenter : public Process {
private:
  Writeable& packets;
  Readable& boxes;
public:
  DistributionCenter(Writeable& packets,
                     Balancer& balancer):
    boxes(balancer.newReadable()),
    packets(packets),
    Process("DistroCenter") {
  }

  void run() {
    boxes.in().asStdIn();
    packets.out().asStdOut();
    while(std::cin) {
      Box box = deserialize<Box>(std::cin);
      log(logger, box);
      serialize(std::cout, box);
      sleep(3);
    }
    packets.close();
    boxes.close();
  }
};

enum RequestType { INTERNET, FRONTDESK};

class Request{
public:
  RequestType getType() {
    return INTERNET;
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

class SellPoint : public Process {
private:
  Readable& packets;
  Storage& storage;
public:
  SellPoint(Balancer& balancer,
            Storage& storage):
    packets(balancer.newReadable()),
    storage(storage),
    Process("SellPoint") {
  }

  void run() {
    packets.in().asStdIn();
    while(std::cin) {
      Box box = deserialize<Box>(std::cin);
      log(logger, box);
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

class I1 : public Init {
private:
  Balancer& b;

public:
  I1(Balancer& b) :b(b) {}
  void operator()(){
    b.close();
  }
};

class INull : public Init {
public:
  void operator()(){}
};

int main(int argc, char ** argv) {
  try {
    int prodCount = 3;
    int distrosCount = 3;
    int sellPointsCount = 3;
    root << "start";

    Mem<bool> stopFlag("/dev/null", 0, false);
    Storage storage;

    Balancer boxes("BoxBalancer");
    // new boxes.pipe 
    std::vector<Producer> producers(prodCount, {boxes, stopFlag});

    Balancer packets("PacketBalancer");
    std::vector<DistributionCenter> distros(distrosCount, {packets, boxes});

    std::vector<SellPoint> sellpoints(sellPointsCount, {packets, storage});
    INull inull;
    I1 closeBox(boxes);
    
    boxes.fork(inull);
    packets.fork(inull);

    forkAll(producers, closeBox);
    forkAll(distros, closeBox);
    forkAll(sellpoints, closeBox);

    boxes.close();
    packets.close();

    sleep(5);
    *stopFlag = true;

    waitAll(producers);
    waitAll(distros);
    waitAll(sellpoints);

    boxes.wait();
    packets.wait();

  } catch(const std::string & msg) {
    std::cerr << msg << std::endl;
  }
  return 0;
}
