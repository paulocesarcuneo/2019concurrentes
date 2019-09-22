#ifndef _PIPE_HPP_
#define _PIPE_HPP_

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <string>
#include <vector>
#include "log.hpp"

class Closeable {
public:
  virtual void close() = 0;
};

class In :public Closeable {
protected:
  int fd;
public:
  In(const int fd): fd(fd) {}
  In(const In & i): fd(i.fd) {}

  size_t read(void* buffer, const int size) {
    return ::read(fd, buffer, size);
  }

  void asStdIn() {
    if(-1 == ::dup2(fd, 0))
      throw std::string(" asStdIn: ") + std::string(strerror(errno)) + "\n";
  }

  void close() {
    ::close(fd);
  }
};

class Out : public Closeable {
protected:
  int fd;
public:
  Out(const int fd): fd(fd) {}
  Out(const Out & o): fd(o.fd) {}

  ssize_t write(const void* buffer, const int size) {
    return ::write(fd, buffer, size);
  }

  template<typename T>
  Out& operator<<(const T & t) {
    std::stringstream ss;
    ss << t;
    std::string s = ss.str();
    write(s.c_str(), s.size());
    return *this;
  }

  void asStdOut() {
    if(-1 == ::dup2(fd, 1))
      throw std::string("asStdOut: ") + std::string(strerror(errno)) + "\n";
  }

  void asStdErr() {
    if(-1 == ::dup2(fd, 2))
      throw std::string("asStdErr: ") + std::string(strerror(errno)) + "\n";
  }

  void close() {
    ::close(fd);
  }
};

class Writeable : public Closeable {
public:
  virtual Out out() = 0;
};

class Readable : public Closeable {
public:
  virtual In in() = 0;
};

class Pipe : public Writeable, public Readable {
private:
	int fds[2];

public:

  Pipe(const Pipe&other) {
    this->fds[0] = other.fds[0];
    this->fds[1] = other.fds[1];
  }

  Pipe& operator=(const Pipe& other) {
    this->fds[0] = other.fds[0];
    this->fds[1] = other.fds[1];
    return *this;
  }

	Pipe() {
    ::pipe(fds);
  }

  ~Pipe() {
  }

  In in() {
    ::close(fds[1]);
    return In(fds[0]);
  }

  Out out() {
    ::close(fds[0]);
    return Out(fds[1]);
  }

  void close() {
    ::close(fds[0]);
    ::close(fds[1]);
  }
};

template<class T>
void closeAll(std::vector<T>& v) {
  for(auto& c: v) {
    c.close();
  }
};

#endif
