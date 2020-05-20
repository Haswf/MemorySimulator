//
// Created by Haswell on 20/05/2020.
//

#ifndef SCHEDULER_MEMORY_ALLOCATOR_H
#define SCHEDULER_MEMORY_ALLOCATOR_H

#include "swapping.h"

typedef struct memory_allocator {
    void* (*allocate_memory)(void*, process_t*);
    void (*use_memory)(void*, process_t*, int);
    void (*free_memory)(void*, process_t*);
    void (*load_memory)(void* memoryList, process_t* process);
    int (*load_time_left)(void* memoryList, process_t* process);
    void* structure;
} memory_allocator_t;

memory_allocator_t* create_memory_allocator(
        void* (*allocate_memory)(void*, process_t*),
        void (*use_memory)(void*, process_t*, int),
        void (*free_memory)(void*, process_t*),
        void (*load_memory)(void* memoryList, process_t* process),
        int (*load_time_left)(void* memoryList, process_t* process),
        void* structure
);
void free_memory_allocator(memory_allocator_t* allocator);
#endif //SCHEDULER_MEMORY_ALLOCATOR_H
