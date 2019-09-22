#ifndef _FLOWER_HPP_
#define _FLOWER_HPP_

#include <iostream>
#include <vector>

enum Flower { ROSE=0, TULIP=1 };

struct Bouquet {
  int producer;
  Flower type;
  Bouquet(){}
  Bouquet(int p, Flower f) : producer(p), type(f) { }
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

enum RequestType { INTERNET, FRONTDESK};

class Request{
public:
  RequestType getType() {
    return INTERNET;
  }
};

template <char c>
std::istream& isChar(std::istream& in) {
  int chr = in.get();
  if(chr  == -1) {
    throw std::string("EOF no expected");
  }
  if(chr != c) {
    throw std::string("Expected ") + c;
  }
  return in;
};

template<class T>
T deserialize(std::istream & in);

template<>
Bouquet deserialize<Bouquet>(std::istream & in) {
  Bouquet result;
  int a;
  in >> result.producer >> isChar<','> >> a >> isChar<';'>;
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
    throw std::string("Broken Box");
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
    throw std::string("Broken Packet");
  }
  return result;
};

// Serialization:

std::ostream&  operator<<(std::ostream & out, const Bouquet& b) {
  out << b.producer << "," << b.type << ";";
  return out;
};

std::ostream& operator<<(std::ostream & out, const Box & box) {
  for(int i=0; i< 10; ++i) {
    out << box.flowers[i];
  }
  out << std::endl;
  return out;
};

std::ostream& operator<<(std::ostream & out, const Packet & packet) {
  for(int i=0; i< 100; ++i) {
    out<< packet.flowers[i];
  }
  out << std::endl;
  return out;
};

#endif
