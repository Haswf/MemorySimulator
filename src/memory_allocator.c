//
// Created by Haswell on 20/05/2020.
//

#include "memory_allocator.h"

void free_memory_allocator(memory_allocator_t* allocator) {
    assert(allocator);
    free(allocator);
}