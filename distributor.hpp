#ifndef _DISTRIBUTOR_HPP_
#define _DISTRIBUTOR_HPP_

#include <vector>
#include <string>

#include "log.hpp"
#include "flower.hpp"
#include "pipe.hpp"

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

#endif
