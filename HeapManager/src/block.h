#pragma once

#include <cstddef>

#include <contiguousrange.h>

class Block {
public:
  Block(ContiguousRange *, void *, std::size_t);

  ContiguousRange *getContiguousRange();
  void setContiguousRange(ContiguousRange *);

  void *getMemoryStart();
  void setMemoryStart(void *);

  std::size_t getMemorySize();
  void setMemorySize(std::size_t);

  bool isInUse();
  void setInUse(bool);

  Block *getNextBlock();
  void setNextBlock(Block *);

private:
  ContiguousRange *m_contigStart;
  void *m_memStart;
  std::size_t m_memSize;
  bool m_inuse;
  Block *m_next;
};
