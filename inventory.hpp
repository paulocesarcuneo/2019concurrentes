#ifndef _INVENTORY_HPP_
#define _INVENTORY_HPP_

#include <signal.h>
#include <stddef.h>
#include <unistd.h>

#include <string>

#include "log.hpp"
#include "flower.hpp"

class Inventory {
private:
  Pipe& in;
  Logger logger;
  const std::string fileName;
  static std::vector<Remit> remits;
  static void printReport(int signo) {
    root.debug("TODO: report");
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
