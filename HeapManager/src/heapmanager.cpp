#include <heapmanager.h>

#include <cmath>
#include <cstdlib>
#include <unistd.h>

Heap::Heap() : m_pageSize(getpagesize()) { mapNPages(1); }

Heap::~Heap() {
  // Mark all blocks as not in use
  Block *b_p = m_blocks;
  while (b_p != nullptr) {
    b_p->setInUse(false);
    ;
    b_p = b_p->getNextBlock();
  }

  // Give allocated memory back to the OS - internal structures also cleaned up
  // here
  coalesceHeap();
  unmapEmptyPages();
}

void Heap::coalesceBlocks(Block *first, Block *second) {
  // ADJUST SIZE
  first->setMemorySize(first->getMemorySize() + second->getMemorySize());
  first->setNextBlock(second->getNextBlock());

  // REMOVE SECOND BLOCK
  delete second;
}

bool Heap::areBlocksCoalesceAble(Block *first, Block *second) {
  bool bothNotInUse = !first->isInUse() && !second->isInUse();
  bool areAdjacentInMemory =
      (first->getMemoryStart() + first->getMemorySize() ==
       second->getMemoryStart());
  bool sameContiguousRegion =
      (first->getContiguousRange() == second->getContiguousRange());

  return bothNotInUse && areAdjacentInMemory && sameContiguousRegion;
}

void Heap::coalesceHeap() {
  Block *b_p = m_blocks;
  while (b_p != nullptr && b_p->getNextBlock() != nullptr) {
    if (areBlocksCoalesceAble(b_p, b_p->getNextBlock())) {
      coalesceBlocks(b_p, b_p->getNextBlock());
    } else {
      b_p = b_p->getNextBlock();
    }
  }
}

void Heap::unmapEmptyPages() {
  Block *b_prev = nullptr;
  Block *b_p = m_blocks;

  while (b_p != nullptr) {
    Block *b_next = b_p->getNextBlock();

    if (!b_p->isInUse() &&
        (b_p->getMemoryStart() == b_p->getContiguousRange()->getStart()) &&
        b_p->getMemorySize() % m_pageSize == 0) {
      // b_p represents some number of pages worth of memory, is not in use,
      // and aligned with the start of a contiguous range of mapped memory
      munmap(b_p->getMemoryStart(), b_p->getMemorySize());

      if (b_prev != nullptr) {
        b_prev->setNextBlock(b_next);
      } else {
        m_blocks = b_next;
      }

      ContiguousRange *b_p_contig = b_p->getContiguousRange();
      b_p_contig->setPages(b_p_contig->getPages() -
                           b_p->getMemorySize() / m_pageSize);
      b_p_contig->setStart(b_p_contig->getStart() + b_p->getMemorySize());

      if (b_p_contig->getPages() == 0) {
        delete b_p_contig;
      }

      delete b_p;
    }

    b_p = b_next;
  }
}

Block *Heap::getLargeEnoughBlock(std::size_t sizeRequired) {
  Block *b_p = m_blocks;
  while (b_p != nullptr) {
    if (b_p->getMemorySize() >= sizeRequired && !b_p->isInUse()) {
      break;
    }

    b_p = b_p->getNextBlock();
  }

  return b_p;
}
void *Heap::assignBlock(std::size_t size) {
  // STEP 1: Find a block with enough space
  Block *suitableBlock = getLargeEnoughBlock(size);

  if (suitableBlock == nullptr) {
    // Reached end of m_blocks - try to get more pages page
    mapNPages(std::ceil((double)size / m_pageSize));
    coalesceHeap();
    suitableBlock = getLargeEnoughBlock(size);
  }

  // STEP 2: Allocate this block to size
  if (suitableBlock->getMemorySize() != size) {
    Block *splitOffBlock = new Block(suitableBlock->getContiguousRange(),
                                     suitableBlock->getMemoryStart() + size,
                                     suitableBlock->getMemorySize() - size);

    // ADJUST SIZING
    suitableBlock->setMemorySize(size);

    // SLOT INTO LINKED LIST
    splitOffBlock->setNextBlock(suitableBlock->getNextBlock());
    suitableBlock->setNextBlock(splitOffBlock);
  }

  suitableBlock->setInUse(true);
  return suitableBlock->getMemoryStart();
}

void Heap::unassignBlock(void *p) {
  Block *b_prev = nullptr;
  Block *b_p = m_blocks;

  while (b_p != nullptr) {
    if (b_p->getMemoryStart() == p) {
      // FOUND BLOCK TO FREE
      b_p->setInUse(false);
      coalesceHeap();
      unmapEmptyPages();
      break;
    }

    b_prev = b_p;
    b_p = b_p->getNextBlock();
  }
}

void Heap::print() {
  std::cout << "HEAP START" << std::endl;

  int blockCount = 0;
  Block *b_p = m_blocks;

  while (b_p != nullptr) {
    std::cout << "BLOCK " << blockCount << " :"
              << " START @ " << b_p->getMemoryStart() << " SIZE "
              << b_p->getMemorySize() << " INUSE: " << b_p->isInUse()
              << " CONTIGUOUS REGION START:"
              << b_p->getContiguousRange()->getStart() << std::endl;

    b_p = b_p->getNextBlock();
    blockCount++;
  }

  std::cout << "HEAP END" << std::endl;
}

Block *Heap::getEndOfBlocks() {
  Block *b_p = m_blocks;
  while (b_p != nullptr && b_p->getNextBlock() != nullptr) {
    b_p = b_p->getNextBlock();
  }
  return b_p;
}

void Heap::mapNPages(std::size_t pageCount) {
  void *start = mmap(NULL, m_pageSize * pageCount, PROT_READ | PROT_WRITE,
                     MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

  ContiguousRange *c_new = new ContiguousRange(pageCount, start);
  Block *b_new = new Block(c_new, start, m_pageSize * pageCount);

  // Attach to the end of the linked-list
  Block *prevBlock = getEndOfBlocks();
  if (prevBlock == nullptr) {
    m_blocks = b_new;
  } else {
    prevBlock->setNextBlock(b_new);
  }
}