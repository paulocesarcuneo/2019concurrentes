#ifndef _STORAGE_HPP_
#define _STORAGE_HPP_

#include <string>
#include <fstream>
#include <vector>
#include "flower.hpp"
#include "log.hpp"

class Storage {
public:
  std::vector<Bouquet> roses;
  std::vector<Bouquet> tulips;
  const std::string & storageFile;

  Storage(const std::string & storageFile) :
    storageFile(storageFile) {
    root.debug(Str() << "StorageFile:" << storageFile << std::endl);
  }

  void loadStock() {
    std::fstream storage;
    storage.open(storageFile);
    while(!storage.eof()) {
      Bouquet bouquet = deserialize<Bouquet>(storage);
      if(bouquet.type == ROSE) {
        roses.push_back(bouquet);
      } else if(bouquet.type == TULIP) {
        tulips.push_back(bouquet);
      } else {
        throw std::string("Unhandler flower type");
      }
    }
    storage.close();
  }

  void storeStock() {
    std::fstream storage;
    storage.open(storageFile);
    for(auto& r: roses) {
      storage << r;
    }
    for(auto& t: tulips) {
      storage << t;
    }
    storage.close();
  }
};

#endif
