#include <heapmanager.h>

#include <cmath>
#include <cstdlib>

Heap::Heap() { mapNPages(1); }

void Heap::coalesceBlocks(Block *first, Block *second) {
  // ADJUST SIZE
  first->memSize += second->memSize;
  first->next = second->next;

  // REMOVE SECOND BLOCK
  free(second);
}

bool Heap::areBlocksCoalesceAble(Block *first, Block *second) {
  bool bothNotInUse = !first->inuse && !second->inuse;
  bool areAdjacentInMemory =
      (first->memStart + first->memSize == second->memStart);
  bool sameContiguousRegion = (first->contigStart == second->contigStart);

  return bothNotInUse && areAdjacentInMemory && sameContiguousRegion;
}

void Heap::coalesceHeap() {
  Block *b_p = blocks;
  while (b_p != nullptr && b_p->next != nullptr) {
    if (areBlocksCoalesceAble(b_p, b_p->next)) {
      coalesceBlocks(b_p, b_p->next);
    } else {
        b_p = b_p->next;
    }
  }
}

void Heap::unmapEmptyPages() {
  Block *b_prev = nullptr;
  Block *b_p = blocks;

  while (b_p != nullptr) {
    Block *b_next = b_p->next;

    if (!b_p->inuse && (b_p->memStart == b_p->contigStart->start) &&
        b_p->memSize % 4096 == 0) {
      // b_p represents some number of pages worth of memory, is not in use,
      // and aligned with the start of a contiguous range of mapped memory
      munmap(b_p->memStart, b_p->memSize);

      if (b_prev != nullptr) {
        b_prev->next = b_next;
      } else {
        blocks = b_next;
      }

      b_p->contigStart->pages -= b_p->memSize / 4096;
      b_p->contigStart->start = b_p->contigStart->start + b_p->memSize;

      if (b_p->contigStart->pages == 0) {
        free(b_p->contigStart);
      }

      free(b_p);
    }

    b_p = b_next;
  }
}

Block *Heap::getLargeEnoughBlock(std::size_t sizeRequired) {
  Block *b_p = blocks;
  while (b_p != nullptr) {
    if (b_p->memSize >= sizeRequired && !b_p->inuse) {
      break;
    }

    b_p = b_p->next;
  }

  return b_p;
}
void *Heap::assignBlock(std::size_t size) {
  if (blocks == nullptr) {
    return nullptr;
  }

  // STEP 1: Find a block with enough space
  Block *suitableBlock = getLargeEnoughBlock(size);

  if (suitableBlock == nullptr) {
    // Reached end of blocks - try to get more pages page
    mapNPages(std::ceil((double)size / 4096));
    coalesceHeap();
    suitableBlock = getLargeEnoughBlock(size);
  }

  // STEP 2: Allocate this block to size
  if (suitableBlock->memSize != size) {
    Block *splitOffBlock = static_cast<Block *>(malloc(sizeof(Block)));
    splitOffBlock->inuse = false;
    splitOffBlock->memStart = suitableBlock->memStart + size;
    splitOffBlock->contigStart = suitableBlock->contigStart;

    // ADJUST SIZING
    splitOffBlock->memSize = suitableBlock->memSize - size;
    suitableBlock->memSize = size;

    // SLOT INTO LINKED LIST
    splitOffBlock->next = suitableBlock->next;
    suitableBlock->next = splitOffBlock;
  }

  suitableBlock->inuse = true;
  return suitableBlock->memStart;
}

void Heap::unassignBlock(void *p) {
  Block *b_prev = nullptr;
  Block *b_p = blocks;

  while (b_p != nullptr) {
    if (b_p->memStart == p) {
      // FOUND BLOCK TO FREE
      b_p->inuse = false;
      coalesceHeap();
      unmapEmptyPages();
      break;
    }

    b_prev = b_p;
    b_p = b_p->next;
  }
}

void Heap::print() {
  std::cout << "HEAP START" << std::endl;

  int blockCount = 0;
  Block *b_p = blocks;

  while (b_p != nullptr) {
    std::cout << "BLOCK " << blockCount << " :"
              << " START @ " << b_p->memStart << " SIZE " << b_p->memSize
              << " INUSE: " << b_p->inuse
              << " CONTIGUOUS REGION START:" << b_p->contigStart->start
              << std::endl;

    b_p = b_p->next;
    blockCount++;
  }

  std::cout << "HEAP END" << std::endl;
}

Block *Heap::getEndOfBlocks() {
  Block *b_p = blocks;
  while (b_p != nullptr && b_p->next != nullptr) {
    b_p = b_p->next;
  }
  return b_p;
}

void Heap::mapNPages(std::size_t pageCount) {
  void *start = mmap(NULL, 4096 * pageCount, PROT_READ | PROT_WRITE,
                     MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

  ContiguousRange *c_new =
      static_cast<ContiguousRange *>(malloc(sizeof(ContiguousRange)));
  c_new->pages = pageCount;
  c_new->start = start;

  Block *b_new = static_cast<Block *>(malloc(sizeof(Block)));
  b_new->memStart = start;
  b_new->memSize = 4096 * pageCount;
  b_new->contigStart = c_new;
  b_new->next = nullptr;

  // Attach to the end of the linked-list
  Block *prevBlock = getEndOfBlocks();
  if (prevBlock == nullptr) {
    blocks = b_new;
  } else {
    prevBlock->next = b_new;
  }
}