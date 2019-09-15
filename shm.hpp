#ifndef _SHM_HPP_
#define _SHM_HPP_

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string>
#include <string.h>
#include <iostream>
#include <errno.h>

template <class T> class Mem {
private:
  int	shmId;
  T*	data;
  int	countAttachedProcesses() const {
    shmid_ds estado;
    shmctl (this->shmId, IPC_STAT, &estado);
    return estado.shm_nattch;
  }
public:
  Mem(const std::string& file,
      const char index,
      const T& init) {
    key_t clave = ftok (file.c_str(), index);
    if(clave <= 0) {
      std::string mensaje = std::string("Error en ftok(): ") + std::string(strerror(errno));
      throw mensaje;
    }

    shmId = shmget (clave, sizeof(T), 0644 | IPC_CREAT);
    if(shmId <= 0) {
      std::string mensaje = std::string("Error en shmget(): ") + std::string(strerror(errno));
      throw mensaje;
    }

    void* tmpPtr = shmat (this->shmId, NULL, 0);
    if (tmpPtr == (void*) -1) {
      std::string mensaje = std::string("Error en shmat(): ") + std::string(strerror(errno));
      throw mensaje;
    }

    data = static_cast<T*> (tmpPtr);
    *data = init;
  }

  Mem (const Mem& origen)
    : shmId(origen.shmId) {
    void* tmpPtr = shmat(origen.shmId, NULL, 0);
    if (tmpPtr == (void*) -1 ) {
      std::string mensaje = std::string("Error en shmat(): ") + std::string(strerror(errno));
      throw mensaje;
    }
    data = static_cast<T*> (tmpPtr);
  }

  ~Mem() {
    int errorDt = shmdt(static_cast<void*> (data));
    if (errorDt == -1) {
      std::cerr << "Error en shmdt(): " << strerror(errno) << std::endl;
    }
    if (countAttachedProcesses() == 0) {
      shmctl(shmId, IPC_RMID, NULL);
    }
  }

  Mem<T>& operator=(const Mem& origen){
    shmId = origen.shmId;
    void* tmpPtr = shmat(this->shmId, NULL, 0);
    if (tmpPtr == (void*) -1) {
      std::string mensaje = std::string("Error en shmat(): ") + std::string(strerror(errno));
      throw mensaje;
    }
    data = static_cast<T*> (tmpPtr);
    return *this;
  }

  Mem<T>& operator*(const T& data) {
    *(this->data) = data;
    return *this;
  }

  T& operator*(void) {
    return *(this->data);
  }
};

void testMem() {
  Mem<int> mem("/dev/null", 0, 0);
  int val = *mem;
  std::cout <<"*Mem : " << val;
  *mem = 1;
  val = *mem;
  std::cout <<"*Mem : " << val;
};

#endif
