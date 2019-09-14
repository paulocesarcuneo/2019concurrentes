#include <stddef.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <sys/wait.h>
#include "launcher.hpp"
#include "log.hpp"
#include "flower.hpp"
#include "queue.hpp"

using namespace std;

class Producer : public Process {
private:
  Queue<Box> &boxes;
  Logger logger;
public:
  Producer(Queue<Box> &boxes):
    boxes(boxes),
    logger("Producer") {
  }

  void run() {
    logger << "start";
    sleep(10);
    logger << "end";
    /*
    while(isUp()) {
      Box* box = randonBox();
      boxes.push(box);
      }
    */
  }

  void pause() {
    // ...
  }
private:
  Box *randonBox() {
    return NULL;
  }
  bool isUp() {
    return true;
  }
};

class DistributionCenter : public Process {
private:
  Queue<Box>    &boxes;
  Queue<Packet> &packets;
  Logger logger;
public:
  DistributionCenter(Queue<Box>    &boxes,
                     Queue<Packet> &packets):
    boxes(boxes),
    packets(packets),
    logger("DistroCenter") {
  }

  void run() {
    logger << "start ";
    sleep(10);
    logger << "end ";
    /*
    while(isUp()) {
      Packet packet;
      while(!packet.isDone()) {
        Box * box = boxes.pull();
        packet.add(box);
      }
      packets.push(&packet);
    }
    */
  };

  void pause() {
    // ...
  }
private:
  bool isUp() {
    return true;
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
  Queue<Packet>&  boxes;
  Queue<Request>& requests;
  Storage& storage;
  Logger logger;
  bool isUp() {
    return true;
  }
public:
  SellPoint(Queue<Packet>&  boxes,
            Queue<Request>& requests,
            Storage& storage):
    boxes(boxes),
    requests(requests),
    storage(storage),
    logger("SellPoint"){
  }
  void run() {
    logger << "start";
    sleep(10);
    logger << "end";
    /*
    while(isUp()) {
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
    }
    */
  }

  void giveToClient(Request* req, Bouquet * flower) {
  }

  void dispatchToBycicle(Request*  req, Bouquet * flower) {

  }
};

int main(int argc, char ** argv) {
  FixQueue<Box>    boxes(1);
  FixQueue<Packet> packets(1);
  FixQueue<Request> requests(1);
  Storage storage;
  std::vector<Producer> producers(3, boxes);
  std::vector<DistributionCenter> distros(3, {boxes, packets});
  std::vector<SellPoint> sellpoints(3, {packets, requests, storage});

  forkAll(producers);
  forkAll(distros);
  forkAll(sellpoints);

  waitAll(producers);
  waitAll(distros);
  waitAll(sellpoints);
  /*
  Producer producer(boxes);
  DistributionCenter distribution(boxes, packets);
  SellPoint sellPoint(packets);


  Launcher prodLauncher(3, producerHello);
  Launcher distroLauncher(3, distroHello);
  Launcher sellPointLauncher(3, sellPointHello);

  prodLauncher.startup();
  distroLauncher.startup();
  sellPointLauncher.startup();

  prodLauncher.shutdown();
  distroLauncher.shutdown();
  sellPointLauncher.shutdown();*/

  return 0;
}
