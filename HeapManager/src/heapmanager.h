#pragma once

#include <cstddef>
#include <iostream>
#include <sys/mman.h>
struct ContiguousRange {
  void *start; // MUST BE PAGE-ALIGNED
  std::size_t pages;
};

struct Block {
  ContiguousRange *contigStart;

  void *memStart;
  std::size_t memSize;
  bool inuse;

  Block *next;
};

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
