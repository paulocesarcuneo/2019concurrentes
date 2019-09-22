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
  Logger logger;
  std::string requestFileName;
public:
  SellPoint(Pipe& packets,
            const std::string & requestFileName):
    packets(packets),
    requestFileName(requestFileName),
    logger("SellPoint") {
  }

  void run() {
    packets.in().asStdIn();

    std::vector<Bouquet> roses;
    std::vector<Bouquet> tulips;
    std::vector<Remit> remits;
    std::fstream requestFile;
    requestFile.open(requestFileName);
    while(std::cin.peek() != -1) {

      Packet packet = deserialize<Packet>(std::cin);
      logger.info(Str() << "in:" << packet);
      for(auto& f : packet.flowers) {
        if(f.type == ROSE) {
          roses.push_back(f);
        } else if(f.type == TULIP) {
          tulips.push_back(f);
        } else {
          throw std::string("Unhandled rose type");
        }
      }
      Request req = nextRequest(requestFile);
      std::vector<Bouquet> remitFlowers;
      logger.debug(Str () << "Process Request: " << req);
      if(req.roses > 0) {
        transferNFlowers(roses, req.roses, remitFlowers);
      }
      if(req.tulips > 0) {
        transferNFlowers(tulips, req.tulips, remitFlowers);
      }
      Remit remit(remitFlowers, req);
      if(req.type == INTERNET) {
        logger.info(Str() << "internet: " << remit);
      } else if(req.type = FRONTDESK){
        logger.info(Str() << "frontdesk: " << remit);
      } else {
        throw std::string("Unhandled rose type");
      }
      remits.push_back(remit);
      // sleep(3);
    }
    logger.debug("finalizing");
    packets.close();
    requestFile.close();
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
