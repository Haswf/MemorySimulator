//
// Created by Haswell on 20/05/2020.
//

#include "memory_allocator.h"

memory_allocator_t* create_memory_allocator(
        void* (*allocate_memory)(void*, process_t*),
        void (*use_memory)(void*, process_t*, int),
        void (*free_memory)(void*, process_t*),
        void (*load_memory)(void* memoryList, process_t* process),
        int (*load_time_left)(void* memoryList, process_t* process),
        void* structure
) {
    memory_allocator_t* allocator = malloc(sizeof(*allocator));
    assert(allocator);
    assert(allocator && use_memory && free_memory);
    allocator->allocate_memory = allocate_memory;
    allocator->use_memory = use_memory;
    allocator->free_memory = free_memory;
    allocator->load_memory = load_memory;
    allocator->load_time_left = load_time_left;
    allocator->structure = structure;
    return allocator;
}

void free_memory_allocator(memory_allocator_t* allocator) {
    assert(allocator);
    free(allocator);
}