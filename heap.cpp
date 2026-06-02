#define MAX_SIZE 1024*1024
#include <array>
#include <cstddef>
#include <iostream>

constexpr static std::size_t MAX_ALIGNMENT = alignof(std::max_align_t);

namespace ethan {
    
    struct Block { // header
        bool isUsed;
        size_t size;
        Block* nextBlock;
    };

    std::array<std::byte, MAX_SIZE> heap; 
    // global, so has static storage already
    Block* head {nullptr};
    
    static bool firstBlockInitialised {false};
    
    void initFirstBlock() {
        Block* start = reinterpret_cast<Block*> (heap.data());
        *start = Block {false, heap.size() - sizeof(Block), nullptr};
        head = start;
        firstBlockInitialised = true;
    }   

    Block* trySplit(Block* block, std::size_t size) {
        // Can split
        if (block->size - size - sizeof(Block) >= 2 ) {
            Block* newBlockPtr = reinterpret_cast<Block*> (reinterpret_cast<std::byte*>(block+1) + size);
            *newBlockPtr = Block{false, block->size - size - sizeof(Block), block->nextBlock};
            block->nextBlock = newBlockPtr;
            block->size = size;
        }

        block->isUsed = true;
        return block;
    }

    void* alloc(std::size_t size) {
        if (!firstBlockInitialised) initFirstBlock();

        //size = alignup(size, alignof(std::max_align_t));

        Block* curr = head;
        while (curr->size < size || curr->isUsed) { // not enough
            if (curr->nextBlock == nullptr) return nullptr; // no more free blocks to give
            curr = curr->nextBlock;
        }

        Block* block = trySplit(curr, size);

        return static_cast<void *>(block+1); // actually don't even have to cast to void *, implicit.
    }

    void free(void* ptr) {
        Block* blockPtr = reinterpret_cast<Block *> (ptr) -1;

        // next block is free
        if (blockPtr->nextBlock && blockPtr->nextBlock->isUsed == false) {
            blockPtr->size += (blockPtr->nextBlock->size) + sizeof(Block);
            blockPtr->nextBlock = blockPtr->nextBlock->nextBlock;
        }

        blockPtr->isUsed = false;

    }


}

int main() {
    int *p = static_cast<int *> (ethan::alloc(sizeof(int)));
    *p = 1;

    int* p2 = static_cast<int*> (ethan::alloc(sizeof(int)));
    *p2 = 2;

    std::cout << p<< "\n";
    std::cout << p2 << "\n";
    std::cout <<*p <<" " << *p2 <<"\n";

    ethan::free(p2);
    
    int* p3 = static_cast<int*> (ethan::alloc(sizeof(int)));
    *p3 = 3;
    std::cout << *p3 << "\n";
    std::cout << p3<< "\n";

    ethan::free(p);

    int* p4 = static_cast<int*> (ethan::alloc(sizeof(int)));
    *p4 = 4;
    std::cout<< *p4 << "\n";
    std:: cout << p4 << "\n";


}
