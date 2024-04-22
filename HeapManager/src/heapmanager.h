#pragma once

#include <cstddef>
#include <sys/mman.h>
#include <iostream>

struct ContiguousRange
{
    void* start; // MUST BE PAGE-ALIGNED
    std::size_t pages;
};

struct Block
{
    ContiguousRange* contigStart;

    void* memStart;
    std::size_t memSize;
    bool inuse;

    Block* next;
};

class Heap
{
public:
    void* assignBlock(std::size_t);
    void unassignBlock(void*);
    static Heap& getHeap()
    {
        static Heap instance;
        return instance;
    }

    Heap(const Heap&) = delete;
    void operator=(const Heap&) = delete;
    void print();

private:
    Heap();

    Block* getLargeEnoughBlock(std::size_t size);

    void coalesceHeap();
    void coalesceBlocks(Block* first, Block* second);
    bool areBlocksCoalesceAble(Block* first, Block* second);

    void unmapEmptyPages();

    Block* getEndOfBlocks();

    void mapNPages(std::size_t pageCount);

    Block* blocks;
};
