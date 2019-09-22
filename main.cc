#include <stddef.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <sys/wait.h>
#include "launcher.hpp"
#include "log.hpp"
#include "flower.hpp"
#include "queue.hpp"
#include "shm.hpp"
#include "pipe.hpp"
#include <string>
#include <sstream>

class Balancer : public Process , public Writeable {
private:
  std::vector<Pipe>& outputPipes;
  Pipe& inputPipe;
public:
  Balancer(Pipe& inputPipe,
           std::vector<Pipe>& outputPipes,
           const std::string & name):
    outputPipes(outputPipes),
    inputPipe(inputPipe),
    Process(name) {
  }

  Out out() {
    for(auto &o:outputPipes) {
      o.close();
    }
    return inputPipe.out();
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
    while(true){
      for(auto &o : outs) {
        logger.debug("eof");
        if(std::cin.eof()) {
          logger.debug("closing");
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

class Producer : public Process {
private:
  Mem<bool>& stopFlag;
  Writeable& pipe;
public:
  Producer(Writeable& pipe, Mem<bool>& stopFlag) :
    pipe(pipe),
    stopFlag(stopFlag),
    Process("Producer") {
  }

  Box randomBox() {
    Box box;
    pid_t i = getpid();
    box.flowers[0] = Bouquet(0, ROSE);
    box.flowers[1] = Bouquet(0, ROSE);
    box.flowers[2] = Bouquet(0, ROSE);
    box.flowers[3] = Bouquet(0, ROSE);
    box.flowers[4] = Bouquet(0, ROSE);
    box.flowers[5] = Bouquet(0, TULIP);
    box.flowers[6] = Bouquet(0, TULIP);
    box.flowers[7] = Bouquet(0, TULIP);
    box.flowers[8] = Bouquet(0, TULIP);
    box.flowers[9] = Bouquet(0, TULIP);
    return box;
  }

  void run() {
    Out out = pipe.out();
    out.asStdOut();
    while(!(*stopFlag)) {
      Box box = randomBox();
      logger.info(Str() << box);
      std::cout << box;
      sleep(3);
    }
    pipe.close();
  }
};

class DistributionCenter : public Process {
private:
  Writeable& packets;
  Readable& boxes;
public:
  DistributionCenter(Writeable& packets,
                     Readable& boxes):
    boxes(boxes),
    packets(packets),
    Process("DistroCenter") {
  }

  void run() {
    boxes.in().asStdIn();
    packets.out().asStdOut();
    while(std::cin) {
      Box box = deserialize<Box>(std::cin);
      logger.info(Str() << box);
      std::cout << box;
      sleep(3);
    }
    packets.close();
    boxes.close();
  }
};



class Storage {
public:
  Bouquet * decrement(Request*request) {
    return NULL;
  }
  void add(Packet* packet){
  }
  bool canFullfill(Request* request){
    return true;
  }
};

class SellPoint : public Process {
private:
  Readable& packets;
  Storage& storage;
public:
  SellPoint(Readable& packets,
            Storage& storage):
    packets(packets),
    storage(storage),
    Process("SellPoint") {
  }

  void run() {
    packets.in().asStdIn();
    while(std::cin) {
      Box box = deserialize<Box>(std::cin);
      logger.info(Str() << box);
      sleep(3);
    }
    packets.close();
  }

  void iterate() {
    /*
      Request* request=requests.pull();
      if(storage.canFullfill(request)){
       Bouquet* flower = storage.decrement(request);
       if(request->getType() == INTERNET) {
         dispatchToBycicle(request, flower);
       } else{
         giveToClient(request, flower);
       }
      } else {
       Packet * packet = distributionCenterTransport.pull();
       storage.add(packet);
      }
    */
  }

  void giveToClient(Request* req, Bouquet * flower) {
  }

  void dispatchToBycicle(Request*  req, Bouquet * flower) {

  }
};
/*
int main2(int argc, char ** argv) {
  try {
    int prodCount = 3;
    int distrosCount = 3;
    int sellPointsCount = 3;
    root.debug("start:");

    Mem<bool> stopFlag("/dev/null", 0, false);
    Storage storage;
    std::vector<Pipe> boxesPipes(distrosCount);

    Balancer boxes("BoxBalancer");

    std::vector<Producer> producers(prodCount, {boxes, stopFlag});

    Balancer packets("PacketBalancer");
    std::vector<DistributionCenter> distros(distrosCount, {packets, boxes});

    std::vector<SellPoint> sellpoints(sellPointsCount, {packets, storage});

    boxes.fork();
    packets.fork();

    forkAll(producers);
    forkAll(distros);
    forkAll(sellpoints);

    boxes.close();
    packets.close();

    sleep(5);
    *stopFlag = true;

    waitAll(producers);
    waitAll(distros);
    waitAll(sellpoints);

    boxes.wait();
    packets.wait();

  } catch(const std::string & msg) {
    std::cerr << msg << std::endl;
  }
  return 0;
}

int main3(){
  try {
    int prodCount = 3;
    int distrosCount = 3;
    int sellPointsCount = 3;
    root.debug("start");

    Mem<bool> stopFlag("/dev/null", 0, false);
    Storage storage;
    Balancer boxes("BoxBalancer");

    std::vector<pid_t> pids;
    pid_t pid = fork();
    if(pid == 0) {
      std::vector<Producer> producers(prodCount, {boxes, stopFlag});
      forkAll(producers);
      waitAll(producers);
      root.debug("Producers fin");
    } else {
      pids.push_back(pid);
      Balancer packets("PacketBalancer");
      pid = fork();
      if(pid == 0) {
        std::vector<DistributionCenter> distros(distrosCount, {packets, boxes});
        forkAll(distros);
        boxes.run();
        waitAll(distros);
        root.debug("Distros fin");
      } else {
        pids.push_back(pid);
        pid = fork();
        if(pid == 0) {
          std::vector<SellPoint> sellpoints(sellPointsCount, {packets, storage});
          forkAll(sellpoints);
          packets.run();
          waitAll(sellpoints);
          root.debug("SellPoint fin");
        } else {
          pids.push_back(pid);
          sleep(5);
          *stopFlag = true;
          root.debug("Main fin");
        }
      }
      packets.close();
      packets.wait();
    }
    boxes.close();
    boxes.wait();
    for(auto & p:pids) {
      waitpid(p, NULL, 0);
    }
    root.debug("fin");
  } catch(const std::string & msg) {
    std::cerr << msg << std::endl;
  }
  return 0;
}

*/
int main(int argc, char ** argv) {
  try{
    Pipe in;
    Pipe r1;
    Pipe r2;
    std::vector<Pipe> v{r1, r2};
    Balancer balancer(in, v , "balancer");
    balancer.fork();
    root.debug("main start.");
    pid_t pid = fork();
    if(pid == 0) {
      Logger logger("sender");
      logger.debug("sending:");

      in.out().write("hola\n", 6);

      sleep(10);
      in.out().write("mundo\n", 7);
      balancer.close();

      /*
      std::cout << "hola" << std::endl;
      std::cout << "mundo" << std::endl;
      */
    } else {
      pid_t pid2 = fork();
      in.close();
      if(pid2 == 0) {
        Logger logger("reciver");
        logger.debug("asStdIn.");
        r2.in().asStdIn();
        logger.debug("reading.");
        std::string str;
        std::getline(std::cin, str);
        logger.debug(Str() << "readed:" << str);
      } else {
        Logger logger("reciver");
        logger.debug("asStdIn.");
        r1.in().asStdIn();
        logger.debug("reading.");
        std::string str;
        std::getline(std::cin, str);
        logger.debug(Str() << "readed:" << str);
        logger.debug("wait childs.");
        balancer.close();
        waitpid(pid, NULL, 0);
        waitpid(pid2, NULL, 0);
        balancer.wait();
      }
    }
    root.debug("close.");
    // balancer.close();
    root.debug("fin.");
  } catch(...) {
    root.debug("failure");
    throw;
  }
  return 0;
}
