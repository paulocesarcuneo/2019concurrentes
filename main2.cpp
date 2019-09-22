#include <sstream>
#include <string>
#include <iostream>


class Str {
private:
  std::stringstream ss;
public:
  template<typename T>
  Str& operator<<(const T & t) {
    ss << t;
    return *this;
  }

  Str& operator<<(std::ostream& (*fun)(std::ostream&)) {
    ss << fun;
    return *this;
  }

  std::string str() const {
    return ss.str();
  }
};

std::ostream &  operator<<(std::ostream & o, const Str & s) {
  o << s.str();
  return o;
}

class Protocol {
public:
  virtual void before() = 0;
  virtual void after() = 0;
};

int main() {
  class : public Protocol {
  public:
    void before() {
      std::cout << "before" << std::endl;
    }
    void after() {
      std::cout << "after" << std::endl;
    }
  } obj1;


  std::cout << (Str() << "hola " << 0 << "mundo" << std::endl);

  obj1.before();
  obj1.after();
  return 0;
}
