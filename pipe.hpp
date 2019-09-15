#ifndef _PIPE_HPP_
#define _PIPE_HPP_

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include <string>

class In {
protected:
  int fd;
public:
  In(const int fd): fd(fd) {}
  In(const In & i): fd(i.fd) {}

  size_t read(void* buffer, const int size) {
    return ::read(fd, buffer, size);
  }

  void asStdIn() {
    if(-1 == dup2(fd, 0))
      throw std::string("asStdIn") + std::string(strerror(errno));
  }
};


class Out {
protected:
  int fd;
public:
  Out(const int fd): fd(fd) {}
  Out(const Out & o): fd(o.fd) {}

  ssize_t write(const void* buffer, const int size) {
    return ::write(fd, buffer, size);
  }

  void asStdOut() {
    if(-1 == dup2(fd, 1))
      throw std::string("asStdOut") + std::string(strerror(errno));
  }

  void asStdErr() {
    if(-1 == dup2(fd, 2))
      throw std::string("asStdErr") + std::string(strerror(errno));
  }
};

class Pipe {
private:
	int fds[2];
  void close() {
    ::close(fds[0]);
    ::close(fds[1]);
  }
public:
	Pipe() {
    ::pipe(fds);
  }

  ~Pipe() {
    close();
  }

  In readEnd() {
    return In(fds[0]);
  }

  Out writeEnd() {
    return Out(fds[1]);
  }
};

#include <sys/stat.h>

class Fifo {
private:
  const std::string path;
public:

  Fifo(const std::string & path) :
    path(path) {
  	if(-1 == ::mknod(static_cast<const char*>(path.c_str()), S_IFIFO|0666, 0))
      throw std::string("mknod") + std::string(strerror(errno));
  }

  ~Fifo() {
    ::unlink(path.c_str());
  }

  class FifoIn : public In {
  public:
    FifoIn(const int fd) : In(fd) {}
    FifoIn(const FifoIn & fi) : FifoIn(fi.fd) {}

    void close() {
      ::close(fd);
    }
  };

  class FifoOut : public Out {
  public:
    FifoOut(const int fd) : Out(fd) {}
    FifoOut(const FifoOut & fo) : FifoOut(fo.fd) {}

    void close() {
      ::close(fd);
    }
  };

  FifoIn readEnd() {
    int fd = open(this->path.c_str(), O_RDONLY);
    if(-1 == fd)
      throw std::string("readEnd") + std::string(strerror(errno));
    return FifoIn(fd);
  }

  FifoOut writeEnd() {
    int fd = open(this->path.c_str(), O_WRONLY)
    if(-1 == fd)
      throw std::string("writeEnd") + std::string(strerror(errno));
    return FifoOut(fd);
  }
};
#endif
