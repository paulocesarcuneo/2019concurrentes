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
  Out<Box> &boxes;
public:
  Producer(Out<Box> &boxes, Mem<bool> &stopFlag):
    boxes(boxes),
    IterableProcess(stopFlag, "Producer") {
  }

  void iterate() {
    logger << "running";
    std::stringstream ss;

    ss << "BOX" << getpid();
    std::string s=ss.str();
    boxes.write(s.c_str(), s.size());
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
  In<Box>    &boxes;
  Out<Packet> &packets;
public:
  DistributionCenter(In<Box>    &boxes,
                     Out<Packet> &packets,
                     Mem<bool> & stopFlag):
    boxes(boxes),
    packets(packets),
    IterableProcess(stopFlag, "DistroCenter") {
  }

  void iterate() {
    char buf[7];
    boxes.read(buf, 4);
    logger << buf;
    packets.write("PACKET", 7);
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
  In<Packet>&  packets;
  Queue<Request>& requests;
  Storage& storage;
  bool isUp() {
    return true;
  }
public:
  SellPoint(In<Packet>&  packets,
            Queue<Request>& requests,
            Storage& storage,
            Mem<bool> & stopFlag):
    packets(packets),
    requests(requests),
    storage(storage),
    IterableProcess(stopFlag, "SellPoint") {
  }

  void iterate() {
    char buf[8];
    packets.read(buf, 7);
    logger << buf;
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
    Pipe<Box>    boxes;
    Pipe<Packet> packets;
    FixQueue<Request> requests(1);
    Storage storage;

    Mem<bool> stopFlag("/dev/null", 0, false);

    Out<Box> boxOut=boxes.writeEnd();
    In<Box> boxIn=boxes.readEnd();
    Out<Packet> packetOut=packets.writeEnd();
    In<Packet> packetIn=packets.readEnd();

    std::vector<Producer> producers(3, {boxOut ,
                                        stopFlag});

    std::vector<DistributionCenter> distros(1, {boxIn,
                                                packetOut,
                                                stopFlag});
    std::vector<SellPoint> sellpoints(1, {packetIn,
                                          requests,
                                          storage,
                                          stopFlag});

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
