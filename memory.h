//
// Created by Madhav on 04-04-2025.
//
#pragma once
#include <stdio.h>
#include <string.h>

#ifndef EVERYTHING_IN_C_MEMORY_H
#define EVERYTHING_IN_C_MEMORY_H

#endif //EVERYTHING_IN_C_MEMORY_H

// Defines
#define POOL_SIZE (1024*1024) // 1 mb

typedef struct Block {
    size_t size;
    int free;
    struct Block* next;
} Block;



// Initializations
static char memory_pool[POOL_SIZE];
static int initialized = 0;
Block* free_list;

// function declarations
void init_memory_pool();
void* s_malloc(size_t);
void* s_reallocate(void *ptr, size_t size);
void s_free(void*);
void* s_calloc(size_t, size_t);
void check_memory_leaks();
void print_memory_blocks();

inline void init_memory_pool(){
    free_list = (Block*)memory_pool;
    free_list->size = POOL_SIZE - sizeof(Block);
    free_list->free = 1;
    free_list->next = NULL;
}

inline void* s_malloc(size_t size){
    if (initialized == 0){
        init_memory_pool();
        initialized = 1;
    }

    Block* curr = free_list;

    while(curr != NULL) {
        if (curr->free && curr->size >= size){
            if (curr->size >= size + sizeof(Block) + 1) {
                Block* new_block = (Block*)((char*)(curr + 1) + size);
                new_block->size = curr->size - size - sizeof(Block);
                new_block->free = 1;
                new_block->next = curr->next;

                curr->size = size;
                curr->next = new_block;
            }
            curr->free = 0;
            return (void*) (curr+1);
        }
        curr = curr->next;
    }
    return NULL;
}

inline void* s_reallocate(void* ptr, size_t size) {
    if (!ptr) return NULL;
    if (size == 0) {
        s_free(ptr);
        return NULL;
    }

    const Block* old_block = (Block*)((char*)ptr - sizeof(Block));
    if(old_block->size >= size){
        return ptr;
    }

    void* new_ptr = s_malloc(size);
    if (!new_ptr) return NULL;

    const size_t copy_size = old_block->size < size ? old_block->size : size;
    memcpy(new_ptr, ptr, copy_size);

    s_free(ptr);

    return new_ptr;
}

inline void* s_calloc(const size_t num, const size_t size) {
    int* arr = s_malloc(size * num);
    if (!arr) return NULL;
    for (size_t i = 0; i < num; i++) {
        arr[i] = 0;
    }
    return arr;
}

inline void s_free(void* ptr){
    if (!ptr) return;

    Block* block = (Block*)((char*)ptr - sizeof(Block));
    block->free = 1;

    Block* curr = free_list;

    while (curr->next != NULL != 0) {
        if (curr->free && curr->next->free) {
            curr->size += curr->next->size + sizeof(Block);
            curr->next = curr->next->next;
        }else {
            curr = curr->next;
        }
    }
}

inline void check_memory_leaks() {
    Block* curr = free_list;
    int leak_found = 0;

    printf("\n[Memory Leak Report]\n");
    while (curr != NULL) {
        if (!curr->free) {
            leak_found = 1;
            printf("Leaked Block at %p | size: %zu bytes\n", (void*)(curr + 1), curr->size);
        }
        curr = curr->next;
    }

    if (!leak_found) {
        printf("No memory leaks detected. All good! ✅\n");
    }
}

inline void print_memory_blocks() {
    printf("\n[Memory Pool State]\n");
    Block* curr = free_list;
    int index = 0;
    while (curr != NULL) {
        printf("Block %d:\n", index++);
        printf("  Addr     : %p\n", (void*)curr);
        printf("  UserAddr : %p\n", (void*)(curr + 1));
        printf("  Size     : %zu bytes\n", curr->size);
        printf("  Status   : %s\n", curr->free ? "FREE" : "USED");
        printf("  Next     : %p\n", (void*)curr->next);
        printf("-------------------------\n");
        curr = curr->next;
    }
}
