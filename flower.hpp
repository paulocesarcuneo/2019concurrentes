#ifndef _FLOWER_HPP_
#define _FLOWER_HPP_

#include <iostream>
#include <vector>

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

enum Flower { ROSE=0, TULIP=1 };

// BOUQUETS
struct Bouquet {
  int producer;
  Flower type;
  Bouquet(){}

  Bouquet(int p, Flower f) : producer(p), type(f) { }

  Bouquet(const Bouquet&other) {
    this->producer = other.producer;
    this->type = other.type;
  }

  Bouquet& operator=(const Bouquet&other) {
    this->producer = other.producer;
    this->type = other.type;
    return *this;
  }

};

template<>
Bouquet deserialize<Bouquet>(std::istream & in) {
  Bouquet result;
  int a;
  in >> result.producer >> isChar<','> >> a >> isChar<';'>;
  result.type = static_cast<Flower>(a);
  return result;
};

std::ostream&  operator<<(std::ostream & out, const Bouquet& b) {
  out << b.producer << "," << b.type << ";";
  return out;
};

// BOXES
struct Box{
  Bouquet flowers[10];
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

std::ostream& operator<<(std::ostream & out, const Box & box) {
  for(int i=0; i< 10; ++i) {
    out << box.flowers[i];
  }
  out << std::endl;
  return out;
};

// PACKETS
#define FLOWER_PACKET_SIZE 20
struct Packet{
  std::vector<Bouquet> flowers;
  Packet(const std::vector<Bouquet> & flowers) : flowers(flowers) {}
};

template<>
Packet deserialize<Packet>(std::istream & in) {
  std::vector<Bouquet> flowers;
  for(int i = 0; i< FLOWER_PACKET_SIZE; ++i) {
    flowers.push_back(deserialize<Bouquet>(in));
  }
  int chr = in.get();
  if(chr != '\n') {
    throw std::string("Broken Packet");
  }
  return Packet(flowers);
};
std::ostream& operator<<(std::ostream & out, const Packet & packet) {
  for(int i=0; i< FLOWER_PACKET_SIZE; ++i) {
    out << packet.flowers[i];
  }
  out << std::endl;
  return out;
};

// REQUEST
enum RequestType { INTERNET = 0, FRONTDESK = 1};

struct Request {
  RequestType type;
  int roses;
  int tulips;
};

template<>
Request deserialize<Request>(std::istream & in) {
  Request result;
  int a;
  in >> a >> isChar<','> >> result.roses >> isChar<','> >> result.tulips >> isChar<';'>;
  result.type = static_cast<RequestType>(a);
  return result;
};

std::ostream& operator<<(std::ostream & out, const Request & request) {
  out << request.type << "," << request.roses << "," << request.tulips << ";";
  return out;
};

struct Remit {
  std::vector<Bouquet> flowers;
  Request request;
  Remit(const std::vector<Bouquet> & flowers,
        const Request & request):
    flowers(flowers),
    request(request) {
  }
};


template<>
Remit deserialize<Remit>(std::istream & in) {
  Request request = deserialize<Request>(in);
  int size;
  in >> size >> isChar<':'>;
  std::vector<Bouquet> flowers;
  for(int i = 0; i< size; ++i) {
    flowers.push_back(deserialize<Bouquet>(in));
  }
  return Remit(flowers, request);
};

std::ostream& operator<<(std::ostream & out, const Remit & remit) {
  out << remit.request
      << remit.flowers.size()
      << ":";
  for(int i=0; i< remit.flowers.size(); ++i) {
    out << remit.flowers.at(i);
  }
  return out;
};

#endif
