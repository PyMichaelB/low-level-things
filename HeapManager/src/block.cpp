#include <block.h>

Block::Block(ContiguousRange *contigStart, void *memStart, std::size_t memSize)
    : m_contigStart(contigStart), m_memStart(memStart), m_memSize(memSize),
      m_inuse(false), m_next(nullptr) {}

ContiguousRange *Block::getContiguousRange() { return m_contigStart; }

void *Block::getMemoryStart() { return m_memStart; }

std::size_t Block::getMemorySize() { return m_memSize; }

bool Block::isInUse() { return m_inuse; }

Block *Block::getNextBlock() { return m_next; }

void Block::setContiguousRange(ContiguousRange *contigStart) {
  m_contigStart = contigStart;
}

void Block::setMemoryStart(void *memStart) { m_memStart = memStart; }

void Block::setMemorySize(std::size_t memSize) { m_memSize = memSize; }

void Block::setInUse(bool inuse) { m_inuse = inuse; }

void Block::setNextBlock(Block *next) { m_next = next; }