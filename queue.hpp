#ifndef _QUEUE_HPP_
#define _QUEUE_HPP_
#include <stddef.h>

template <typename T>
class Queue {
public:
  virtual T * pull() = 0;
  virtual bool push(T* item) = 0;
};

template <class T>
class FixQueue : public Queue<T> {
public:
  FixQueue(int capacity) {
    this->capacity = capacity + 1;
    this->items = new T[this->capacity];
    this->last = 0;
    this->first = 0;
  }

  ~FixQueue(){
    delete items;
  }

  T* pull() {
    if(isEmpty())
      return NULL;

    T* current = &(items[ first % capacity ]);
    ++first;
    return current;
  }

  bool push(T * i) {
    if(size() + 1 == capacity)
      return false;

    items[last % capacity] = *i;
    ++last;
    return true;
  }
private:
  T *items;
  int capacity;
  int last;
  int first;

  inline int size() {
    int result = last - first;
    if(result < 0)
      return capacity + result;
    return result;
  }

  inline bool isEmpty(){
    return first == last;
  }
};
/*
  0 1 2 3 4 5 6 7
          l   f        4 - 6 = -2 = 6
          f   l        6 - 4 =  2

  first <= last
  last - first <= capacity

*/

#endif
