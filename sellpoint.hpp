#ifndef _SELLPOINT_HPP_
#define _SELLPOINT_HPP_
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <iterator>

#include "log.hpp"
#include "flower.hpp"
#include "pipe.hpp"

class SellPoint  {
private:
  Pipe& packets;
  Pipe& inventory;
  Logger logger;
  std::string requestFileName;
  Storage storage;

public:
  SellPoint(Pipe& packets,
            Pipe& inventory,
            const std::string & requestFileName,
            const std::string & storageFile):
    packets(packets),
    inventory(inventory),
    requestFileName(requestFileName),
    storage(storageFile),
    logger("SellPoint") {
  }

  void run() {
    packets.in().asStdIn();

    storage.loadStock();
    Out remits = inventory.out();

    std::fstream requestFile;
    requestFile.open(requestFileName);
    while(std::cin.peek() != -1) {

      Packet packet = deserialize<Packet>(std::cin);
      logger.info(Str() << "in:" << packet);
      for(auto& f : packet.flowers) {
        if(f.type == ROSE) {
          storage.roses.push_back(f);
        } else if(f.type == TULIP) {
          storage.tulips.push_back(f);
        } else {
          throw std::string("Unhandled rose type");
        }
      }
      Request req = nextRequest(requestFile);

      std::vector<Bouquet> remitFlowers;
      logger.debug(Str () << "Process Request: " << req);
      if(req.roses > 0) {
        transferNFlowers(storage.roses, req.roses, remitFlowers);
      }
      if(req.tulips > 0) {
        transferNFlowers(storage.tulips, req.tulips, remitFlowers);
      }

      Remit remit(remitFlowers, req);

      if(req.type == INTERNET) {
        logger.info(Str() << "internet: " << remit);
      } else if(req.type = FRONTDESK){
        logger.info(Str() << "frontdesk: " << remit);
      } else {
        throw std::string("Unhandled rose type");
      }
      remits << remit;
    }
    logger.debug("finalizing");

    storage.storeStock();

    requestFile.close();

    packets.close();
    requestFile.close();
    inventory.close();
  }

  void transferNFlowers(std::vector<Bouquet>& in, int size, std::vector<Bouquet> & out) {
    int actualSize = (in.size() <= size)? in.size() : size;
    if(actualSize > 0) {
      std::copy_n(in.begin(), actualSize, std::back_inserter(out));
      in.erase(in.begin(), in.begin() + actualSize);
    }
  }

  Request nextRequest(std::fstream & file) {
    // return deserialize<Request>(file);
    Request req;
    req.roses = 5;
    req.tulips = 5;
    req.type = INTERNET;
    return req;
  }
};

#endif
