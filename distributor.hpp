#ifndef _DISTRIBUTOR_HPP_
#define _DISTRIBUTOR_HPP_

#include <vector>
#include <string>
#include <algorithm>


#include "log.hpp"
#include "flower.hpp"
#include "pipe.hpp"
#include "storage.hpp"

class Distributor  {
private:
  Pipe& packets;
  Pipe& boxes;
  Logger logger;
  Storage storage;

public:
  Distributor(
              Pipe& boxes,
              Pipe& packets,
              const std::string & storageFile):

    boxes(boxes),
    packets(packets),
    storage(storageFile),
    logger("Distributor") {
  }

  void run() {
    boxes.in().asStdIn();
    packets.out().asStdOut();

    storage.loadStock();

    while(std::cin.peek() != -1) {
      Box box = deserialize<Box>(std::cin);
      logger.info(Str() << " in: " << box);
      for(auto& flower: box.flowers) {
        if(flower.type == ROSE) {
          storage.roses.push_back(flower);
        } else if(flower.type == TULIP) {
          storage.tulips.push_back(flower);
        } else {
          throw std::string("Unhandler flower type");
        }
      }
      attemptToDispatch(storage.roses, "roses");
      attemptToDispatch(storage.tulips, "tulips");
    }
    logger.debug("finalizing");

    storage.storeStock();

    packets.close();
    boxes.close();
  }

  Packet pack(std::vector<Bouquet> & flowers) {
    std::vector<Bouquet> result(FLOWER_PACKET_SIZE);
    std::copy_n(flowers.begin(), FLOWER_PACKET_SIZE, result.begin());
    flowers.erase(flowers.begin(), flowers.begin() + FLOWER_PACKET_SIZE);
    return Packet(result);
  }

  void attemptToDispatch(std::vector<Bouquet> & flowers, const std::string& type) {
    if(flowers.size() >= FLOWER_PACKET_SIZE) {
      Packet packet = pack(flowers);
      logger.info(Str() << " out: " << packet);
      std::cout << packet;
    } else {
      logger.info(Str() << " no ready to send " << type << ":" << flowers.size());
    }
  }

};

#endif
