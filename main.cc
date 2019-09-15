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

class IterableProcess : public Process {
private:
  Mem<bool> &stopFlag;
protected:
  Logger logger;

  IterableProcess(Mem<bool> &stopFlag, const std::string& logger):
    stopFlag(stopFlag),
    logger(logger) {
  }

public:
  virtual void iterate() = 0;

  void run() {
    logger.debug("start");
    while(!*stopFlag) {
      iterate();
    }
    logger.debug("end");
  }
};

class Producer : public IterableProcess {
private:
  Queue<Box> &boxes;
public:
  Producer(Queue<Box> &boxes, Mem<bool> &stopFlag):
    boxes(boxes),
    IterableProcess(stopFlag, "Producer") {
  }

  void iterate() {
    logger << "running";
    sleep(3);
    /*
      Box* box = randonBox();
      boxes.push(box);
    */
  }

private:
  Box *randonBox() {
    return NULL;
  }
};

class DistributionCenter : public IterableProcess {
private:
  Queue<Box>    &boxes;
  Queue<Packet> &packets;
public:
  DistributionCenter(Queue<Box>    &boxes,
                     Queue<Packet> &packets,
                     Mem<bool> & stopFlag):
    boxes(boxes),
    packets(packets),
    IterableProcess(stopFlag, "DistroCenter") {
  }

  void iterate() {
    logger << "running";
    sleep(3);
    /*
      Packet packet;
      while(!packet.isDone()) {
       Box * box = boxes.pull();
       packet.add(box);
      }
      packets.push(&packet);
    */
  };
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

class SellPoint : public IterableProcess {
private:
  Queue<Packet>&  boxes;
  Queue<Request>& requests;
  Storage& storage;
  bool isUp() {
    return true;
  }
public:
  SellPoint(Queue<Packet>&  boxes,
            Queue<Request>& requests,
            Storage& storage,
            Mem<bool> & stopFlag):
    boxes(boxes),
    requests(requests),
    storage(storage),
    IterableProcess(stopFlag, "SellPoint") {
  }

  void iterate() {
    logger << "running";
    sleep(3);
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

int main(int argc, char ** argv) {
  try {
    FixQueue<Box>    boxes(1);
    FixQueue<Packet> packets(1);
    FixQueue<Request> requests(1);
    Storage storage;

    Mem<bool> stopFlag("/dev/null", 0, false);

    std::vector<Producer> producers(3, {boxes, stopFlag});
    std::vector<DistributionCenter> distros(3, {boxes, packets, stopFlag});
    std::vector<SellPoint> sellpoints(3, {packets, requests, storage, stopFlag});

    forkAll(producers);
    forkAll(distros);
    forkAll(sellpoints);

    sleep(10);

    *stopFlag = true;

    waitAll(producers);
    waitAll(distros);
    waitAll(sellpoints);

  } catch(const std::string & msg) {
    std::cerr << msg << std::endl;
  }
  return 0;
}
