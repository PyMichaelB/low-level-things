#pragma once

#include <cstddef>

#include <heapmanager.h>

template <class T> class BasicAllocator {
public:
  typedef T value_type;

  T *allocate(std::size_t);
  void deallocate(T *, std::size_t);
};

template <class T> T *BasicAllocator<T>::allocate(std::size_t n) {
  if (n == 0) {
    return nullptr;
  }

  std::size_t maxBytesAllowedToAllocate = static_cast<std::size_t>(-1);
  std::size_t maxOfTAllowedToAllocate = maxBytesAllowedToAllocate / sizeof(T);

  if (maxOfTAllowedToAllocate < n) {
    return nullptr;
  }

  return static_cast<T *>(Heap::getHeap().assignBlock(n * sizeof(T)));
}

template <class T> void BasicAllocator<T>::deallocate(T *p, std::size_t n) {
  Heap::getHeap().unassignBlock(p);
}