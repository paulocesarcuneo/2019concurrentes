#ifndef _PRODUCER_HPP_
#define _PRODUCER_HPP_

#include <vector>
#include <string>

#include "shm.hpp"
#include "log.hpp"

#include "flower.hpp"
#include "pipe.hpp"


class Producer {
private:
  Mem<bool>& stopFlag;
  Pipe& output;
  Logger logger;
  int id;
public:
  Producer(int id,
           Pipe& output,
           Mem<bool>& stopFlag):
    id(id),
    output(output),
    stopFlag(stopFlag),
    logger("Producer") {
  }

  Box randomBox() {
    Box box;
    box.flowers[0] = Bouquet(id, ROSE);
    box.flowers[1] = Bouquet(id, ROSE);
    box.flowers[2] = Bouquet(id, ROSE);
    box.flowers[3] = Bouquet(id, ROSE);
    box.flowers[4] = Bouquet(id, ROSE);
    box.flowers[5] = Bouquet(id, TULIP);
    box.flowers[6] = Bouquet(id, TULIP);
    box.flowers[7] = Bouquet(id, TULIP);
    box.flowers[8] = Bouquet(id, TULIP);
    box.flowers[9] = Bouquet(id, TULIP);
    return box;
  }

  void run() {
    Out out = output.out();
    out.asStdOut();
    while(!(*stopFlag)) {
      Box box = randomBox();
      logger.info(Str() << box);
      std::cout << box;
      sleep(1);
    }
    logger.debug("finalizing");
    output.close();
  }
};

#endif
