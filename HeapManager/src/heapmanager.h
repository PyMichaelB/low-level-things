#pragma once

#include <cstddef>
#include <iostream>
#include <sys/mman.h>

#include <block.h>
#include <contiguousrange.h>

class Heap {
public:
  void *assignBlock(std::size_t);
  void unassignBlock(void *);
  static Heap &getHeap() {
    static Heap instance;
    return instance;
  }

  Heap(const Heap &) = delete;
  void operator=(const Heap &) = delete;
  void print();

private:
  Heap();
  ~Heap();

  Block *getLargeEnoughBlock(std::size_t);
  void coalesceHeap();
  void coalesceBlocks(Block *, Block *);
  bool areBlocksCoalesceAble(Block *, Block *);
  void unmapEmptyPages();
  Block *getEndOfBlocks();
  void mapNPages(std::size_t);

  Block *m_blocks;
  std::size_t m_pageSize;
};
