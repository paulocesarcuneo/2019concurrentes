#include <stddef.h>
#include <unistd.h>
#include <iostream>
#include <sys/wait.h>
using namespace std;


template <typename Type>
class Queue {
public:
  Type * pull(){
    return NULL;
  }

  void push(Type* item) {
  }

};

class Box{
  // 10 ramos
  // varios tipos de ramos
};

class Packet{
  // 100 ramos
  // un solo tipo de ramo
public:
  bool isDone() {
    return true;
  }
  void add(Box*box){
  }
};

class Bouquet {
};

enum RequestType { INTERNET, FRONTDESK};

class Request{
public:
  RequestType getType() {
    return INTERNET;
  }
};

class Producer {
private:
  Queue<Box> distributionCenterTransport;
  Box *randonBox() {
    return NULL;
  }
  bool isUp() {
    return true;
  }
public:
  Producer(Queue<Box> distributionCenterTransport) {
    this->distributionCenterTransport = distributionCenterTransport;
  }
  void run() {
    while(isUp()) {
      Box* box = randonBox();
      distributionCenterTransport.push(box);
    }
  }

  void pause() {
    // ...
  }

};

class DistributionCenter {
private:
  Queue<Box>    productorTransport;
  Queue<Packet> sellPointTransport;
  bool isUp() {
    return true;
  }
public:
  DistributionCenter(Queue<Box>    productorTransport,
                     Queue<Packet> sellPointTransport) {
    this->productorTransport = productorTransport;
    this->sellPointTransport = sellPointTransport;
  }
  void run() {
    while(isUp()) {
      Packet packet;
      while(!packet.isDone()) {
        Box * box = productorTransport.pull();
        packet.add(box);
      }
      sellPointTransport.push(&packet);
    }
  };

  void pause() {
    // ...
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

class SellPoint{
private:
  Queue<Packet>  distributionCenterTransport;
  Queue<Request> requests;
  Storage storage;
  bool isUp() {
    return true;
  }
public:
  SellPoint( Queue<Packet>  distributionCenterTransport) {}
  void run() {
    while(isUp()) {
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
    }
  }

  void giveToClient(Request* req, Bouquet * flower) {
  }

  void dispatchToBycicle(Request*  req, Bouquet * flower) {

  }

};

int main(int argc, char ** argv) {
  Queue<Box>    producerToDistribution;
  Queue<Packet> distributionToSellPoint;
  /*
  Producer producer(producerToDistribution);
  DistributionCenter distribution(producerToDistribution, distributionToSellPoint);
  SellPoint sellPoint(distributionToSellPoint);
  */
  int producersCount = 3; // leer config
  pid_t producers[producersCount];

  int distroCenter = 3;
  int sellPoint = 3 ;
  bool isMain = true;
  std::cout << "Producers" << std::endl;
  for (int i = 0; i < producersCount && isMain; ++i) {
    pid_t pid = fork();
    if(pid == 0) {
      // Producer producer(producerToDistribution);
      std::cout << "Producer start: " << i << std::endl;
      isMain = false;
      sleep(1000*i);
      exit(0);
    } else {
      producers[i]=pid;
    }
  }
  std::cout << "Distro" << std::endl;
  for (int i = 0; i < distroCenter && isMain; ++i) {
    pid_t pid = fork();
    if(pid == 0) {
      // DistributionCenter distribution(producerToDistribution, distributionToSellPoint);
      std::cout << "Distribution Center start: "<< i << std::endl;
      isMain = false;
      exit(0);
    }
  }
  std::cout << "SellPoint" << std::endl;

  for (int i = 0; i < sellPoint && isMain; ++i) {
    pid_t pid = fork();
    if(pid == 0) {
      // SellPoint 
      std::cout << "Sell Point start: "<< i << std::endl;
      isMain = false;
      exit(0);
    }
  }

  if(isMain) {
    int status;
    wait(&status);
    std::cout << "FIN" << std::endl;
  }
  return 0;
}
