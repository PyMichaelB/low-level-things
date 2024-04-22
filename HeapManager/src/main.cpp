#include <vector>
#include <iostream>
#include <set>

#include <heapallocator.h>
#include <chrono>

int main(int argc, char* argv[]) {
    std::vector<int, BasicAllocator<int>> myVector;

    for(int i = 0; i < 1200; ++i)
    {
        myVector.push_back(i);
    }

    std::set<int, std::less<int>, BasicAllocator<int>> mySet;
    for(int i = 0; i < 10; ++i)
    {
        mySet.insert(i);
    }
    return 0;
}