//
// Created by Haswell on 20/05/2020.
//

#ifndef SCHEDULER_MEMORY_ALLOCATOR_H
#define SCHEDULER_MEMORY_ALLOCATOR_H

#include "process.h"
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
typedef struct memory_allocator {
    void* (*malloc)(void*, process_t*, int);
    void (*info)(void*, process_t*, int);
    void (*use)(void*, process_t*, int);
    void (*free)(void*, process_t*, int);
    void (*load)(void*, process_t*);
    int (*load_time_left)(void*, process_t*);
    int (*require_allocation)(void*, process_t*);
    int (*page_fault)(void*, process_t*);
    void* structure;
} memory_allocator_t;
#endif //SCHEDULER_MEMORY_ALLOCATOR_H
