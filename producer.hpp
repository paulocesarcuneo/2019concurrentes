#ifndef _PRODUCER_HPP_
#define _PRODUCER_HPP_

#include <vector>
#include <string>
#include <stdlib.h>

#include "shm.hpp"
#include "log.hpp"

#include "flower.hpp"
#include "pipe.hpp"


class Producer {
private:
  bool& stopFlag;
  Pipe& output;
  Logger logger;
  int producerId;
  int flowerCounter;
public:
  Producer(int id,
           Pipe& output,
           bool& stopFlag):
    producerId(id),
    output(output),
    stopFlag(stopFlag),
    flowerCounter(0),
    logger("Producer") {
  }

  Box randomBox() {
    Box box;
    box.flowers[0] = Bouquet(++flowerCounter, producerId, static_cast<Flower>(rand()%2));
    box.flowers[1] = Bouquet(++flowerCounter, producerId, static_cast<Flower>(rand()%2));
    box.flowers[2] = Bouquet(++flowerCounter, producerId, static_cast<Flower>(rand()%2));
    box.flowers[3] = Bouquet(++flowerCounter, producerId, static_cast<Flower>(rand()%2));
    box.flowers[4] = Bouquet(++flowerCounter, producerId, static_cast<Flower>(rand()%2));
    box.flowers[5] = Bouquet(++flowerCounter, producerId, static_cast<Flower>(rand()%2));
    box.flowers[6] = Bouquet(++flowerCounter, producerId, static_cast<Flower>(rand()%2));
    box.flowers[7] = Bouquet(++flowerCounter, producerId, static_cast<Flower>(rand()%2));
    box.flowers[8] = Bouquet(++flowerCounter, producerId, static_cast<Flower>(rand()%2));
    box.flowers[9] = Bouquet(++flowerCounter, producerId, static_cast<Flower>(rand()%2));
    return box;
  }

  void run() {
    Out out = output.out();
    out.asStdOut();
    while(!stopFlag) {
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
