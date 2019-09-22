#ifndef _DISTRIBUTOR_HPP_
#define _DISTRIBUTOR_HPP_

#include <vector>
#include <string>
#include <algorithm>


#include "log.hpp"
#include "flower.hpp"
#include "pipe.hpp"

class Distributor  {
private:
  Pipe& packets;
  Pipe& boxes;
  Logger logger;
public:
  Distributor(Pipe& boxes,
              Pipe& packets):
    boxes(boxes),
    packets(packets),
    logger("Distributor") {
  }

  void run() {
    boxes.in().asStdIn();
    packets.out().asStdOut();

    std::vector<Bouquet> roses;
    std::vector<Bouquet> tulips;

    while(std::cin.peek() != -1) {
      Box box = deserialize<Box>(std::cin);
      logger.info(Str() << " in: " << box);
      for(auto& flower: box.flowers) {
        if(flower.type == ROSE) {
          roses.push_back(flower);
        } else if(flower.type == TULIP) {
          tulips.push_back(flower);
        } else {
          throw std::string("Unhandler flower type");
        }
      }
      attemptToDispatch(roses);
      attemptToDispatch(tulips);
    }
    logger.debug("finalizing");
    // TODO persist local roses & tulips
    packets.close();
    boxes.close();
  }

  Packet pack(std::vector<Bouquet> & flowers) {
    std::vector<Bouquet> result(FLOWER_PACKET_SIZE);
    std::copy_n(flowers.begin(), FLOWER_PACKET_SIZE, result.begin());
    flowers.erase(flowers.begin(), flowers.begin() + FLOWER_PACKET_SIZE);
    return Packet(result);
  }

  void attemptToDispatch(std::vector<Bouquet> & flowers) {
    if(flowers.size() >= FLOWER_PACKET_SIZE) {
      Packet packet = pack(flowers);
      logger.info(Str() << " out: " << packet);
      std::cout << packet;
    } else {
      logger.info(Str() << " no ready to send: " << flowers.size());
    }
  }
};

#endif
