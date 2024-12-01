#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK_SIZE 32  // Size of each block in bytes
#define NUM_BLOCKS 16  // Number of blocks in the memory pool

// The memory pool is a block of memory
static char memory_pool[NUM_BLOCKS * BLOCK_SIZE];

// The bitmap will track the allocation status of each block
static unsigned char bitmap[NUM_BLOCKS / 8];  // 1 bit per block, NUM_BLOCKS / 8 bytes for the bitmap

// Initialize the memory allocator
void memory_init() {
    memset(memory_pool, 0, sizeof(memory_pool));
    memset(bitmap, 0, sizeof(bitmap));  // All blocks are free initially
}

// // Allocate a block of memory of size 'size'
// void* my_malloc(size_t size) {
//     if (size > BLOCK_SIZE) {
//         printf("Error: Requested size exceeds block size.\n");
//         return NULL;
//     }

//     // Find the first free block
//     for (int i = 0; i < NUM_BLOCKS; i++) {
//         int byte_index = i / 8;
//         int bit_index = i % 8;

//         if ((bitmap[byte_index] & (1 << bit_index)) == 0) {
//             // Block is free, mark it as allocated
//             bitmap[byte_index] |= (1 << bit_index);
//             return (void*)(memory_pool + i * BLOCK_SIZE);
//         }
//     }

//     printf("Error: No free blocks available.\n");
//     return NULL;
// }

void* my_malloc(size_t size) {
    if (size > BLOCK_SIZE) {
        printf("Error: Requested size exceeds block size.\n");
        return NULL;
    }

    // Search for the first free byte in the bitmap
    for (int byte_index = 0; byte_index < NUM_BLOCKS / 8; byte_index++) {
        if (bitmap[byte_index] != 0xFF) { // Not all bits in this byte are allocated
            // Locate the first free bit in this byte
            for (int bit_index = 0; bit_index < 8; bit_index++) {
                if ((bitmap[byte_index] & (1 << bit_index)) == 0) {
                    // Mark the block as allocated
                    bitmap[byte_index] |= (1 << bit_index);
                    int block_index = byte_index * 8 + bit_index;
                    return (void*)(memory_pool + block_index * BLOCK_SIZE);
                }
            }
        }
    }

    printf("Error: No free blocks available.\n");
    return NULL;
}

// Free a previously allocated block
void my_free(void* ptr) {
    if (ptr == NULL) {
        return;
    }

    // Calculate the block index
    int block_index = ((char*)ptr - memory_pool) / BLOCK_SIZE;
    int byte_index = block_index / 8;
    int bit_index = block_index % 8;

    // Mark the block as free
    bitmap[byte_index] &= ~(1 << bit_index);
}

// Print the allocation status of the memory pool
void dump_memory() {
    printf("Memory Pool Allocation Status:\n");
    for (int i = 0; i < NUM_BLOCKS; i++) {
        int byte_index = i / 8;
        int bit_index = i % 8;
        if ((bitmap[byte_index] & (1 << bit_index)) == 0) {
            printf("Block %d: Free\n", i);
        } else {
            printf("Block %d: Allocated\n", i);
        }
    }
}

int main() {
    memory_init();

    // Allocate some blocks
    void* ptr1 = my_malloc(16);  // Allocate a block
    void* ptr2 = my_malloc(16);  // Allocate another block
    void* ptr3 = my_malloc(16);  // Allocate another block

    dump_memory();  // Show current memory allocation status

    // Free a block
    my_free(ptr2);

    dump_memory();  // Show memory status after freeing a block

    return 0;
}
