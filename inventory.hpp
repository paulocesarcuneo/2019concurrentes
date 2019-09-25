#ifndef _INVENTORY_HPP_
#define _INVENTORY_HPP_

#include <signal.h>
#include <stddef.h>
#include <unistd.h>

#include <string>
#include <unordered_map>

#include "log.hpp"
#include "flower.hpp"

class Inventory {
private:
  Pipe& in;
  Logger logger;
  const std::string fileName;
  static std::vector<Remit> remits;

  static void printReport(int signo) {
    std::unordered_map<int,int> producerTotal;
    std::unordered_map<Flower,int> flowerTotal;
    for(auto& r:remits) {
      for(auto& b:r.flowers) {
        auto produceCount = producerTotal.find(b.producer);
        if (produceCount != producerTotal.end()){
          ++(produceCount->second);
        } else {
          producerTotal[b.producer] = 1;
        }

        auto flowerCount = flowerTotal.find(b.type);
        if (flowerCount != flowerTotal.end()){
          ++(flowerCount->second);
        } else {
          flowerTotal[b.type] = 1;
        }
      }
    }

    Str reportProducer;
    reportProducer << "Producers:";
    int max = producerTotal.begin()->second;
    int maxProducer = producerTotal.begin()->first;
    for (auto & pair:producerTotal) {
      reportProducer << " " << pair.first << " <- " << pair.second << ";";
      if(pair.second > max) {
        maxProducer = pair.first;
        max = pair.second;
      }
    }
    root.info(reportProducer << "Max: " << maxProducer << " total " << max << ".");

    Str reportFlower;
    reportFlower << "Flowers:";
    max = flowerTotal.begin()->second;
    Flower maxFlower = flowerTotal.begin()->first;
    for (auto & pair:flowerTotal) {
      reportFlower << " " << pair.first << " <- " << pair.second << ";";
      if(pair.second > max) {
        maxFlower = pair.first;
        max = pair.second;
      }
    }

    root.info(reportFlower << "Max: " << maxFlower << " total " << max << ".");
  }

public:
  Inventory(Pipe&in, const std::string& fileName) :
    in(in),
    fileName(fileName),
    logger("Inventory") {
    signal(SIGQUIT, Inventory::printReport);
  }

  void run() {
    in.in().asStdIn();
    while(std::cin.peek() != -1) {
      Remit remit = deserialize<Remit>(std::cin);
      logger.debug("ACK Remit");
      Inventory::remits.push_back(remit);
    }
    logger.debug("finalizing");
  }
};

std::vector<Remit> Inventory::remits;

#endif
