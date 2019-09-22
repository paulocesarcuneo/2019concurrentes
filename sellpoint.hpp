#ifndef _SELLPOINT_HPP_
#define _SELLPOINT_HPP_
#include <vector>
#include <string>

#include "log.hpp"
#include "flower.hpp"
#include "pipe.hpp"

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

#endif
