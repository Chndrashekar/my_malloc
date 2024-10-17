/*
Simple memory allocator using stack
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#define CAPACITY 640000
#define CHUNK_CAPACITY 1024

#define UNIMPLEMENTED {\
    do { \
        fprintf(stderr, "%s:%d: TODO: %s is not implemented yet\n", __FILE__, __LINE__, __func__);\
    } while (0); \
}

typedef struct {
    char* start;
    size_t size;
} Chunk;

typedef struct {
    size_t size;
    Chunk chunks[CHUNK_CAPACITY];
} Chunk_List;

char heap[CAPACITY] = {0};
Chunk_List alloced_list = {0};
Chunk_List tmp_list = {0};

Chunk_List free_list = {
    .size = 1,
    .chunks = {
        [0] = {.start = heap, .size = CAPACITY}
    }
};

void heap_chunk_dump(Chunk_List* list)
{
    printf("Total chunks: %zu\n", list->size);
    for (size_t i=0; i<list->size; ++i)
    {
        Chunk chunk = list->chunks[i];
        printf("Chunk start = %p, size: %zu\n",
            chunk.start, chunk.size);
    }
}

void heap_chunk_insert(Chunk_List* list, void* ptr, size_t size)
{
    assert(list->size < CHUNK_CAPACITY);

    list->chunks[list->size].start = ptr;
    list->chunks[list->size].size = size;

    for (int i=list->size; i>0 && list->chunks[i-1].start > list->chunks[i].start; --i) {
        Chunk t = list->chunks[i];
        list->chunks[i] = list->chunks[i-1];
        list->chunks[i-1] = t;
    }
    list->size++;
}

int chunk_compar(const void *a, const void *b)
{
    const Chunk *a_chunk = a;
    const Chunk *b_chunk = b;
    return a_chunk->start - b_chunk->start;
}

int heap_chunk_find(Chunk_List* list, void* ptr)
{
    // for (size_t i=0; i<list->size; ++i) {
    //     if (list->chunks[i].start == ptr) {
    //         return i;
    //     }
    // }
    // return -1;
    
    Chunk key = {
        .start = ptr
    };

    Chunk *result = bsearch(&key, list->chunks,
                     list->size, sizeof(list->chunks[0]),
                     chunk_compar);
    
    if (result != 0) {
        assert(list->chunks <= result);
        int index = result - list->chunks; // ptr arithmetic
        return index;
    } else {
        return -1;
    }
}

void heap_chunk_remove(Chunk_List* list, size_t index)
{
    assert(index < list->size);
    for (size_t i=index; i < list->size-1; ++i) {
        list->chunks[i] = list->chunks[i+1];
    }
    list->size -= 1;
}

void heap_chunk_merge(Chunk_List *dst, const Chunk_List *src) {
    dst->size = 0;
    for (size_t i = 0; i < src->size; ++i) {
        const Chunk chunk = src->chunks[i];
        if (dst->size != 0) {
            Chunk *tmp = &dst->chunks[dst->size - 1];
            if (tmp->start + tmp->size == chunk.start) {
                tmp->size += chunk.size;
            } else {
                heap_chunk_insert(dst, chunk.start, chunk.size);
            }
        } else {
            heap_chunk_insert(dst, chunk.start, chunk.size);
        }
    }
}

void* heap_alloc(size_t size)
{
    if (size == 0) return NULL;
    heap_chunk_merge(&tmp_list, &free_list);
    free_list = tmp_list;

    for (size_t i=0; i<free_list.size; ++i) {
        if (free_list.chunks[i].size >= size) {
            Chunk c = free_list.chunks[i];
            heap_chunk_remove(&free_list, i);
            heap_chunk_insert(&alloced_list, c.start, size);
            size_t remaining_size = c.size - size;
            if (remaining_size > 0)
                heap_chunk_insert(&free_list, c.start+size, remaining_size);
            return c.start;
        }
    }
    return NULL;
}

void heap_free(void* ptr)
{
    if (ptr == NULL) return;
    int index = heap_chunk_find(&alloced_list, ptr);
    assert(index >= 0);
    assert(ptr == alloced_list.chunks[index].start);
    heap_chunk_insert(&free_list,
        alloced_list.chunks[index].start,
        alloced_list.chunks[index].size);
    
    heap_chunk_remove(&alloced_list, (size_t) index);
    
}

#define N 10

int main() {
    void *ptr[N] = {0};
    
    for(int i=0; i<N; i++) {
        ptr[i] = heap_alloc(i);
    }

    for(int i=0; i<N; i++) {
        if (i%2 == 0)
        heap_free(ptr[i]);
    }

    heap_alloc(10);

    heap_chunk_dump(&alloced_list);
    heap_chunk_dump(&free_list);

    return 0;
}