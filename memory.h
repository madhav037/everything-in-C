//
// Created by Madhav on 04-04-2025.
//
#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#ifndef EVERYTHING_IN_C_MEMORY_H
#define EVERYTHING_IN_C_MEMORY_H

#endif //EVERYTHING_IN_C_MEMORY_H

// Defines
#define POOL_SIZE (1024*1024) // 1 mb
#define MMAP_THRESHOLD 131072  // 128 KB


typedef enum {
    FROM_POOL,
    FROM_MMAP
} SourceType;

typedef struct Block {
    size_t size;
    int free;
    SourceType source;
    struct Block* next;
} Block;



// Initializations
static char memory_pool[POOL_SIZE];
static Block* pool_list = NULL;
static Block* mmap_list = NULL;
static int initialized = 0;

// function declarations
void init_memory_pool();
void split_block(Block*, size_t);
void* s_malloc(size_t);
void* s_calloc(size_t, size_t);
void* s_reallocate(void *ptr, size_t size);
void s_free(void*);
void check_memory_leaks();
void print_memory_blocks();

inline void init_memory_pool() {
    pool_list = (Block*)memory_pool;
    pool_list->size = POOL_SIZE - sizeof(Block);
    pool_list->free = 1;
    pool_list->source = FROM_POOL;
    pool_list->next = NULL;
    initialized = 1;
}

inline void split_block(Block* block, const size_t size) {
    Block* new_block = (Block*)((char*)(block + 1) + size);
    new_block->size = block->size - size - sizeof(Block);
    new_block->free = 1;
    new_block->source = FROM_POOL;
    new_block->next = block->next;

    block->size = size;
    block->free = 0;
    block->next = new_block;
}

inline void* s_malloc(size_t size) {
    if (!initialized) {
        init_memory_pool();
    }

    size = (size + 7) & ~7;

    if (size > MMAP_THRESHOLD) {
        const size_t total_size = sizeof(Block) + size;
        void* mem = mmap(NULL, total_size, PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (mem == MAP_FAILED) {
            return NULL;
        }
        Block* block = (Block*)mem;
        block->size = size;
        block->free = 0;
        block->source = FROM_MMAP;
        block->next = mmap_list;
        mmap_list = block;
        return (void*)(block + 1);
    }

    Block* curr = pool_list;
    while (curr) {
        if (curr->free && curr->size >= size) {
            if (curr->size >= size + sizeof(Block) + 8) {
                split_block(curr, size);
            }
            curr->free = 0;
            return (void*)(curr + 1);
        }
        curr = curr->next;
    }

    return NULL; // Pool exhausted and size < threshold
}

inline void* s_calloc(const size_t num, const size_t size) {
    const size_t total = num * size;
    void* ptr = s_malloc(total);
    if (ptr) {
        memset(ptr, 0, total);
    }
    return ptr;
}


inline void* s_reallocate(void* ptr, const size_t size) {
    if (!ptr) return s_malloc(size);
    const Block* block = (Block*)ptr - 1;

    if (block->size >= size) {
        return ptr;
    }

    void* new_ptr = s_malloc(size);
    if (!new_ptr) return NULL;

    memcpy(new_ptr, ptr, block->size);
    s_free(ptr);
    return new_ptr;
}

inline void s_free(void* ptr) {
    if (!ptr) return;
    Block* block = (Block*)ptr - 1;

    if (block->source == FROM_POOL) {
        block->free = 1;
    } else if (block->source == FROM_MMAP) {
        // Remove from mmap tracking list before unmapping
        Block** curr = &mmap_list;
        while (*curr) {
            if (*curr == block) {
                *curr = block->next;
                break;
            }
            curr = &(*curr)->next;
        }

        // Now unmap the memory
        size_t total_size = sizeof(Block) + block->size;
        munmap((void*)block, total_size);
    }
}


inline void print_memory_blocks() {
    printf("\n[Pool Memory Blocks]\n");
    Block* curr = pool_list;
    int idx = 0;
    while (curr) {
        printf("Block %d:\n", idx++);
        printf("  Addr     : %p\n", (void*)curr);
        printf("  UserAddr : %p\n", (void*)(curr + 1));
        printf("  Size     : %zu bytes\n", curr->size);
        printf("  Status   : %s\n", curr->free ? "FREE" : "USED");
        printf("  Next     : %p\n", (void*)curr->next);
        printf("-------------------------\n");
        curr = curr->next;
    }

    printf("\n[MMAP Blocks]\n");
    curr = mmap_list;
    idx = 0;
    while (curr) {
        printf("MMAP Block %d:\n", idx++);
        printf("  Addr     : %p\n", (void*)curr);
        printf("  UserAddr : %p\n", (void*)(curr + 1));
        printf("  Size     : %zu bytes\n", curr->size);
        printf("  Status   : USED (MMAP)\n");
        printf("  Next     : %p\n", (void*)curr->next);
        printf("-------------------------\n");
        curr = curr->next;
    }
}

inline void check_memory_leaks() {
    Block* curr = pool_list;
    while (curr) {
        if (!curr->free) {
            printf("Leaked pool block at %p | size: %zu bytes\n", (void*)(curr + 1), curr->size);
        }
        curr = curr->next;
    }

    curr = mmap_list;
    while (curr) {
        printf("Leaked mmap block at %p | size: %zu bytes\n", (void*)(curr + 1), curr->size);
        curr = curr->next;
    }
}