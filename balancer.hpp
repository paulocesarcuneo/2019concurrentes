#ifndef _BALANCER_HPP_
#define _BALANCER_HPP_

#include <iostream>
#include <vector>
#include "log.hpp"
#include "pipe.hpp"
#include <string>

class Balancer {
private:
  Pipe & inputPipe;
  std::vector<Pipe> & outputPipes;
  Logger logger;
public:
  Balancer(Pipe & in,
           std::vector<Pipe> & out,
           const std::string & name):
    inputPipe(in),
    outputPipes(out),
    logger(name) {
  }

  void close() {
    inputPipe.close();
    for(auto &o:outputPipes) {
      o.close();
    }
  }

  void run() {
    inputPipe.in().asStdIn();
    std::vector<Out> outs;
    for(auto&o: outputPipes) {
      outs.push_back(o.out());
    }
    while(true) {
      for(auto &o : outs) {
        if(std::cin.peek() == -1) {
          logger.debug("finalizing");
          close();
          return;
        }
        std::string lineIn;
        logger.debug("reading.");
        std::getline(std::cin, lineIn);
        logger.debug("read:" + lineIn);
        logger.debug("writing.");
        std::string lineOut = lineIn + "\n";
        o.write(lineOut.c_str(), lineOut.size());
        logger.debug("write:" + lineOut);
      }
    }
  }
};

#endif
