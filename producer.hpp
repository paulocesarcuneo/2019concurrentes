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
public:
  Producer(Pipe& output, Mem<bool>& stopFlag):
    output(output),
    stopFlag(stopFlag),
    logger("Producer") {
  }

  Box randomBox() {
    Box box;
    pid_t i = getpid();
    box.flowers[0] = Bouquet(i, ROSE);
    box.flowers[1] = Bouquet(i, ROSE);
    box.flowers[2] = Bouquet(i, ROSE);
    box.flowers[3] = Bouquet(i, ROSE);
    box.flowers[4] = Bouquet(i, ROSE);
    box.flowers[5] = Bouquet(i, TULIP);
    box.flowers[6] = Bouquet(i, TULIP);
    box.flowers[7] = Bouquet(i, TULIP);
    box.flowers[8] = Bouquet(i, TULIP);
    box.flowers[9] = Bouquet(i, TULIP);
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
    output.close();
  }
};

#endif
