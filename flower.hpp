#ifndef _FLOWER_HPP_
#define _FLOWER_HPP_

#include <iostream>
#include <vector>

enum Flower { ROSE=0, TULIP=1 };

struct Bouquet {
  int producer;
  Flower type;
};

struct Box{
  Bouquet flowers[10];
};

struct Packet{
  Bouquet flowers[100];
public:
  bool isDone() {
    return true;
  }
  void add(Box*box){
  }
};

std::istream& comma(std::istream& in) {
  int chr = in.get();
  if(chr != ',') {
    throw "Expected ','";
  }
  return in;
};

std::istream& dot(std::istream& in) {
  int chr = in.get();
  if(chr != '.') {
    throw "Expected '.'";
  }
  return in;
};

std::istream& colon(std::istream& in) {
  int chr = in.get();
  if(chr != ';') {
    throw "Expected ';'";
  }
  return in;
};

template<class T>
T deserialize(std::istream & in);

template<>
Bouquet deserialize<Bouquet>(std::istream & in) {
  Bouquet result;
  int a;
  in >> result.producer >> comma >> a >> colon;
  result.type = static_cast<Flower>(a);
  return result;
};


template<>
Box deserialize<Box>(std::istream & in) {
  Box result;
  for(int i = 0; i< 10; ++i) {
    result.flowers[i] = deserialize<Bouquet>(in);
  }
  int chr = in.get();
  if(chr != '\n') {
    throw "Broken Box";
  }
  return result;
};

template<>
Packet deserialize<Packet>(std::istream & in) {
  Packet result;
  for(int i = 0; i< 100; ++i) {
    result.flowers[i] = deserialize<Bouquet>(in);
  }
  int chr = in.get();
  if(chr != '\n') {
    throw "Broken Packet";
  }
  return result;
};

std::ostream&  serialize(std::ostream & out, const Bouquet& b) {
  out << b.producer << "," << b.type << ";";
  return out;
};

std::ostream& serialize(std::ostream & out, const Box & box) {
  for(int i=0; i< 10; ++i) {
    serialize(out, box.flowers[i]);
  }
  out << std::endl;
  return out;
};

std::ostream& serialize(std::ostream & out, const Packet & packet) {
  for(int i=0; i< 100; ++i) {
    serialize(out, packet.flowers[i]);
  }
  out << std::endl;
  return out;
};

#endif
