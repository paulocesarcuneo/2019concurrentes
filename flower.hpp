#ifndef _FLOWER_HPP_
#define _FLOWER_HPP_

enum Flower { ROSE, TULIP };


struct Bouquet {
};

struct Box{
  Flower flowers[10];
  // 10 ramos
  // varios tipos de ramos
};

struct Packet{
  Flower flowers[100];
  // 100 ramos
  // un solo tipo de ramo
public:
  bool isDone() {
    return true;
  }
  void add(Box*box){
  }
};

#endif
