#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/mman.h>

typedef struct block {
    size_t size;
    bool is_free;
    struct block* next;
    struct block* start;
} block;

#define META_SIZE sizeof(block)
block *head;

void defrag();
block* create_new_block(size_t);

void init()
{
    head = create_new_block(0);
    head->is_free = false;

    printf("head: %p\n", (void*) head);
}

block* heap_allocate(block* curr, size_t bytes) {
    curr->is_free = false;
    if (curr->size > bytes + META_SIZE) {
        size_t new_size = curr->size - bytes - META_SIZE;
        
        block *new_block = curr + bytes + META_SIZE; // Tricky
        new_block->size = new_size;
        new_block->is_free = true;
        new_block->start = new_block + 1;
        new_block->next = curr->next;

        printf("Old block size: %zu, New block size: %zu\n", curr->size, new_block->size);
        curr->next = new_block;
        curr->size -= new_size + META_SIZE;
    }
    return curr;
}

block* create_new_block(size_t bytes)
{
    // // Allocate using mmap()
    // void* ret = mmap(NULL, META_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    // if (ret == MAP_FAILED) {
    //     perror("mmap failed");
    //     exit(1);
    // }

    // Allocation using sbrk();
    block *ret = sbrk(bytes + META_SIZE);
    if (ret == (void*)-1) {
        perror("sbrk failed");
        exit(1);
    }
    
    block* new_block = (block*) ret;
    new_block->size = bytes;
    new_block->is_free = true;
    new_block->next = NULL;
    new_block->start = new_block + 1;

    if (head == NULL) {
        return new_block;
    }

    block* curr = head;
    while (curr->next != NULL) {
        curr = curr->next;
    }

    // Attach to end of list
    curr->next = new_block;

    // printf("Create new block: %zu\n", new_block->size);
    return new_block;
}

void *my_malloc(size_t bytes)
{
    // printf("Requested bytes: %zu\n", bytes);
    defrag();
    block* curr = head;
    while (curr) {
        if (curr->is_free && curr->size >= bytes + META_SIZE) {
            // printf("Free block available: %p\n", curr);
            block* new_block = heap_allocate(curr, bytes);
            if (new_block == NULL) return NULL;
            return new_block->start;
        }
        curr = curr->next;
    }

    if (curr == NULL) {
        block* new_block = create_new_block(bytes);
        new_block = heap_allocate(new_block, bytes);
        if (new_block == NULL) return NULL;
        // printf("New block allocated: %zu\n", new_block->size);
        return new_block->start;
    }

    return NULL;
}

void defrag()
{
    block* prev = head;
    block* curr = head->next;
    while (curr) {
        if (prev->is_free && curr->is_free) {
            prev->size += curr->size + META_SIZE; // No need of space for extra metadata
            prev->next = curr->next;
        } else {
            prev = curr;
        }
        curr = curr->next;
    }
}

void free(void *ptr)
{
    block* curr = head;
    while (curr) {
        if (curr->start == ptr) {
            curr->is_free = true;
            // printf("freed the block: %zu\n", curr->size);
            return;
        }
        curr = curr->next;
    }
}

void dump_list(bool is_free) {
    if (is_free) printf("Dump List: Free\n");
    else printf("Dump List: Allocated\n");
    block* curr = head->next;
    while(curr) {
        if (curr->is_free == is_free) {
            printf("block: %p size: %zu\n", (void*) curr, curr->size);
        }
        curr = curr->next;
    }
}

int get_total_size() {
    int size = 0;
    block *curr = head;
    while(curr) {
        size += curr->size + META_SIZE;
        curr = curr -> next;
    }
    return size;
}

#define N 10
int main(void)
{
    init();
    void *ptr[N];
    for (int i=0; i<N; i++) {
        ptr[i] = my_malloc(i);
        free(ptr[i]);
    }
    
    printf("Total heap size: %d\n", get_total_size());
    // for (int i=0; i<N; i++) {
    //     if (i%2 == 0)
    //         free(ptr[i]);
    // }
    
    dump_list(false);
    dump_list(true);

    return 0;
}